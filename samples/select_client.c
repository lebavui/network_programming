#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    // Khai bao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi cua server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    // Ket noi den server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        printf("Khong ket noi duoc den server!\n");
        return 1;
    }
     
    fd_set fdread;
    char buf[256];

    while (1)
    {
        // Khoi tao tap fdread
        FD_ZERO(&fdread);

        // Gan cac descriptor vao tap fdread
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(client, &fdread);
        
        // Cho su kien xay ra
        int ret = select(client + 1, &fdread, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select() failed");
            break;
        }
        
        // Kiem tra su kien co du lieu tu ban phim
        if (FD_ISSET(STDIN_FILENO, &fdread))
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);

            // Nếu nhập "exit" thì kết thúc
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }

        // Kiem tra su kien co du lieu tu socket
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

    // Ket thuc, dong socket
    close(client);

    return 0;
}