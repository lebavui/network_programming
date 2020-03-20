#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable: 4996)

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addrinfo* info;
    SOCKADDR_IN addr;
    
    // Phan giai ten mien de ket noi
    int ret = getaddrinfo("bunoc.net", "http", NULL, &info);

    if (ret != 0)
    {
        printf("Get address failed\n");
        return 1;
    }

    memcpy(&addr, info->ai_addr, info->ai_addrlen);

    // Ket noi den server qua dia chi da phan giai
    ret = connect(client, (SOCKADDR*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError();
        printf("connect() failed: %d\n", ret);
        return 1;
    }

    // Send HEAD to get size of file 
    char buf[2048] = "HEAD /test.jpg HTTP/1.1\r\nHost: bunoc.net\r\n\r\n";
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("Connection lost\n");
        return 1;
    }
    if (ret < sizeof(buf))
        buf[ret] = 0;
    printf("%s", buf);

    int fileSize = 0;

    char* pos = strstr(buf, "Content-Length:");
    if (pos != NULL)
        fileSize = atoi(pos + 16);
    
    printf("Filesize: %d\n", fileSize);

    // Send GET to download file
    // Try to get size of file 
    strcpy(buf, "GET /test.jpg HTTP/1.1\r\nHost: bunoc.net\r\n\r\n");
    send(client, buf, strlen(buf), 0);

    FILE* f = fopen("C:\\Test\\test.jpg", "wb");
    int started = 0;
    int downloaded = 0;

    printf("Downloading: ");

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);

        if (ret <= 0)
            break;

        // Tim CRLFCRLF de bat dau ghi du lieu vao file
        if (started == 0)
        {
            pos = strstr(buf, "\r\n\r\n");
            
            if (pos == NULL)
                continue;

            started = 1;
            fwrite(pos + 4, 1, ret - (pos - buf) - 4, f);

            downloaded += ret - (pos - buf) - 4;
        }
        else
        {
            fwrite(buf, 1, ret, f);
            downloaded += ret;
        }

        // Hien thi tien do download file
        if (fileSize > 0)
            printf("\r%d/%d", downloaded, fileSize);
        else
            printf("\r%d", downloaded);
    }

    fclose(f);

    closesocket(client);
    WSACleanup();
}