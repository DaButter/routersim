#include "routing_table.hpp"
#include <arpa/inet.h>
#include <algorithm>
#include <iomanip>

void RoutingTable::addRoute(const std::string& network_cidr, const std::string& interface,
                            const std::string& next_hop, int metric) {
    auto [network, mask] = parseCIDR(network_cidr);

    RouteEntry route;
    route.network = network;
    route.subnet_mask = mask;
    route.interface = interface;
    route.next_hop = next_hop.empty() ? 0 : stringToIP(next_hop);
    route.metric = metric;

    routes.push_back(route);

    /* sort by subnet mask (longest prefix first) for proper longest prefix matching.
       most specific routes come first */
    std::sort(routes.begin(), routes.end(),
              [](const RouteEntry& a, const RouteEntry& b) {
                  return a.subnet_mask > b.subnet_mask;
              });
}

std::string RoutingTable::lookupRoute(const uint32_t& dst_ip) {
    for (const auto& route : routes) {
        if ((dst_ip & route.subnet_mask) == route.network) {
            return route.interface;
        }
    }
    return "";
}

void RoutingTable::printTable() {
    std::cout << "\nRouting Table:\n";
    std::cout << std::left << std::setw(18) << "Network"
              << std::setw(16) << "Mask"
              << std::setw(10) << "Interface"
              << std::setw(16) << "Next Hop"
              << "Metric\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& route : routes) {
        std::cout << std::left 
                  << std::setw(18) << ipToString(route.network)
                  << std::setw(16) << ipToString(route.subnet_mask)
                  << std::setw(10) << route.interface
                  << std::setw(16) << (route.next_hop ? ipToString(route.next_hop) : "Direct")
                  << route.metric << "\n";
    }
    std::cout << "\n";
}

std::pair<uint32_t, uint32_t> RoutingTable::parseCIDR(const std::string& cidr) {
    size_t slash_pos = cidr.find('/');
    if (slash_pos == std::string::npos) {
        // no CIDR notation, assume /32 prefix
        uint32_t ip = stringToIP(cidr);
        return {ip, 0xFFFFFFFF};
    }

    std::string ip_str = cidr.substr(0, slash_pos);
    int prefix_len = std::stoi(cidr.substr(slash_pos + 1));

    uint32_t mask = (prefix_len == 0) ? 0 : (0xFFFFFFFF << (32 - prefix_len));
    uint32_t network = stringToIP(ip_str) & mask;

    return {network, mask};
}

// returns IP in HOST byte order
uint32_t RoutingTable::stringToIP(const std::string& ip_str) {
    struct in_addr addr;
    inet_aton(ip_str.c_str(), &addr);
    return ntohl(addr.s_addr);
}

// returns IP string in HOST byte order
std::string RoutingTable::ipToString(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    return std::string(inet_ntoa(addr));
}
