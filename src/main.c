#include "config.h"
#include "network.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *config_filename;
    config_t config;

    if (argc != 3) {
        printf("Usage: %s <config_file> <--up|--down>\n", argv[0]);
        return EXIT_FAILURE;
    }

    config_filename = argv[1];

    init_config(&config);
    int result = 0;
    ;
    if ((result = parse_config_file(config_filename, &config)) != 0) {
        fprintf(stderr,
                "ERROR: Failed to parse network topology configuration with "
                "code %d\n",
                result);
        return EXIT_FAILURE;
    }

    int status = 0;
    if (strcmp(argv[2], "--up") == 0) {
        status = network_up(&config);
        if (status != 0) {
            fprintf(stderr,
                    "ERROR: Failed to initialize network with code %d\n",
                    status);
            goto out_delete;
        }
    } else if (strcmp(argv[2], "--down") == 0) {
        status = network_down(&config);
        if (status != 0) {
            fprintf(stderr, "ERROR: Failed to clean up network with code %d\n",
                    status);
            goto out_delete;
        }
    } else {
        fprintf(stderr, "Invalid argument: %s\n", argv[2]);
        goto out_delete;
    }

    free_config(&config);

    return 0;

out_delete:
    free_config(&config);
    return EXIT_FAILURE;
}
