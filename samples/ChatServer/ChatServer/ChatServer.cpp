// ChatServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

typedef struct
{
	SOCKET client;
	char* id;
} CLIENT_INFO;

// Luu thong tin client dang nhap thanh cong
CLIENT_INFO clients[64];
int numClients = 0;

CRITICAL_SECTION cs;

DWORD WINAPI ClientThread(LPVOID);
void RemoveClient(SOCKET);
BOOL ProcessConnect(SOCKET, char*, char*);
void ProcessSend(SOCKET, char*, char*);
void ProcessList(SOCKET, char*);
BOOL ProcessDisconnect(SOCKET, char*, char*);
void SendNotification(SOCKET, char*, int);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	InitializeCriticalSection(&cs);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	while (1)
	{
		printf("Dang cho cac ket noi...\n");
		SOCKET client = accept(listener, NULL, NULL);
		printf("Ket noi moi: %d", client);

		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}

	DeleteCriticalSection(&cs);
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET*)lpParam;

	int ret;
	char buf[256];
	char cmd[16], id[256];
	BOOL isRegistered = FALSE;

	const char* helloMsg = "Dang nhap theo cu phap \"CONNECT [your_id]\".\n";
	send(client, helloMsg, strlen(helloMsg), 0);

	while (1)
	{
		// Nhan du lieu
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
		{
			if (isRegistered)
			{
				SendNotification(client, id, 2);
				RemoveClient(client);
			}
			
			closesocket(client);
			return 0;
		}

		buf[ret] = 0;
		printf("Received: %s\n", buf);

		memset(cmd, 0, sizeof(cmd));
		sscanf(buf, "%s", cmd);

		if (!isRegistered) // Chua dang nhap
		{
			// Kiem tra cu phap
			if (strcmp(cmd, "CONNECT") == 0)
				isRegistered = ProcessConnect(client, buf, id);
			else
			{
				const char* errorMsg = "ERROR Lenh khong hop le.\n";
				send(client, errorMsg, strlen(errorMsg), 0);
			}
		}
		else // Da dang nhap
		{
			if (strcmp(cmd, "SEND") == 0)
				ProcessSend(client, id, buf);
			else if (strcmp(cmd, "LIST") == 0)
				ProcessList(client, buf);
			else if (strcmp(cmd, "DISCONNECT") == 0)
				isRegistered = !ProcessDisconnect(client, id, buf);
			else
			{
				const char* errorMsg = "ERROR Lenh khong hop le.\n";
				send(client, errorMsg, strlen(errorMsg), 0);
			}
		}
	}
}

void RemoveClient(SOCKET client)
{
	int i = 0;
	for (; i < numClients; i++)
		if (clients[i].client == client)
			break;
	if (i < numClients)
	{
		EnterCriticalSection(&cs);
		if (i < numClients - 1)
			clients[i] = clients[numClients - 1];
		numClients--;
		LeaveCriticalSection(&cs);
	}
}

BOOL ProcessConnect(SOCKET client, char* buf, char* id)
{
	char tmp[32];
	int ret = sscanf(buf + strlen("CONNECT"), "%s %s", id, tmp);
	if (ret == 1)
	{
		int i;
		for (i = 0; i < numClients; i++)
			if (strcmp(id, clients[i].id) == 0)
				break;

		if (i < numClients)
		{
			const char* errorMsg = "CONNECT ERROR ID da duoc su dung. Hay chon ID khac.\n";
			send(client, errorMsg, strlen(errorMsg), 0);
			return FALSE;
		}
		else if (strcmp(id, "ALL") == 0)
		{
			const char* errorMsg = "CONNECT ERROR ID khong hop le. Hay chon ID khac.\n";
			send(client, errorMsg, strlen(errorMsg), 0);
			return FALSE;
		}
		else
		{
			const char* okMsg = "CONNECT OK\n";
			send(client, okMsg, strlen(okMsg), 0);

			EnterCriticalSection(&cs);
			clients[numClients].id = id;
			clients[numClients].client = client;
			numClients++;
			LeaveCriticalSection(&cs);

			SendNotification(client, id, 1);

			return TRUE;
		}
	}
	else
	{
		const char* errorMsg = "CONNECT ERROR Sai cu phap lenh CONNECT\n";
		send(client, errorMsg, strlen(errorMsg), 0);
		return FALSE;
	}
}

void ProcessSend(SOCKET client, char* id, char* buf)
{
	char sendBuf[256];
	char target[32];
	int ret = sscanf(buf + strlen("SEND"), "%s", target);

	if (ret == -1)
	{
		const char* errorMsg = "SEND ERROR Sai cu phap lenh SEND.\n";
		send(client, errorMsg, strlen(errorMsg), 0);
	}
	else
	{
		char* msgPointer = buf + strlen("SEND") + strlen(target) + 2;
		if (strcmp(target, "ALL") == 0)
		{
			sprintf(sendBuf, "MESSAGE_ALL %s %s", id, msgPointer);
			for (int i = 0; i < numClients; i++)
				if (clients[i].client != client)
					send(clients[i].client, sendBuf, strlen(sendBuf), 0);

			const char* okMsg = "SEND OK\n";
			send(client, okMsg, strlen(okMsg), 0);
		}
		else
		{
			sprintf(sendBuf, "MESSAGE %s %s", id, msgPointer);
			int i;
			for (i = 0; i < numClients; i++)
				if (strcmp(clients[i].id, target) == 0) break;
			if (i < numClients)
			{
				send(clients[i].client, sendBuf, strlen(sendBuf), 0);
				const char* okMsg = "SEND OK\n";
				send(client, okMsg, strlen(okMsg), 0);
			}
			else
			{
				const char* errorMsg = "SEND ERROR Khong tim thay user nhan tin nhan.\n";
				send(client, errorMsg, strlen(errorMsg), 0);
			}
		}
	}
}

void ProcessList(SOCKET client, char* buf)
{
	char tmp[32];
	int ret = sscanf(buf + strlen("LIST"), "%s", tmp);
	if (ret != -1)
	{
		const char* errorMsg = "LIST ERROR Sai cu phap lenh LIST.\n";
		send(client, errorMsg, strlen(errorMsg), 0);
	}
	else
	{
		char sendBuf[256] = "LIST OK ";
		for (int i = 0; i < numClients; i++)
		{
			strcat(sendBuf, clients[i].id);
			strcat(sendBuf, ",");
		}

		// Xoa dau phay cuoi cung
		int len = strlen(sendBuf);
		sendBuf[len - 1] = '\n';

		send(client, sendBuf, strlen(sendBuf), 0);
	}
}

BOOL ProcessDisconnect(SOCKET client, char* id, char* buf)
{
	char tmp[32];
	int ret = sscanf(buf + strlen("DISCONNECT"), "%s", tmp);
	if (ret != -1)
	{
		const char* errorMsg = "DISCONNECT ERROR Sai cu phap lenh DISCONNECT.\n";
		send(client, errorMsg, strlen(errorMsg), 0);
		return FALSE;
	}
	else
	{
		SendNotification(client, id, 2);
		const char* okMsg = "DISCONNECT OK\n";
		send(client, okMsg, strlen(okMsg), 0);
		RemoveClient(client);
		return TRUE;
	}
}

void SendNotification(SOCKET client, char* id, int type)
{
	char sendBuf[32];
	if (type == 1)
	{
		sprintf(sendBuf, "USER_CONNECT %s\n", id);
		for (int i = 0; i < numClients; i++)
			if (clients[i].client != client)
				send(clients[i].client, sendBuf, strlen(sendBuf), 0);
	}
	else if (type == 2)
	{
		sprintf(sendBuf, "USER_DISCONNECT %s\n", id);
		for (int i = 0; i < numClients; i++)
			if (clients[i].client != client)
				send(clients[i].client, sendBuf, strlen(sendBuf), 0);
	}
}