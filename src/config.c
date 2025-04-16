#include <stddef.h>
#define _GNU_SOURCE
#include "config.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_KEY_PARTS 3

typedef enum {
    CONFIG_KEY_UNKNOWN,                  // Error or unhandled key
    CONFIG_KEY_ENABLE_IPV4_FORWARDING,   // Simple: bool
    CONFIG_KEY_NAT_OUTGOING_INTERFACE,   // Simple: string
    CONFIG_KEY_NAMESPACE,                // Complex: Definition or Property
    CONFIG_KEY_BRIDGE,                   // Complex: Definition or Property
    CONFIG_KEY_FIREWALL_FORWARD_DEFAULT, // Simple: enum fw_action_t
    CONFIG_KEY_FIREWALL_ALLOW_FORWARD,   // Add item: fw_rule_t
    CONFIG_KEY_ENABLE_NAT                // Add item: nat_rule_t
} config_key_t;

config_key_t map_config_key(char *key, char *key_parts[], int *num_parts) {
    *num_parts = 0;
    char *token;
    char *rest = key;

    // Simple on dot
    while ((token = strtok_r(rest, ".", &rest)) != NULL &&
           *num_parts < MAX_KEY_PARTS) {
        key_parts[*num_parts] = token;
        (*num_parts)++;
    }

    if (*num_parts == 0)
        return CONFIG_KEY_UNKNOWN; // empty key

    const char *base_key = key_parts[0];

    if (strcmp(base_key, "enable_ipv4_forwarding") == 0)
        return CONFIG_KEY_ENABLE_IPV4_FORWARDING;
    if (strcmp(base_key, "nat_outgoing_interface") == 0)
        return CONFIG_KEY_NAT_OUTGOING_INTERFACE;
    if (strcmp(base_key, "namespace") == 0)
        return CONFIG_KEY_NAMESPACE;
    if (strcmp(base_key, "bridge") == 0)
        return CONFIG_KEY_BRIDGE;
    if (strcmp(base_key, "firewall_forward_default") == 0)
        return CONFIG_KEY_FIREWALL_FORWARD_DEFAULT;
    if (strcmp(base_key, "firewall_allow_forward") == 0)
        return CONFIG_KEY_FIREWALL_ALLOW_FORWARD;
    if (strcmp(base_key, "enable_nat") == 0)
        return CONFIG_KEY_ENABLE_NAT;

    return CONFIG_KEY_UNKNOWN;
}

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
    FILE *fd = fopen(filename, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fd == NULL) {
        perror("Failed to open config file");
        return -1;
    }

    int status = 0;
    while ((read = getline(&line, &len, fd)) != -1) {
        status = parse_config_line(line, config);
        if (status != 0) {
            break; // parse failed
        }
    }

    fclose(fd);
    if (line) {
        free(line);
    }

    return status;
}

char *ltrim(char *s) {
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s) {
    char *back = s + strlen(s);
    while (back > s && isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s) { return rtrim(ltrim(s)); }

bridge_t *find_bridge_by_name(config_t *config, const char *br_name) {
    bridge_t *br = NULL;

    for (int i = 0; i < config->bridge_count; i++) {
        if (strcmp(config->bridges[i].name, br_name) == 0) {
            br = &config->bridges[i];
        }
    }

    return br;
}

namespace_t *find_namespace_by_name(config_t *config, const char *ns_name) {
    namespace_t *ns = NULL;

    for (int i = 0; i < config->namespace_count; i++) {
        if (strcmp(config->namespaces[i].name, ns_name) == 0) {
            ns = &config->namespaces[i];
        }
    }

    return ns;
}

int parse_config_line(char *line, config_t *config) {
    if (line == NULL || config == NULL) {
        return -1;
    }

    char *trimmed = trim(line);

    // ignore comments and empty lines
    if (*trimmed == '#' || *trimmed == '\0') {
        return 0;
    }

    // get key and value trimmed
    char *key = strtok(trimmed, "=");
    if (key == NULL) {
        return -1; /* Invalid line */
    }
    key = trim(key);
    char *value = strtok(NULL, "=");
    if (value == NULL) {
        return -1; /* Invalid line */
    }
    value = trim(value);

    // remove end of line comments
    char *comment = strchr(value, '#');
    if (comment) {
        *comment = '\0';
    }
    value = trim(value);

    char *key_parts[MAX_KEY_PARTS];
    int num_parts = 0;
    config_key_t key_t = map_config_key(key, key_parts, &num_parts);

    switch (key_t) {
    case CONFIG_KEY_ENABLE_IPV4_FORWARDING:
        if (num_parts != 1) {
            return -1; // only top level
        }
        config->ipv4_forwrd = strcmp(value, "true") == 0;
        break;
    case CONFIG_KEY_NAT_OUTGOING_INTERFACE:
        if (num_parts != 1) {
            return -1; // only top level
        }
        strncpy(config->nat_outgoing_interface, value,
                sizeof(config->nat_outgoing_interface) - 1);
        break;
    case CONFIG_KEY_NAMESPACE:
        if (num_parts == 1) {
            config->namespace_count++;
            config->namespaces =
                realloc(config->namespaces,
                        config->namespace_count * sizeof(namespace_t));
            if (config->namespaces == NULL) {
                return -1; // Memory allocation failed
            }
            namespace_t *ns = &config->namespaces[config->namespace_count - 1];
            strncpy(ns->name, value, sizeof(ns->name) - 1);
        } else if (num_parts == 3) {
            const char *ns_name = key_parts[1];
            const char *ns_prop = key_parts[2];
            namespace_t *ns = find_namespace_by_name(config, ns_name);
            if (ns == NULL) {
                return -1; // Namespace not defined
            }

            if (strcmp(ns_prop, "ip") == 0) {
                if (parse_cidr(value, &ns->ip_addr, &ns->mask) != 0) {
                    printf("HERE1");
                    return -1; // Invalid IP address
                }
            } else if (strcmp(ns_prop, "gateway") == 0) {
                if (inet_pton(AF_INET, value, &ns->gateway) != 1) {
                    printf("HERE2");
                    return -1; // Invalid IP
                }
            } else if (strcmp(ns_prop, "connect_via") == 0) {
                if (strcmp(value, "veth") == 0) {
                    ns->connect_type = CONNECT_VETH;
                } else {
                    ns->connect_type = CONNECT_BRIDGE;
                }
                strncpy(ns->connect_name, value, sizeof(ns->connect_name) - 1);
            } else {
                printf("prop: %s\n", ns_prop);
                return -1; // Invalid prop
            }
        }
        break;
    case CONFIG_KEY_BRIDGE:
        if (num_parts == 1) {
            config->bridge_count++;
            config->bridges = realloc(config->bridges,
                                      config->bridge_count * sizeof(bridge_t));
            if (config->bridges == NULL) {
                return -1; // Memory allocation failed
            }
            bridge_t *br = &config->bridges[config->bridge_count - 1];
            strncpy(br->name, value, sizeof(br->name) - 1);
        } else if (num_parts == 3) {
            const char *br_name = key_parts[1];
            const char *br_prop = key_parts[2];
            bridge_t *br = find_bridge_by_name(config, br_name);
            if (br == NULL) {
                return -1; // Bridge not defined
            }

            if (strcmp(br_prop, "ip") == 0) {
                if (parse_cidr(value, &br->ip_addr, &br->mask) != 0) {
                    return -1; // Invalid IP address
                }
            } else {
                return -1; // Invalid prop
            }
        }
        break;
    case CONFIG_KEY_FIREWALL_FORWARD_DEFAULT:
        if (num_parts != 1) {
            return -1; // only top level
        }
        if (strcmp(value, "ALLOW") == 0) {
            config->fw_default_action = FW_ALLOW;
        } else if (strcmp(value, "DROP") == 0) {
            config->fw_default_action = FW_DROP;
        } else {
            return -1; // invalid firewall rule
        }
        break;
    case CONFIG_KEY_FIREWALL_ALLOW_FORWARD:
        if (num_parts != 1) {
            return -1; // only top level
        }
        config->fw_rule_count++;
        config->fw_rules = realloc(config->fw_rules,
                                   config->fw_rule_count * sizeof(fw_rule_t));
        if (config->fw_rules == NULL) {
            return -1; // Memory allocation failed
        }
        fw_rule_t *fw_rule = &config->fw_rules[config->fw_rule_count - 1];
        if (parse_fw_rule(value, fw_rule) != 0) {
            return -1; // Invalid FW rule
        }
        break;
    case CONFIG_KEY_ENABLE_NAT:
        if (num_parts != 1) {
            return -1; // only top level
        }
        config->nat_rule_count++;
        config->nat_rules = realloc(
            config->nat_rules, (config->nat_rule_count * sizeof(nat_rule_t)));
        if (config->nat_rules == NULL) {
            return -1; // Memory allocation failed
        }
        nat_rule_t *nat_rule = &config->nat_rules[config->nat_rule_count - 1];
        if (parse_cidr(value, &nat_rule->network, &nat_rule->mask) != 0) {
            return -1; // Invalid CIDR
        }
        break;
    case CONFIG_KEY_UNKNOWN:
        return -1;
    }

    return 0;
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
        fprintf(fp, "Namespace %s:\n", ns->name);
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
        fprintf(fp, "Bridge %s:\n", br->name);
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
