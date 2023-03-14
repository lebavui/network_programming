#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

void* thread_proc(void *arg);

int clients[64];
int numClients = 0;

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

    pthread_t thread_id;

    while (1)
    {
        printf("Waiting for new client ...\n");
        // Chap nhan ket noi
        int client = accept(listener, NULL, NULL);
        if (client == -1)
            continue;
        printf("New client connected: %d\n", client);
        clients[numClients++] = client;
    
        int ret = pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
        if (ret != 0) 
        {
            printf("Could not create thread!\n");   
        }

        pthread_detach(thread_id);
        sched_yield();
    }

    return 0;
}

void* thread_proc(void *arg)
{
    printf("child thread created.\n");
    int client = *(int *)arg;
    char buf[2048];
    while (1)
    {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0)
            break;
        buf[len] = 0;
        printf("%s", buf);

        // send this message to other clients
        for (int i = 0; i < numClients; i++)
            if (clients[i] != client)
                send(clients[i], buf, strlen(buf), 0);
    }
    close(client);
    printf("child thread finished.\n");
}