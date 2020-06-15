// WebClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <openssl/ssl.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "winsock2.h"

#pragma comment(lib, "ws2_32")

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("111.65.250.2");
    addr.sin_port = htons(443);

    connect(client, (SOCKADDR*)&addr, sizeof(addr));

    SSL_library_init();
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* context = SSL_CTX_new(method);
    SSL* ssl = SSL_new(context);
    if (!ssl) 
    {
        printf("Error creating SSL.\n");
        return -1;
    }
    SSL_set_fd(ssl, client);
    int ret = SSL_connect(ssl);
    if (ret <= 0)
    {
        printf("Error creating SSL connection.");
        return -1;
    }

    char buf[256] = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
    SSL_write(ssl, buf, strlen(buf));

    while (1)
    {
        ret = SSL_read(ssl, buf, sizeof(buf));
        if (ret <= 0)
            break;
        if (ret < sizeof(buf))
            buf[ret] = 0;
        // printf("%s", buf);
        printf("%.*s", ret, buf);
    }

    closesocket(client);
    WSACleanup();
}