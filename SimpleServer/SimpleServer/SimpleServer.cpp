// SimpleServer.cpp
//

#include <iostream>

#include <winsock2.h>

int main()
{
    // Khoi tao thu vien Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Tao doi tuong socket
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi cua server
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Chuyen sang sang thai cho ket noi
    bind(listener, (SOCKADDR*)&addr, sizeof(addr));
    listen(listener, 5);

    // Chap nhan ket noi
    SOCKET client = accept(listener, NULL, NULL);

    char msg[256];

    // Nhan cau chao tu client
    int ret = recv(client, msg, sizeof(msg), 0);

    if (ret <= 0)
    {
        printf("Loi ket noi!");
        system("pause");
        return 1;
    }

    // Them ky tu ket thuc xau va in ra man hinh
    if (ret < 256)
        msg[ret] = 0;
    printf("Received: %s\n", msg);

    // Lien tuc nhap chuoi ky tu tu ban phim va gui sang client
    while (1)
    {
        printf("Nhap xau ky tu: ");
        fgets(msg, sizeof(msg), stdin);
        if (strcmp(msg, "exit") == 0)
            break;

        send(client, msg, strlen(msg), 0);
    }

    closesocket(client);
    closesocket(listener);

    WSACleanup();

    return 0;
}