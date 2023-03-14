#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

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
    addr.sin_port = htons(9090);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    fd_set fdread;
    
    int clients[64];
    int numClients = 0;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    char buf[2048];

    while (1)
    {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;
        for (int i = 0; i < numClients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (clients[i] + 1 > maxdp)
                maxdp = clients[i] + 1;
        }
        
        // reset timeval struct
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        printf("Waiting for new event.\n");
        int ret = select(maxdp, &fdread, NULL, NULL, &tv);
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

        if (FD_ISSET(listener, &fdread)) 
        {
            int client = accept(listener, NULL, NULL);
            clients[numClients] = client;
            numClients++;

            // TODO: Check limit
        }
        
        for (int i = 0; i < numClients; i++)
            if (FD_ISSET(clients[i], &fdread)) 
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected\n", clients[i]);
                    // Remove from clients
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