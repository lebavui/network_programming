// FTPClient.cpp

#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

SOCKET controlSocket;

int GetCommand();
int Login();
void ListCurrentDirectory();
void ChangeCurrentDirectory();
void MakeDirectory();
void RemoveDirectory();
void DownloadFile();
void UploadFile();
void RenameFile();
void RemoveFile();
void Quit();

SOCKET OpenServerDataSocket();

void main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN controlAddr;
	controlAddr.sin_family = AF_INET;
	controlAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	controlAddr.sin_port = htons(21);

	controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int ret = connect(controlSocket, (SOCKADDR*)&controlAddr, sizeof(controlAddr));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed.\n");
		return;
	}

	char buf[256];
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
	
	int isLoggedIn = 0;

	while (1)
	{
		if (!isLoggedIn)
			isLoggedIn = Login() == 0;
		else
		{
			int cmd = GetCommand();
			if (cmd == 0)
			{
				Quit();
				break;
			}
			else if (cmd == 1)
				ListCurrentDirectory();
			else if (cmd == 2)
				ChangeCurrentDirectory();
			else if (cmd == 3)
				MakeDirectory();
			else if (cmd == 4)
				RemoveDirectory();
			else if (cmd == 5)
				DownloadFile();
			else if (cmd == 6)
				UploadFile();
			else if (cmd == 7)
				RenameFile();
			else if (cmd == 8)
				RemoveFile();
		}
	}

	closesocket(controlSocket);
	WSACleanup();
}

int GetCommand() 
{
	printf("FTP Client\n");
	printf("1. Hien thi noi dung thu muc hien tai\n");
	printf("2. Di chuyen den thu muc khac\n");
	printf("3. Tao thu muc\n");
	printf("4. Xoa thu muc\n");
	printf("5. Download file\n");
	printf("6. Upload file\n");
	printf("7. Doi ten file\n");
	printf("8. Xoa file\n");
	printf("0. Thoat\n");

	int cmd = -1;
	while (cmd < 0 || cmd > 8)
	{
		printf("Chon chuc nang: ");
		scanf("%d", &cmd);
	}
	
	return cmd;
}

void Quit()
{
	char buf[256] = "QUIT\n";
	int ret;

	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

int Login()
{
	char buf[256];
	char user[32], pass[32];
	int ret;

	printf("Nhap username va password: ");
	scanf("%s %s", user, pass);

	sprintf(buf, "USER %s\n", user);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;

	if (strstr(buf, "331") == NULL)
	{
		printf("%s\n", buf);
		return 1;
	}

	sprintf(buf, "PASS %s\n", pass);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;

	if (strstr(buf, "230") == NULL)
	{
		printf("%s\n", buf);
		return 1;
	}	

	return 0;
}

SOCKET OpenServerDataSocket()
{
	char buf[256];
	int ret;

	strcpy(buf, "PASV\n");
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;

	if (strstr(buf, "227") == NULL)
	{
		printf("%s\n", buf);
		return INVALID_SOCKET;
	}

	// Get server data port
	char* p = strtok(buf, "(,)");
	char* b3 = strtok(NULL, "(,)");
	char* b2 = strtok(NULL, "(,)");
	char* b1 = strtok(NULL, "(,)");
	char* b0 = strtok(NULL, "(,)");
	char* hiByte = strtok(NULL, "(,)");
	char* loByte = strtok(NULL, "(,)");

	sprintf(buf, "%s.%s.%s.%s", b3, b2, b1, b0);
	int port = atoi(hiByte) * 256 + atoi(loByte);

	SOCKADDR_IN dataAddr;
	dataAddr.sin_family = AF_INET;
	dataAddr.sin_addr.s_addr = inet_addr(buf);
	dataAddr.sin_port = htons(port);

	SOCKET dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ret = connect(dataSocket, (SOCKADDR*)&dataAddr, sizeof(dataAddr));
	if (ret == SOCKET_ERROR)
		return INVALID_SOCKET;
	else
		return dataSocket;
}

void ListCurrentDirectory()
{
	SOCKET dataSocket = OpenServerDataSocket();
	if (dataSocket == INVALID_SOCKET)
		return;

	char buf[256];
	int ret;

	strcpy(buf, "TYPE A\n");
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	strcpy(buf, "LIST\n");
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
	
	// read data from data socket
	while (1)
	{
		ret = recv(dataSocket, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
		if (ret < sizeof(buf))
			buf[ret] = 0;
		printf("%s\n", buf);
	}
	closesocket(dataSocket);

	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void ChangeCurrentDirectory()
{
	char buf[256];
	char path[32];
	int ret;

	printf("Nhap ten thu muc muon di chuyen den: ");
	scanf("%s", path);

	sprintf(buf, "CWD %s\n", path);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void MakeDirectory()
{
	char buf[256];
	char path[32];
	int ret;

	printf("Nhap ten thu muc moi: ");
	scanf("%s", path);

	sprintf(buf, "MKD %s\n", path);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void RemoveDirectory()
{
	char buf[256];
	char path[32];
	int ret;

	printf("Nhap ten thu muc can xoa: ");
	scanf("%s", path);

	sprintf(buf, "RMD %s\n", path);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void DownloadFile()
{
	char path[64], filename[32];
	printf("Nhap ten file can download: ");
	scanf("%s", filename);

	SOCKET dataSocket = OpenServerDataSocket();
	if (dataSocket == INVALID_SOCKET)
		return;

	char buf[2048];
	int ret;

	strcpy(buf, "TYPE I\n");
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	sprintf(buf, "RETR %s\n", filename);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	sprintf(path, "C:\\Test\\%s", filename);
	FILE* f = fopen(path, "wb");

	// read data from data socket
	while (1)
	{
		ret = recv(dataSocket, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
		fwrite(buf, 1, sizeof(buf), f);
	}

	closesocket(dataSocket);
	fclose(f);

	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void UploadFile()
{
	char path[64], filename[32];
	printf("Nhap ten file can upload: ");
	scanf("%s", filename);

	SOCKET dataSocket = OpenServerDataSocket();
	if (dataSocket == INVALID_SOCKET)
		return;

	char buf[2048];
	int ret;

	strcpy(buf, "TYPE I\n");
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	sprintf(buf, "STOR %s\n", filename);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	sprintf(path, "C:\\Test\\%s", filename);
	FILE* f = fopen(path, "rb");

	// read data from data socket
	while (!feof(f))
	{
		ret = fread(buf, 1, sizeof(buf), f);
		if (ret <= 0)
			break;
		send(dataSocket, buf, ret, 0);
	}

	closesocket(dataSocket);
	fclose(f);

	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void RenameFile()
{
	char buf[256];
	char oldname[32], newname[32];
	int ret;

	printf("Nhap ten file cu: ");
	scanf("%s", oldname);
	printf("Nhap ten file moi: ");
	scanf("%s", newname);

	sprintf(buf, "RNFR %s\n", oldname);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	sprintf(buf, "RNTO %s\n", newname);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void RemoveFile()
{
	char buf[256];
	char filename[32];
	int ret;

	printf("Nhap ten file can xoa: ");
	scanf("%s", filename);

	sprintf(buf, "DELE %s\n", filename);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}