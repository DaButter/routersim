#include "icmp.hpp"
#include "logger.hpp"
#include "packet_builders.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

ICMPHeader ICMP::parseHeader(const std::vector<uint8_t>& packet, size_t offset) {
    if (packet.size() < offset + ICMP_HEADER_SIZE) {
        log_error("Packet too short for ICMP header");
        return {};
    }

    ICMPHeader header;
    std::memcpy(&header, packet.data() + offset, sizeof(ICMPHeader));
    header.checksum = ntohs(header.checksum);
    header.identifier = ntohs(header.identifier);
    header.sequence = ntohs(header.sequence);

    log_debug("Parsed ICMP header - Type: %d (%s), Code: %d, ID: %d, Seq: %d",
              header.type, getTypeName(header.type).c_str(), header.code,
              header.identifier, header.sequence);

    return header;
}

ICMPHeader ICMP::createHeader(uint8_t type, uint16_t identifier, uint16_t sequence) {
    ICMPHeader header = {};
    header.type = type;
    header.code = 0;              // 0 for ping
    header.identifier = identifier;
    header.sequence = sequence;
    header.checksum = 0x0000;    // placeholder, to be calculated later

    log_debug("Created sample ICMP header - Type: %d (%s), ID: %d, Seq: %d", type, getTypeName(type).c_str(), identifier, sequence);
    return header;
}

std::vector<uint8_t> ICMP::serializeHeader(const ICMPHeader& header) {
    std::vector<uint8_t> icmp_header(ICMP_HEADER_SIZE);

    icmp_header[0] = header.type;
    icmp_header[1] = header.code;
    icmp_header[2] = (header.checksum >> 8) & 0xFF;
    icmp_header[3] = header.checksum & 0xFF;
    icmp_header[4] = (header.identifier >> 8) & 0xFF;
    icmp_header[5] = header.identifier & 0xFF;
    icmp_header[6] = (header.sequence >> 8) & 0xFF;
    icmp_header[7] = header.sequence & 0xFF;

    return icmp_header;
}

uint16_t ICMP::calculateChecksum(const std::vector<uint8_t>& icmp_data) {
    /* Calculate ICMP checksum according to RFC 792 (Internet Control Message Protocol):
       The checksum covers the entire ICMP message (header + data) as 16-bit words.
       For messages with odd length, the last byte is padded with zero.
       The checksum is the one's complement of the sum of all 16-bit words.
    */
    uint32_t sum = 0;

    for (size_t i = 0; i < icmp_data.size(); i += 2) {
        if (i + 1 < icmp_data.size()) {
            sum += (icmp_data[i] << 8) + icmp_data[i + 1];
        } else {
            sum += icmp_data[i] << 8;  // pad odd length
        }
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void ICMP::printHeader(const ICMPHeader& h) {
    std::cout << "ICMP Header:\n"
              << "  Type: " << static_cast<int>(h.type) 
              << " (" << getTypeName(h.type) << ")"
              << ", Code: " << static_cast<int>(h.code) << "\n"
              << "  Identifier: " << h.identifier
              << ", Sequence: " << h.sequence 
              << ", Checksum: 0x" << std::hex << h.checksum << std::dec << "\n";
}

std::string ICMP::getTypeName(uint8_t type) {
    switch (type) {
        case ICMP_ECHO_REPLY:   return "Echo Reply";
        case ICMP_DEST_UNREACH: return "Destination Unreachable";
        case ICMP_ECHO_REQUEST: return "Echo Request";
        case ICMP_TIME_EXCEED:  return "Time Exceeded";
        default:                return "Unknown";
    }
}
