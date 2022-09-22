#include <stdio.h>
#include <arpa/inet.h>
typedef unsigned short u16;
typedef unsigned long u32;
int main(int argc, char *argv[])
{
    u16 host_port = 0x1234;
    u16 net_port;
    u32 host_addr = 0x12345678;
    u32 net_addr;
    net_port = htons(host_port);
    net_addr = htonl(host_addr);
    printf("Host ordered port: %#x \n", host_port);
    printf("Network ordered port: %#x \n", net_port);
    printf("Host ordered address: %#lx \n", host_addr);
    printf("Network ordered address: %#lx \n", net_addr);
    return 0;
}