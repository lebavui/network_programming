#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main() {
    // Khai báo socket kết nối đến server
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai báo địa chỉ của server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    // Kết nối đến server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        perror("connect() failed");
        return 1;
    }
    
    // Gửi tin nhắn đến server
    char *msg = "Hello server";
    send(client, msg, strlen(msg), 0);

    // Nhận tin nhắn từ server
    char buf[2048];
    int len = recv(client, buf, sizeof(buf), 0);

    // Nếu kết nối bị đóng hoặc bị lỗi thì kết thúc chương trình
    if (len <= 0)
    {
        printf("recv() failed.\n");
        return 1;
    }

    // Xử lý dữ liệu nhận được
    // Thêm ký tự kết thúc xâu và in ra màn hình
    if (len < sizeof(buf))
        buf[len] = 0;
    printf("Data received: %s\n", buf);

    // Kết thúc, đóng socket
    close(client);

    return 0;
}