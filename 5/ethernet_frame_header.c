#include <stdint.h>
struct ethernet_frame_header {
    uint8_t  dest_mac[6];   // 目的MAC地址
    uint8_t  src_mac[6];    // 源MAC地址
    uint16_t eth_type;      // 类型字段（如 0x0800 表示 IP）
};