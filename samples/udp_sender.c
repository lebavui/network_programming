#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    // Tạo socket theo giao thức UDP
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    // Khai báo địa chỉ bên nhận
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9090);
    
    // Gửi tin nhắn
    char buf[256];
    while (1)
    {
        printf("Enter message: ");
        fgets(buf, sizeof(buf), stdin);
        sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
    }
    
    return 0;
}