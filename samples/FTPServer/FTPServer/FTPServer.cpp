#include <stdio.h>
#include <time.h>
#include <winsock2.h>

// Dinh nghia cau truc du lieu de luu thong tin can thiet cho moi client
typedef struct {
	char username[32];
	char password[32];

	SOCKET controlSocket;		// Ket noi de nhan lenh va tra ve phan hoi
	SOCKET dataSocket;			// Ket noi de truyen nhan du lieu
	int dataPort;				// Cong ket noi o che do Passive
	char rootDir[64];			// Thu muc goc user
	char curDir[256];			// Thu muc hien tai

	char curCommandParams[64];	// Tham so cua lenh hien tai

	char prevCommand[8];		// Lenh truoc do
	char prevCommandParams[64];	// Tham so cua lenh truoc do
} CLIENT_INFO;

// Khai bao prototype cac ham phuc vu luong
DWORD WINAPI ClientThread(LPVOID);
DWORD WINAPI ResponseListCommandThread(LPVOID);
DWORD WINAPI ResponseDownloadCommandThread(LPVOID);
DWORD WINAPI ResponseUploadCommandThread(LPVOID);

// Khai bao cac ham tien ich
void ProcessCommand(CLIENT_INFO*, char*);
void ResponseCommand(CLIENT_INFO*, const char*);
char* GetPasswordOfUser(char*);
int GetPassiveDataPort(CLIENT_INFO*);
SOCKET OpenDataConnection(int);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Server cho o cong 2121
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2121);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	// Chap nhan nhieu ket noi theo mo hinh da luong
	while (1)
	{
		SOCKET client = accept(listener, NULL, NULL);
		printf("LOG: New client accepted: %d\n", client);
		CreateThread(0, 0, ClientThread, &client, 0, 0);
	}
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET client = *(SOCKET*)lpParam;
	char buf[256];
	int ret;
	int dataPort = 0;

	// Tao du lieu cho moi client ket noi den server
	CLIENT_INFO clientInfo;
	clientInfo.controlSocket = client;
	strcpy(clientInfo.curDir, "/");
	strcpy(clientInfo.username, "");
	strcpy(clientInfo.password, "");

	// Gui thong diep hello den client
	ResponseCommand(&clientInfo, "220 Service ready for new user.\n");

	// Nhan lenh tu client
	while (1)
	{
		ret = recv(client, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
		if (ret < sizeof(buf))
			buf[ret] = 0;

		printf("LOG: Received command: %s\n", buf);

		// Xu ly lenh
		ProcessCommand(&clientInfo, buf);
	}

	closesocket(client);
	return 0;
}

void ProcessCommand(CLIENT_INFO* pClientInfo, char* buf)
{
	// Tach phan command de xu ly
	char cmd[16] = "";
	int ret = sscanf(buf, "%s", cmd);
	if (ret == 0)
		return;

	// Chuyen cac ky tu chu thuong thanh chu hoa
	for (int i = 0; i < strlen(cmd); i++)
		if (islower(cmd[i]))
			cmd[i] = toupper(cmd[i]);

	// Tach phan tham so
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
	{
		// Check username
		char* pass;
		pass = GetPasswordOfUser(params);
		if (pass != 0)
		{
			strcpy(pClientInfo->username, params);
			strcpy(pClientInfo->password, pass);
			ResponseCommand(pClientInfo, "331 User name okay, need password.\n");
		}
		else
			ResponseCommand(pClientInfo, "332 Need account for login.\n");
	}		
	else if (strcmp(cmd, "PASS") == 0)
	{
		// Check previous command is USER
		if (strcmp(pClientInfo->prevCommand, "USER") != 0)
			ResponseCommand(pClientInfo, "503 Bad sequence of commands.\n");
		else
		{
			// Check password
			if (strncmp(pClientInfo->password, params, strlen(params)) == 0)
			{
				strcpy(pClientInfo->rootDir, "C:/FTPServer/");
				strcat(pClientInfo->rootDir, pClientInfo->username);
				ResponseCommand(pClientInfo, "230 User logged in, proceed.\n");
			}
			else
			{
				ResponseCommand(pClientInfo, "332 Need account for login.\n");
			}
		}
	}		
	else if (strcmp(cmd, "TYPE") == 0)
	{
		if (strcmp(params, "A") == 0)
			ResponseCommand(pClientInfo, "200 Command okay.\n");
		else
			ResponseCommand(pClientInfo, "504 Command not implemented for that parameter.\n");
	}
	else if (strcmp(cmd, "MODE") == 0)
	{
		if (strcmp(params, "S") == 0)
			ResponseCommand(pClientInfo, "200 Command okay.\n");
		else
			ResponseCommand(pClientInfo, "504 Command not implemented for that parameter.\n");
	}	
	else if (strcmp(cmd, "STRU") == 0)
	{
		if (strcmp(params, "F") == 0)
			ResponseCommand(pClientInfo, "200 Command okay.\n");
		else
			ResponseCommand(pClientInfo, "504 Command not implemented for that parameter.\n");
	}
	else if (strcmp(cmd, "PASV") == 0)
		pClientInfo->dataPort = GetPassiveDataPort(pClientInfo);
	else if (strcmp(cmd, "LIST") == 0)
	{
		ResponseCommand(pClientInfo, "150 File status okay; about to open data connection.\n");
		CreateThread(0, 0, ResponseListCommandThread, pClientInfo, 0, 0);
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
		if (strcmp(pClientInfo->prevCommand, "RNFR") == 0)
		{
			char pathFrom[256];
			strcpy(pathFrom, pClientInfo->rootDir);
			strcat(pathFrom, pClientInfo->curDir);
			if (strcmp(pClientInfo->curDir, "/") != 0)
				strcat(pathFrom, "/");
			strcat(pathFrom, pClientInfo->prevCommandParams);

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
	else if (strcmp(cmd, "RETR") == 0)
	{
		ResponseCommand(pClientInfo, "150 File status okay; about to open data connection.\n");
		strcpy(pClientInfo->curCommandParams, params);
		CreateThread(0, 0, ResponseDownloadCommandThread, pClientInfo, 0, 0);
	}
	else if (strcmp(cmd, "STOR") == 0)
	{
		ResponseCommand(pClientInfo, "150 File status okay; about to open data connection.\n");
		strcpy(pClientInfo->curCommandParams, params);
		CreateThread(0, 0, ResponseUploadCommandThread, pClientInfo, 0, 0);
	}
	else if (strcmp(cmd, "NOOP") == 0)
		ResponseCommand(pClientInfo, "200 Command okay.\n");
	else if (strcmp(cmd, "QUIT") == 0)
	{
		ResponseCommand(pClientInfo, "221 Service closing control connection.\n");
		closesocket(pClientInfo->controlSocket);
	}
	else
		ResponseCommand(pClientInfo, "502 Command not implemented.\n");

	strcpy(pClientInfo->prevCommand, cmd);
	strcpy(pClientInfo->prevCommandParams, params);
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

DWORD WINAPI ResponseListCommandThread(LPVOID lpParam)
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

DWORD WINAPI ResponseDownloadCommandThread(LPVOID lpParam)
{
	CLIENT_INFO* pClientInfo = (CLIENT_INFO*)lpParam;
	SOCKET dataClient = OpenDataConnection(pClientInfo->dataPort);

	// Build full path of the requested file on server side
	char path[256];
	strcpy(path, pClientInfo->rootDir);
	strcat(path, pClientInfo->curDir);
	if (strcmp(pClientInfo->curDir, "/") != 0)
		strcat(path, "/");
	strcat(path, pClientInfo->curCommandParams);

	// Read file and send via data connection
	FILE* f = NULL;
	fopen_s(&f, path, "rb");

	if (f == 0)
	{
		// Cannot open file to read, send error response
		ResponseCommand(pClientInfo, "550 Requested action not taken.File unavailable.\n");
	}
	else
	{
		char buf[1024];
		int ret;
		while (1)
		{
			ret = fread_s(buf, sizeof(buf), 1, sizeof(buf), f);
			if (ret <= 0)
				break;
			send(dataClient, buf, ret, 0);
		}
		fclose(f);

		ResponseCommand(pClientInfo, "226 Closing data connection.\n");
	}

	closesocket(dataClient);
	return 0;
}

DWORD WINAPI ResponseUploadCommandThread(LPVOID lpParam)
{
	CLIENT_INFO* pClientInfo = (CLIENT_INFO*)lpParam;
	SOCKET dataClient = OpenDataConnection(pClientInfo->dataPort);

	// Build full path of the requested file on server side
	char path[256];
	strcpy(path, pClientInfo->rootDir);
	strcat(path, pClientInfo->curDir);
	if (strcmp(pClientInfo->curDir, "/") != 0)
		strcat(path, "/");
	strcat(path, pClientInfo->curCommandParams);

	// Received data via data connection and save to file
	FILE* f = NULL;
	fopen_s(&f, path, "wb");

	if (f == 0)
	{
		// Cannot open file to write, send error response
		ResponseCommand(pClientInfo, "553 Requested action not taken. File unavailable.\n");
	}
	else
	{
		char buf[1024];
		int ret;
		while (1)
		{
			ret = recv(dataClient, buf, sizeof(buf), 0);
			if (ret <= 0)
				break;
			fwrite(buf, 1, ret, f);
		}
		fclose(f);

		ResponseCommand(pClientInfo, "226 Closing data connection.\n");
	}

	closesocket(dataClient);
	return 0;
}

// Ham lay password cua username tu file du lieu
// Tra ve buffer chua password neu tim thay
// Tra ve 0 (NULL) neu khong tim thay hoac bi loi 
char* GetPasswordOfUser(char* username)
{
	FILE* f = NULL;
	fopen_s(&f, "C:/FTPServer/users.txt", "r");
	if (f == 0)
		return 0;
	else
	{
		int found = 0;
		char buf[64];
		while (!feof(f))
		{
			fgets(buf, sizeof(buf), f);
			if (strncmp(buf, username, strlen(username)) == 0)
			{
				found = 1;
				break;
			}
		}
		fclose(f);

		if (found)
			return buf + strlen(username) + 1;
		else
			return 0;
	}
}