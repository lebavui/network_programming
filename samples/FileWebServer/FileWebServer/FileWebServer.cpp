// FileWebServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

void ProcessRequest(SOCKET, char*);
void HtmlDecode(char*);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	// Chap nhan ket noi va truyen nhan du lieu
	while (1)
	{
		SOCKET client = accept(listener, NULL, NULL);
		char buf[1024];
		int ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			continue;
		buf[ret] = 0;
		printf("Received: %s\n", buf);

		ProcessRequest(client, buf);
		closesocket(client);
	}
}

void ProcessRequest(SOCKET client, char* request)
{
	char cmd[16], path[256];
	int ret = sscanf(request, "%s %s", cmd, path);

	if (strcmp(path, "/favicon.ico") == 0)
		return;

	// Tao duong dan den thu muc/file tren o dia
	char fullPath[256];
	sprintf(fullPath, "C:%s", path);

	HtmlDecode(fullPath);

	// Check file or directory
	DWORD at = GetFileAttributesA(fullPath);
	if (at == INVALID_FILE_ATTRIBUTES)
	{
		// File hoac thu muc khong ton tai thi thong bao loi
		const char* response = "HTTP/1.1 404 File not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>File not found</h1></body></html>";
		send(client, response, strlen(response), 0);
	}
	else if (at & FILE_ATTRIBUTE_DIRECTORY)
	{
		// Gui phan header
		const char* header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
		send(client, header, strlen(header), 0);

		send(client, "<html><body>", 12, 0);
		char link[256];

		// Neu la thu muc thi them *.* de liet ke cac thu muc con, file
		strcat(fullPath, "*.*");
		
		WIN32_FIND_DATAA DATA;
		HANDLE h = FindFirstFileA(fullPath, &DATA);
		do {
			if (DATA.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || DATA.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
				continue;

			if (DATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				sprintf(link, "<a href='%s%s/'><b>%s</b></a><br/>", path, DATA.cFileName, DATA.cFileName);
				send(client, link, strlen(link), 0);
			}
			else
			{
				sprintf(link, "<a href='%s%s'><i>%s</i></a><br/>", path, DATA.cFileName, DATA.cFileName);
				send(client, link, strlen(link), 0);
			}
		} while (FindNextFileA(h, &DATA));

		send(client, "</body></html>", 14, 0);
	}
	else
	{
		FILE* f = fopen(fullPath, "rb");
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		char header[256];

		// Download file
		if (strstr(path, ".jpg"))
		{
			sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", size);
			send(client, header, strlen(header), 0);
		}
		else if (strstr(path, ".mp3"))
		{
			sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg3\r\nContent-Length: %d\r\n\r\n", size);
			send(client, header, strlen(header), 0);
		}
		else if (strstr(path, ".mp4"))
		{
			sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\nContent-Length: %d\r\n\r\n", size);
			send(client, header, strlen(header), 0);
		}
		else if (strstr(path, ".html"))
		{
			sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", size);
			send(client, header, strlen(header), 0);
		}
		else
		{
			sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", size);
			send(client, header, strlen(header), 0);
		}

		char buf[2048];
		while (!feof(f))
		{
			int ret = fread(buf, 1, sizeof(buf), f);
			send(client, buf, ret, 0);
		}

		fclose(f);
	}
}

void HtmlDecode(char* path)
{
	// Thay chuoi %20 thanh dau cach
	char* p = NULL;
	while ((p = strstr(path, "%20")) != NULL)
	{
		p[0] = 0x20;
		int i = 1;
		while (p[i + 2] != 0) 
		{
			p[i] = p[i + 2];
			i++;
		}
		p[i] = 0;
	}
}