#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/resource.h>

#define MAX_CLIENTS 1024

// Hàm xóa client ra khỏi mảng thăm dò
void removeClient(struct pollfd *fds, nfds_t *nfds, int index) 
{
    if (index < *nfds - 1) 
        fds[index] = fds[*nfds - 1];
    *nfds = *nfds - 1;
}

int main()
{
    // struct rlimit lim;
    // getrlimit(RLIMIT_NOFILE, &lim);
    // printf("Soft limit: %ld\n", lim.rlim_cur);
    // printf("Hard limit: %ld\n", lim.rlim_max);
    
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    // Mảng thăm dò sự kiện
    struct pollfd fds[MAX_CLIENTS];
    nfds_t nfds = 1;

    // Thêm socket listener vào mảng
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[2048];

    while (1)
    {
        // Chờ sự kiện xảy ra hoặc hết giờ
        printf("Waiting for new event.\n");
        int ret = poll(fds, nfds, 5000);
        if (ret < 0)
        {
            printf("poll() failed.\n");
            return 1;
        }
        if (ret == 0)
        {
            printf("Timed out.\n");
            continue;
        }

        // Nếu sự kiện có yêu cầu kết nối
        if (fds[0].revents & POLLIN) 
        {
            int client = accept(listener, NULL, NULL);

            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected %d\n", client);
                // Lưu vào mảng để thăm dò
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
            }
            else
            {
                // Đã vượt quá số kết nối tối đa
                close(client);
            }
        }
        
        // Kiểm tra sự kiện dữ liệu đến socket
        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & (POLLIN | POLLERR)) 
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected\n", fds[i].fd);
                    // Remove from clients
                    removeClient(fds, &nfds, i);
                    i--;
                    continue;
                }
                buf[ret] = 0;
                printf("Received data from client %d: %s\n", fds[i].fd, buf);
            }
    }

    return 0;
}