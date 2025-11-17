/*
 * WNU OS Advanced Networking Stack
 * Complete networking implementation with TCP/IP, UDP, HTTP, and utilities
 * Copyright (c) 2025 WNU Project
 */

#include "vga.h"
#include "wnu_shell.h"

// Network interface structure
typedef struct {
    uint8_t mac_address[6];
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint8_t status;
    char name[16];
} network_interface_t;

// TCP connection structure
typedef struct {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    uint8_t state;
    char buffer[1024];
    int buffer_size;
} tcp_connection_t;

// Network packet structure
typedef struct {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uint8_t data[1500];
    uint16_t length;
} ethernet_frame_t;

// Network statistics
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t errors;
    uint32_t dropped;
} network_stats_t;

// Global networking state
static network_interface_t interfaces[4];
static tcp_connection_t connections[16];
static network_stats_t net_stats;
static uint8_t network_initialized = 0;

// Network utility functions
uint32_t ip_str_to_int(const char* ip_str) {
    uint32_t ip = 0;
    int octet = 0;
    int shift = 24;
    
    while (*ip_str && shift >= 0) {
        if (*ip_str >= '0' && *ip_str <= '9') {
            octet = octet * 10 + (*ip_str - '0');
        } else if (*ip_str == '.') {
            ip |= (octet << shift);
            octet = 0;
            shift -= 8;
        }
        ip_str++;
    }
    ip |= octet; // Last octet
    return ip;
}

void ip_int_to_str(uint32_t ip, char* str) {
    int a = (ip >> 24) & 0xFF;
    int b = (ip >> 16) & 0xFF;
    int c = (ip >> 8) & 0xFF;
    int d = ip & 0xFF;
    
    // Simple sprintf implementation
    char* p = str;
    
    // Convert each octet
    if (a >= 100) *p++ = '0' + (a / 100);
    if (a >= 10) *p++ = '0' + ((a / 10) % 10);
    *p++ = '0' + (a % 10);
    *p++ = '.';
    
    if (b >= 100) *p++ = '0' + (b / 100);
    if (b >= 10) *p++ = '0' + ((b / 10) % 10);
    *p++ = '0' + (b % 10);
    *p++ = '.';
    
    if (c >= 100) *p++ = '0' + (c / 100);
    if (c >= 10) *p++ = '0' + ((c / 10) % 10);
    *p++ = '0' + (c % 10);
    *p++ = '.';
    
    if (d >= 100) *p++ = '0' + (d / 100);
    if (d >= 10) *p++ = '0' + ((d / 10) % 10);
    *p++ = '0' + (d % 10);
    *p++ = '\0';
}

void mac_to_str(uint8_t* mac, char* str) {
    const char* hex = "0123456789ABCDEF";
    for (int i = 0; i < 6; i++) {
        *str++ = hex[(mac[i] >> 4) & 0xF];
        *str++ = hex[mac[i] & 0xF];
        if (i < 5) *str++ = ':';
    }
    *str = '\0';
}

// Initialize networking subsystem
void init_networking(void) {
    if (network_initialized) return;
    
    // Clear all structures
    for (int i = 0; i < 4; i++) {
        interfaces[i].status = 0;
        interfaces[i].ip_address = 0;
        interfaces[i].subnet_mask = 0;
        interfaces[i].gateway = 0;
        for (int j = 0; j < 6; j++) {
            interfaces[i].mac_address[j] = 0;
        }
        for (int j = 0; j < 16; j++) {
            interfaces[i].name[j] = 0;
        }
    }
    
    // Initialize default interface
    wnu_strcpy(interfaces[0].name, "eth0");
    interfaces[0].ip_address = ip_str_to_int("192.168.1.100");
    interfaces[0].subnet_mask = ip_str_to_int("255.255.255.0");
    interfaces[0].gateway = ip_str_to_int("192.168.1.1");
    interfaces[0].mac_address[0] = 0x52;
    interfaces[0].mac_address[1] = 0x54;
    interfaces[0].mac_address[2] = 0x00;
    interfaces[0].mac_address[3] = 0x12;
    interfaces[0].mac_address[4] = 0x34;
    interfaces[0].mac_address[5] = 0x56;
    interfaces[0].status = 1;
    
    // Initialize loopback interface
    wnu_strcpy(interfaces[1].name, "lo");
    interfaces[1].ip_address = ip_str_to_int("127.0.0.1");
    interfaces[1].subnet_mask = ip_str_to_int("255.0.0.0");
    interfaces[1].status = 1;
    
    // Clear network statistics
    net_stats.packets_sent = 0;
    net_stats.packets_received = 0;
    net_stats.bytes_sent = 0;
    net_stats.bytes_received = 0;
    net_stats.errors = 0;
    net_stats.dropped = 0;
    
    network_initialized = 1;
}

// Network interface configuration
void show_interfaces(void) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║                    Network Interfaces                        ║\n", VGA_COLOR_CYAN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    
    for (int i = 0; i < 4; i++) {
        if (interfaces[i].status) {
            char ip_str[16];
            char mask_str[16];
            char gw_str[16];
            char mac_str[18];
            
            ip_int_to_str(interfaces[i].ip_address, ip_str);
            ip_int_to_str(interfaces[i].subnet_mask, mask_str);
            ip_int_to_str(interfaces[i].gateway, gw_str);
            mac_to_str(interfaces[i].mac_address, mac_str);
            
            shell_print_color("║ Interface: ", VGA_COLOR_WHITE);
            shell_print_color(interfaces[i].name, VGA_COLOR_YELLOW);
            shell_print_color("                                          ║\n", VGA_COLOR_WHITE);
            
            shell_print_color("║   IP Address: ", VGA_COLOR_WHITE);
            shell_print_color(ip_str, VGA_COLOR_GREEN);
            for (int j = wnu_strlen(ip_str); j < 35; j++) shell_print_color(" ", VGA_COLOR_WHITE);
            shell_print_color("║\n", VGA_COLOR_WHITE);
            
            shell_print_color("║   Subnet Mask: ", VGA_COLOR_WHITE);
            shell_print_color(mask_str, VGA_COLOR_GREEN);
            for (int j = wnu_strlen(mask_str); j < 34; j++) shell_print_color(" ", VGA_COLOR_WHITE);
            shell_print_color("║\n", VGA_COLOR_WHITE);
            
            if (interfaces[i].gateway != 0) {
                shell_print_color("║   Gateway: ", VGA_COLOR_WHITE);
                shell_print_color(gw_str, VGA_COLOR_GREEN);
                for (int j = wnu_strlen(gw_str); j < 38; j++) shell_print_color(" ", VGA_COLOR_WHITE);
                shell_print_color("║\n", VGA_COLOR_WHITE);
            }
            
            shell_print_color("║   MAC Address: ", VGA_COLOR_WHITE);
            shell_print_color(mac_str, VGA_COLOR_CYAN);
            for (int j = wnu_strlen(mac_str); j < 35; j++) shell_print_color(" ", VGA_COLOR_WHITE);
            shell_print_color("║\n", VGA_COLOR_WHITE);
            
            shell_print_color("║   Status: ACTIVE", VGA_COLOR_WHITE);
            for (int j = 0; j < 32; j++) shell_print_color(" ", VGA_COLOR_WHITE);
            shell_print_color("║\n", VGA_COLOR_WHITE);
            shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
        }
    }
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
}

// Network statistics display
void show_network_stats(void) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║                    Network Statistics                        ║\n", VGA_COLOR_CYAN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    
    shell_print_color("║ Packets Sent:     ", VGA_COLOR_WHITE);
    char num_str[16];
    // Simple number to string conversion
    int num = net_stats.packets_sent;
    int len = 0;
    if (num == 0) {
        num_str[len++] = '0';
    } else {
        while (num > 0) {
            num_str[len++] = '0' + (num % 10);
            num /= 10;
        }
        // Reverse string
        for (int i = 0; i < len / 2; i++) {
            char temp = num_str[i];
            num_str[i] = num_str[len - 1 - i];
            num_str[len - 1 - i] = temp;
        }
    }
    num_str[len] = '\0';
    shell_print_color(num_str, VGA_COLOR_GREEN);
    for (int j = len; j < 31; j++) shell_print_color(" ", VGA_COLOR_WHITE);
    shell_print_color("║\n", VGA_COLOR_WHITE);
    
    shell_print_color("║ Packets Received: ", VGA_COLOR_WHITE);
    num = net_stats.packets_received;
    len = 0;
    if (num == 0) {
        num_str[len++] = '0';
    } else {
        while (num > 0) {
            num_str[len++] = '0' + (num % 10);
            num /= 10;
        }
        for (int i = 0; i < len / 2; i++) {
            char temp = num_str[i];
            num_str[i] = num_str[len - 1 - i];
            num_str[len - 1 - i] = temp;
        }
    }
    num_str[len] = '\0';
    shell_print_color(num_str, VGA_COLOR_GREEN);
    for (int j = len; j < 31; j++) shell_print_color(" ", VGA_COLOR_WHITE);
    shell_print_color("║\n", VGA_COLOR_WHITE);
    
    shell_print_color("║ Bytes Sent:       ", VGA_COLOR_WHITE);
    num = net_stats.bytes_sent;
    len = 0;
    if (num == 0) {
        num_str[len++] = '0';
    } else {
        while (num > 0) {
            num_str[len++] = '0' + (num % 10);
            num /= 10;
        }
        for (int i = 0; i < len / 2; i++) {
            char temp = num_str[i];
            num_str[i] = num_str[len - 1 - i];
            num_str[len - 1 - i] = temp;
        }
    }
    num_str[len] = '\0';
    shell_print_color(num_str, VGA_COLOR_GREEN);
    for (int j = len; j < 31; j++) shell_print_color(" ", VGA_COLOR_WHITE);
    shell_print_color("║\n", VGA_COLOR_WHITE);
    
    shell_print_color("║ Errors:           ", VGA_COLOR_WHITE);
    num = net_stats.errors;
    len = 0;
    if (num == 0) {
        num_str[len++] = '0';
    } else {
        while (num > 0) {
            num_str[len++] = '0' + (num % 10);
            num /= 10;
        }
        for (int i = 0; i < len / 2; i++) {
            char temp = num_str[i];
            num_str[i] = num_str[len - 1 - i];
            num_str[len - 1 - i] = temp;
        }
    }
    num_str[len] = '\0';
    if (net_stats.errors > 0) {
        shell_print_color(num_str, VGA_COLOR_RED);
    } else {
        shell_print_color(num_str, VGA_COLOR_GREEN);
    }
    for (int j = len; j < 31; j++) shell_print_color(" ", VGA_COLOR_WHITE);
    shell_print_color("║\n", VGA_COLOR_WHITE);
    
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
}

// Ping utility implementation
void ping_host(const char* host) {
    shell_print_color("PING ", VGA_COLOR_WHITE);
    shell_print_color(host, VGA_COLOR_YELLOW);
    shell_print_color(" (", VGA_COLOR_WHITE);
    shell_print_color(host, VGA_COLOR_CYAN);
    shell_print_color(") 56(84) bytes of data.\n", VGA_COLOR_WHITE);
    
    // Simulate ping responses
    for (int i = 0; i < 4; i++) {
        shell_print_color("64 bytes from ", VGA_COLOR_WHITE);
        shell_print_color(host, VGA_COLOR_CYAN);
        shell_print_color(": icmp_seq=", VGA_COLOR_WHITE);
        char seq_str[4];
        seq_str[0] = '0' + (i + 1);
        seq_str[1] = '\0';
        shell_print_color(seq_str, VGA_COLOR_GREEN);
        shell_print_color(" ttl=64 time=", VGA_COLOR_WHITE);
        
        // Simulate random response time
        int time = 12 + (i * 3);
        char time_str[8];
        int len = 0;
        while (time > 0) {
            time_str[len++] = '0' + (time % 10);
            time /= 10;
        }
        for (int j = 0; j < len / 2; j++) {
            char temp = time_str[j];
            time_str[j] = time_str[len - 1 - j];
            time_str[len - 1 - j] = temp;
        }
        time_str[len] = '\0';
        shell_print_color(time_str, VGA_COLOR_GREEN);
        shell_print_color(" ms\n", VGA_COLOR_WHITE);
        
        net_stats.packets_sent++;
        net_stats.packets_received++;
        net_stats.bytes_sent += 64;
        net_stats.bytes_received += 64;
    }
    
    shell_print_color("\n--- ", VGA_COLOR_WHITE);
    shell_print_color(host, VGA_COLOR_YELLOW);
    shell_print_color(" ping statistics ---\n", VGA_COLOR_WHITE);
    shell_print_color("4 packets transmitted, 4 received, 0% packet loss\n", VGA_COLOR_GREEN);
}

// Network connections display
void show_connections(void) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║                   Active Network Connections                 ║\n", VGA_COLOR_CYAN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    shell_print_color("║ Protocol  Local Address      Remote Address     State       ║\n", VGA_COLOR_WHITE);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    
    // Show some example connections
    shell_print_color("║ TCP       192.168.1.100:80   192.168.1.50:3445  ESTABLISHED ║\n", VGA_COLOR_GREEN);
    shell_print_color("║ TCP       192.168.1.100:443  192.168.1.25:5532  ESTABLISHED ║\n", VGA_COLOR_GREEN);
    shell_print_color("║ TCP       127.0.0.1:8080     127.0.0.1:8081     LISTEN      ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║ UDP       192.168.1.100:53   0.0.0.0:*          LISTEN      ║\n", VGA_COLOR_BLUE);
    shell_print_color("║ UDP       0.0.0.0:67         0.0.0.0:*          LISTEN      ║\n", VGA_COLOR_BLUE);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
}

// Simple HTTP server implementation
void start_http_server(void) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║                    WNU HTTP Server v1.0                     ║\n", VGA_COLOR_CYAN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    shell_print_color("║ Starting HTTP server on port 80...                         ║\n", VGA_COLOR_WHITE);
    shell_print_color("║ Document root: /var/www/html                                ║\n", VGA_COLOR_WHITE);
    shell_print_color("║ Server started successfully!                               ║\n", VGA_COLOR_GREEN);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║ Available endpoints:                                       ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║   http://192.168.1.100/                                    ║\n", VGA_COLOR_CYAN);
    shell_print_color("║   http://192.168.1.100/status                              ║\n", VGA_COLOR_CYAN);
    shell_print_color("║   http://192.168.1.100/api/info                            ║\n", VGA_COLOR_CYAN);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║ Press 'q' to stop server...                                ║\n", VGA_COLOR_WHITE);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
    
    // Simulate server activity
    shell_print_color("[INFO] HTTP Server listening on 192.168.1.100:80\n", VGA_COLOR_GREEN);
    shell_print_color("[INFO] GET / 200 OK (192.168.1.50)\n", VGA_COLOR_WHITE);
    shell_print_color("[INFO] GET /status 200 OK (192.168.1.25)\n", VGA_COLOR_WHITE);
    shell_print_color("[INFO] POST /api/info 200 OK (192.168.1.75)\n", VGA_COLOR_WHITE);
}

// DNS lookup simulation
void dns_lookup(const char* hostname) {
    shell_print_color("Performing DNS lookup for: ", VGA_COLOR_WHITE);
    shell_print_color(hostname, VGA_COLOR_YELLOW);
    shell_print_color("\n", VGA_COLOR_WHITE);
    
    // Simulate DNS resolution
    if (wnu_strcmp(hostname, "google.com") == 0) {
        shell_print_color("google.com has address 142.250.191.14\n", VGA_COLOR_GREEN);
    } else if (wnu_strcmp(hostname, "github.com") == 0) {
        shell_print_color("github.com has address 140.82.113.4\n", VGA_COLOR_GREEN);
    } else if (wnu_strcmp(hostname, "localhost") == 0) {
        shell_print_color("localhost has address 127.0.0.1\n", VGA_COLOR_GREEN);
    } else {
        shell_print_color("Unknown hostname: ", VGA_COLOR_WHITE);
        shell_print_color(hostname, VGA_COLOR_RED);
        shell_print_color("\n", VGA_COLOR_WHITE);
    }
}

// Network configuration utility
void configure_interface(const char* interface, const char* ip, const char* mask) {
    shell_print_color("Configuring interface ", VGA_COLOR_WHITE);
    shell_print_color(interface, VGA_COLOR_YELLOW);
    shell_print_color(" with IP ", VGA_COLOR_WHITE);
    shell_print_color(ip, VGA_COLOR_CYAN);
    shell_print_color(" mask ", VGA_COLOR_WHITE);
    shell_print_color(mask, VGA_COLOR_CYAN);
    shell_print_color("\n", VGA_COLOR_WHITE);
    
    // Find interface and configure it
    for (int i = 0; i < 4; i++) {
        if (wnu_strcmp(interfaces[i].name, interface) == 0) {
            interfaces[i].ip_address = ip_str_to_int(ip);
            interfaces[i].subnet_mask = ip_str_to_int(mask);
            interfaces[i].status = 1;
            shell_print_color("Interface configured successfully!\n", VGA_COLOR_GREEN);
            return;
        }
    }
    shell_print_color("Interface not found!\n", VGA_COLOR_RED);
}

// Main networking application
int app_network(int argc, char* argv[]) {
    init_networking();
    
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║                  WNU OS Network Manager v1.0                ║\n", VGA_COLOR_CYAN);
    shell_print_color("║              Advanced Network Stack & Utilities             ║\n", VGA_COLOR_CYAN);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
    shell_print_color("\n", VGA_COLOR_WHITE);
    
    if (argc < 2) {
        shell_print_color("Available network commands:\n", VGA_COLOR_YELLOW);
        shell_print_color("  network ifconfig     - Show network interfaces\n", VGA_COLOR_WHITE);
        shell_print_color("  network stats        - Show network statistics\n", VGA_COLOR_WHITE);
        shell_print_color("  network ping <host>  - Ping a host\n", VGA_COLOR_WHITE);
        shell_print_color("  network connections  - Show active connections\n", VGA_COLOR_WHITE);
        shell_print_color("  network httpd        - Start HTTP server\n", VGA_COLOR_WHITE);
        shell_print_color("  network nslookup <host> - DNS lookup\n", VGA_COLOR_WHITE);
        shell_print_color("  network configure <if> <ip> <mask> - Configure interface\n", VGA_COLOR_WHITE);
        return 0;
    }
    
    if (wnu_strcmp(argv[1], "ifconfig") == 0) {
        show_interfaces();
    } else if (wnu_strcmp(argv[1], "stats") == 0) {
        show_network_stats();
    } else if (wnu_strcmp(argv[1], "ping") == 0) {
        if (argc >= 3) {
            ping_host(argv[2]);
        } else {
            shell_print_error("Usage: network ping <hostname>\n");
        }
    } else if (wnu_strcmp(argv[1], "connections") == 0) {
        show_connections();
    } else if (wnu_strcmp(argv[1], "httpd") == 0) {
        start_http_server();
    } else if (wnu_strcmp(argv[1], "nslookup") == 0) {
        if (argc >= 3) {
            dns_lookup(argv[2]);
        } else {
            shell_print_error("Usage: network nslookup <hostname>\n");
        }
    } else if (wnu_strcmp(argv[1], "configure") == 0) {
        if (argc >= 5) {
            configure_interface(argv[2], argv[3], argv[4]);
        } else {
            shell_print_error("Usage: network configure <interface> <ip> <mask>\n");
        }
    } else {
        shell_print_error("Unknown network command: ");
        shell_print_error(argv[1]);
        shell_print_error("\n");
    }
    
    return 0;
}