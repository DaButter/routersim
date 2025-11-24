#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct __attribute__((packed)) TCPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_number;
    uint32_t ack_number;
    uint8_t data_offset_flags;  // data offset (4 bits) + Reserved (3 bits) + NS flag (1 bit)
    uint8_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
};

constexpr uint8_t TCP_FIN = 0x01;
constexpr uint8_t TCP_SYN = 0x02;
constexpr uint8_t TCP_RST = 0x04;
constexpr uint8_t TCP_PSH = 0x08;
constexpr uint8_t TCP_ACK = 0x10;
constexpr uint8_t TCP_URG = 0x20;
constexpr uint8_t TCP_ECE = 0x40;
constexpr uint8_t TCP_CWR = 0x80;

class TCP {
  public:
    static TCPHeader parseHeader(const std::vector<uint8_t>& packet, size_t offset);

    static TCPHeader createHeader(uint16_t src_port, uint16_t dst_port, uint8_t flags = TCP_SYN);
    static std::vector<uint8_t> serializeHeader(const TCPHeader& header);
    static uint16_t calculateChecksum(const uint32_t& src_ip, const uint32_t& dst_ip, const std::vector<uint8_t>& tcp_data);

    static void printHeader(const TCPHeader& header);
};
