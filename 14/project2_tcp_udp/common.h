#ifndef P2_COMMON_H
#define P2_COMMON_H

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

#define P2_DEFAULT_PORT 9100
#define P2_CHUNK_SIZE 4096
#define P2_UDP_MAGIC_DATA 0x13572468u
#define P2_UDP_MAGIC_END 0x86427531u

typedef struct UdpHeader {
    uint32_t magic;
    uint32_t seq;
    uint32_t data_len;
} UdpHeader;

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

static int recv_all(SOCKET sock, char* buf, int total) {
    int got = 0;
    while (got < total) {
        int n = recv(sock, buf + got, total - got, 0);
        if (n <= 0) {
            return -1;
        }
        got += n;
    }
    return 0;
}

static int send_all(SOCKET sock, const char* buf, int total) {
    int sent = 0;
    while (sent < total) {
        int n = send(sock, buf + sent, total - sent, 0);
        if (n <= 0) {
            return -1;
        }
        sent += n;
    }
    return 0;
}

#endif
