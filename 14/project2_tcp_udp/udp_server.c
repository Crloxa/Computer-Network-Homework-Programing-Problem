#include "common.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <output_file> [port]\n", argv[0]);
        return 1;
    }

    const char* output_file = argv[1];
    int port = (argc >= 3) ? atoi(argv[2]) : P2_DEFAULT_PORT;

    FILE* fp = fopen(output_file, "wb");
    if (!fp) {
        perror("fopen output");
        return 1;
    }

    if (socket_init() != 0) {
        fclose(fp);
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket failed\n");
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed\n");
        close_socket(sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    unsigned char packet[sizeof(UdpHeader) + P2_CHUNK_SIZE];
    size_t total_bytes = 0;
    uint32_t expect_seq = 0;
    double start = now_seconds();

    while (1) {
        int n = recv(sock, (char*)packet, sizeof(packet), 0);
        if (n < (int)sizeof(UdpHeader)) {
            continue;
        }

        UdpHeader* h = (UdpHeader*)packet;
        uint32_t magic = ntohl(h->magic);
        uint32_t seq = ntohl(h->seq);
        uint32_t len = ntohl(h->data_len);

        if (magic == P2_UDP_MAGIC_END) {
            break;
        }
        if (magic != P2_UDP_MAGIC_DATA) {
            continue;
        }
        if (len > P2_CHUNK_SIZE || (int)(sizeof(UdpHeader) + len) > n) {
            continue;
        }

        if (seq != expect_seq) {
            printf("[P2 UDP Server] warn: expect seq=%u, got=%u\n", expect_seq, seq);
            expect_seq = seq;
        }

        fwrite(packet + sizeof(UdpHeader), 1, len, fp);
        total_bytes += len;
        expect_seq++;
    }

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }

    printf("[P2 UDP Server] Received %.2f MB in %.3f s, speed=%.2f MB/s\n",
           (double)total_bytes / (1024.0 * 1024.0), elapsed,
           ((double)total_bytes / (1024.0 * 1024.0)) / elapsed);

    close_socket(sock);
    fclose(fp);
    socket_cleanup();
    return 0;
}
