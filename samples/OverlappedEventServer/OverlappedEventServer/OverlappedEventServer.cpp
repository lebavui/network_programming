// OverlappedEventServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

	// Chap nhan ket noi va truyen nhan du lieu su dung doi tuong overlapped
	SOCKET client = accept(listener, NULL, NULL);

	OVERLAPPED overlapped;	
	WSAEVENT receivedEvent = WSACreateEvent();	// Tao doi tuong event
	memset(&overlapped, 0, sizeof(overlapped));	// Khoi tao cau truc overlapped
	overlapped.hEvent = receivedEvent;			// Gan event voi cau truc overlapped

	char buf[256];
	WSABUF databuf;					// Khai bao cau truc buffer nhan du lieu
	databuf.buf = buf;
	databuf.len = sizeof(buf);

	DWORD bytesReceived;
	DWORD flags = 0;
	int ret;

	while (1)
	{
		// Gui yeu cau nhan du lieu
		ret = WSARecv(client, &databuf, 1, &bytesReceived, &flags, &overlapped, FALSE);
		if (ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			// Neu loi la WSA_IO_PENDING nghia la dang cho du lieu
			if (ret != WSA_IO_PENDING)
			{
				printf("WSARecv() failed: error code %d\n", ret);
				break;
			}
		}

		// Doi den khi co du lieu duoc gui den
		ret = WSAWaitForMultipleEvents(1, &receivedEvent, FALSE, 5000, FALSE);
		if (ret == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents() failed\n");
			break;
		}
		if (ret == WSA_WAIT_TIMEOUT)
		{
			printf("Timed out\n");
			continue;
		}

		WSAResetEvent(receivedEvent);

		// Nhan du lieu vao buffer
		ret = WSAGetOverlappedResult(client, &overlapped, &bytesReceived, FALSE, &flags);
		if (bytesReceived == 0)
			break;

		// Xu ly du lieu
		buf[bytesReceived] = 0;
		printf("Received: %s\n", buf);
	}

	closesocket(client);
	closesocket(listener);
	WSACleanup();
}
