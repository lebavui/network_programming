// TestClient.cpp
//

#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

int main(int argc, char* argv[])
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

    int port = 9000;    // default port
    if (argc == 2)
    {
        port = atoi(argv[1]);
        if (port == 0)
            port = 9000;
    }

    addr.sin_port = htons(port);

    int ret = connect(client, (SOCKADDR*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        printf("connect() failed\n");
        return 1;
    }

    char msg[256];
    
    // Lien tuc nhap chuoi ky tu tu ban phim va gui sang client
    while (1)
    {
        printf("Nhap xau ky tu: ");
        gets_s(msg);
        if (strcmp(msg, "exit") == 0)
            break;
        send(client, msg, strlen(msg), 0);
    }

    closesocket(client);
    WSACleanup();

    return 0;
}