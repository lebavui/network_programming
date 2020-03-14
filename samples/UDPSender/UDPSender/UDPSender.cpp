// UDPSender.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
    // Khoi tao Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Tao socket de gui du lieu
    SOCKET sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Khai bao dia chi nhan du lieu
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8000);

    const char* msg = "Hello. I am sending you a message again.";

    // Gui du lieu bang ham sendto() va kiem tra ket qua tra ve
    int ret = sendto(sender, msg, strlen(msg), 0, (SOCKADDR*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR) 
    {
        ret = WSAGetLastError();
        printf("sendto() failed: error code %d\n", ret);
    }
    else
    {
        printf("%d bytes are sent\n", ret);
    }

    closesocket(sender);
    WSACleanup();
}