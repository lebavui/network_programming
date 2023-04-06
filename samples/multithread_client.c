#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

void* thread_proc(void *arg);

int main() 
{
    // Tạo socket kết nối đến server
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Khai báo cấu trúc địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    // Kết nối đến server
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Khong the ket noi den server.\n");
        return 1;
    }

    // Tạo luồng mới
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, thread_proc, (void *)&client))
    {
        printf("Khong the tao luong!\n");
        return 1;   
    }

    // Chuyển luồng sang chế độ tự giải phóng
    pthread_detach(thread_id);
    
    char buf[256];

    // Trong luồng chính, chờ dữ liệu từ bàn phím
    while (1)
    {
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);
        if (strncmp(buf, "exit", 4) == 0)
            break;
    }

    close(client);
    return 0;
}

void* thread_proc(void *arg)
{
    // Trong luồng con, chờ dữ liệu từ socket
    
    printf("child thread created.\n");
    int client = *(int *)arg;
    char buf[256];
    while (1)
    {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0)
            break;
        buf[len] = 0;
        printf("%s\n", buf);
    }
    close(client);
    printf("child thread finished.\n");
    return 0;
}