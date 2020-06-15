// POP3Client.cpp
//

#include <stdio.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

typedef struct {
	char from[64];
	char to[64];
	char content[1024];
} EMAIL_INFO;

char user[32], pass[32];

SOCKET client;
SOCKADDR_IN addr;

int Login();
void GetEmails();
void ShowEmails();

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(110);

	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	connect(client, (SOCKADDR*)&addr, sizeof(addr));
	Sleep(100);

	// Receive welcome message
	char buf[256];
	int ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	// Login
	while (Login() != 0);

	int c;
	while (1)
	{
		printf("POP3 Client\n");
		printf("1. Get emails\n");
		printf("2. Show emails\n");
		printf("0. Quit\n");
		printf("Select your option: ");
		scanf("%d", &c);
		if (c == 1)
			GetEmails();
		else
			break;
	}

	closesocket(client);
	WSACleanup();
}

int Login()
{
	printf("Nhap username va password: ");
	scanf("%s%s", user, pass);

	char buf[256];
	int ret;

	// Send USER command
	sprintf(buf, "user %s\n", user);
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	// Send PASS command
	sprintf(buf, "pass %s\n", pass);
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	int isLogedIn = strncmp(buf, "+OK", 3) == 0;

	// Send QUIT command
	strcpy(buf, "quit\n");
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	closesocket(client);

	return isLogedIn ? 0 : 1;
}

void GetEmails()
{
	char buf[1024];
	int ret;

	char response[32];
	int n;

	int emailId[64];

	// Connect again
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	connect(client, (SOCKADDR*)&addr, sizeof(addr));
	Sleep(100);

	// Receive welcome message
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	// Send USER command
	sprintf(buf, "user %s\n", user);
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	// Send PASS command
	sprintf(buf, "pass %s\n", pass);
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	time_t t = time(NULL);
	
	// Send LIST command
	strcpy(buf, "list\n");
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);

	// Process email list
	if (strncmp(buf, "+OK", 3) == 0)
	{
		if (strncmp(buf, "+OK 0", 5) == 0)
		{
			printf("No new email to get!\n");
		}
		else {
			// Get number of email
			sscanf(buf, "%s %d", response, &n);
			char* p = strstr(buf, "\r\n");

			// Parse to get email ids
			for (int i = 0; i < n; i++)
			{
				sscanf(p + 2, "%d", &emailId[i]);
				p = strstr(p + 2, "\r\n");
			}

			for (int i = 0; i < n; i++)
			{
				// Send RETR command to download file and save to local file
				sprintf(buf, "retr %d\n", emailId[i]);
				send(client, buf, strlen(buf), 0);

				time_t t = time(NULL);
				char filePath[64];
				sprintf(filePath, "D:\\Test\\mails\\%d_%d.txt", (int)t, emailId[i]);

				FILE* f = fopen(filePath, "wb");

				while (1)
				{
					ret = recv(client, buf, sizeof(buf), 0);
					if (ret <= 0)
						break;

					fwrite(buf, 1, ret, f);

					if (strstr(buf, "\r\n.\r\n"))
						break;
				}
				
				fclose(f);
			}
		}
	}
	else
	{
		printf("Got error when download new emails!\n");
	}
	
	// Send QUIT command
	strcpy(buf, "quit\n");
	send(client, buf, strlen(buf), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	buf[ret] = 0;
	printf("%s", buf);
}

void ShowEmails()
{
	// Read emails from local files
}