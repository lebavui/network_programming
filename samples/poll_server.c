#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <limits.h>

void removeClient(struct pollfd *fds, int *nfds, int index) 
{
    if (index < *nfds - 1) 
        fds[index] = fds[*nfds - 1];
    *nfds = *nfds - 1;
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

    struct pollfd fds[64];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[2048];

    while (1)
    {
        printf("Waiting for new event.\n");
        int ret = poll(fds, nfds, -1);
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

        if (fds[0].revents & POLLIN) 
        {
            int client = accept(listener, NULL, NULL);
            printf("New client connected %d\n", client);
            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;

            // TODO: Check limit
        }
        
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