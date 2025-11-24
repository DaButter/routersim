#include "internet_protocol.hpp"
#include "icmp.hpp"
#include "tcp.hpp"
#include "udp.hpp"
#include "packet_builders.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>

// example of a dummy hardcoded routing table
void InternetProtocol::initRoutingTable() {
    routingTable.addRoute("192.168.1.0/24", "wlan0");                   // home WiFi network
    routingTable.addRoute("127.0.0.0/8", "lo");                         // loopback (localhost)
    routingTable.addRoute("8.8.8.8/32", "wlan0", "192.168.1.1", 1);     // google DNS via router
    routingTable.addRoute("1.1.1.1/32", "wlan0", "192.168.1.1", 1);     // cloudflare DNS via router
    routingTable.addRoute("0.0.0.0/0", "wlan0", "192.168.1.1", 10);     // everything else via home router
}

void InternetProtocol::addRoute(const std::string& network, const std::string& interface,
                      const std::string& next_hop, int metric) {
    routingTable.addRoute(network, interface, next_hop, metric);
}

void InternetProtocol::printRoutingTable() {
    routingTable.printTable();
}

void InternetProtocol::parsePacket(const std::vector<uint8_t>& packet) {
    log_debug("Starting packet parsing, packet size: %zu bytes", packet.size());

    uint8_t version = (packet[0] >> 4) & 0x0F;
    if (version != 4) {
        log_error("Unsupported IP version: %u (expected 4)", version);
        return;
    }

    if (packet.size() < IPv4_HEADER_SIZE) {
        log_error("Packet too short");
        return;
    }

    IPv4Header header;
    std::memcpy(&header, packet.data(), sizeof(IPv4Header));
    header.total_length = ntohs(header.total_length);
    header.identification = ntohs(header.identification);
    header.flags_fragment_offset = ntohs(header.flags_fragment_offset);
    header.header_checksum = ntohs(header.header_checksum);
    header.src_ip = ntohl(header.src_ip);
    header.dst_ip = ntohl(header.dst_ip);

    log_debug("Parsed packet - TTL: %d, Protocol: %d, Total Length: %d",
                header.ttl, header.protocol, header.total_length);

    printIPHeader(header);
    printTransportLayerHeader(packet, header);
    simulateForwarding(header);
}

void InternetProtocol::printIPHeader(const IPv4Header& h) {
    std::cout << "IPv4 Header:\n"
              << "  Source IP: "      << inet_ntoa({htonl(h.src_ip)}) << "\n"
              << "  Destination IP: " << inet_ntoa({htonl(h.dst_ip)}) << "\n"
              << "  TTL: "            << static_cast<int>(h.ttl)      << "\n"
              << "  Protocol: "       << static_cast<int>(h.protocol) << "\n";
}

void InternetProtocol::simulateForwarding(const IPv4Header& h) {
    struct in_addr dst_addr;
    dst_addr.s_addr = htonl(h.dst_ip);
    std::string dst_ip_str = inet_ntoa(dst_addr);

    log_debug("Attempting to forward packet to destination: %s", dst_ip_str.c_str());

    if (h.ttl == 0) {
        log_warning("Packet dropped: TTL expired for destination %s", dst_ip_str.c_str());
        std::cout << "Packet dropped: TTL expired\n";
        return;
    }

    std::string interface = routingTable.lookupRoute(h.dst_ip);
    if (!interface.empty()) {
        log_info("Forwarding packet to interface %s for destination %s", interface.c_str(), dst_ip_str.c_str());
        std::cout << "Forwarding packet to interface " << interface << "\n";
    } else {
        log_warning("No route found for destination %s. Dropping packet", dst_ip_str.c_str());
        std::cout << "No route found. Dropping packet.\n";
    }
}

void InternetProtocol::printTransportLayerHeader(const std::vector<uint8_t>& packet, const IPv4Header& ip_header) {
    if (packet.size() < IPv4_HEADER_SIZE) {
        log_error("Packet too short for stated IP header length");
        return;
    }

    log_debug("Parsing Layer 4 header for protocol %d", ip_header.protocol);

    switch (ip_header.protocol) {
        case PROTOCOL_TCP: {
            TCPHeader tcp_header = TCP::parseHeader(packet, IPv4_HEADER_SIZE);
            TCP::printHeader(tcp_header);
            break;
        }
        case PROTOCOL_UDP: {
            UDPHeader udp_header = UDP::parseHeader(packet, IPv4_HEADER_SIZE);
            UDP::printHeader(udp_header);
            break;
        }
        case PROTOCOL_ICMP: {
            ICMPHeader icmp_header = ICMP::parseHeader(packet, IPv4_HEADER_SIZE);
            ICMP::printHeader(icmp_header);
            break;
        }
        default:
            log_warning("Unknown or unsupported protocol: %d", ip_header.protocol);
            std::cout << "Unknown or unsupported transport layer protocol: " << (int)ip_header.protocol << "\n";
            break;
    }
}

// not used anywhere currently, but could be useful later
// void InternetProtocol::decrementTTL(IPv4Header& header) {
//     if (header.ttl > 0) {
//         header.ttl--;
//     }
// }
