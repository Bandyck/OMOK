#include "stdafx.h"
#include "WinApiProjectServerOMOK.h"
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"msimg32.lib")
#define WM_ASYNC WM_USER+2
#define MAX_LOADSTRING 100

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINAPIPROJECTSERVEROMOK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
	{	return FALSE;	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPIPROJECTSERVEROMOK));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{	TranslateMessage(&msg);
			DispatchMessage(&msg);	}
	}
	return (int)msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_WINAPIPROJECTSERVEROMOK));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINAPIPROJECTSERVEROMOK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{	return FALSE;	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static WSADATA wsadata;
	static SOCKET server, client1, client2;
	static SOCKADDR_IN	sddr = { 0 }, c1ddr, c2ddr;
	int c1size, c2size;
	switch (message)
	{
	case WM_CREATE:
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		server = socket(AF_INET, SOCK_STREAM, 0);
		sddr.sin_family = AF_INET;	sddr.sin_port = 20;	 sddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		if (bind(server, (LPSOCKADDR)&sddr, sizeof(sddr)))
		{
			MessageBox(NULL, _T("server binding failed"), _T("Error"), MB_OK);
			return 0;
		}
		else
		{
			MessageBox(NULL, _T("server binding success"), _T("Success"), MB_OK);
		}
		if (listen(server, 2) == SOCKET_ERROR)
		{
			MessageBox(NULL, _T("listen failed"), _T("Error"), MB_OK);
			return 0;
		}
		else
		{
			MessageBox(NULL, _T("listen success"), _T("Success"), MB_OK);
		}
		WSAAsyncSelect(server, hWnd, WM_ASYNC, FD_ACCEPT);
		//c1size = sizeof(c1ddr);
		//c2size = sizeof(c2ddr);
		//if (client1 = accept(server, (LPSOCKADDR)&c1ddr, &c1size) != 0)
		//	MessageBox(NULL, _T("1번 클라이언트 접속을 확인했습니다."), _T("Server 메시지"), MB_OK);
		//if (client2 = accept(server, (LPSOCKADDR)&c2ddr, &c2size) != 0)
		//	MessageBox(NULL, _T("2번 클라이언트 접속을 확인했습니다."), _T("Server 메시지"), MB_OK);
		break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_ACCEPT:
			c1size = sizeof(c1ddr);
			c2size = sizeof(c2ddr);
			client1 = accept(server, (LPSOCKADDR)&c1ddr, &c1size);
			client2 = accept(server, (LPSOCKADDR)&c2ddr, &c2size);
			break;
		case FD_READ:
			break;
		default:
			break;
		}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		closesocket(server);
		closesocket(client1);
		closesocket(client2);
		WSACleanup();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}