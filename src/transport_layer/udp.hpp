#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct __attribute__((packed)) UDPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
};

class UDP {
public:
    static UDPHeader parseHeader(const std::vector<uint8_t>& packet, size_t offset);

    static UDPHeader createHeader(uint16_t src_port, uint16_t dst_port, uint16_t data_length);
    static std::vector<uint8_t> serializeHeader(const UDPHeader& header);
    static uint16_t calculateChecksum(const uint32_t& src_ip, const uint32_t& dst_ip, const std::vector<uint8_t>& udp_data);

    static void printHeader(const UDPHeader& header);
};
