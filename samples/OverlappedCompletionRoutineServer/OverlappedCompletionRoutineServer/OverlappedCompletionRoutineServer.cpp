// OverlappedCompletionRoutineServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

// Khai bao bien toan cuc
SOCKET client;
char buf[256];
WSABUF databuf;

// Protype cua ham callback
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

	// Chap nhan ket noi va truyen nhan du lieu
	client = accept(listener, NULL, NULL);

	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));

	databuf.buf = buf;
	databuf.len = sizeof(buf);

	DWORD bytesReceived;
	DWORD flags = 0;

	// Gui yeu cau du lieu lan dau
	int ret = WSARecv(client, &databuf, 1, &bytesReceived, &flags, &overlapped, CompletionRoutine);

	// Chuyen luong sang trang thai alertable
	while (1)
	{
		ret = SleepEx(5000, TRUE);
		if (ret == 0)
		{
			printf("Timed out\n");
		}
	}

	closesocket(client);
	closesocket(listener);
	WSACleanup();
}


void CALLBACK CompletionRoutine(DWORD dwError, DWORD dwBytesReceived, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	// Kiem tra loi
	if (dwError != 0 || dwBytesReceived == 0)
	{
		closesocket(client);
		return;
	}

	// Xu ly du lieu nhan duoc
	buf[dwBytesReceived] = 0;
	printf("Received: %s\n", buf);

	// Gui yeu cau du lieu tiep theo
	dwFlags = 0;
	int ret = WSARecv(client, &databuf, 1, &dwBytesReceived, &dwFlags, lpOverlapped, CompletionRoutine);
}