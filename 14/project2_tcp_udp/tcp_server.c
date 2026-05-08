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

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        fprintf(stderr, "socket failed\n");
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    int reuse = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed\n");
        close_socket(listen_sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    if (listen(listen_sock, 1) == SOCKET_ERROR) {
        fprintf(stderr, "listen failed\n");
        close_socket(listen_sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    printf("[P2 TCP Server] waiting on port %d...\n", port);
    SOCKET conn = accept(listen_sock, NULL, NULL);
    if (conn == INVALID_SOCKET) {
        fprintf(stderr, "accept failed\n");
        close_socket(listen_sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    uint64_t net_size = 0;
    if (recv_all(conn, (char*)&net_size, (int)sizeof(net_size)) != 0) {
        fprintf(stderr, "recv file size failed\n");
        close_socket(conn);
        close_socket(listen_sock);
        fclose(fp);
        socket_cleanup();
        return 1;
    }

    uint64_t file_size = net_size;
    char buf[P2_CHUNK_SIZE];
    uint64_t received = 0;
    double start = now_seconds();

    while (received < file_size) {
        int want = (int)((file_size - received) > P2_CHUNK_SIZE ? P2_CHUNK_SIZE : (file_size - received));
        int n = recv(conn, buf, want, 0);
        if (n <= 0) {
            fprintf(stderr, "recv body failed\n");
            break;
        }
        fwrite(buf, 1, n, fp);
        received += (uint64_t)n;
    }

    double elapsed = now_seconds() - start;
    if (elapsed <= 0.0) {
        elapsed = 1e-6;
    }

    printf("[P2 TCP Server] Received %.2f MB in %.3f s, speed=%.2f MB/s\n",
           (double)received / (1024.0 * 1024.0), elapsed,
           ((double)received / (1024.0 * 1024.0)) / elapsed);

    close_socket(conn);
    close_socket(listen_sock);
    fclose(fp);
    socket_cleanup();
    return 0;
}
