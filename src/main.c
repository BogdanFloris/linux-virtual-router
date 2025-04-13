#include "config.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char *config_filename = "topology.ini";
    config_t config;
    int result;

    printf("Parsing network topology configuration: %s\n", config_filename);

    init_config(&config);
    result = parse_config_file(config_filename, &config);
    if (result != 0) {
        printf("ERROR: Failed to parse network topology configuration with "
               "code %d\n",
               result);
        return EXIT_FAILURE;
    }

    printf("Successfully parsed:\n");
    printf("IPv4 forwarding: %s\n", config.ipv4_forwrd ? "enabled" : "disabled");
    printf("NAT outgoing interface: %s\n", config.nat_outgoing_interface);
    printf("Number of namespaces: %d\n", config.namespace_count);
    printf("Number of bridges: %d\n", config.bridge_count);
    printf("Number of firewall rules: %d\n", config.fw_rule_count);
    printf("Number of NAT rules: %d\n", config.nat_rule_count);

    free_config(&config);

    return 0;
}
