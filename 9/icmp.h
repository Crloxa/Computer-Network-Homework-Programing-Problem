// icmp.h
#ifndef ICMP_H
#define ICMP_H

#include <iostream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

using namespace std;

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

struct ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
};

typedef struct {
    unsigned char ip_hl;
    unsigned char ip_tos;
    unsigned short ip_len;
    unsigned short ip_id;
    unsigned short ip_offset;
    unsigned char ip_ttl;
    unsigned char ip_protocol;
    unsigned short ip_checksum;
    unsigned int ip_src;
    unsigned int ip_dst;
} IP;

uint16_t checksum(void* data, int len) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(uint8_t*)ptr;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (uint16_t)(~sum);
}

#endif