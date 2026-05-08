#ifndef P1_COMMON_H
#define P1_COMMON_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
typedef int SOCKET;
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define P1_MCAST_ADDR "239.0.0.1"
#define P1_PORT 9000
#define P1_CHUNK_SIZE 1400
#define P1_MAGIC_DATA 0xABCD1234u
#define P1_MAGIC_END 0xDCBA4321u

typedef struct PacketHeader {
    uint32_t magic;
    uint32_t seq;
    uint32_t data_len;
} PacketHeader;

static int socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }
#endif
    return 0;
}

static void socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static void close_socket(SOCKET s) {
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

static double now_seconds(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

#endif
