#include "common.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <output_file> [port]\n", argv[0]);
        return 1;
    }

    const char* output_file = argv[1];
    int port = (argc >= 3) ? atoi(argv[2]) : P1_PORT;

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
        fprintf(stderr, "socket create failed\n");
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons((unsigned short)port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed\n");
        close_socket(sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(P1_MCAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
        fprintf(stderr, "join multicast group failed\n");
        close_socket(sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    unsigned char packet[sizeof(PacketHeader) + P1_CHUNK_SIZE];
    double start = now_seconds();
    size_t total_bytes = 0;
    uint32_t expect_seq = 0;

    while (1) {
        int n = recv(sock, (char*)packet, sizeof(packet), 0);
        if (n < (int)sizeof(PacketHeader)) {
            continue;
        }

        PacketHeader* header = (PacketHeader*)packet;
        uint32_t magic = ntohl(header->magic);
        uint32_t seq = ntohl(header->seq);
        uint32_t data_len = ntohl(header->data_len);

        if (magic == P1_MAGIC_END) {
            break;
        }
        if (magic != P1_MAGIC_DATA) {
            continue;
        }
        if (data_len > P1_CHUNK_SIZE || (int)(sizeof(PacketHeader) + data_len) > n) {
            continue;
        }

        if (seq != expect_seq) {
            printf("[P1 Receiver] warn: expect seq=%u, got=%u\n", expect_seq, seq);
            expect_seq = seq;
        }

        fwrite(packet + sizeof(PacketHeader), 1, data_len, fp);
        total_bytes += data_len;
        expect_seq++;
    }

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }

    printf("[P1 Receiver] Received %.2f MB in %.3f s, speed=%.2f MB/s\n",
           (double)total_bytes / (1024.0 * 1024.0), elapsed,
           ((double)total_bytes / (1024.0 * 1024.0)) / elapsed);

    fclose(fp);
    close_socket(sock);
    socket_cleanup();
    return 0;
}
