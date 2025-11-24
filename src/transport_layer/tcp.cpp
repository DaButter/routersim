#include "tcp.hpp"
#include "logger.hpp"
#include "packet_builders.hpp"
#include "internet_protocol.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

TCPHeader TCP::parseHeader(const std::vector<uint8_t>& packet, size_t offset) {
    if (packet.size() < offset + TCP_HEADER_SIZE) {
        log_error("Packet too short for TCP header");
        return {};
    }

    TCPHeader header;
    std::memcpy(&header, packet.data() + offset, sizeof(TCPHeader));
    header.src_port = ntohs(header.src_port);
    header.dst_port = ntohs(header.dst_port);
    header.seq_number = ntohl(header.seq_number);
    header.ack_number = ntohl(header.ack_number);
    header.window_size = ntohs(header.window_size);
    header.checksum = ntohs(header.checksum);
    header.urgent_pointer = ntohs(header.urgent_pointer);

    log_debug("Parsed TCP header - Src Port: %d, Dst Port: %d, Flags: 0x%02x",
              header.src_port, header.dst_port, header.flags);

    return header;
}

TCPHeader TCP::createHeader(uint16_t src_port, uint16_t dst_port, uint8_t flags) {
    TCPHeader header = {};
    header.src_port = src_port;
    header.dst_port = dst_port;
    header.seq_number = 0x12345678;                          // dummy sequence number
    header.ack_number = (flags & TCP_ACK) ? 0x87654321 : 0;  // dummy ack if ACK flag set
    header.data_offset_flags = 0x50;                         // 20-byte header (5 * 4), no reserved bits
    header.flags = flags;
    header.window_size = 8192;       // 8KB window
    header.checksum = 0x0000;       // placeholder, to be calculated later
    header.urgent_pointer = 0;

    log_debug("Created sample TCP header - Port %d -> %d, Flags: 0x%02x", src_port, dst_port, flags);
    return header;
}

std::vector<uint8_t> TCP::serializeHeader(const TCPHeader& header) {
    std::vector<uint8_t> tcp_header(TCP_HEADER_SIZE);

    tcp_header[0] = (header.src_port >> 8) & 0xFF;
    tcp_header[1] = header.src_port & 0xFF;
    tcp_header[2] = (header.dst_port >> 8) & 0xFF;
    tcp_header[3] = header.dst_port & 0xFF;
    tcp_header[4] = (header.seq_number >> 24) & 0xFF;
    tcp_header[5] = (header.seq_number >> 16) & 0xFF;
    tcp_header[6] = (header.seq_number >> 8) & 0xFF;
    tcp_header[7] = header.seq_number & 0xFF;
    tcp_header[8] = (header.ack_number >> 24) & 0xFF;
    tcp_header[9] = (header.ack_number >> 16) & 0xFF;
    tcp_header[10] = (header.ack_number >> 8) & 0xFF;
    tcp_header[11] = header.ack_number & 0xFF;
    tcp_header[12] = header.data_offset_flags;
    tcp_header[13] = header.flags;
    tcp_header[14] = (header.window_size >> 8) & 0xFF;
    tcp_header[15] = header.window_size & 0xFF;
    tcp_header[16] = (header.checksum >> 8) & 0xFF;
    tcp_header[17] = header.checksum & 0xFF;
    tcp_header[18] = (header.urgent_pointer >> 8) & 0xFF;
    tcp_header[19] = header.urgent_pointer & 0xFF;

    return tcp_header;
}

uint16_t TCP::calculateChecksum(const uint32_t& src_ip, const uint32_t& dst_ip, const std::vector<uint8_t>& tcp_data) {
    /* Calculate TCP checksum according to RFC 793 (Transmission Control Protocol):
       The checksum covers the TCP header, TCP payload, and a pseudo-header
       containing source IP, destination IP, protocol number, and TCP length.
       The pseudo-header helps ensure the packet is delivered to the correct endpoints.
    */
    uint32_t sum = 0;

    sum += (src_ip >> 16) + (src_ip & 0xFFFF);
    sum += (dst_ip >> 16) + (dst_ip & 0xFFFF);
    sum += PROTOCOL_TCP;
    sum += tcp_data.size();

    for (size_t i = 0; i < tcp_data.size(); i += 2) {
        if (i + 1 < tcp_data.size()) {
            sum += (tcp_data[i] << 8) + tcp_data[i + 1];
        } else {
            sum += tcp_data[i] << 8; // pad odd length
        }
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void TCP::printHeader(const TCPHeader& h) {
    std::cout << "TCP Header:\n"
              << "  Source Port: " << h.src_port
              << ", Destination Port: " << h.dst_port << "\n"
              << "  Sequence Number: " << h.seq_number
              << ", Acknowledgment Number: " << h.ack_number << "\n"
              << "  Window Size: " << h.window_size
              << ", Checksum: 0x" << std::hex << h.checksum << std::dec << "\n";

    std::cout << "  Flags: ";
    struct Flag {
        uint8_t mask;
        const char* name;
    };
    Flag tcpFlags[] = {
        {TCP_FIN, "FIN"}, {TCP_SYN, "SYN"}, {TCP_RST, "RST"},
        {TCP_PSH, "PSH"}, {TCP_ACK, "ACK"}, {TCP_URG, "URG"}
    };
    for (const auto& f : tcpFlags) {
        if (h.flags & f.mask) std::cout << f.name << " ";
    }
    std::cout << "(0x" << std::hex << static_cast<int>(h.flags) << std::dec << ")\n";
}
