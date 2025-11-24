#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct __attribute__((packed)) ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
};

constexpr uint8_t ICMP_ECHO_REPLY   = 0;
constexpr uint8_t ICMP_DEST_UNREACH = 3;
constexpr uint8_t ICMP_ECHO_REQUEST = 8;
constexpr uint8_t ICMP_TIME_EXCEED  = 11;

class ICMP {
  public:
    static ICMPHeader parseHeader(const std::vector<uint8_t>& packet, size_t offset);
    static ICMPHeader createHeader(uint8_t type = ICMP_ECHO_REQUEST, uint16_t identifier = 1234, uint16_t sequence = 1);
    static std::vector<uint8_t> serializeHeader(const ICMPHeader& header);
    static uint16_t calculateChecksum(const std::vector<uint8_t>& icmp_data);

    static void printHeader(const ICMPHeader& header);
    static std::string getTypeName(uint8_t type);
};
