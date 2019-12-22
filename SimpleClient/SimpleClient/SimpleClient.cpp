#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

int main()
{
    // Khoi tao thu vien Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Tao doi tuong socket
    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi cua server
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    // Ket noi den server
    int ret = connect(client, (SOCKADDR*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR) 
    {
        printf("Loi ket noi!");
        system("pause");
        return 1;
    }

    // Gui cau chao len server
    char msg[256] = "Hello server! I am a new client.";
    send(client, msg, strlen(msg), 0);

    // Lien tuc nhan thong diep tu server va in ra man hinh
    while (1)
    {
        ret = recv(client, msg, sizeof(msg), 0);

        if (ret <= 0)
        {
            printf("Loi ket noi!");
            system("pause");
            break;
        }

        // Them ky tu ket thuc xau va in ra man hinh
        if (ret < 256)
            msg[ret] = 0;
        printf("Received: %s\n", msg);
    }

    closesocket(client);
    WSACleanup();

    return 0;
}