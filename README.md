# Linux Virtual Router

A configurable virtual router implementation for Linux systems using network namespaces.

## Overview

This project creates isolated network environments using Linux network namespaces and connects them through a configurable virtual router. It can be used for network testing, development environments, or to study networking concepts.

Features:
- Create multiple isolated network namespaces
- Connect namespaces via bridges or veth pairs
- Configure IP addressing and routing
- Implement firewall rules between namespaces
- Set up NAT for internet access
- All configuration through a simple INI file

## Requirements

- Linux kernel with network namespace support
- Administrative (root) privileges
- Common utilities: ip, iptables, bridge-utils

## Quick Start

1. Clone the repository
2. Edit `topology.ini` to define your network
3. Build the project: `make`
4. Run with root privileges: `sudo bin/router`

## Configuration

The virtual router is configured through `topology.ini`. Here's an example:

```ini
# General Settings
enable_ipv4_forwarding = true
nat_outgoing_interface = ens160

# --- Namespace Definitions ---
namespace = private1
namespace.private1.ip = 192.168.100.2/24
namespace.private1.gateway = 192.168.100.1
namespace.private1.connect_via = bridge:br0

namespace = private2
namespace.private2.ip = 192.168.101.2/24
namespace.private2.gateway = 192.168.101.1
namespace.private2.connect_via = veth

# --- Bridge Definitions ---
bridge = br0
bridge.br0.ip = 192.168.100.1/24

# --- Firewall Rules ---
firewall_forward_default = DROP
firewall_allow_forward = private1 -> INTERNET
firewall_allow_forward = private2 -> INTERNET
firewall_allow_forward = private1 -> private2

# --- NAT Rules ---
enable_nat = 192.168.100.0/24
enable_nat = 192.168.101.0/24
```

## Project Structure

- `src/` - Source code
- `include/` - Header files
- `bin/` - Compiled binaries
- `obj/` - Object files
- `test/` - Test files

## Development

### Building

```bash
# Build the router
make

# Build and run tests
make test

# Generate compile_commands.json for IDE integration
make compiledb
```

### Adding Tests

Create a new test file in the `test/` directory with the prefix `test_`. The test will be automatically compiled and run with `make test`.

## License

[MIT License](LICENSE)