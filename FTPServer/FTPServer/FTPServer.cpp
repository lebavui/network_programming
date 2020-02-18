#include <stdio.h>
#include <time.h>
#include <winsock2.h>

typedef struct {
	SOCKET controlSocket;
	SOCKET dataSocket;
	int dataPort;
	char rootDir[64];
	char curDir[256];

	char lastCommand[8];
	char lastCommandParams[64];
} CLIENT_INFO;

DWORD WINAPI ClientThread(LPVOID);
DWORD WINAPI DataThread(LPVOID);

void ProcessCommand(CLIENT_INFO*, char*);
void ResponseCommand(CLIENT_INFO*, const char*);

int GetPassiveDataPort(CLIENT_INFO*);
SOCKET OpenDataConnection(int);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2121);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	while (1)
	{
		SOCKET client = accept(listener, NULL, NULL);
		printf("New client accepted: %d\n", client);
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET*)lpParam;
	char buf[256];
	int ret;
	int dataPort = 0;

	CLIENT_INFO clientInfo;
	clientInfo.controlSocket = client;
	strcpy(clientInfo.rootDir, "C:/FTPServer/user1");
	strcpy(clientInfo.curDir, "/");

	ResponseCommand(&clientInfo, "220 Service ready for new user.\n");

	while (1)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
		if (ret < sizeof(buf))
			buf[ret] = 0;

		printf("Received: %s\n", buf);

		ProcessCommand(&clientInfo, buf);
	}

	closesocket(client);
	return 0;
}

void ProcessCommand(CLIENT_INFO* pClientInfo, char* buf)
{
	char cmd[16] = "";
	int ret = sscanf(buf, "%s", cmd);
	if (ret == 0)
		return;

	// Chuyen cac ky tu chu thuong thanh chu hoa
	for (int i = 0; i < strlen(cmd); i++)
		if (islower(cmd[i]))
			cmd[i] = toupper(cmd[i]);

	// Tach param
	char params[64];
	int paramOffset = strlen(cmd) + 1;
	int paramLength = strlen(buf) - strlen(cmd) - 3;
	if (paramLength > 0)
		memcpy(params, buf + paramOffset, paramLength);
	if (paramLength < 0)
		paramLength = 0;
	params[paramLength] = 0;

	// So sanh va xu ly cac lenh FTP

	if (strcmp(cmd, "USER") == 0)
		ResponseCommand(pClientInfo, "331 User name okay, need password.\n");
	else if (strcmp(cmd, "PASS") == 0)
		ResponseCommand(pClientInfo, "230 User logged in, proceed.\n");
	else if (strcmp(cmd, "TYPE") == 0)
		ResponseCommand(pClientInfo, "200 Command okay.\n");
	else if (strcmp(cmd, "MODE") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "PASV") == 0)
		pClientInfo->dataPort = GetPassiveDataPort(pClientInfo);
	else if (strcmp(cmd, "LIST") == 0)
	{
		ResponseCommand(pClientInfo, "150 File status okay; about to open data connection.\n");
		CreateThread(0, 0, DataThread, pClientInfo, 0, 0);
	}		
	else if (strcmp(cmd, "PORT") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "PWD") == 0)
	{
		char reply[256];
		sprintf(reply, "257 '%s' is current directory.\n", pClientInfo->curDir);
		ResponseCommand(pClientInfo, reply);
	}		
	else if (strcmp(cmd, "CWD") == 0)
	{
		// TODO: check whether folder is existed

		// update current directory
		if (params[0] == '/')
			strcpy(pClientInfo->curDir, params);
		else if (strncmp(params, "..", 1) == 0)
		{
			// change to parent of the current directory
			int pos = strlen(pClientInfo->curDir) - 1;
			while (pClientInfo->curDir[pos] != '/') pos--;
			if (pos == 0)
				pClientInfo->curDir[1] = 0;
			else
				pClientInfo->curDir[pos] = 0;
		}
		else
		{
			if (strcmp(pClientInfo->curDir, "/") != 0)
				strcat(pClientInfo->curDir, "/");
			strcat(pClientInfo->curDir, params);
		}			

		char reply[256];
		sprintf(reply, "250 CDW completed. '%s' is current directory.\n", pClientInfo->curDir);
		ResponseCommand(pClientInfo, reply);
	}
	else if (strcmp(cmd, "CDUP") == 0)
	{
		// change to parent of the current directory
		int pos = strlen(pClientInfo->curDir) - 1;
		while (pClientInfo->curDir[pos] != '/') pos--;
		if (pos == 0)
			pClientInfo->curDir[1] = 0;
		else
			pClientInfo->curDir[pos] = 0;
		
		char reply[256];
		sprintf(reply, "200 CDUP completed. '%s' is current directory.\n", pClientInfo->curDir);
		ResponseCommand(pClientInfo, reply);
	}
	else if (strcmp(cmd, "MKD") == 0)
	{
		char path[256];
		strcpy(path, pClientInfo->rootDir);
		strcat(path, pClientInfo->curDir);
		if (strcmp(pClientInfo->curDir, "/") != 0)
			strcat(path, "/");
		strcat(path, params);
		
		// check whether folder is existed
		if (!CreateDirectoryA(path, NULL))
		{
			ResponseCommand(pClientInfo, "550 Requested action not taken.\n");
		}
		else
		{
			char reply[256];
			sprintf(reply, "257 \"%s\" directory created.\n", params);
			ResponseCommand(pClientInfo, reply);
		}
	}
	else if (strcmp(cmd, "RMD") == 0)
	{
		char path[256];
		strcpy(path, pClientInfo->rootDir);
		strcat(path, pClientInfo->curDir);
		if (strcmp(pClientInfo->curDir, "/") != 0)
			strcat(path, "/");
		strcat(path, params);

		// check whether folder is existed
		if (!RemoveDirectoryA(path))
		{
			printf("Error Code: %d\n", GetLastError());
			ResponseCommand(pClientInfo, "550 Requested action not taken.\n");
		}
		else
		{
			char reply[256];
			sprintf(reply, "250 \"%s\" directory removed.\n", params);
			ResponseCommand(pClientInfo, reply);
		}
	}
	else if (strcmp(cmd, "RMD") == 0)
	{
		char path[256];
		strcpy(path, pClientInfo->rootDir);
		strcat(path, pClientInfo->curDir);
		if (strcmp(pClientInfo->curDir, "/") != 0)
			strcat(path, "/");
		strcat(path, params);

		// check whether folder is existed
		if (!RemoveDirectoryA(path))
		{
			printf("Error Code: %d\n", GetLastError());
			ResponseCommand(pClientInfo, "550 Requested action not taken.\n");
		}
		else
		{
			char reply[256];
			sprintf(reply, "250 \"%s\" directory removed.\n", params);
			ResponseCommand(pClientInfo, reply);
		}
	}
	else if (strcmp(cmd, "DELE") == 0)
	{
		char path[256];
		strcpy(path, pClientInfo->rootDir);
		strcat(path, pClientInfo->curDir);
		if (strcmp(pClientInfo->curDir, "/") != 0)
			strcat(path, "/");
		strcat(path, params);
		
		// check whether folder is existed
		if (!DeleteFileA(path))
		{
			printf("Error Code: %d\n", GetLastError());
			ResponseCommand(pClientInfo, "550 Requested action not taken.\n");			
		}
		else
		{
			char reply[256];
			sprintf(reply, "250 \"%s\" file deleted.\n", params);
			ResponseCommand(pClientInfo, reply);
		}
	}
	else if (strcmp(cmd, "RNFR") == 0)
		ResponseCommand(pClientInfo, "350 Requested file action pending further information.\n");
	else if (strcmp(cmd, "RNTO") == 0)
	{
		if (strcmp(pClientInfo->lastCommand, "RNFR") == 0)
		{
			char pathFrom[256];
			strcpy(pathFrom, pClientInfo->rootDir);
			strcat(pathFrom, pClientInfo->curDir);
			if (strcmp(pClientInfo->curDir, "/") != 0)
				strcat(pathFrom, "/");
			strcat(pathFrom, pClientInfo->lastCommandParams);

			char pathTo[256];
			strcpy(pathTo, pClientInfo->rootDir);
			strcat(pathTo, pClientInfo->curDir);
			if (strcmp(pClientInfo->curDir, "/") != 0)
				strcat(pathTo, "/");
			strcat(pathTo, params);

			if (!MoveFileA(pathFrom, pathTo))
			{
				printf("Error Code: %d\n", GetLastError());
				ResponseCommand(pClientInfo, "553 Requested action not taken.\n");
			}
			else
			{
				ResponseCommand(pClientInfo, "250 Filename updated.\n");
			}
		}
		else
			ResponseCommand(pClientInfo, "503 Bad sequence of commands.\n");
	}
	else if (strcmp(cmd, "STRU") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "RETR") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "STOR") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "NOOP") == 0)
		ResponseCommand(pClientInfo, "200 Command okay.\n");
	else if (strcmp(cmd, "QUIT") == 0)
	{
		ResponseCommand(pClientInfo, "221 Service closing control connection.\n");
		closesocket(pClientInfo->controlSocket);
	}
	else
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");

	strcpy(pClientInfo->lastCommand, cmd);
	strcpy(pClientInfo->lastCommandParams, params);
}

void ResponseCommand(CLIENT_INFO* pClientInfo, const char* msg)
{
	send(pClientInfo->controlSocket, msg, strlen(msg), 0);
	printf("LOG: %s", msg);
}

int GetPassiveDataPort(CLIENT_INFO* pClientInfo)
{
	int port;
	int hiByte, loByte;
	
	srand(GetTickCount64());
	hiByte = rand() % 9 + 200;  // high byte from 200 to 208
	loByte = rand() % 256;		// low byte from 0 to 255
	port = hiByte * 256 + loByte;

	pClientInfo->dataPort = port;

	printf("Data Port: %d\n", port);

	char reply[256];
	sprintf(reply, "227 Entering Passive Mode (127,0,0,1,%d,%d).\n", hiByte, loByte);
	ResponseCommand(pClientInfo, reply);

	return port;
}

SOCKET OpenDataConnection(int dataPort)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(dataPort);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	SOCKET dataClient = accept(listener, NULL, NULL);
	printf("New data connection accepted: %d\n", dataClient);

	if (dataClient == INVALID_SOCKET)
	{
		int errorCode = GetLastError();
		printf("Error Code: %d\n", errorCode);
	}

	return dataClient;
}

DWORD WINAPI DataThread(LPVOID lpParam)
{
	CLIENT_INFO* pClientInfo = (CLIENT_INFO*)lpParam;
	SOCKET dataClient = OpenDataConnection(pClientInfo->dataPort);

	char path[256];
	strcpy(path, pClientInfo->rootDir);
	strcat(path, pClientInfo->curDir);
	if (strcmp(pClientInfo->curDir, "/") != 0)
		strcat(path, "/");
	strcat(path, "*.*");

	// Get folder content and send to client
	WIN32_FIND_DATAA DATA;
	HANDLE h = FindFirstFileA(path, &DATA);
	do {
		char sendBuf[256] = "";

		if (strcmp(DATA.cFileName, ".") == 0 || strcmp(DATA.cFileName, "..") == 0)
			continue;

		SYSTEMTIME updatedTime;
		FileTimeToSystemTime(&DATA.ftLastWriteTime, &updatedTime);
		char timeBuf[64];
		sprintf_s(timeBuf, sizeof(timeBuf), "modify=%04d%02d%02d%02d%02d%02d;", updatedTime.wYear, updatedTime.wMonth, updatedTime.wDay,
			updatedTime.wHour, updatedTime.wMinute, updatedTime.wSecond);

		if (DATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			strcat(sendBuf, "type=dir;");
			strcat(sendBuf, timeBuf);
			strcat(sendBuf, " ");
			strcat(sendBuf, DATA.cFileName);
		}
		else 
		{
			strcat(sendBuf, "type=file;");
			strcat(sendBuf, timeBuf);
			char sizeBuf[64];
			double fileSize = (double)DATA.nFileSizeHigh * (double)MAXDWORD + (double)DATA.nFileSizeLow;
			sprintf_s(sizeBuf, sizeof(sizeBuf), "size=%.0f;", fileSize);
			strcat(sendBuf, sizeBuf);
			strcat(sendBuf, " ");
			strcat(sendBuf, DATA.cFileName);
		}
		strcat(sendBuf, "\r\n");
		send(dataClient, sendBuf, strlen(sendBuf), 0);
	} while (FindNextFileA(h, &DATA));
	
	FindClose(h);

	ResponseCommand(pClientInfo, "226 Closing data connection.\n");

	closesocket(dataClient);
	return 0;
}