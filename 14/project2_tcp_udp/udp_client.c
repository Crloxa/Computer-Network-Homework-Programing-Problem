#include "common.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <input_file> [port]\n", argv[0]);
        return 1;
    }

    const char* server_ip = argv[1];
    const char* input_file = argv[2];
    int port = (argc >= 4) ? atoi(argv[3]) : P2_DEFAULT_PORT;

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
        fprintf(stderr, "socket failed\n");
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    unsigned char packet[sizeof(UdpHeader) + P2_CHUNK_SIZE];
    UdpHeader* h = (UdpHeader*)packet;
    unsigned char* payload = packet + sizeof(UdpHeader);

    uint32_t seq = 0;
    size_t total_bytes = 0;
    double start = now_seconds();

    while (1) {
        size_t n = fread(payload, 1, P2_CHUNK_SIZE, fp);
        if (n == 0) {
            break;
        }

        h->magic = htonl(P2_UDP_MAGIC_DATA);
        h->seq = htonl(seq);
        h->data_len = htonl((uint32_t)n);

        int sent = sendto(sock, (const char*)packet, (int)(sizeof(UdpHeader) + n), 0,
                          (struct sockaddr*)&addr, sizeof(addr));
        if (sent == SOCKET_ERROR) {
            fprintf(stderr, "sendto failed at seq=%u\n", seq);
            break;
        }

        total_bytes += n;
        seq++;
    }

    h->magic = htonl(P2_UDP_MAGIC_END);
    h->seq = htonl(seq);
    h->data_len = htonl(0);
    (void)sendto(sock, (const char*)packet, (int)sizeof(UdpHeader), 0,
                 (struct sockaddr*)&addr, sizeof(addr));

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }

    printf("[P2 UDP Client] Sent %.2f MB in %.3f s, speed=%.2f MB/s, packets=%u\n",
           (double)total_bytes / (1024.0 * 1024.0), elapsed,
           ((double)total_bytes / (1024.0 * 1024.0)) / elapsed, seq);

    close_socket(sock);
    fclose(fp);
    socket_cleanup();
    return 0;
}
