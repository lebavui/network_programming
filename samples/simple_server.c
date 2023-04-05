#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

int main() 
{
    // Tạo socket chờ kết nối
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai báo cấu trúc địa chỉ của server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gắn socket với cấu trúc địa chỉ
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyển socket sang trạng thái chờ kết nối
    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    printf("waiting for a new client ...\n");

    // Chờ và chấp nhận kết nối
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("new client connected: %d\n", client);

    // Nhận dữ liệu từ client
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);

    // Kiểm tra kết nối có bị đóng hoặc hủy không
    if (ret <= 0)
    {
        printf("recv() failed.\n");
        return 1;
    }

    // Thêm ký tự kết thúc xâu và in ra màn hình
    if (ret < sizeof(buf))
        buf[ret] = 0;
    puts(buf);

    // Gửi dữ liệu sang client
    send(client, buf, strlen(buf), 0);
    
    // Đóng kết nối
    close(client);
    close(listener);    

    return 0;
}