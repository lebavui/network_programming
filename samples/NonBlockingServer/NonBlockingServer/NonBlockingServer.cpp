// NonBlockingServer.cpp
// Vi du minh hoa server hoat dong o che do non-blocking, cac ham vao ra tra ve ket qua
// ngay lap tuc. Ung dung can kiem tra ket qua tra ve cua cac ham vao ra de thuc hien cac
// cong viec tuong ung.

#include <stdio.h>
#include <winsock2.h>

void RemoveClient(SOCKET*, int*, int);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Khai bao server socket
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Chuyen socket sang che do non-blocking
	unsigned long ul = 1;
	int ret = ioctlsocket(listener, FIONBIO, &ul);
	if (ret == SOCKET_ERROR)
	{
		printf("Failed to process!");
		return 1;
	}

	// Khai bao dia chi server
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	// Chuyen socket sang trang thai cho ket noi
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	// Khai bao mang de luu cac socket da ket noi den server
	SOCKET clients[64];
	int numClients = 0;

	char buf[1024];

	while (1)
	{
		// Chap nhan ket noi moi
		SOCKET client = accept(listener, NULL, NULL);
		if (client == INVALID_SOCKET)
		{
			// Neu socket khong hop le, kiem tra ma loi
			// Neu ma loi khong phai la WSAWOULDBLOCK thi ket thuc
			ret = WSAGetLastError();
			if (ret != WSAEWOULDBLOCK)
			{
				printf("Failed to accept client!");
				break;
			}
		}
		else
		{
			// Neu socket hop le thi them vao mang
			clients[numClients++] = client;
		}

		// Nhan du lieu tu cac socket client
		for (int i = 0; i < numClients; i++)
		{
			ret = recv(clients[i], buf, sizeof(buf), 0);

			if (ret < 0)
			{
				// Neu bi loi thi kiem tra ma loi
				// Neu khong phai la WSAWOULDBLOCK thi ket noi bi huy
				ret = WSAGetLastError();
				if (ret != WSAEWOULDBLOCK)
				{
					printf("Failed to receive data from client!");
					// Xoa socket khoi mang
					RemoveClient(clients, &numClients, i);
					i--;
				}

				// Chuyen sang nhan du lieu tu socket tiep theo
				continue;
			}

			if (ret == 0)
			{
				// Neu ket noi bi ngat => can xoa socket khoi mang
				RemoveClient(clients, &numClients, i);
				i--;

				// Chuyen sang nhan du lieu tu socket tiep theo
				continue;
			}

			// Them ky tu ket thuc xau va in ra man hinh
			if (ret < 1024)
				buf[ret] = 0;
			printf("%s\n", buf);
		}
	}

	closesocket(listener);
	WSACleanup();
}

void RemoveClient(SOCKET* clients, int* pNumClients, int index)
{
	// Xoa client trong mang clients bang cach gan phan tu cuoi cung trong mang
	if (index < *pNumClients)
	{
		if (index != *pNumClients - 1)
			clients[index] = clients[*pNumClients - 1];

		(*pNumClients)--;
	}
}