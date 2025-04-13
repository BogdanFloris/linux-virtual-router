#include "config.h"

#include <arpa/inet.h>
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

