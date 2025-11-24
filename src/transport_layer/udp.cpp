#include "udp.hpp"
#include "logger.hpp"
#include "packet_builders.hpp"
#include "internet_protocol.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

UDPHeader UDP::parseHeader(const std::vector<uint8_t>& packet, size_t offset) {
    if (packet.size() < offset + UDP_HEADER_SIZE) {
        log_error("Packet too short for UDP header");
        return {};
    }

    UDPHeader header;
    std::memcpy(&header, packet.data() + offset, sizeof(UDPHeader));
    header.src_port = ntohs(header.src_port);
    header.dst_port = ntohs(header.dst_port);
    header.length = ntohs(header.length);
    header.checksum = ntohs(header.checksum);

    log_debug("Parsed UDP header - Src Port: %d, Dst Port: %d, Length: %d",
              header.src_port, header.dst_port, header.length);

    return header;
}

UDPHeader UDP::createHeader(uint16_t src_port, uint16_t dst_port, uint16_t data_length) {
    UDPHeader header = {};
    header.src_port = src_port;
    header.dst_port = dst_port;
    header.length = UDP_HEADER_SIZE + data_length;
    header.checksum = 0x0000;

    log_debug("Created sample UDP header - Port %d -> %d, Length: %d", src_port, dst_port, header.length);
    return header;
}

std::vector<uint8_t> UDP::serializeHeader(const UDPHeader& header) {
    std::vector<uint8_t> udp_header(UDP_HEADER_SIZE);

    udp_header[0] = (header.src_port >> 8) & 0xFF;
    udp_header[1] = header.src_port & 0xFF;
    udp_header[2] = (header.dst_port >> 8) & 0xFF;
    udp_header[3] = header.dst_port & 0xFF;
    udp_header[4] = (header.length >> 8) & 0xFF;
    udp_header[5] = header.length & 0xFF;
    udp_header[6] = (header.checksum >> 8) & 0xFF;
    udp_header[7] = header.checksum & 0xFF;

    return udp_header;
}

uint16_t UDP::calculateChecksum(const uint32_t& src_ip, const uint32_t& dst_ip, const std::vector<uint8_t>& udp_data) {
    /* Calculate UDP checksum according to RFC 768 (User Datagram Protocol):
       The checksum covers the pseudo-header (source IP, destination IP, protocol, UDP length),
       the UDP header, and the payload data as 16-bit words.
       If the length is odd, the last byte is padded with zero.
       The checksum is the one's complement of the sum of all 16-bit words.
    */
    uint32_t sum = 0;

    sum += (src_ip >> 16) + (src_ip & 0xFFFF);
    sum += (dst_ip >> 16) + (dst_ip & 0xFFFF);
    sum += PROTOCOL_UDP;
    sum += udp_data.size();

    for (size_t i = 0; i < udp_data.size(); i += 2) {
        if (i + 1 < udp_data.size()) {
            sum += (udp_data[i] << 8) + udp_data[i + 1];
        } else {
            sum += udp_data[i] << 8;  // pad odd length
        }
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void UDP::printHeader(const UDPHeader& h) {
    std::cout << "UDP Header:\n"
              << "  Source Port: " << h.src_port
              << ", Destination Port: " << h.dst_port << "\n"
              << "  Length: " << h.length << " bytes"
              << ", Checksum: 0x" << std::hex << h.checksum << std::dec << "\n";
}
