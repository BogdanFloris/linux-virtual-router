#define _GNU_SOURCE
#include "network.h"

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE 65536 // Stack size for namespace child processes
#define NETNS_RUN_DIR "/var/run/netns"
#define PROC_PATH "/proc/self/ns/net"

int network_init(config_t *config) {
    int status = 0;
    if ((status = create_namespaces(config->namespaces,
                                    config->namespace_count)) != 0) {
        return status;
    }

    return 0;
}

int network_cleanup(config_t *config) { return 0; }

int create_namespace(void *arg) {
    const namespace_t ns = *(namespace_t *)arg;

    char ns_path[100];
    snprintf(ns_path, 100, "%s/%s", NETNS_RUN_DIR, ns.name);

    // create filesystem state
    int fd = open(ns_path, O_RDONLY | O_CREAT | O_CLOEXEC, 0);
    if (fd < 0) {
        fprintf(stderr, "Cannot open network namespace %s: %s", ns.name,
                strerror(errno));
        return -1;
    }
    printf("Created network namespace %d\n", getpid());
    close(fd);

    // bind netns
    if (mount(PROC_PATH, ns_path, "none", MS_BIND, NULL) < 0) {
        fprintf(stderr, "Bind %s -> %s failed: %s\n", PROC_PATH, ns_path,
                strerror(errno));
    }

    return 0;
}

int create_namespaces(namespace_t *namespaces, int count) {
    pid_t ns_pids[count];
    char *stacks[count];

    for (int i = 0; i < count; i++) {
        const namespace_t ns = namespaces[i];
        char *stack_top;
        int flags = CLONE_NEWNET;

        stacks[i] = malloc(STACK_SIZE);
        if (stacks[i] == NULL) {
            return -1; // malloc failed
        }
        stack_top = stacks[i] + STACK_SIZE;

        if ((ns_pids[i] = clone(create_namespace, stack_top, flags | SIGCHLD,
                                (void *)&ns)) == -1) {
            fprintf(stderr, "clone failed: %s", strerror(errno));
            free(stacks[i]);
            return -1; // clone failed
        }
    }

    for (int i = 0; i < count; i++) {
        if (waitpid(ns_pids[i], NULL, 0) == -1) {
            fprintf(stderr, "waitpid: %s\n", strerror(errno));
            return -1; // waitpid failed
        }

        if (stacks[i] != NULL) {
            free(stacks[i]);
        }
    }

    return 0;
}
