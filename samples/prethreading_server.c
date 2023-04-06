#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

void* thread_proc(void *arg);

int main() 
{
    // Tạo socket chờ kết nối
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Khai báo cấu trúc địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gắn địa chỉ với socket
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    int num_threads = 8;
    pthread_t thread_id;

    // Tạo trước các luồng
    for (int i = 0; i < num_threads; i++)
    {
        int ret = pthread_create(&thread_id, NULL, thread_proc, &listener);
        if (ret != 0)
            printf("Could not create new thread.\n");
        sched_yield();
    }
    
    // Đợi vô thời hạn, đảm bảo chương trình tiếp tục hoạt động
    pthread_join(thread_id, NULL);
    return 0;
}

void* thread_proc(void *arg)
{
    int listener = *(int *)arg;
    char buf[256];

    while (1)
    {
        // Chờ kết nối
        int client = accept(listener, NULL, NULL);
        printf("New client %d accepted in thread %ld with pid %d\n", client, pthread_self(), getpid());

        // Chờ dữ liệu từ client
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            continue;

        // Xử lý dữ liệu, trả lại kết quả cho client
        buf[ret] = 0;
        printf("Received from client %d: %s\n", client, buf);
        send(client, buf, strlen(buf), 0);

        // Đóng kết nối
        close(client);
    }

    return NULL;
}