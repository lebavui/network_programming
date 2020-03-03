// MultiThreadServer.cpp
// Vi du tao server chap nhan nhieu ket noi va xu ly du lieu tu cac ket noi

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

// Khai bao prototype 
DWORD WINAPI ClientThread(LPVOID);

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

	while (1)
	{
		// Chap nhan ket noi
		SOCKET client = accept(listener, NULL, NULL);
		printf("New client accepted: %d\n", client);
		
		// Tao luong moi phuc vu truyen nhan du lieu cho ket noi moi
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET*)lpParam;
	char buf[256];
	int ret;

	while (1)
	{
		// Nhan du lieu tu client
		ret = recv(client, buf, sizeof(buf), 0);

		// Kiem tra neu ket noi bi ngat hoac bi huy
		if (ret <= 0)
			break;

		// Them ky tu ket thuc xau
		if (ret < sizeof(buf))
			buf[ret] = 0;

		// Hien thi du lieu nhan duoc ra man hinh
		printf("Received from %d: %s\n", client, buf);

		// Kiem tra neu client gui chuoi ky tu quit thi dong ket noi
		if (strcmp(buf, "quit\n") == 0)
		{
			// Gui thong diep cho client truoc khi dong ket noi
			const char msg[] = "bye\n";
			send(client, msg, strlen(msg), 0);
			break;
		}			
	}
	
	// Dong ket noi
	closesocket(client);

	return 0;
}