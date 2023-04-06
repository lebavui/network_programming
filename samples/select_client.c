#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    // Khai báo socket client
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai báo địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    // Thực hiện kết nối đến server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        printf("Khong ket noi duoc den server!\n");
        return 1;
    }
    
    // Khai báo tập fdset
    fd_set fdread;
    char buf[256];

    while (1)
    {
        // Khởi tạo lại tập fdread
        FD_ZERO(&fdread);

        // Gắn các descriptor vào tập fdread
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(client, &fdread);
        
        // Chờ đến khi sự kiện xảy ra
        int ret = select(client + 1, &fdread, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select() failed");
            break;
        }
        
        // Kiểm tra sự kiện có dữ liệu từ bàn phím
        if (FD_ISSET(STDIN_FILENO, &fdread))
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);

            // Nếu nhập "exit" thì kết thúc
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến qua socket
        if (FD_ISSET(client, &fdread))
        {
            ret = recv(client, buf, sizeof(buf), 0);

            // Nếu ngắt kết nối thì kết thúc
            if (ret <= 0)
                break;
            
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }

    // Kết thúc, đóng socket
    close(client);

    return 0;
}