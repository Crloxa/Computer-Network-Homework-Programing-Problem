#include<stdio.h>

int is_in_net(unsigned char *ip, unsigned char *netip, unsigned char *mask){
    for(int i=0;i<4;i++){
        if((ip[i]&mask[i])!=netip[i]) return 0;
        //子网掩码和IP地址的与运算检查其网络地址
    }
    return 1;
    // 不匹配返回0 匹配返回1
}

int main(){
    unsigned char test_ip1[4]={114,51,41,91};
    unsigned char test_ip2[4]={114,5,141,91};
    unsigned char test_net[4]={114,51,41,0};
    unsigned char mask[4]={255,255,255,0};
    printf("ip match answer(1 true, 0 false)\n");
    printf("ip1: %d\n",is_in_net(test_ip1,test_net,mask));
    printf("ip2: %d\n",is_in_net(test_ip2,test_net,mask));
    return 0;
}