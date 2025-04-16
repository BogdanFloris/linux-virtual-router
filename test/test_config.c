#include "config.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(condition, message)                                        \
    do {                                                                       \
        if (!(condition)) {                                                    \
            printf("ASSERTION FAILED: %s\n", message);                         \
            printf("  In file: %s, line: %d\n", __FILE__, __LINE__);           \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

void test_parse_cidr() {
    printf("Testing parse_cidr()...\n");

    // Test case 1: Valid CIDR
    {
        const char *cidr_str = "192.168.1.0/24";
        struct in_addr addr;
        u_int8_t mask;

        int result = parse_cidr(cidr_str, &addr, &mask);
        TEST_ASSERT(result == 0, "parse_cidr should return 0 for valid input");

        // Convert address to string for comparison
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, ip_str, INET_ADDRSTRLEN);

        TEST_ASSERT(strcmp(ip_str, "192.168.1.0") == 0,
                    "IP address should be correctly parsed");
        TEST_ASSERT(mask == 24, "Mask should be correctly parsed");
    }

    // Test case 2: Invalid CIDR (bad IP)
    {
        const char *cidr_str = "300.168.1.0/24";
        struct in_addr addr;
        u_int8_t mask;

        int result = parse_cidr(cidr_str, &addr, &mask);
        TEST_ASSERT(result != 0,
                    "parse_cidr should return error for invalid IP");
    }

    // Test case 3: Invalid CIDR (bad mask)
    {
        const char *cidr_str = "192.168.1.0/33";
        struct in_addr addr;
        u_int8_t mask;

        int result = parse_cidr(cidr_str, &addr, &mask);
        TEST_ASSERT(result != 0,
                    "parse_cidr should return error for invalid mask");
    }

    // Test case 4: Invalid format (no mask)
    {
        const char *cidr_str = "192.168.1.0";
        struct in_addr addr;
        u_int8_t mask;

        int result = parse_cidr(cidr_str, &addr, &mask);
        TEST_ASSERT(result != 0,
                    "parse_cidr should return error for missing mask");
    }

    printf("parse_cidr() tests passed!\n");
}

void test_parse_fw_rule() {
    printf("Testing parse_fw_rule()...\n");

    // Test case 1: Valid rule with namespace source and internet destination
    {
        const char *rule_str = "private1 -> INTERNET";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result == 0,
                    "parse_fw_rule should return 0 for valid input");

        TEST_ASSERT(rule.src_type == ENDPOINT_NS,
                    "Source type should be ENDPOINT_NS");
        TEST_ASSERT(strcmp(rule.src_name, "private1") == 0,
                    "Source name should be 'private1'");

        TEST_ASSERT(rule.dst_type == ENDPOINT_INTERNET,
                    "Destination type should be ENDPOINT_INTERNET");
        TEST_ASSERT(strcmp(rule.dst_name, "INTERNET") == 0,
                    "Destination name should be 'INTERNET'");

        TEST_ASSERT(rule.action == FW_ALLOW,
                    "Default action should be FW_ALLOW");
    }

    // Test case 2: Valid rule with internet source and namespace destination
    {
        const char *rule_str = "INTERNET -> private2";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result == 0,
                    "parse_fw_rule should return 0 for valid input");

        TEST_ASSERT(rule.src_type == ENDPOINT_INTERNET,
                    "Source type should be ENDPOINT_INTERNET");
        TEST_ASSERT(strcmp(rule.src_name, "INTERNET") == 0,
                    "Source name should be 'INTERNET'");

        TEST_ASSERT(rule.dst_type == ENDPOINT_NS,
                    "Destination type should be ENDPOINT_NS");
        TEST_ASSERT(strcmp(rule.dst_name, "private2") == 0,
                    "Destination name should be 'private2'");

        TEST_ASSERT(rule.action == FW_ALLOW,
                    "Default action should be FW_ALLOW");
    }

    // Test case 3: Valid rule between two namespaces
    {
        const char *rule_str = "private1 -> private2";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result == 0,
                    "parse_fw_rule should return 0 for valid input");

        TEST_ASSERT(rule.src_type == ENDPOINT_NS,
                    "Source type should be ENDPOINT_NS");
        TEST_ASSERT(strcmp(rule.src_name, "private1") == 0,
                    "Source name should be 'private1'");

        TEST_ASSERT(rule.dst_type == ENDPOINT_NS,
                    "Destination type should be ENDPOINT_NS");
        TEST_ASSERT(strcmp(rule.dst_name, "private2") == 0,
                    "Destination name should be 'private2'");
    }

    // Test case 4: Invalid format (no arrow)
    {
        const char *rule_str = "private1 INTERNET";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result != 0,
                    "parse_fw_rule should return error for missing arrow");
    }

    // Test case 5: Invalid format (missing destination)
    {
        const char *rule_str = "private1 ->";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(
            result != 0,
            "parse_fw_rule should return error for missing destination");
    }

    // Test case 6: Invalid format (missing source)
    {
        const char *rule_str = "-> INTERNET";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result != 0,
                    "parse_fw_rule should return error for missing source");
    }

    // Test case 7: Empty string
    {
        const char *rule_str = "";
        fw_rule_t rule;

        int result = parse_fw_rule(rule_str, &rule);
        TEST_ASSERT(result != 0,
                    "parse_fw_rule should return error for empty string");
    }

    printf("parse_fw_rule() tests passed!\n");
}

void test_parse_config_line() {
    printf("Testing parse_config_line()...\n");

    config_t config;

    // Test case 1: Simple boolean flag
    {
        char line[] = "enable_ipv4_forwarding = true";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse boolean flag successfully");
        TEST_ASSERT(config.ipv4_forwrd == true,
                    "Should set boolean flag to true");

        free_config(&config);
    }

    // Test case 2: Interface setting
    {
        char line[] = "nat_outgoing_interface = eth0";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse interface setting successfully");
        TEST_ASSERT(strcmp(config.nat_outgoing_interface, "eth0") == 0,
                    "Should set interface name correctly");

        free_config(&config);
    }

    // Test case 3: Namespace definition
    {
        char line[] = "namespace = private1";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0,
                    "Should parse namespace definition successfully");
        TEST_ASSERT(config.namespace_count == 1,
                    "Should increment namespace count");
        TEST_ASSERT(strcmp(config.namespaces[0].name, "private1") == 0,
                    "Should set namespace name correctly");

        free_config(&config);
    }

    // Test case 4: Namespace property (IP)
    {
        char lines[][100] = {"namespace = private1",
                             "namespace.private1.ip = 192.168.100.2/24"};
        init_config(&config);

        int result1 = parse_config_line(lines[0], &config);
        int result2 = parse_config_line(lines[1], &config);

        TEST_ASSERT(result1 == 0 && result2 == 0,
                    "Should parse namespace property successfully");

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &config.namespaces[0].ip_addr, ip_str,
                  INET_ADDRSTRLEN);

        TEST_ASSERT(strcmp(ip_str, "192.168.100.2") == 0,
                    "Should set namespace IP correctly");
        TEST_ASSERT(config.namespaces[0].mask == 24,
                    "Should set namespace mask correctly");

        free_config(&config);
    }

    // Test case 5: Firewall rule
    {
        char line[] = "firewall_allow_forward = private1 -> INTERNET";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse firewall rule successfully");
        TEST_ASSERT(config.fw_rule_count == 1,
                    "Should increment firewall rule count");
        TEST_ASSERT(config.fw_rules[0].src_type == ENDPOINT_NS,
                    "Should set source type correctly");
        TEST_ASSERT(strcmp(config.fw_rules[0].src_name, "private1") == 0,
                    "Should set source name correctly");
        TEST_ASSERT(config.fw_rules[0].dst_type == ENDPOINT_INTERNET,
                    "Should set destination type correctly");
        TEST_ASSERT(strcmp(config.fw_rules[0].dst_name, "INTERNET") == 0,
                    "Should set destination name correctly");

        free_config(&config);
    }

    // Test case 6: Comment line
    {
        char line[] = "# This is a comment";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should ignore comment lines");

        free_config(&config);
    }

    // Test case 7: Empty line
    {
        char line[] = "";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should ignore empty lines");

        free_config(&config);
    }

    // Test case 8: Line with trailing comment
    {
        char line[] = "enable_ipv4_forwarding = true # Enable IPv4 forwarding";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse line with trailing comment");
        TEST_ASSERT(config.ipv4_forwrd == true,
                    "Should set boolean flag correctly with trailing comment");

        free_config(&config);
    }

    // Test case 9: Malformed line (no equals sign)
    {
        char line[] = "enable_ipv4_forwarding true";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result != 0,
                    "Should reject malformed line with no equals sign");

        free_config(&config);
    }

    // Test case 10: NAT rule
    {
        char line[] = "enable_nat = 192.168.100.0/24";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse NAT rule successfully");
        TEST_ASSERT(config.nat_rule_count == 1,
                    "Should increment NAT rule count");

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &config.nat_rules[0].network, ip_str,
                  INET_ADDRSTRLEN);

        TEST_ASSERT(strcmp(ip_str, "192.168.100.0") == 0,
                    "Should set NAT network correctly");
        TEST_ASSERT(config.nat_rules[0].mask == 24,
                    "Should set NAT mask correctly");

        free_config(&config);
    }

    // Test case 11: Firewall default action
    {
        char line[] = "firewall_forward_default = DROP";
        init_config(&config);

        int result = parse_config_line(line, &config);
        TEST_ASSERT(result == 0, "Should parse firewall default action");
        TEST_ASSERT(config.fw_default_action == FW_DROP,
                    "Should set firewall default action to DROP");

        free_config(&config);
    }

    printf("parse_config_line() tests passed!\n");
}

int main() {
    test_parse_cidr();
    test_parse_fw_rule();
    test_parse_config_line();
    return 0;
}
