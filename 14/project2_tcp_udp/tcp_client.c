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

    fseek(fp, 0, SEEK_END);
    uint64_t file_size = (uint64_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (socket_init() != 0) {
        fclose(fp);
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "connect failed\n");
        close_socket(sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    if (send_all(sock, (const char*)&file_size, (int)sizeof(file_size)) != 0) {
        fprintf(stderr, "send file size failed\n");
        close_socket(sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    char buf[P2_CHUNK_SIZE];
    uint64_t sent_bytes = 0;
    double start = now_seconds();

    while (1) {
        size_t n = fread(buf, 1, P2_CHUNK_SIZE, fp);
        if (n == 0) {
            break;
        }
        if (send_all(sock, buf, (int)n) != 0) {
            fprintf(stderr, "send body failed\n");
            break;
        }
        sent_bytes += (uint64_t)n;
    }

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }

    printf("[P2 TCP Client] Sent %.2f MB in %.3f s, speed=%.2f MB/s\n",
           (double)sent_bytes / (1024.0 * 1024.0), elapsed,
           ((double)sent_bytes / (1024.0 * 1024.0)) / elapsed);

    close_socket(sock);
    fclose(fp);
    socket_cleanup();
    return 0;
}
