#include <netdb.h>

/* Bridge configuration */
typedef struct {
    struct in_addr ip_addr; /* IP address for the bridge interface */
    u_int8_t mask;          /* CIDR notation subnet mask */
} bridge_t;
