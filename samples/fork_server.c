#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>

void signalHandler(int signo) 
{
    pid_t pid;
    int stat;
    printf("signo = %d\n", signo);
    pid = wait(&stat);
    printf("child %d terminated.\n", pid);
    return;
}

int main() 
{
    // Tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener != -1)
        printf("Socket created: %d\n", listener);

    // Khai bao cau truc dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan dia chi voi socket
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    pid_t pid;

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d\n", client);
        if ((pid = fork()) == 0)
        {
            // in child process
            close(listener);

            char buf[256];
            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                printf("Received from client %d: %s\n", client, buf);
                send(client, buf, strlen(buf), 0);
            }
            
            close(client);
            exit(0);
        }
        close(client);
    }

    return 0;
}