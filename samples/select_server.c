#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_CLIENTS FD_SETSIZE

// Xóa client ra khỏi mảng
void removeClient(int *clients, int *numClients, int clientIndex) 
{
    if (clientIndex < *numClients - 1) 
        clients[clientIndex] = clients[*numClients - 1];
    *numClients = *numClients - 1;
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    fd_set fdread;
    
    // Mảng clients lưu các socket đã được chấp nhận
    // Sử dụng trong việc thăm dò sự kiện
    int clients[MAX_CLIENTS];
    int numClients = 0;

    // Cấu trúc thời gian đợi
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    char buf[2048];

    while (1)
    {
        // Khởi tạo lại tập fdread
        FD_ZERO(&fdread);

        // Gắn các socket listener và clients vào tập fdread
        // maxdp lưu giá trị descriptor lớn nhất 
        FD_SET(listener, &fdread);
        int maxdp = listener;
        for (int i = 0; i < numClients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (clients[i] > maxdp)
                maxdp = clients[i];
        }
        
        // Khởi tạo lại giá trị cấu trúc thời gian
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Chờ đến khi sự kiện xảy ra hoặc hết giờ
        printf("Waiting for new event.\n");
        int ret = select(maxdp + 1, &fdread, NULL, NULL, &tv);
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

        // Thăm dò sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread)) 
        {
            int client = accept(listener, NULL, NULL);

            if (numClients < MAX_CLIENTS)
            {
                printf("New client connected %d\n", client);
                // Lưu vào mảng để thăm dò sự kiện
                clients[numClients] = client;
                numClients++;
            }
            else
            {
                // Đã vượt quá số kết nối tối đa
                close(client);
            }
        }
        
        // Thăm dò sự kiện có dữ liệu truyền đến các socket client
        for (int i = 0; i < numClients; i++)
            if (FD_ISSET(clients[i], &fdread)) 
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected\n", clients[i]);
                    // Xóa client khỏi mảng
                    removeClient(clients, &numClients, i);
                    i--;
                    continue;
                }
                buf[ret] = 0;
                printf("Received data from client %d: %s\n", clients[i], buf);
            }
    }

    return 0;
}