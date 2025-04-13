#include <netdb.h>

/* Connection type for network namespaces */
typedef enum {
    CONNECT_BRIDGE, /* Connect namespace via bridge */
    CONNECT_VETH    /* Connect namespace via veth pair */
} connect_t;

/* Network namespace configuration */
typedef struct {
    struct in_addr ip_addr; /* IP address of the namespace interface */
    u_int8_t mask;          /* CIDR notation subnet mask (e.g., 24 for /24) */
    struct in_addr gateway; /* Default gateway IP for the namespace */
    connect_t connect_type; /* How this namespace connects to the host (bridge
                               or veth) */
    char connect_name[32];  /* Name of bridge or veth pair to use */
} namespace_t;
