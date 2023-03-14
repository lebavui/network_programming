#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

int main()
{
    // Tạo socket theo giao thức UDP
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    // Khai báo địa chỉ bên nhận
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9090);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    // Nhận tin nhắn
    char buf[16];
    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
        if (ret == -1) 
        {
            printf("recvfrom() failed\n");
            break;
        }
        else 
        {
            buf[ret] = 0;
            printf("%d - %s\n", ret, buf); 
        }        
    }
     
    return 0;
}