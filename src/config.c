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

int parse_config_file(const char *filename, config_t *config) {
    return -1;
}

void init_config(config_t *config) {
    if (config == NULL) {
        return;
    }
    config->ipv4_forwrd = false;
    memset(config->nat_outgoing_interface, 0, sizeof config->nat_outgoing_interface);

    config->namespace_count = 0;
    config->namespaces = NULL;

    config->bridge_count = 0;
    config->bridges = NULL;

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

