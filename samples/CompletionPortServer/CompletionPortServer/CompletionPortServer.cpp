// CompletionPortServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

// Khai bao cau truc du lieu socket
typedef struct _PER_HANDLE_DATA
{
	SOCKET Socket;
	SOCKADDR_STORAGE ClientAddr;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

// Khai bao cau truc du lieu overlapped
typedef struct _PER_IO_DATA
{
	OVERLAPPED Overlapped;
	WSABUF DataBuf;
	char buf[1024];
} PER_IO_DATA, * LPPER_IO_DATA;

DWORD WINAPI ServerWorkerThread(LPVOID);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Tao doi tuong CompletionPort
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Lay thong tin he thong
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// Tao cac worker thread ung voi so processor
	for (int i = 0; i < systemInfo.dwNumberOfProcessors; i++)
	{
		HANDLE hThread = CreateThread(NULL, 0, ServerWorkerThread, completionPort, 0, NULL);
		CloseHandle(hThread);
	}

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	while (1)
	{
		SOCKADDR_IN clientAddr;
		int clientAddrLen = sizeof(clientAddr);

		// Chap nhan ket noi
		SOCKET client = accept(listener, (SOCKADDR*)&clientAddr, &clientAddrLen);

		// Cap phat bo nho cho cau truc du lieu
		PER_HANDLE_DATA* pHandle = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));

		// Luu thong tin socket vao cau truc du lieu socket
		printf("Socket number %d connected\n", client);
		pHandle->Socket = client;
		memcpy(&pHandle->ClientAddr, &clientAddr, clientAddrLen);

		// Gan socket voi doi tuong CompletionPort
		CreateIoCompletionPort((HANDLE)client, completionPort, (DWORD)pHandle, 0);
		
		// Cap phat bo nho cho cau truc du lieu vao ra overlapped
		PER_IO_DATA* pIO = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));

		pIO->DataBuf.len = sizeof(pIO->buf);
		pIO->DataBuf.buf = pIO->buf;
		
		DWORD bytesReceived, flags = 0;
		// Gui yeu cau du lieu lan dau
		WSARecv(client, &(pIO->DataBuf), 1, &bytesReceived, &flags, &(pIO->Overlapped), NULL);
	}

	return 0;
}

// Worker thread xu ly viec cho du lieu va xu ly du lieu
DWORD WINAPI ServerWorkerThread(LPVOID lpParam)
{
	HANDLE completionPort = (HANDLE)lpParam;
	LPPER_HANDLE_DATA pHandle;
	LPPER_IO_DATA pIO;
	DWORD bytesReceived;
	DWORD flags = 0;

	while (TRUE)
	{	 // Cho den khi 1 yeu cau vao ra hoan tat
		bool ret = GetQueuedCompletionStatus(completionPort, &bytesReceived, (LPDWORD)&pHandle, (LPOVERLAPPED*)&pIO, INFINITE);
		if (bytesReceived == 0) // Neu ket noi bi ngat
		{
			closesocket(pHandle->Socket);	// Ngat ket noi
			GlobalFree(pHandle);	 		// Giai phong bo nho da cap phat
			GlobalFree(pIO);
			continue;
		}

		// Xu ly du lieu nhan duoc
		pIO->buf[bytesReceived] = 0;
		printf("Received Data: %s\n", pIO->buf);

		// Gui yeu cau du lieu tiep theo
		WSARecv(pHandle->Socket, &(pIO->DataBuf), 1, &bytesReceived, &flags, &(pIO->Overlapped), NULL);
	}
}