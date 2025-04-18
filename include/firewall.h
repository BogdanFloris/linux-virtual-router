#ifndef _FIREWALL_H
#define _FIREWALL_H

#include "constants.h"

#include <netdb.h>

/* Firewall action types */
typedef enum {
    FW_ALLOW, /* Allow traffic matching this rule */
    FW_DROP,  /* Drop traffic matching this rule */
} fw_action_t;

/* Network endpoint types for firewall rules */
typedef enum {
    ENDPOINT_NS,       /* A specific network namespace */
    ENDPOINT_INTERNET, /* The external internet/WAN */
} endpoint_t;

/* Firewall rule structure */
typedef struct {
    endpoint_t src_type; /* Source endpoint type */
    char
        src_name[MAX_NAME_LEN]; /* Source name (namespace name or "INTERNET") */
    endpoint_t dst_type;        /* Destination endpoint type */
    char dst_name[MAX_NAME_LEN]; /* Destination name (namespace name or
                                    "INTERNET") */
    fw_action_t
        action; /* Action to take on matching traffic (default: FW_ALLOW) */
} fw_rule_t;

/* NAT rule structure */
typedef struct {
    struct in_addr network; /* Network address to NAT */
    u_int8_t mask;          /* CIDR notation subnet mask */
} nat_rule_t;

#endif // !_FIREWALL_H
