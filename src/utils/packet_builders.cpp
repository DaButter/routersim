#include "packet_builders.hpp"
#include "icmp.hpp"
#include "tcp.hpp"
#include "udp.hpp"
#include "logger.hpp"
#include <stdexcept>
#include <arpa/inet.h>
#include <cstring>

std::vector<uint8_t> IPv4PacketBuilder::createIPHeader(uint16_t total_length, uint8_t protocol) const {
    std::vector<uint8_t> ipv4header(IPv4_HEADER_SIZE);

    ipv4header[0] = 0x45;                                        // Version (4) + IHL (5) - 20 byte header
    ipv4header[1] = ipv4_tos;                                    // Type of Service
    ipv4header[2] = (total_length >> 8) & 0xFF;                  // Total Length (high byte)
    ipv4header[3] = total_length & 0xFF;                         // Total Length (low byte)
    ipv4header[4] = (ipv4_identification >> 8) & 0xFF;           // Identification (high byte)
    ipv4header[5] = ipv4_identification & 0xFF;                  // Identification (low byte)
    ipv4header[6] = (ipv4_flags_fragment_offset >> 8) & 0xFF;    // Flags + Fragment Offset (high)
    ipv4header[7] = ipv4_flags_fragment_offset & 0xFF;           // Fragment Offset (low)
    ipv4header[8] = ipv4_ttl;                                    // TTL
    ipv4header[9] = protocol;                                    // Protocol
    ipv4header[10] = 0x00;                                       // Header Checksum (high byte) - to be calculated
    ipv4header[11] = 0x00;                                       // Header Checksum (low byte) - to be calculated

    // convert source/destination IP to network byte order
    uint32_t src_ip_int = htonl(ipStringToInt(ipv4_src_ip));
    uint32_t dst_ip_int = htonl(ipStringToInt(ipv4_dst_ip));

    std::memcpy(&ipv4header[12], &src_ip_int, sizeof(src_ip_int));
    std::memcpy(&ipv4header[16], &dst_ip_int, sizeof(dst_ip_int));

    // calculate checksum
    uint32_t sum = 0;
    for (size_t i = 0; i < IPv4_HEADER_SIZE; i += 2) {
        sum += (ipv4header[i] << 8) + ipv4header[i + 1];
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    sum = ~sum;

    ipv4header[10] = (sum >> 8) & 0xFF;
    ipv4header[11] = sum & 0xFF;

    return ipv4header;
}

uint32_t IPv4PacketBuilder::ipStringToInt(const std::string& ip) const {
    uint32_t result = 0;
    size_t start = 0;
    int shift = 24;

    for (int i = 0; i < 4; i++) {
        size_t dot = ip.find('.', start);
        if (dot == std::string::npos && i < 3) {
            throw std::invalid_argument("Invalid IP address format");
        }

        std::string part = (i == 3) ? ip.substr(start) : ip.substr(start, dot - start);
        uint32_t ip_byte = std::stoi(part);

        if (ip_byte > 255) {
            throw std::invalid_argument("Invalid IP address: byte value > 255");
        }

        result |= (ip_byte << shift);
        shift -= 8;
        start = dot + 1;
    }

    return result;
}

std::vector<uint8_t> ICMPPacketBuilder::build() const {
    try {
        // create ICMP header
        ICMPHeader icmp_header_templ = ICMP::createHeader(icmp_type, icmp_id, icmp_seq);
        std::vector<uint8_t> icmp_header = ICMP::serializeHeader(icmp_header_templ);

        // serialize ICMP payload
        std::vector<uint8_t> payload_data(icmp_payload.begin(), icmp_payload.end());

        // build and wrap packet in IPv4 header
        std::vector<uint8_t> packet = createIPHeader(IPv4_HEADER_SIZE + ICMP_HEADER_SIZE + payload_data.size(), PROTOCOL_ICMP);
        packet.insert(packet.end(), icmp_header.begin(), icmp_header.end());
        packet.insert(packet.end(), payload_data.begin(), payload_data.end());

        // calculate checksum of ICMP header + payload
        uint16_t checksum = ICMP::calculateChecksum(std::vector<uint8_t>(IPv4_HEADER_SIZE + packet.begin(), packet.end()));
        packet[IPv4_HEADER_SIZE + ICMP_CHECKSUM_OFFSET] = (checksum >> 8) & 0xFF;
        packet[IPv4_HEADER_SIZE + ICMP_CHECKSUM_OFFSET + 1] = checksum & 0xFF;

        log_debug("Built ICMP packet: %zu bytes total", packet.size());
        return packet;
    } catch (const std::exception& e) {
        log_error("Failed to build ICMP packet: %s (src: %s, dst: %s) - dropping packet", e.what(), ipv4_src_ip.c_str(), ipv4_dst_ip.c_str());
        return {};
    }
}

std::vector<uint8_t> TCPPacketBuilder::build() const {
    try {
        // create TCP header
        TCPHeader tcp_header = TCP::createHeader(tcp_src_port, tcp_dst_port, tcp_flags);
        std::vector<uint8_t> tcp_data = TCP::serializeHeader(tcp_header);

        // serialize TCP payload
        std::vector<uint8_t> payload_data(tcp_payload.begin(), tcp_payload.end());

        // build and wrap packet in IPv4 header
        std::vector<uint8_t> packet = createIPHeader(IPv4_HEADER_SIZE + TCP_HEADER_SIZE + payload_data.size(), PROTOCOL_TCP);
        packet.insert(packet.end(), tcp_data.begin(), tcp_data.end());
        packet.insert(packet.end(), payload_data.begin(), payload_data.end());

        // calculate checksum of TCP header + payload
        uint16_t tcp_checksum = TCP::calculateChecksum(ipStringToInt(ipv4_src_ip), ipStringToInt(ipv4_dst_ip), std::vector<uint8_t>(IPv4_HEADER_SIZE + packet.begin(), packet.end()));
        packet[IPv4_HEADER_SIZE + TCP_CHECKSUM_OFFSET] = (tcp_checksum >> 8) & 0xFF;
        packet[IPv4_HEADER_SIZE + TCP_CHECKSUM_OFFSET + 1] = tcp_checksum & 0xFF;

        log_debug("Built TCP packet: %zu bytes total", packet.size());
        return packet;
    } catch (const std::exception& e) {
        log_error("Failed to build TCP packet: %s (src: %s, dst: %s) - dropping packet", e.what(), ipv4_src_ip.c_str(), ipv4_dst_ip.c_str());
        return {};
    }
}

std::vector<uint8_t> UDPPacketBuilder::build() const {
    try {
        // serialize UDP payload
        std::vector<uint8_t> payload_data(udp_payload.begin(), udp_payload.end());

        // create UDP header
        UDPHeader udp_header = UDP::createHeader(udp_src_port, udp_dst_port, payload_data.size());
        std::vector<uint8_t> udp_data = UDP::serializeHeader(udp_header);

        // build and wrap packet in IPv4 header
        std::vector<uint8_t> packet = createIPHeader(IPv4_HEADER_SIZE + UDP_HEADER_SIZE + payload_data.size(), PROTOCOL_UDP);
        packet.insert(packet.end(), udp_data.begin(), udp_data.end());
        packet.insert(packet.end(), payload_data.begin(), payload_data.end());

        // calculate checksum of UDP header + payload
        uint16_t udp_checksum = UDP::calculateChecksum(ipStringToInt(ipv4_src_ip), ipStringToInt(ipv4_dst_ip), std::vector<uint8_t>(IPv4_HEADER_SIZE + packet.begin(), packet.end()));
        packet[IPv4_HEADER_SIZE + UDP_CHECKSUM_OFFSET] = (udp_checksum >> 8) & 0xFF;
        packet[IPv4_HEADER_SIZE + UDP_CHECKSUM_OFFSET + 1] = udp_checksum & 0xFF;

        log_debug("Built UDP packet: %zu bytes total", packet.size());
        return packet;
    } catch (const std::exception& e) {
        log_error("Failed to build UDP packet: %s (src: %s, dst: %s) - dropping packet", e.what(), ipv4_src_ip.c_str(), ipv4_dst_ip.c_str());
        return {};
    }
}
