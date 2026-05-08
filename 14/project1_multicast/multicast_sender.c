#include "common.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file> [port]\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    int port = (argc >= 3) ? atoi(argv[2]) : P1_PORT;

    FILE* fp = fopen(input_file, "rb");
    if (!fp) {
        perror("fopen input");
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

    int ttl = 1;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));

    struct sockaddr_in mcast_addr;
    memset(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_port = htons((unsigned short)port);
    mcast_addr.sin_addr.s_addr = inet_addr(P1_MCAST_ADDR);

    unsigned char packet[sizeof(PacketHeader) + P1_CHUNK_SIZE];
    PacketHeader* header = (PacketHeader*)packet;
    unsigned char* payload = packet + sizeof(PacketHeader);

    uint32_t seq = 0;
    size_t total_bytes = 0;
    double start = now_seconds();

    while (1) {
        size_t n = fread(payload, 1, P1_CHUNK_SIZE, fp);
        if (n == 0) {
            break;
        }

        header->magic = htonl(P1_MAGIC_DATA);
        header->seq = htonl(seq);
        header->data_len = htonl((uint32_t)n);

        int sent = sendto(sock, (const char*)packet, (int)(sizeof(PacketHeader) + n), 0,
                          (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
        if (sent == SOCKET_ERROR) {
            fprintf(stderr, "sendto failed at seq=%u\n", seq);
            break;
        }

        total_bytes += n;
        seq++;
    }

    header->magic = htonl(P1_MAGIC_END);
    header->seq = htonl(seq);
    header->data_len = htonl(0);
    (void)sendto(sock, (const char*)packet, (int)sizeof(PacketHeader), 0,
                 (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }
    printf("[P1 Sender] Sent %.2f MB in %.3f s, speed=%.2f MB/s, packets=%u\n",
           (double)total_bytes / (1024.0 * 1024.0), elapsed,
           ((double)total_bytes / (1024.0 * 1024.0)) / elapsed, seq);

    fclose(fp);
    close_socket(sock);
    socket_cleanup();
    return 0;
}
