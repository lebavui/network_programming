#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

int main() {
    // Khai báo socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai báo địa chỉ của server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    // Kết nối đến server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        printf("Khong ket noi duoc den server!\n");
        return 1;
    }

    // Mảng cấu trúc thăm dò    
    struct pollfd fds[2];

    // Thêm descriptor của sự kiện bàn phím
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    // Thêm descriptor của sự kiện socket
    fds[1].fd = client;
    fds[1].events = POLLIN;

    char buf[256];

    while (1)
    {
        // Chờ đến khi sự kiện xảy ra, không sử dụng timeout
        int ret = poll(fds, 2, -1);
        
        // Nếu sự kiện là có dữ liệu từ bàn phím
        if (fds[0].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);

            // Nếu nhập "exit" thì kết thúc
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }

        // Nếu sự kiện là có dữ liệu từ socket
        if (fds[1].revents & POLLIN)
        {
            ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }

    close(client);

    return 0;
}