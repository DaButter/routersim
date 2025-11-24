#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "internet_protocol.hpp"
#include "icmp.hpp"
#include "tcp.hpp"
#include "udp.hpp"

constexpr size_t IPv4_HEADER_SIZE = 20;
constexpr size_t ICMP_HEADER_SIZE = 8;
constexpr size_t TCP_HEADER_SIZE = 20;
constexpr size_t UDP_HEADER_SIZE = 8;

constexpr size_t ICMP_CHECKSUM_OFFSET = 2;
constexpr size_t TCP_CHECKSUM_OFFSET = 16;
constexpr size_t UDP_CHECKSUM_OFFSET = 6;

class IPv4PacketBuilder {
public:
    std::string ipv4_src_ip = "192.168.1.100";
    std::string ipv4_dst_ip = "192.168.1.50";
    uint8_t ipv4_ttl = 64;
    uint16_t ipv4_identification = 0;
    uint8_t ipv4_tos = 0;
    uint16_t ipv4_flags_fragment_offset = 0x4000; // don't fragment flag set

protected:
    std::vector<uint8_t> createIPHeader(uint16_t total_length, uint8_t protocol) const;
    uint32_t ipStringToInt(const std::string& ip) const;
};

class ICMPPacketBuilder : public IPv4PacketBuilder {
public:
    uint8_t icmp_type = ICMP_ECHO_REQUEST; // default to Echo Request
    uint16_t icmp_id = 1234;
    uint16_t icmp_seq = 1;
    std::string icmp_payload = "Hello, ICMP World!";

    std::vector<uint8_t> build() const;
};

class TCPPacketBuilder : public IPv4PacketBuilder {
public:
    uint16_t tcp_src_port = 12345;
    uint16_t tcp_dst_port = 80;
    uint8_t tcp_flags = TCP_SYN;
    std::string tcp_payload = "";

    std::vector<uint8_t> build() const;
};

class UDPPacketBuilder : public IPv4PacketBuilder {
public:
    uint16_t udp_src_port = 12345;
    uint16_t udp_dst_port = 53;
    std::string udp_payload = "Hello, UDP World!";

    std::vector<uint8_t> build() const;
};
