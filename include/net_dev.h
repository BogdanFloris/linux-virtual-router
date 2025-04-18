#ifndef _NET_DEV_H
#define _NET_DEV_H

#include "constants.h"
#include <netdb.h>

/* Bridge configuration */
typedef struct {
    char name[MAX_NAME_LEN]; /* Name of the bridge */
    struct in_addr ip_addr;  /* IP address for the bridge interface */
    u_int8_t mask;           /* CIDR notation subnet mask */
} bridge_t;

#endif // !_NET_DEV_H
