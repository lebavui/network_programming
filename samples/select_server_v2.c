#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    // Khai báo tập fdread chứa các socket và tập fdtest để thăm dò sự kiện
    fd_set fdread, fdtest;
    
    // Cấu trúc thời gian đợi
    struct timeval tv;
    char buf[2048];

    // Khởi tạo tập fdread
    FD_ZERO(&fdread);

    // Gắn socket listener vào tập fdread
    FD_SET(listener, &fdread);

    while (1)
    {
        // Tập fdtest để thăm dò sự kiện, giữ nguyên các socket trong tập fdread
        fdtest = fdread;

        // Khởi tạo lại giá trị cấu trúc thời gian
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Chờ đến khi sự kiện xảy ra hoặc hết giờ
        printf("Waiting for new event.\n");
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);
        if (ret < 0) 
        {
            printf("select() failed.\n");
            return 1;
        }
        if (ret == 0)
        {
            printf("Timed out.\n");
            continue;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    // Socket listener có sự kiện yêu cầu kết nối
                    int client = accept(listener, NULL, NULL);

                    if (client < FD_SETSIZE)
                    {
                        printf("New client connected %d\n", client);
                        
                        // Thêm vào tập fdread
                        FD_SET(client, &fdread);
                    }
                    else
                    {
                        // Đã vượt quá số kết nối tối đa
                        close(client);
                    }
                }
                else
                {
                    // Socket client có sự kiện nhận dữ liệu
                    ret = recv(i, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        printf("Client %d disconnected\n", i);
                        FD_CLR(i, &fdread);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received data from client %d: %s\n", i, buf);
                    }                    
                }
            }
    }

    return 0;
}