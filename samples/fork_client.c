#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/signal.h>

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
        
    char buf[256];

    int cid = fork();
    if (cid == 0)
    {
        // Tiến trình con
        while (1)
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }
    }
    else
    {
        // Tiến trình cha
        while (1)
        {
            int ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }

    // Ket thuc, dong socket
    close(client);

    // Dừng các tiến trình
    killpg(0, SIGKILL);

    return 0;
}