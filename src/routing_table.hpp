#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

struct RouteEntry {
    uint32_t network;
    uint32_t subnet_mask;
    std::string interface;
    uint32_t next_hop;
    int metric;
};

class RoutingTable {
public:
    void addRoute(const std::string& network_cidr, const std::string& interface, 
                  const std::string& next_hop = "", int metric = 1);
    std::string lookupRoute(const uint32_t& dst_ip);
    void printTable();
    
private:
    std::vector<RouteEntry> routes;
    std::pair<uint32_t, uint32_t> parseCIDR(const std::string& cidr);
    uint32_t stringToIP(const std::string& ip_str);
    std::string ipToString(uint32_t ip);
};
