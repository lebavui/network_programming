#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>

// Hàm xử lý tín hiệu SIGCHLD
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
    // Hiển thị số tiến trình tối đa có thể tạo
    // struct rlimit lim;
    // getrlimit(RLIMIT_NPROC, &lim);
    // printf("Soft limit: %ld\n", lim.rlim_cur);
    // printf("Hard limit: %ld\n", lim.rlim_max);

    // Tạo socket chờ kết nối
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Khai báo cấu trúc địa chỉ của server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gắn địa chỉ với socket và chuyển sang chờ kết nối
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    pid_t pid;

    // Đăng ký xử lý tín hiệu SIGCHLD
    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d\n", client);

        // Tạo tiến trình cho kết nối mới
        if ((pid = fork()) == 0)
        {
            // Trong tiến trình con

            // Đóng socket listener vì không dùng đến
            close(listener);

            // Nhận và xử lý dữ liệu
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
            
            // Đóng kết nối
            close(client);

            // Kết thúc tiến trình con
            exit(0);
        }

        // Trong tiến trình cha, đóng socket client do không dùng đến
        close(client);
    }

    return 0;
}