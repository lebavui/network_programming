#include <stdio.h>
#include <time.h>
#include <winsock2.h>

typedef struct {
	SOCKET controlSocket;
	SOCKET dataSocket;
	int dataPort;
	char curDir[256];
} CLIENT_INFO;

DWORD WINAPI ClientThread(LPVOID);
DWORD WINAPI DataThread(LPVOID);

void ProcessCommand(CLIENT_INFO*, char*);
void ResponseCommand(CLIENT_INFO*, const char*);

int GetPassiveDataPort(CLIENT_INFO*);
SOCKET OpenDataConnection(int);

int main()
{
	srand(time(NULL));

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
	else if (strcmp(cmd, "STRU") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "RETR") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "STOR") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else if (strcmp(cmd, "NOOP") == 0)
		ResponseCommand(pClientInfo, "200 Command okay.\n");
	else if (strcmp(cmd, "QUIT") == 0)
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
	else
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");
}

void ResponseCommand(CLIENT_INFO* pClientInfo, const char* msg)
{
	send(pClientInfo->controlSocket, msg, strlen(msg), 0);
}

int GetPassiveDataPort(CLIENT_INFO* pClientInfo)
{
	int port;
	int hiByte, loByte;
	
	hiByte = rand() % 9 + 200;  // high byte from 200 to 208
	loByte = rand() % 256;		// low byte from 0 to 255
	port = hiByte * 256 + loByte;

	pClientInfo->dataPort = port;

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

	return dataClient;
}

DWORD WINAPI DataThread(LPVOID lpParam)
{
	CLIENT_INFO* pClientInfo = (CLIENT_INFO*)lpParam;
	SOCKET dataClient = OpenDataConnection(pClientInfo->dataPort);

	char data[256] = "drwxr-xr-x 1 ftp ftp 0 Feb 05 10:11 hello1\r\n";
	send(dataClient, data, strlen(data), 0);
	
	ResponseCommand(pClientInfo, "226 Closing data connection.\n");

	closesocket(dataClient);
	return 0;
}