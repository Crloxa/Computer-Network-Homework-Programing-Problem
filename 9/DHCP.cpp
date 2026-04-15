#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#endif
#include <WS2tcpip.h>

using namespace std;

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_ACK      5

#define BUFFER_SIZE 1024

// 固定分配 IP
const char* FIXED_IP = "192.168.1.2";
const char* SERVER_IP = "192.168.1.1";

// DHCP 报文结构（简化）
#pragma pack(push, 1)
struct DHCPPacket {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    char sname[64];
    char file[128];
    uint8_t options[312];
};
#pragma pack(pop)

// 获取 DHCP 消息类型
int get_dhcp_type(uint8_t* options) {
    int i = 4; // 跳过 magic cookie
    while (i < 312) {
        if (options[i] == 0xff) break;
        if (options[i] == 53) {
            return options[i + 2];
        }
        i += options[i + 1] + 2;
    }
    return -1;
}

// 构造 DHCP OFFER / ACK
void build_response(DHCPPacket& resp, DHCPPacket& req, int msg_type) {
    memset(&resp, 0, sizeof(resp));

    resp.op = 2; // BOOTREPLY
    resp.htype = 1;
    resp.hlen = 6;
    resp.xid = req.xid;

    memcpy(resp.chaddr, req.chaddr, 16);

    resp.yiaddr = inet_addr(FIXED_IP);
    resp.siaddr = inet_addr(SERVER_IP);

    uint8_t* opt = resp.options;

    // magic cookie
    opt[0] = 0x63; opt[1] = 0x82; opt[2] = 0x53; opt[3] = 0x63;

    int idx = 4;

    // DHCP Message Type
    opt[idx++] = 53;
    opt[idx++] = 1;
    opt[idx++] = msg_type;

    // Subnet Mask
    opt[idx++] = 1;
    opt[idx++] = 4;
    *(uint32_t*)(opt + idx) = inet_addr("255.255.255.0");
    idx += 4;

    // Router
    opt[idx++] = 3;
    opt[idx++] = 4;
    *(uint32_t*)(opt + idx) = inet_addr(SERVER_IP);
    idx += 4;

    // Lease Time
    opt[idx++] = 51;
    opt[idx++] = 4;
    *(uint32_t*)(opt + idx) = htonl(3600);
    idx += 4;

    // Server Identifier
    opt[idx++] = 54;
    opt[idx++] = 4;
    *(uint32_t*)(opt + idx) = inet_addr(SERVER_IP);
    idx += 4;

    opt[idx++] = 255; // end
}

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket error");
        return -1;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DHCP_SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    cout << "DHCP Server started..." << endl;

    while (true) {
        DHCPPacket req{}, resp{};
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);

        int n = recvfrom(sock, (char*)&req, sizeof(req), 0,
                         (sockaddr*)&client_addr, &len);

        if (n <= 0) continue;

        int type = get_dhcp_type(req.options);

        if (type == DHCP_DISCOVER) {
            cout << "Received DISCOVER" << endl;

            build_response(resp, req, DHCP_OFFER);

            sockaddr_in broadcast{};
            broadcast.sin_family = AF_INET;
            broadcast.sin_port = htons(DHCP_CLIENT_PORT);
            broadcast.sin_addr.s_addr = INADDR_BROADCAST;

            sendto(sock, (char*)&resp, sizeof(resp), 0,
                   (sockaddr*)&broadcast, sizeof(broadcast));

            cout << "Sent OFFER: " << FIXED_IP << endl;
        }

        else if (type == DHCP_REQUEST) {
            cout << "Received REQUEST" << endl;

            build_response(resp, req, DHCP_ACK);

            sockaddr_in broadcast{};
            broadcast.sin_family = AF_INET;
            broadcast.sin_port = htons(DHCP_CLIENT_PORT);
            broadcast.sin_addr.s_addr = INADDR_BROADCAST;

            sendto(sock, (char*)&resp, sizeof(resp), 0,
                   (sockaddr*)&broadcast, sizeof(broadcast));

            cout << "Sent ACK: " << FIXED_IP << endl;
        }
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}