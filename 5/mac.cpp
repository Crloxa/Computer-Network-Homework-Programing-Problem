#include <iostream>
#include <cstring>

#define MAC_ADDRESS_LENGTH 6
typedef unsigned char MAC_address[MAC_ADDRESS_LENGTH];

struct EthernetFrame {
    MAC_address dest_addr;
    MAC_address src_addr;
    unsigned char type[2];
    unsigned char data[1500];
    unsigned char CRC[4]; 
};


MAC_address this_mac_address = {0x11, 0x45, 0x14, 0x11, 0x45, 0x14};

int mac_address_match(const struct EthernetFrame *frame) {
    // 目的地址 T
    const unsigned char* T = frame->dest_addr;

    // T == 本网卡地址？
    if (memcmp(T, this_mac_address, MAC_ADDRESS_LENGTH) == 0) {
        return 1;
    }

    // T == 广播地址？
    if (T[0] == 0xFF && T[1] == 0xFF && T[2] == 0xFF && T[3] == 0xFF && T[4] == 0xFF && T[5] == 0xFF) {
        return 1;
    }

    // T == 多播地址？
    // 注意，接下来通常会有个多播逻辑的继续实现，检查自己是否在多播的组合中，但是没规则不好写
    if (T[0] & 0x01) {
        return 1;
    }

    // 以上均不匹配，丢弃
    return 0;
}

// 测试代码
int main() {
    EthernetFrame test_frame;
    
    // 本机地址
    memcpy(test_frame.dest_addr, this_mac_address, MAC_ADDRESS_LENGTH);
    std::cout << "Match Local: " << mac_address_match(&test_frame) << std::endl;

    // 广播地址
    memset(test_frame.dest_addr, 0xFF, MAC_ADDRESS_LENGTH);
    std::cout << "Match Broadcast: " << mac_address_match(&test_frame) << std::endl;

    // 多播地址 (例如 IP 多播 01:00:5E:...)
    MAC_address multicast = {0x01, 0x14, 0x51, 0x41, 0x14, 0x14};
    memcpy(test_frame.dest_addr, multicast, MAC_ADDRESS_LENGTH);
    std::cout << "Match Multicast: " << mac_address_match(&test_frame) << std::endl;

    // 错误的单播地址
    MAC_address wrong_mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    memcpy(test_frame.dest_addr, wrong_mac, MAC_ADDRESS_LENGTH);
    std::cout << "Match Wrong: " << mac_address_match(&test_frame) << std::endl;

    return 0;
}