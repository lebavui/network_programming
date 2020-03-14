// UDPReceiver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
    // Khoi tao thu vien Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Khai bao socket nhan du lieu theo giao thuc UDP
    SOCKET receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Khai bao dia chi cuc bo nhan du lieu
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan dia chi vao socket
    bind(receiver, (SOCKADDR*)&addr, sizeof(addr));

    char buf[256];
    int ret;

    // Khai bao dia chi ben gui
    SOCKADDR_IN senderAddr;
    int senderAddrLen = sizeof(senderAddr);

    while (1)
    {
        // Nhan du lieu va kiem tra loi
        ret = recvfrom(receiver, buf, sizeof(buf), 0, (SOCKADDR *)&senderAddr, &senderAddrLen);
        if (ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            if (ret != 10040) {
                printf("sendto() failed: error code %d\n", ret);
                break;
            }
        }
        else
        {
            // In ra man hinh so byte nhan duoc, dia chi ben gui, va du lieu nhan duoc
            buf[ret] = 0;
            printf("Received %d bytes from %s:%d: %s", ret, inet_ntoa(senderAddr.sin_addr), ntohs(senderAddr.sin_port), buf);
        }
    }

    // Dong ket noi, giai phong Winsock
    closesocket(receiver);
    WSACleanup();
}
