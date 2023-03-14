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
    FD_ZERO(&fdread);

    char buf[256];

    while (1)
    {
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(client, &fdread);
        
        int maxdp = client + 1; // STDIN_FILENO is 0

        int ret = select(client + 1, &fdread, NULL, NULL, NULL);
        if (ret == -1)
        {
            break;
        }
        
        if (FD_ISSET(STDIN_FILENO, &fdread))
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }
        if (FD_ISSET(client, &fdread))
        {
            ret = recv(client, buf, sizeof(buf), 0);
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }

    // Ket thuc, dong socket
    close(client);

    return 0;
}