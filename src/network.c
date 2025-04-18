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

int network_up(config_t *config) {
    int status = 0;
    if ((status = create_namespaces(config->namespaces,
                                    config->namespace_count)) != 0) {
        return status;
    }

    return 0;
}

int network_down(config_t *config) {
    int status = 0;
    if ((status = remove_namespaces(config->namespaces,
                                    config->namespace_count)) != 0) {
        return status;
    }
    return 0;
}

int create_namespace(void *arg) {
    const namespace_t ns = *(namespace_t *)arg;

    char ns_path[100];
    snprintf(ns_path, 100, "%s/%s", NETNS_RUN_DIR, ns.name);

    // create filesystem state
    int fd = open(ns_path, O_RDONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0);
    if (fd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "Network namespace %s already exists\n", ns.name);
            return 0;
        } else {
            fprintf(stderr, "Cannot create network namespace %s: %s\n", ns.name,
                    strerror(errno));
            return -1;
        }
    }
    close(fd);

    // bind netns
    if (mount(PROC_PATH, ns_path, "none", MS_BIND, NULL) < 0) {
        fprintf(stderr, "Bind %s -> %s failed: %s\n", PROC_PATH, ns_path,
                strerror(errno));
        unlink(ns_path);
        return -1;
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
            fprintf(stderr, "clone failed: %s\n", strerror(errno));
            free(stacks[i]);
            return -1; // clone failed
        }
    }

    int overall_status = 0;
    for (int i = 0; i < count; i++) {
        int status;
        if (waitpid(ns_pids[i], &status, 0) == -1) {
            fprintf(stderr, "waitpid failed for pid %d: %s\n", ns_pids[i],
                    strerror(errno));
            overall_status = -1;
        } else {
            // check if the child process exited with an error
            if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
                fprintf(stderr, "Namespace creation failed for pid %d\n",
                        ns_pids[i]);
                overall_status = -1;
            }
        }

        free(stacks[i]);
    }

    return overall_status;
}

int remove_namespace(char *ns_name) {
    char ns_path[100];
    snprintf(ns_path, 100, "%s/%s", NETNS_RUN_DIR, ns_name);

    if (umount(ns_path) != 0) {
        if (!(errno == ENOENT || errno == EINVAL)) {
            fprintf(stderr, "Unmount failed for %s: %s\n", ns_name,
                    strerror(errno));
            return -1;
        }
    }

    if (unlink(ns_path) != 0) {
        if (errno != ENOENT) {
            fprintf(stderr, "Unlink failed for %s: %s\n", ns_name,
                    strerror(errno));
        }
    }

    return 0;
}

int remove_namespaces(namespace_t *namespaces, int count) {
    int cleanup_status = 0;
    for (int i = 0; i < count; i++) {
        if ((cleanup_status = remove_namespace(namespaces[i].name)) != 0) {
            fprintf(stderr, "Failed to remove namespace %s\n",
                    namespaces[i].name);
        }
    }
    return cleanup_status;
}
