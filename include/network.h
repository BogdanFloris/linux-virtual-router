/*
 * network.h
 *
 * Setup and teardown for the network topology
 */
#ifndef _NETWORK_H
#define _NETWORK_H

#include "config.h"

/**
 * Initialize the network environment based on configuration
 *
 * @param config Pointer to a parsed config_t structure
 * @return 0 on success, -1 on failure
 */
int network_up(config_t *config);

/**
 * Setup IPv4 forwarding based on configuration
 *
 * @param enable Whether to enable IPv4 forwarding
 * @return 0 on success, -1 on failure
 */
int setup_ipv4_forwarding(bool enable);

/**
 * Create all namespaces defined in configuration
 *
 * @param namespaces Array of namespace_t structures
 * @param count Number of namespaces to create
 * @return 0 on success, -1 on failure
 */
int create_namespaces(namespace_t *namespaces, int count);

/**
 * Remove all namespaces created
 *
 * @param namespaces Array of namespace_t structures
 * @param count Number of namespaces to remove
 * @return 0 on success, -1 on failure
 */
int remove_namespaces(namespace_t *namespaces, int count);

/**
 * Create all bridges defined in configuration
 *
 * @param bridges Array of bridge_t structures
 * @param count Number of bridges to create
 * @return 0 on success, -1 on failure
 */
int create_bridges(bridge_t *bridges, int count);

/**
 * Connect namespaces to bridges or directly to host
 *
 * @param namespaces Array of namespace_t structures
 * @param count Number of namespaces to connect
 * @param bridges Array of bridge_t structures
 * @param bridge_count Number of bridges available
 * @return 0 on success, -1 on failure
 */
int connect_namespaces(namespace_t *namespaces, int count, bridge_t *bridges,
                       int bridge_count);

/**
 * Setup network interfaces inside namespaces
 *
 * @param namespaces Array of namespace_t structures
 * @param count Number of namespaces to configure
 * @return 0 on success, -1 on failure
 */
int setup_namespace_networking(namespace_t *namespaces, int count);

/**
 * Setup firewall rules based on configuration
 *
 * @param config Pointer to a parsed config_t structure
 * @return 0 on success, -1 on failure
 */
int setup_firewall(config_t *config);

/**
 * Setup NAT based on configuration
 *
 * @param config Pointer to a parsed config_t structure
 * @return 0 on success, -1 on failure
 */
int setup_nat(config_t *config);

/**
 * Cleanup and tear down the entire network configuration
 *
 * @param config Pointer to the config_t structure used to set up the network
 * @return 0 on success, -1 on failure
 */
int network_down(config_t *config);

#endif /* _NETWORK_H */
