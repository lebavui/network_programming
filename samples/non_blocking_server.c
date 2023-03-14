#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

void removeClient(int *clients, int *pNumClients, int index)
{
    if (index < *pNumClients - 1)
        clients[index] = clients[*pNumClients - 1];
    *pNumClients = *pNumClients - 1;
}

int main() 
{
    // Tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener != -1)
        printf("Socket created: %d\n", listener);
    else
    {
        printf("Failed to create socket.\n");
        exit(1);
    }

    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    // Khai bao cau truc dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan dia chi voi socket
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        printf("bind() failed.\n");
        exit(1);
    }

    if (listen(listener, 5)) 
    {
        printf("listen() failed.\n");
        exit(1);
    }

    int clients[64];
    int numClients = 0;
    char buf[256];
    
    while (1)
    {
        // Chap nhan ket noi
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            if (errno != EWOULDBLOCK)
            {
                printf("accept() failed.\n");
                exit(1);
            }
            else
            {
                // do nothing
            }
        }
        else
        {
            printf("New client connected: %d\n", client);
            clients[numClients++] = client;
            ul = 1;
            ioctl(client, FIONBIO, &ul);
        }
        
        // Nhan du lieu tu client
        for (int i = 0; i < numClients; i++)
        {
            int ret = recv(clients[i], buf, sizeof(buf), 0);
            if (ret == -1)
            {
                if (errno != EWOULDBLOCK)
                {
                    printf("recv() failed.\n");
                    close(clients[i]);
                    removeClient(clients, &numClients, i--);
                    continue;
                }
                else
                {
                    // do nothing
                }
            }
            else if (ret == 0)
            {
                printf("client disconnected.\n");
                close(clients[i]);
                removeClient(clients, &numClients, i--);
                continue;
            }
            else
            {
                // Them ky tu ket thuc xau va in ra man hinh
                if (ret < sizeof(buf))
                    buf[ret] = 0;
                puts(buf);
                // Gui du lieu sang client
                send(clients[i], buf, strlen(buf), 0);
            }
        }
    }
    
    close(listener);    

    return 0;
}