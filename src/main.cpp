#include "internet_protocol.hpp"
#include "packet_builders.hpp"
#include <queue>

/*
TODO:
5. Add readme file explaining what is made.
6. ??? Add NAT functionality.
*/

void addPacketIfValid(std::queue<std::vector<uint8_t>>& packet_queue,
                      const std::vector<uint8_t>& packet,
                      const std::string& description) {
    if (!packet.empty()) {
        packet_queue.push(packet);
        log_debug("Queued packet: %s", description.c_str());
    } else {
        log_warning("Skipping invalid packet: %s", description.c_str());
    }
}

int main() {
    Logger::getInstance().init("routing_debug.log", LogLevel::DEBUG);

    InternetProtocol ip;
    ip.initRoutingTable();

    std::cout << "=== Routing Simulation ===\n";
    std::queue<std::vector<uint8_t>> packet_queue;

    /* Packet 1:
       simulating ICMP ping packet sending from one device to another on the same WiFi network
    */
    ICMPPacketBuilder local_ping;
    local_ping.ipv4_src_ip = "192.168.1.100";
    local_ping.ipv4_dst_ip = "192.168.1.50";
    local_ping.ipv4_ttl = 64;
    local_ping.icmp_type = ICMP_ECHO_REQUEST;
    local_ping.icmp_id = 1234;
    local_ping.icmp_seq = 1;
    local_ping.icmp_payload = "ping";
    addPacketIfValid(packet_queue, local_ping.build(),
                     "Local WiFi ping: " + local_ping.ipv4_src_ip + " -> " + local_ping.ipv4_dst_ip);

    /* Packet 2:
       simulating a DNS query being sent to Google's public DNS server
    */
    UDPPacketBuilder google_dns;
    google_dns.ipv4_src_ip = "192.168.1.100";
    google_dns.ipv4_dst_ip = "8.8.8.8";
    google_dns.ipv4_ttl = 64;
    google_dns.udp_src_port = 54321;
    google_dns.udp_dst_port = 53;
    google_dns.udp_payload = "DNS_QUERY_google.com_A";
    addPacketIfValid(packet_queue, google_dns.build(),
                     "Google DNS query: " + google_dns.ipv4_src_ip + " -> " + google_dns.ipv4_dst_ip);

    /* Packet 3:
       simulating a DNS query being sent to Cloudflare's public DNS server
    */
    UDPPacketBuilder cloudflare_dns;
    cloudflare_dns.ipv4_src_ip = "192.168.1.100";
    cloudflare_dns.ipv4_dst_ip = "1.1.1.1";
    cloudflare_dns.ipv4_ttl = 64;
    cloudflare_dns.udp_src_port = 54322;
    cloudflare_dns.udp_dst_port = 53;
    cloudflare_dns.udp_payload = "DNS_QUERY_cloudflare.com_A";
    addPacketIfValid(packet_queue, cloudflare_dns.build(),
                     "Cloudflare DNS query: " + cloudflare_dns.ipv4_src_ip + " -> " + cloudflare_dns.ipv4_dst_ip);

    /* Packet 4:
       simulating a TCP connection to localhost (Loopback interface)
    */
    TCPPacketBuilder localhost;
    localhost.ipv4_src_ip = "127.0.0.1";
    localhost.ipv4_dst_ip = "127.0.0.1";
    localhost.ipv4_ttl = 64;
    localhost.tcp_src_port = 45678;
    localhost.tcp_dst_port = 8080;
    localhost.tcp_flags = TCP_SYN;
    localhost.tcp_payload = "";
    addPacketIfValid(packet_queue, localhost.build(),
                     "Localhost connection: " + localhost.ipv4_src_ip + " -> " + localhost.ipv4_dst_ip);

    /* Packet 5:
       simulating a HTTPS connection to YouTube (Default route via router)
    */
    TCPPacketBuilder youtube;
    youtube.ipv4_src_ip = "192.168.1.100";
    youtube.ipv4_dst_ip = "142.250.184.78";
    youtube.ipv4_ttl = 64;
    youtube.tcp_src_port = 33445;
    youtube.tcp_dst_port = 443;
    youtube.tcp_flags = TCP_SYN;
    youtube.tcp_payload = "";
    addPacketIfValid(packet_queue, youtube.build(),
                     "YouTube HTTPS: " + youtube.ipv4_src_ip + " -> " + youtube.ipv4_dst_ip);

    /* Packet 6:
       simulating a UDP packet with expired TTL (should be dropped)
    */
    UDPPacketBuilder expired_packet;
    expired_packet.ipv4_src_ip = "192.168.1.100";
    expired_packet.ipv4_dst_ip = "8.8.8.8";
    expired_packet.ipv4_ttl = 0;
    expired_packet.udp_src_port = 12345;
    expired_packet.udp_dst_port = 53;
    expired_packet.udp_payload = "expired_query";
    addPacketIfValid(packet_queue, expired_packet.build(),
                     "Expired packet: " + expired_packet.ipv4_src_ip + " -> " + expired_packet.ipv4_dst_ip + " (TTL=0)");

    size_t packet_count = 0;
    while (!packet_queue.empty()) {
        packet_count++;
        std::cout << "\n--- Processing Packet " << packet_count << " ---\n";
        ip.parsePacket(packet_queue.front());
        packet_queue.pop();
    }

    ip.printRoutingTable();
    log_info("Routing simulation completed");
    return 0;
}
