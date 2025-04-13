#include "firewall.h"
#include "net_dev.h"
#include "net_ns.h"

#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>

/* Overall configuration structure */
typedef struct {
    bool ipv4_forwrd; /* Enable IPv4 forwarding */
    char
        nat_outgoing_interface[32]; /* Interface with internet access for NAT */
    namespace_t *namespaces;        /* Array of namespace configurations */
    int namespace_count;            /* Number of namespaces */
    bridge_t *bridges;              /* Array of bridge configurations */
    int bridge_count;               /* Number of bridges */
    fw_rule_t *fw_rules;            /* Array of firewall rules */
    int fw_rule_count;              /* Number of firewall rules */
    nat_rule_t *nat_rules;          /* Array of NAT rules */
    int nat_rule_count;             /* Number of NAT rules */
} config_t;

/**
 * Parse a CIDR notation string (e.g., "192.168.1.0/24") into address and mask
 *
 * @param cidr_str The CIDR notation string to parse
 * @param addr Pointer to store the parsed IP address
 * @param mask Pointer to store the parsed subnet mask
 * @return 0 on success, -1 on failure
 */
int parse_cidr(const char *cidr_str, struct in_addr *addr, u_int8_t *mask);

/**
 * Parse a configuration file into a config_t structure
 *
 * @param filename Path to the configuration file
 * @param config Pointer to config_t structure to fill
 * @return 0 on success, -1 on failure
 */
int parse_config_file(const char *filename, config_t *config);

/**
 * Parse a line from the configuration file
 *
 * @param line The line to parse
 * @param config Pointer to config_t structure to update
 * @return 0 on success, -1 on failure
 */
int parse_config_line(char *line, config_t *config);

/**
 * Free all dynamically allocated memory in a config_t structure
 *
 * @param config Pointer to config_t structure to free
 */
void free_config(config_t *config);

/**
 * Initialize an empty config_t structure
 *
 * @param config Pointer to config_t structure to initialize
 */
void init_config(config_t *config);

/**
 * Parse a firewall rule string (e.g., "private1 -> INTERNET")
 *
 * @param rule_str The rule string to parse
 * @param rule Pointer to fw_rule_t structure to fill
 * @return 0 on success, -1 on failure
 */
int parse_fw_rule(const char *rule_str, fw_rule_t *rule);

/**
 * Debug function to print the entire configuration
 *
 * @param config Pointer to config_t structure to print
 * @param fp File pointer to print to (stdout, stderr, etc.)
 */
void print_config(const config_t *config, FILE *fp);
