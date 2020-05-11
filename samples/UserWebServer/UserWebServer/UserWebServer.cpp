// CompletionPortServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)
#pragma warning(disable:6054)

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
int ProcessRequest(SOCKET, char*);
int SendHeader(SOCKET, const char*);
int SendFile(SOCKET, const char*);
int ProcessSignUp(char*);
int CheckUsername(char*);
int InsertUser(char*, char*, char*, char*);
int ProcessSignIn(char*);
int CheckUsernamePassword(char*, char*);
int SendUserData(SOCKET);

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
		CreateIoCompletionPort((HANDLE)client, completionPort, (ULONG_PTR)pHandle, 0);

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
		bool ret = GetQueuedCompletionStatus(completionPort, &bytesReceived, (PULONG_PTR)&pHandle, (LPOVERLAPPED*)&pIO, INFINITE);
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

		ProcessRequest(pHandle->Socket, pIO->buf);
		
		closesocket(pHandle->Socket);
		GlobalFree(pHandle);	 		// Giai phong bo nho da cap phat
		GlobalFree(pIO);

		// Gui yeu cau du lieu tiep theo
		// WSARecv(pHandle->Socket, &(pIO->DataBuf), 1, &bytesReceived, &flags, &(pIO->Overlapped), NULL);
	}
}

int ProcessRequest(SOCKET client, char* req)
{
	char cmd[16];
	char dir[256];

	int ret = sscanf(req, "%s %s", cmd, dir);
	if (ret != 2)
	{
		return -1;
	}
	if (strcmp(cmd, "GET") != 0 && strcmp(cmd, "POST") != 0)
	{
		return -1;
	}

	if (strcmp(cmd, "GET") == 0 && strcmp(dir, "/signup") == 0)
	{
		SendHeader(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		SendFile(client, "signup.html");
	}
	else if (strcmp(cmd, "POST") == 0 && strcmp(dir, "/signup") == 0)
	{
		if (ProcessSignUp(req))
		{
			SendHeader(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
			SendFile(client, "signup_error.html");
		}
		else
		{
			SendHeader(client, "HTTP/1.1 303 Sign up success\r\nLocation: /signin\r\n\r\n");
		}
	}
	else if (strcmp(cmd, "GET") == 0 && strcmp(dir, "/signin") == 0)
	{
		SendHeader(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		SendFile(client, "signin.html");
	}
	else if (strcmp(cmd, "POST") == 0 && strcmp(dir, "/signin") == 0)
	{
		if (ProcessSignIn(req))
		{
			SendHeader(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
			SendFile(client, "signin_error.html");
		}
		else
		{
			SendHeader(client, "HTTP/1.1 303 Sign in success\r\nLocation: /users\r\n\r\n");
		}
	}
	else if (strcmp(cmd, "GET") == 0 && strcmp(dir, "/users") == 0)
	{
		SendHeader(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
		SendUserData(client);
	}

	return 0;
}

int SendHeader(SOCKET client, const char* buf)
{
	send(client, buf, strlen(buf), 0);
	return 0;
}

int SendFile(SOCKET client, const char* filename)
{
	char buf[2048];
	int ret;

	char fbuf[1024];
	GetModuleFileNameA(NULL, fbuf, sizeof(fbuf));
	int pos = strlen(fbuf) - 1;
	while (fbuf[pos] != '\\') pos--;
	fbuf[pos + 1] = 0;
	strcat(fbuf, filename);

	FILE* f = fopen(fbuf, "rb");
	while (1)
	{
		ret = fread(buf, 1, sizeof(buf), f);
		if (ret <= 0)
			break;
		send(client, buf, ret, 0);
	}
	fclose(f);

	return 0;
}

int ProcessSignIn(char* req)
{
	char* crlf = strstr(req, "\r\n\r\n");
	if (crlf == NULL)
		return -1;

	char* username, * password;

	char* pItem = strtok(crlf + 4, "&");
	username = strstr(pItem, "=") + 1;
	pItem = strtok(NULL, "&");
	password = strstr(pItem, "=") + 1;

	return CheckUsernamePassword(username, password);
}

int CheckUsernamePassword(char* username, char* password)
{
	FILE* f = fopen("C:\\Test\\users.txt", "r");
	if (f == NULL)
		return -2;
	char line[256], uname[16], fname[32], email[64], pwd[64];
	int check = -1, ret;
	while (!feof(f))
	{
		fgets(line, sizeof(line), f);
		ret = sscanf(line, "%s %s %s %s", uname, fname, email, pwd);
		if (ret != 4) continue;
		if (strcmp(username, uname) == 0 && strcmp(password, pwd) == 0)
		{
			check = 0;
			break;
		}
	}
	fclose(f);
	return check;
}

int ProcessSignUp(char* req)
{
	char* crlf = strstr(req, "\r\n\r\n");
	if (crlf == NULL)
		return -1;

	char* username, * fullname, * email, * password;

	char* pItem = strtok(crlf + 4, "&");
	username = strstr(pItem, "=") + 1;

	// Check username is available to add
	if (CheckUsername(username))
	{
		return -1;
	}

	pItem = strtok(NULL, "&");
	fullname = strstr(pItem, "=") + 1;

	pItem = strtok(NULL, "&");
	email = strstr(pItem, "=") + 1;

	pItem = strtok(NULL, "&");
	password = strstr(pItem, "=") + 1;

	InsertUser(username, fullname, email, password);

	return 0;
}

int CheckUsername(char* username)
{
	FILE* f = fopen("C:\\Test\\users.txt", "r");
	if (f == NULL)
		return -1;
	char line[256];
	int check = 0;
	while (!feof(f))
	{
		fgets(line, sizeof(line), f);
		if (strncmp(line, username, strlen(username)) == 0)
		{
			check = -1;
			break;
		}
	}
	fclose(f);
	return check;
}

int InsertUser(char* username, char* fullname, char* email, char* password)
{
	FILE* f = fopen("C:\\Test\\users.txt", "a");
	if (f == NULL)
		return -1;
	fprintf(f, "%s\t%s\t%s\t%s\n", username, fullname, email, password);
	fclose(f);
	return 0;
}

int SendUserData(SOCKET client)
{
	char buf[1024];
	char uname[16], fname[32], email[64];

	char line[256];
	char fbuf[256];
	GetModuleFileNameA(NULL, fbuf, sizeof(fbuf));
	int pos = strlen(fbuf) - 1;
	while (fbuf[pos] != '\\') pos--;
	fbuf[pos + 1] = 0;
	strcat(fbuf, "users.html");

	int ret;

	FILE* f = fopen(fbuf, "r");
	while (!feof(f))
	{
		memset(line, 0, sizeof(line));
		fgets(line, sizeof(line), f);
		if (strstr(line, "[user_list]") != NULL)
		{
			// Send user's data
			FILE* f1 = fopen("c:\\test\\users.txt", "r");
			while (!feof(f1))
			{
				memset(buf, 0, sizeof(buf));
				fgets(buf, sizeof(buf), f1);
				ret = sscanf(buf, "%s %s %s", uname, fname, email);
				if (ret != 3)
					continue;

				sprintf(buf, "<tr><td>%s</td><td>%s</td><td>%s</td>", uname, fname, email);
				send(client, buf, strlen(buf), 0);
				sprintf(buf, "<td><button type=\"button\" class=\"btn\"><i class=\"fas fa-edit\"></i></button></td>");
				send(client, buf, strlen(buf), 0);
				sprintf(buf, "<td><button type=\"button\" class=\"btn\"><i class=\"far fa-trash-alt\"></i></button></td></tr>");
				send(client, buf, strlen(buf), 0);
			}
			fclose(f1);
		}
		else
			send(client, line, strlen(line), 0);
	}
	fclose(f);

	return 0;
}