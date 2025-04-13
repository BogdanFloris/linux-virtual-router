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

int main() {
    test_parse_cidr();

    printf("All tests passed successfully!\n");
    return 0;
}
