// MultiThreadOverlappedCompletionRoutineServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

typedef struct
{
	OVERLAPPED overlapped;
	SOCKET socket;
	char buf[256];
	WSABUF databuf;
} CLIENT_INFO;

// Protype cua ham callback
DWORD WINAPI ClientThread(LPVOID);
void CALLBACK CompletionRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

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
		// Chap nhan ket noi va truyen nhan du lieu
		SOCKET client = accept(listener, NULL, NULL);

		printf("New client accepted: %d\n", client);
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}

	closesocket(listener);
	WSACleanup();
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	CLIENT_INFO client;

	client.socket = *(SOCKET*)lpParam;
	memset(&client.overlapped, 0, sizeof(client.overlapped));

	client.databuf.buf = client.buf;
	client.databuf.len = sizeof(client.buf);

	DWORD bytesReceived;
	DWORD flags = 0;

	// Gui yeu cau du lieu lan dau
	int ret = WSARecv(client.socket, &client.databuf, 1, &bytesReceived, &flags, &client.overlapped, CompletionRoutine);

	// Chuyen luong sang trang thai alertable
	while (1)
	{
		ret = SleepEx(5000, TRUE);
		if (ret == 0)
		{
			printf("Timed out\n");
		}
	}

	closesocket(client.socket);
}


void CALLBACK CompletionRoutine(DWORD dwError, DWORD dwBytesReceived, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	CLIENT_INFO client = *(CLIENT_INFO*)lpOverlapped;
	// Kiem tra loi
	if (dwError != 0 || dwBytesReceived == 0)
	{
		closesocket(client.socket);
		return;
	}

	// Xu ly du lieu nhan duoc
	client.buf[dwBytesReceived] = 0;
	printf("Received: %s\n", client.buf);

	// Gui yeu cau du lieu tiep theo
	dwFlags = 0;
	int ret = WSARecv(client.socket, &client.databuf, 1, &dwBytesReceived, &dwFlags, lpOverlapped, CompletionRoutine);
}