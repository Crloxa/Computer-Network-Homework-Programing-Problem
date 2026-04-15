// ping_server.cpp
#include "icmp.h"

int main() {

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    char buffer[1024];

    while (true) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);

        int n = recvfrom(sock, buffer, sizeof(buffer), 0,
                         (sockaddr*)&client, &len);

        if (n <= 0) continue;

        IP* ip = (IP*)buffer;
        int iphdr_len = ip->ip_hl << 2;

        ICMPHeader* icmp = (ICMPHeader*)(buffer + iphdr_len);

        if (icmp->type == ICMP_ECHO) {

            cout << "Received ping from "
                 << inet_ntoa(client.sin_addr) << endl;

            icmp->type = ICMP_ECHOREPLY;
            icmp->checksum = 0;
            icmp->checksum = checksum(icmp, n - iphdr_len);

            sendto(sock, (char*)icmp, n - iphdr_len, 0,
                   (sockaddr*)&client, len);
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}