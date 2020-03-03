// SelectServer.cpp
// Xay dung ung dung server hoat dong theo mo hinh select


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

	fd_set fdread;

	SOCKET clients[64];
	int numClients = 0;

	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	char buf[256];
	int ret;

	while (1)
	{
		// Khoi tao lai tap fd
		FD_ZERO(&fdread);

		// Gan cac socket vao tap fd
		FD_SET(listener, &fdread);
		for (int i = 0; i < numClients; i++)
			FD_SET(clients[i], &fdread);

		// Su dung ham select de cho den khi co su kien
		ret = select(0, &fdread, NULL, NULL, &tv);
		
		// Kiem tra loi
		if (ret < 0)
		{
			printf("select() failed\n");
			break;
		}

		// Neu het gio, co the thuc hien cac cong viec khac neu co
		if (ret == 0)
		{
			printf("Timed out. Can do something else...\n");
			continue;
		}

		// Kiem tra su kien xay ra cua socket nao
		if (FD_ISSET(listener, &fdread))
		{
			// Su kien co ket noi moi
			SOCKET client = accept(listener, NULL, NULL);

			// Them socket vao mang
			clients[numClients] = client;
			numClients++;
		}

		for (int i = 0; i < numClients; i++)
			if (FD_ISSET(clients[i], &fdread))
			{
				ret = recv(clients[i], buf, sizeof(buf), 0);

				if (ret <= 0)
				{
					// Ket noi bi ngat
					// Xoa client khoi mang
					continue;
				}

				// Them ky tu ket thuc xau
				if (ret < sizeof(buf))
					buf[ret] = 0;
				printf("Received from %d: %s\n", clients[i], buf);

				// Xu ly du lieu nhan duoc theo giao thuc
			}
	}
}
