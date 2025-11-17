/*
 * WNU OS Networking Header
 * Network stack definitions and function declarations
 * Copyright (c) 2025 WNU Project
 */

#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdint.h>

// Network protocol definitions
#define ETH_TYPE_IP     0x0800
#define ETH_TYPE_ARP    0x0806
#define ETH_TYPE_RARP   0x8035

#define IP_PROTO_ICMP   1
#define IP_PROTO_TCP    6
#define IP_PROTO_UDP    17

// Network interface states
#define IF_STATE_DOWN   0
#define IF_STATE_UP     1
#define IF_STATE_ERROR  2

// TCP connection states
#define TCP_CLOSED      0
#define TCP_LISTEN      1
#define TCP_SYN_SENT    2
#define TCP_SYN_RCVD    3
#define TCP_ESTABLISHED 4
#define TCP_FIN_WAIT1   5
#define TCP_FIN_WAIT2   6
#define TCP_CLOSE_WAIT  7
#define TCP_CLOSING     8
#define TCP_LAST_ACK    9
#define TCP_TIME_WAIT   10

// Network utility functions
uint32_t ip_str_to_int(const char* ip_str);
void ip_int_to_str(uint32_t ip, char* str);
void mac_to_str(uint8_t* mac, char* str);

// Network subsystem functions
void init_networking(void);
void show_interfaces(void);
void show_network_stats(void);
void ping_host(const char* host);
void show_connections(void);
void start_http_server(void);
void dns_lookup(const char* hostname);
void configure_interface(const char* interface, const char* ip, const char* mask);

// Main application entry point
int app_network(int argc, char* argv[]);

#endif // NETWORKING_H