#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "routing_table.hpp"
#include "logger.hpp"

constexpr uint8_t PROTOCOL_ICMP = 1;
constexpr uint8_t PROTOCOL_TCP  = 6;
constexpr uint8_t PROTOCOL_UDP  = 17;

struct __attribute__((packed)) IPv4Header {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
};

class InternetProtocol {
public:
    void parsePacket(const std::vector<uint8_t>& packet);
    void initRoutingTable();
    void addRoute(const std::string& network, const std::string& interface,
                  const std::string& next_hop = "", int metric = 1);
    void printRoutingTable();

private:
    RoutingTable routingTable;
    void simulateForwarding(const IPv4Header& header);
    void printIPHeader(const IPv4Header& header);
    void printTransportLayerHeader(const std::vector<uint8_t>& packet, const IPv4Header& ip_header);
    // void decrementTTL(IPv4Header& header);
};
