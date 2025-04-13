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
    print_config(&config, stdout);

    free_config(&config);

    return 0;
}
