// SimpleHTTPServer.cpp
// Vi du tao 1 HTTP Server hien thi Hello World tren trinh duyet khi co yeu cau

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

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

	char buf[2048];
	int ret;
	const char msg[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1 style='color:blue;'>Hello World</h1></body></html>";

	while (1)
	{
		// Chap nhan ket noi moi
		SOCKET client = accept(listener, NULL, NULL);
		printf("New client accepted: %d\n", client);

		// Nhan du lieu tu ket noi, neu bi loi thi bo qua
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			continue;

		// Them ky tu ket thuc xau
		if (ret < sizeof(buf))
			buf[ret] = 0;

		printf("Request: %s\n", buf);

		// Tra lai doan HTML cho trinh duyet
		send(client, msg, strlen(msg), 0);

		// Xong, dong ket noi
		closesocket(client);
	}
}