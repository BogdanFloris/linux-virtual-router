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

int main() {
    test_parse_cidr();
    test_parse_fw_rule();
    return 0;
}
