#include<stdio.h>

int classwise(unsigned char *ip) {
    unsigned char first_byte = ip[0];
    // A 类：0xxxxxxx (0-127)
    if ((first_byte & 0x80) == 0) {
        return 0;
    }
    // B 类：10xxxxxx (128-191)
    if ((first_byte & 0xC0) == 0x80) {
        return 1;
    }
    // C 类：110xxxxx (192-223)
    if ((first_byte & 0xE0) == 0xC0) {
        return 2;
    }
    // D 类：1110xxxx (224-239)
    if ((first_byte & 0xF0) == 0xE0) {
        return 3;
    }
    // E 类：1111xxxx (240-255)
    return 4;
}

#include <stdio.h>

int main() {
    unsigned char ips[5][4] = {
        {10, 0, 0, 1},      // A 类
        {172, 16, 0, 1},    // B 类
        {192, 168, 1, 1},   // C 类
        {224, 0, 0, 18},    // D 类 (组播)
        {245, 0, 0, 1}      // E 类 (保留)
    };

    char *labels[] = {"A", "B", "C", "D", "E"};

    printf("IP Address Classification:\n");
    for (int i = 0; i < 5; i++) {
        int res = classwise(ips[i]);
        printf("%d.%d.%d.%d is Class %s (Return: %d)\n", 
               ips[i][0], ips[i][1], ips[i][2], ips[i][3], labels[res], res);
    }

    return 0;
}