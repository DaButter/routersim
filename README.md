# Router Simulation

A C++ network router simulation that demonstrates IPv4 packet processing, routing table lookups, and protocol handling.

## Features

- **IPv4 Packet Processing**: Parses and validates IPv4 headers with checksum verification
- **Multi-Protocol Support**: Handles ICMP, TCP, and UDP protocols
- **Routing Table**: CIDR-based routing with longest prefix matching
- **Packet Building**: Creates realistic network packets for testing
- **Comprehensive Logging**: Thread-safe logging system with multiple log levels

## Quick Start

```bash
# Build and run
make
./router_sim

# Debug build (no optimizations)
make debug
./router_sim

# Release build (optimized)
make release
./router_sim
```

## What It Does

The simulation creates various network packets (ping, DNS queries, HTTPS connections) and processes them through a realistic routing pipeline:

1. **Packet Creation**: Builds IPv4 packets with proper headers and checksums
2. **Routing**: Uses CIDR routing table to determine next hop
3. **Protocol Processing**: Parses ICMP/TCP/UDP headers and displays details
4. **TTL Handling**: Drops packets with expired TTL values

## Sample Output

```
=== Routing Simulation ===

--- Processing Packet 1 ---
IPv4 Header: 192.168.1.100 → 192.168.1.50 (Protocol: ICMP, TTL: 64)
ICMP Header: Echo Request (Type: 8, ID: 1234, Seq: 1)
→ Forwarding to interface wlan0

--- Processing Packet 6 ---
IPv4 Header: 192.168.1.100 → 8.8.8.8 (Protocol: UDP, TTL: 0)
⚠️ Packet dropped: TTL expired
```

## Project Structure

```
src/
├── main.cpp                 # Main simulation
├── routing_table.*          # CIDR routing implementation  
├── network_layer/           # IPv4 and ICMP protocols
├── transport_layer/         # TCP and UDP protocols
└── utils/                   # Logging and packet builders
```

## Build Requirements

- C++17 compatible compiler (g++)
- Make

Builds tested on Linux. Check `routing_debug.log` for detailed packet processing logs.

## VS Code Setup
- `.vscode/launch.json` - GDB debugger configuration
- `.vscode/tasks.json` - Make build task
- `.vscode/settings.json` - Project-specific editor settings
