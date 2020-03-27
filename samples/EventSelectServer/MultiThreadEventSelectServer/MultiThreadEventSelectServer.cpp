// MultiThreadEventSelectServer.cpp : Tao server theo mo hinh WSAEventSelect
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

#define MAX_EVENTS 80

// Khai bao mang socket va event
SOCKET sockets[MAX_EVENTS];
WSAEVENT events[MAX_EVENTS];
int numEvents = 0;

DWORD WINAPI WorkerThread(LPVOID);

int main()
{

	// Khoi tao thu vien Winsock
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Khai bao dia chi cuc bo cua server
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	// Khai bao socket, gan voi dia chi va chuyen sang trang thai cho ket noi
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	while (1)
	{
		// Chap nhan ket noi
		SOCKET client = accept(listener, NULL, NULL);
		// Kiem tra neu vuot qua so luong su kien toi da thi bo qua
		if (numEvents > MAX_EVENTS)
		{
			printf("Too many connections");
			closesocket(client);
			continue;
		}

		// Neu khong vuot qua thi tao doi tuong su kien
		WSAEVENT newEvent = WSACreateEvent();
		// Gan voi socket va dang ky su kien FD_READ va FD_CLOSE
		WSAEventSelect(client, newEvent, FD_READ | FD_CLOSE);

		// Them vao mang
		sockets[numEvents] = client;
		events[numEvents] = newEvent;
		numEvents++;

		printf("Client accepted: %d\n", client);
		if (numEvents % 64 == 1)
		{
			int threadIndex = numEvents - 1; // 0, 64, 128, ...
			CreateThread(0, 0, WorkerThread, &threadIndex, 0, 0);
		}
	}	
}

DWORD WINAPI WorkerThread(LPVOID lpParam)
{
	int threadIndex = *(int*)lpParam;

	// Khai bao bien chua ket qua 
	WSANETWORKEVENTS networkEvents;
	char buf[256];
	int ret, idx;

	while (1)
	{
		int numEventsToWait = numEvents - threadIndex;
		if (numEventsToWait > 64)
			numEventsToWait = 64;

		// Cho cho den khi 1 su kien duoc bao hieu
		ret = WSAWaitForMultipleEvents(numEventsToWait, &events[threadIndex], FALSE, WSA_INFINITE, FALSE);
		idx = ret - WSA_WAIT_EVENT_0;

		// Duyet tu su kien duoc bao hieu den cac su kien tiep theo trong mang
		for (int i = idx; i < numEventsToWait; i++)
		{
			int j = i + threadIndex;

			// Kiem tra su kien co xay ra hay khong
			ret = WSAWaitForMultipleEvents(1, &events[j], TRUE, 0, FALSE);
			if (ret == WSA_WAIT_FAILED || ret == WSA_WAIT_TIMEOUT)
				continue;

			// Khoi tao lai doi tuong su kien
			WSAResetEvent(events[j]);

			// Lay ket qua
			WSAEnumNetworkEvents(sockets[j], events[j], &networkEvents);

			// Xu ly su kien co du lieu den server
			if (networkEvents.lNetworkEvents & FD_READ)
			{
				if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					printf("FD_READ failed with error code %d\n", networkEvents.iErrorCode[FD_READ_BIT]);
					continue;
				}

				ret = recv(sockets[j], buf, sizeof(buf), 0);

				buf[ret] = 0;
				printf("Received from %d: %s\n", sockets[j], buf);

			}

			// Xu ly su kien ngat ket noi
			if (networkEvents.lNetworkEvents & FD_CLOSE)
			{
				if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
				{
					printf("FD_CLOSE failed with error code %d\n", networkEvents.iErrorCode[FD_CLOSE_BIT]);
				}

				printf("Client disconnected: %d\n", sockets[j]);
			}
		}
	}
}
