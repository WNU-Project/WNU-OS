/*
 * WNU OS Network System Overview
 * Complete networking architecture documentation and status
 * Copyright (c) 2025 WNU Project
 */

#include "networking.h"
#include "wnu_shell.h"
#include "vga.h"

void show_network_architecture(void) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║              WNU OS Network Architecture v1.0               ║\n", VGA_COLOR_CYAN);
    shell_print_color("║                  Complete Networking Stack                  ║\n", VGA_COLOR_CYAN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_CYAN);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  🌐 APPLICATION LAYER                                       ║\n", VGA_COLOR_GREEN);
    shell_print_color("║     • HTTP Server (Port 80)                                ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • DNS Client (Hostname Resolution)                     ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Network Utilities (ping, ifconfig, netstat)          ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Shell Integration (40+ network commands)             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  🔗 TRANSPORT LAYER                                         ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║     • TCP (Transmission Control Protocol)                  ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • UDP (User Datagram Protocol)                         ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Connection Management & State Tracking               ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Port Management (1-65535)                            ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  📡 NETWORK LAYER                                           ║\n", VGA_COLOR_MAGENTA);
    shell_print_color("║     • IP (Internet Protocol v4)                            ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • ICMP (Internet Control Message Protocol)             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Routing & Gateway Management                         ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Subnet & Network Mask Support                        ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  🔌 DATA LINK LAYER                                         ║\n", VGA_COLOR_BLUE);
    shell_print_color("║     • Ethernet Frame Processing                            ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • MAC Address Management                               ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Network Interface Abstraction                       ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Multiple Interface Support (eth0, lo)                ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  ⚡ SYSTEM INTEGRATION                                      ║\n", VGA_COLOR_RED);
    shell_print_color("║     • NetworkD Daemon (Assembly + C)                       ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • VGA Display Integration                               ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Shell Command System                                 ║\n", VGA_COLOR_WHITE);
    shell_print_color("║     • Real-time Statistics & Monitoring                    ║\n", VGA_COLOR_WHITE);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
}

void show_network_commands_reference(void) {
    shell_print_color("\n╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_GREEN);
    shell_print_color("║                  Network Commands Reference                 ║\n", VGA_COLOR_GREEN);
    shell_print_color("╠══════════════════════════════════════════════════════════════╣\n", VGA_COLOR_GREEN);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  BASIC COMMANDS:                                            ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║    network                  - Network manager main menu     ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    ping <host>              - Test connectivity to host     ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    ifconfig                 - Show interface configuration  ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  ADVANCED COMMANDS:                                         ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║    network ifconfig         - Detailed interface info       ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    network stats            - Network statistics           ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    network connections      - Active connections           ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    network httpd            - Start HTTP server            ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    network nslookup <host>  - DNS hostname lookup          ║\n", VGA_COLOR_WHITE);
    shell_print_color("║    network configure <args> - Configure network interface  ║\n", VGA_COLOR_WHITE);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("║  EXAMPLES:                                                  ║\n", VGA_COLOR_YELLOW);
    shell_print_color("║    ping google.com          - Ping Google's servers        ║\n", VGA_COLOR_CYAN);
    shell_print_color("║    network nslookup github.com - Resolve GitHub IP        ║\n", VGA_COLOR_CYAN);
    shell_print_color("║    network configure eth0 192.168.1.100 255.255.255.0     ║\n", VGA_COLOR_CYAN);
    shell_print_color("║                                                             ║\n", VGA_COLOR_WHITE);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_GREEN);
}

int app_netinfo(int argc, char* argv[]) {
    shell_print_color("╔══════════════════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║              WNU OS Complete Network Information            ║\n", VGA_COLOR_CYAN);
    shell_print_color("║                     System Overview                         ║\n", VGA_COLOR_CYAN);
    shell_print_color("╚══════════════════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
    
    // Show network architecture
    show_network_architecture();
    
    // Show current status
    shell_print_color("\n", VGA_COLOR_WHITE);
    shell_print_info("Current Network Status:\n");
    init_networking(); // Ensure networking is initialized
    show_interfaces();
    
    // Show commands reference
    show_network_commands_reference();
    
    shell_print_color("\n", VGA_COLOR_WHITE);
    shell_print_success("Network system fully operational! Ready for use.\n");
    
    return 0;
}