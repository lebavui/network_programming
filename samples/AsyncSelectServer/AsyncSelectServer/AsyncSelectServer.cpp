#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

#define WM_SOCKET WM_USER + 1

bool CALLBACK WinProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	if (wMsg == WM_SOCKET)
	{
		// Kiem tra loi
		if (WSAGETSELECTERROR(lParam))
		{
			printf("Connection Error");
			closesocket(wParam);
			return TRUE;
		}

		// Kiem tra su kien co ket noi den server
		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept(wParam, NULL, NULL);

			// Gan socket client voi cua so
			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}

		// Kiem tra su kien co du lieu gui den socket
		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int ret = recv(wParam, buf, sizeof(buf), 0);
			if (ret <= 0)
			{
				printf("recv() failed\n");
				return TRUE;
			}

			buf[ret] = 0;
			printf("Received: %s\n", buf);
		}

		// Kiem tra su kien ket noi bi dong
		if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			printf("Connection closed");
			closesocket(wParam);
		}
	}

	return TRUE;
}

int main()
{
	// Khoi tao Winsock
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Khai bao dia chi server cho o cong 9000
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	// Tao socket, gan voi dia chi va chuyen sang trang thai cho ket noi
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	// Khai bao lop cua so
	WNDCLASS wndclass;
	const CHAR* providerClass = "AsyncSelect";
	
	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)providerClass;

	if (RegisterClass(&wndclass) == 0)
		return NULL;

	// Tao doi tuong cua so
	HWND hWnd;
	if ((hWnd = CreateWindow((LPCWSTR)providerClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL)) == NULL)
		return NULL;

	// Gan socket voi cua so
	WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
