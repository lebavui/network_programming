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

void DownloadFolder();
void DownloadFolderRecursively(char*, char*);
void UploadFolder();
void UploadFolderRecursively(char*, char*);

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

	Sleep(100);

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
			else if (cmd == 9)
				DownloadFolder();
			else if (cmd == 10)
				UploadFolder();
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
	printf("9. Download folder\n");
	printf("10. Upload folder\n");
	printf("0. Thoat\n");

	int cmd = -1;
	while (cmd < 0 || cmd > 10)
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
	char* dataBuf = NULL;
	int currentSize = 0;

	// read data from data socket
	char* dataBuf = NULL;
	int currentSize = 0;

	// read data from data socket
	while (1)
	{
		ret = recv(dataSocket, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
<<<<<<< HEAD

=======
		
>>>>>>> 37d67b1bbf154d3e78842371e0623ef1e00c9dd6
		if (currentSize == 0)
			dataBuf = (char*)malloc(ret);
		else
			dataBuf = (char*)realloc(dataBuf, currentSize + ret);

		memcpy(dataBuf + currentSize, buf, ret);
		currentSize += ret;
	}
	closesocket(dataSocket);
	
	dataBuf = (char*)realloc(dataBuf, currentSize + 1);
	dataBuf[currentSize] = 0;
	printf("%s\n", dataBuf);

	free(dataBuf);

	dataBuf = (char*)realloc(dataBuf, currentSize + 1);
	dataBuf[currentSize] = 0;
	printf("%s\n", dataBuf);

	free(dataBuf);

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
		fwrite(buf, 1, ret, f);
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

void DownloadFolder()
{
<<<<<<< HEAD
	char parentPath[256] = "C:\\Test";
=======
	char parentPath[256] = "D:\\Test";
>>>>>>> 37d67b1bbf154d3e78842371e0623ef1e00c9dd6
	char folderName[32] = "hello";
	DownloadFolderRecursively(parentPath, folderName);
}

void DownloadFolderRecursively(char* parentPath, char* folderName)
{
	char buf[256];
	int ret;

	// step 1: change to the folder
	sprintf(buf, "CWD %s\n", folderName);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	// step 2: make folder on local
	char fullPath[256];
	sprintf(fullPath, "%s\\%s", parentPath, folderName);
	CreateDirectoryA(fullPath, NULL);

	// step 3: list folder on server
	SOCKET dataSocket = OpenServerDataSocket();
	if (dataSocket == INVALID_SOCKET)
		return;

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

	char* dataBuf = NULL;
	int currentSize = 0;

	// read data from data socket
	while (1)
	{
		ret = recv(dataSocket, buf, sizeof(buf), 0);
		if (ret <= 0)
			break;
<<<<<<< HEAD

=======
		
>>>>>>> 37d67b1bbf154d3e78842371e0623ef1e00c9dd6
		if (currentSize == 0)
			dataBuf = (char*)malloc(ret);
		else
			dataBuf = (char*)realloc(dataBuf, currentSize + ret);

		memcpy(dataBuf + currentSize, buf, ret);
		currentSize += ret;
	}
	closesocket(dataSocket);

	dataBuf = (char*)realloc(dataBuf, currentSize + 1);
	dataBuf[currentSize] = 0;

	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	// process the returned list of folders and files
	char* p1 = dataBuf;
	char* p2 = strstr(dataBuf, "\r\n");
	while (p1[0] != 0)
	{
		if (p1[0] == 'd')
		{
			// step 4: repeat from step 1
			char newParentPath[256];
			sprintf(newParentPath, "%s\\%s", parentPath, folderName);

			int nameLength = p2 - p1 - 49;
			char* newFolderName = (char*)malloc(nameLength + 1);
			memcpy(newFolderName, p1 + 49, nameLength);
			newFolderName[nameLength] = 0;

			DownloadFolderRecursively(newParentPath, newFolderName);

			free(newFolderName);
		}
		else if (p1[0] == '-')
		{
			// step 5: download file
			int nameLength = p2 - p1 - 49;
			char* filename = (char*)malloc(nameLength + 1);
			memcpy(filename, p1 + 49, nameLength);
			filename[nameLength] = 0;

			SOCKET dataSocket = OpenServerDataSocket();
			if (dataSocket == INVALID_SOCKET)
				return;

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

			sprintf(fullPath, "%s\\%s\\%s", parentPath, folderName, filename);
			FILE* f = fopen(fullPath, "wb");

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

			free(filename);
		}
		p1 = p2 + 2;
		p2 = strstr(p1, "\r\n");
	}

	free(dataBuf);

	// step 6: change to the parent folder
	sprintf(buf, "CWD ..\n", folderName);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}

void UploadFolder()
{
<<<<<<< HEAD
	char parentPath[256] = "C:\\Test";
=======
	char parentPath[256] = "D:\\Test";
>>>>>>> 37d67b1bbf154d3e78842371e0623ef1e00c9dd6
	char folderName[32] = "hello";
	UploadFolderRecursively(parentPath, folderName);
}

void UploadFolderRecursively(char* parentPath, char* folderName)
{
	char buf[256];
	int ret;

	// step 1: make folder on server
	sprintf(buf, "MKD %s\n", folderName);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	// step 2: change to the new folder
	sprintf(buf, "CWD %s\n", folderName);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);

	// step 3: list folder on local
	char fullPath[256];
	sprintf(fullPath, "%s\\%s\\*.*", parentPath, folderName);
	WIN32_FIND_DATAA data;
	HANDLE h = FindFirstFileA(fullPath, &data);
	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((strcmp(data.cFileName, ".") == 0) || (strcmp(data.cFileName, "..") == 0))
				continue;

			// step 4: repeat from step 1
			char newParentPath[256];
			sprintf(newParentPath, "%s\\%s", parentPath, folderName);
			UploadFolderRecursively(newParentPath, data.cFileName);
		}
		else
		{
			// step 5: upload file to server
			SOCKET dataSocket = OpenServerDataSocket();
			if (dataSocket == INVALID_SOCKET)
				return;

			char fbuf[2048];

			strcpy(buf, "TYPE I\n");
			send(controlSocket, buf, strlen(buf), 0);
			ret = recv(controlSocket, buf, sizeof(buf), 0);
			buf[ret] = 0;
			printf("%s\n", buf);

			sprintf(buf, "STOR %s\n", data.cFileName);
			send(controlSocket, buf, strlen(buf), 0);
			ret = recv(controlSocket, buf, sizeof(buf), 0);
			buf[ret] = 0;
			printf("%s\n", buf);

			sprintf(fullPath, "%s\\%s\\%s", parentPath, folderName, data.cFileName);
			FILE* f = fopen(fullPath, "rb");

			// read data from file stream and send to data socket
			while (!feof(f))
			{
				ret = fread(fbuf, 1, sizeof(fbuf), f);
				if (ret <= 0)
					break;
				send(dataSocket, fbuf, ret, 0);
			}

			closesocket(dataSocket);
			fclose(f);

			ret = recv(controlSocket, buf, sizeof(buf), 0);
			buf[ret] = 0;
			printf("%s\n", buf);
		}
<<<<<<< HEAD
	} while (FindNextFileA(h, &data));
=======
	} 
	while (FindNextFileA(h, &data));
>>>>>>> 37d67b1bbf154d3e78842371e0623ef1e00c9dd6

	// step 6: change to the parent folder
	sprintf(buf, "CWD ..\n", folderName);
	send(controlSocket, buf, strlen(buf), 0);
	ret = recv(controlSocket, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s\n", buf);
}