// SimpleHTTPClient.cpp
//

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Phan giai ten mien
	addrinfo* info;
	SOCKADDR_IN addr;
	int ret = getaddrinfo("genk.vn", "http", NULL, &info);

	// Neu phan giai thanh cong thi sao chep dia chi vao bien addr
	if (ret == 0)
		memcpy(&addr, info->ai_addr, info->ai_addrlen);
	else 
	{
		printf("Khong phan giai duoc ten mien");
		return 1;
	}

	// Ket noi den server
	ret = connect(client, (SOCKADDR*)&addr, sizeof(addr));

	// Gui lenh GET
	char msg[1024] = "GET / HTTP/1.1\nHost: genk.vn\n\n";
	send(client, msg, strlen(msg), 0);

	while (1) 
	{
		// Nhan du lieu tu server, moi lan nhan toi da 1024 byte
		ret = recv(client, msg, sizeof(msg), 0);
		if (ret <= 0)
			break;
		if (ret < sizeof(msg))
			msg[ret] = 0;
		printf("%s", msg);
	}

	closesocket(client);
	WSACleanup();
}