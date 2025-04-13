#include "config.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int parse_cidr(const char *cidr_str, struct in_addr *addr, u_int8_t *mask) {
    char cidr_copy[32];
    char *ip_str, *mask_str;

    if (strlen(cidr_str) >= sizeof cidr_copy) {
        return -1; /* Input too long */
    }

    strncpy(cidr_copy, cidr_str, sizeof(cidr_copy) - 1);

    ip_str = strtok(cidr_copy, "/");
    if (!ip_str) {
        return -1; /* Invalid cidr_str */
    }

    mask_str = strtok(NULL, "/");
    if (!mask_str) {
        return -1; /* Invalid cidr_str */
    }

    /* Convert IP address string to binary */
    if (inet_pton(AF_INET, ip_str, addr) != 1) {
        return -1; /* Invalid IP */
    }

    char *end_ptr;
    long mask_val = strtol(mask_str, &end_ptr, 10);
    if (*end_ptr != '\0' || mask_val < 0 || mask_val > 32) {
        return -1; /* Invalid mask */
    }
    *mask = (u_int8_t)mask_val;

    return 0;
}

int parse_config_file(const char *filename, config_t *config) { return -1; }

int parse_config_line(char *line, config_t *config) {
    return -1; /* Not implemented yet */
}

int parse_fw_rule(const char *rule_str, fw_rule_t *rule) {
    if (rule_str == NULL || rule == NULL) {
        return -1;
    }

    rule->action = FW_ALLOW;

    char rule_str_cpy[64];
    if (strlen(rule_str) >= sizeof rule_str_cpy) {
        return -1; /* Input too long */
    }
    strncpy(rule_str_cpy, rule_str, sizeof(rule_str_cpy) - 1);

    /* Parse string */
    char *src_name = strtok(rule_str_cpy, " ");
    if (!src_name) {
        return -1; /* Invalid rule_str */
    }
    /* Drop -> */
    strtok(NULL, " ");
    char *dst_name = strtok(NULL, " ");
    if (!dst_name) {
        return -1; /* Invalid rule_str */
    }

    /* Assign to rule struct */
    rule->src_type =
        strcmp(src_name, "INTERNET") == 0 ? ENDPOINT_INTERNET : ENDPOINT_NS;
    strncpy(rule->src_name, src_name, sizeof(rule->src_name) - 1);
    rule->dst_type =
        strcmp(dst_name, "INTERNET") == 0 ? ENDPOINT_INTERNET : ENDPOINT_NS;
    strncpy(rule->dst_name, dst_name, sizeof(rule->dst_name) - 1);

    return 0;
}

void init_config(config_t *config) {
    if (config == NULL) {
        return;
    }
    config->ipv4_forwrd = false;
    memset(config->nat_outgoing_interface, 0,
           sizeof config->nat_outgoing_interface);

    config->namespace_count = 0;
    config->namespaces = NULL;

    config->bridge_count = 0;
    config->bridges = NULL;

    config->fw_default_action = FW_DROP; /* Default to DROP for security */
    config->fw_rule_count = 0;
    config->fw_rules = NULL;

    config->nat_rule_count = 0;
    config->nat_rules = NULL;
}

void free_config(config_t *config) {
    if (config == NULL) {
        return;
    }

    free(config->namespaces);
    free(config->bridges);
    free(config->fw_rules);
    free(config->nat_rules);

    // Reset pointers and counts to prevent use after free
    config->namespace_count = 0;
    config->namespaces = NULL;

    config->bridge_count = 0;
    config->bridges = NULL;

    config->fw_rule_count = 0;
    config->fw_rules = NULL;

    config->nat_rule_count = 0;
    config->nat_rules = NULL;
}

void print_config(const config_t *config, FILE *fp) {
    if (config == NULL || fp == NULL) {
        return;
    }

    char ip_str[INET_ADDRSTRLEN];

    fprintf(fp, "=== Configuration ===\n");
    fprintf(fp, "IPv4 Forwarding: %s\n",
            config->ipv4_forwrd ? "Enabled" : "Disabled");
    fprintf(fp, "NAT Outgoing Interface: %s\n", config->nat_outgoing_interface);
    fprintf(fp, "Default Firewall Action: %s\n",
            config->fw_default_action == FW_ALLOW ? "ALLOW" : "DROP");

    // Print namespaces
    fprintf(fp, "\n--- Namespaces (%d) ---\n", config->namespace_count);
    for (int i = 0; i < config->namespace_count; i++) {
        namespace_t *ns = &config->namespaces[i];

        inet_ntop(AF_INET, &ns->ip_addr, ip_str, INET_ADDRSTRLEN);
        fprintf(fp, "Namespace %d:\n", i + 1);
        fprintf(fp, "  IP Address: %s/%u\n", ip_str, ns->mask);

        inet_ntop(AF_INET, &ns->gateway, ip_str, INET_ADDRSTRLEN);
        fprintf(fp, "  Gateway: %s\n", ip_str);

        fprintf(fp, "  Connection: %s (%s)\n",
                ns->connect_type == CONNECT_BRIDGE ? "Bridge" : "Veth",
                ns->connect_name);
        fprintf(fp, "\n");
    }

    // Print bridges
    fprintf(fp, "\n--- Bridges (%d) ---\n", config->bridge_count);
    for (int i = 0; i < config->bridge_count; i++) {
        bridge_t *br = &config->bridges[i];

        inet_ntop(AF_INET, &br->ip_addr, ip_str, INET_ADDRSTRLEN);
        fprintf(fp, "Bridge %d:\n", i + 1);
        fprintf(fp, "  IP Address: %s/%u\n", ip_str, br->mask);
        fprintf(fp, "\n");
    }

    // Print firewall rules
    fprintf(fp, "\n--- Firewall Rules (%d) ---\n", config->fw_rule_count);
    for (int i = 0; i < config->fw_rule_count; i++) {
        fw_rule_t *rule = &config->fw_rules[i];

        fprintf(fp, "Rule %d: ", i + 1);

        // Source
        if (rule->src_type == ENDPOINT_NS) {
            fprintf(fp, "%s ", rule->src_name);
        } else {
            fprintf(fp, "INTERNET ");
        }

        fprintf(fp, "-> ");

        // Destination
        if (rule->dst_type == ENDPOINT_NS) {
            fprintf(fp, "%s", rule->dst_name);
        } else {
            fprintf(fp, "INTERNET");
        }

        // Action (if specified)
        fprintf(fp, " (%s)\n", rule->action == FW_ALLOW ? "ALLOW" : "DROP");
    }

    // Print NAT rules
    fprintf(fp, "\n--- NAT Rules (%d) ---\n", config->nat_rule_count);
    for (int i = 0; i < config->nat_rule_count; i++) {
        nat_rule_t *rule = &config->nat_rules[i];

        inet_ntop(AF_INET, &rule->network, ip_str, INET_ADDRSTRLEN);
        fprintf(fp, "NAT Rule %d: %s/%u\n", i + 1, ip_str, rule->mask);
    }

    fprintf(fp, "====================\n");
}
