// ping_client.cpp
#include "icmp.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: ping_client <ip>\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    char sendbuf[64];
    char recvbuf[1024];

    ICMPHeader* icmp = (ICMPHeader*)sendbuf;

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->id = htons(1234);
    icmp->seq = htons(1);

    memset(sendbuf + sizeof(ICMPHeader), 'A', 32);

    icmp->checksum = 0;
    icmp->checksum = checksum(sendbuf, sizeof(ICMPHeader) + 32);

    for (int i = 0; i < 4; i++) {
        icmp->seq = htons(i);

        sendto(sock, sendbuf, sizeof(ICMPHeader) + 32, 0,
               (sockaddr*)&addr, sizeof(addr));

        sockaddr_in from{};
        socklen_t len = sizeof(from);

        int n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                         (sockaddr*)&from, &len);

        if (n > 0) {
            IP* ip = (IP*)recvbuf;
            int iphdr_len = ip->ip_hl << 2;

            ICMPHeader* reply = (ICMPHeader*)(recvbuf + iphdr_len);

            if (reply->type == ICMP_ECHOREPLY) {
                cout << "Reply from "
                     << inet_ntoa(from.sin_addr)
                     << " seq=" << ntohs(reply->seq)
                     << endl;
            }
        }

#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}