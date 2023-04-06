#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <sys/resource.h>

void* thread_proc(void *arg);

int main() 
{
    // Hiển thị số luồng tối đa có thể tạo
    // struct rlimit lim;
    // getrlimit(RLIMIT_NPROC, &lim);
    // printf("Soft limit: %ld\n", lim.rlim_cur);
    // printf("Hard limit: %ld\n", lim.rlim_max);

    // Tạo socket chờ kết nối
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Khai báo cấu trúc địa chỉ server chờ ở cổng 9000
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gắn địa chỉ với socket và chuyển sang trạng thái chờ kết nối
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    pthread_t thread_id;

    while (1)
    {
        printf("Waiting for new client ...\n");
        // Chấp nhận kết nối mới
        int client = accept(listener, NULL, NULL);
        if (client == -1)
            continue;
        printf("New client connected: %d\n", client);
    
        // Tạo luồng để xử lý yêu cầu từ client
        int ret = pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
        if (ret != 0) 
        {
            printf("Could not create thread!\n");   
        }

        // Yêu cầu luồng tự giải phóng khi kết thúc
        pthread_detach(thread_id);
        // Ưu tiên luồng mới tạo bắt đầu chạy
        sched_yield();
    }

    return 0;
}

void* thread_proc(void *arg)
{
    // Luồng xử lý yêu cầu từ client

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
    }
    close(client);
    printf("child thread finished.\n");
}