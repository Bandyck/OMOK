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
// >> : OMOK Start
#define MAXCLIENT 4							// 최대 접속 클라이언트 수
#define LINENUMBER 18						// 바둑판 사이즈 19 × 19 (0부터 이므로 18)
int board[LINENUMBER + 8][LINENUMBER + 8];	// 서버와 클라이언트가 서로 주고 받는 board 배열
BOOL WinLossDecision(int x, int y);			// 승패 판정
// >> : OMOK End
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
	// 네트워크 부분
	static WSADATA wsadata;
	static SOCKET server;
	static SOCKET client[MAXCLIENT];
	static SOCKADDR_IN	sddr = { 0 }, cddr[MAXCLIENT];
	int clientindex = 0;
	int csize[MAXCLIENT];
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
		if (listen(server, MAXCLIENT) == SOCKET_ERROR)
		{
			MessageBox(NULL, _T("listen failed"), _T("Error"), MB_OK);
			return 0;
		}
		else
		{
			MessageBox(NULL, _T("listen success"), _T("Success"), MB_OK);
		}
		WSAAsyncSelect(server, hWnd, WM_ASYNC, FD_ACCEPT);
		break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_ACCEPT:
			if (clientindex >= MAXCLIENT - 1)
				break;
			csize[clientindex] = sizeof(cddr[clientindex]);
			client[clientindex] = accept(server, (LPSOCKADDR)&cddr[clientindex], &csize[clientindex]);
			WSAAsyncSelect(client[clientindex], hWnd, WM_ASYNC, FD_READ);
			clientindex++;
			break;
		case FD_READ:
			recv(client[clientindex], , , 0);
			WinLossDecision
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
BOOL WinLossDecision(int x, int y)
{
	// 승리판단 4개의 배열
	int WLArray1[9] = { board[x - 4][y - 4],board[x - 3][y - 3],board[x - 2][y - 2],board[x - 1][y - 1],
		board[x][y],board[x + 1][y + 1],board[x + 2][y + 2],board[x + 3][y + 3],board[x + 4][y + 4] };		// ↘ 모양
	int WLArray2[9] = { board[x - 4][y + 4],board[x - 3][y + 3],board[x - 2][y + 2],board[x - 1][y + 1],
		board[x][y],board[x + 1][y - 1],board[x + 2][y - 2],board[x + 3][y - 3],board[x + 4][y - 4] };		// ↙ 모양
	int WLArray3[9] = { board[x][y - 4],board[x][y - 3],board[x][y - 2],board[x][y - 1],
		board[x][y],board[x][y + 1],board[x][y + 2],board[x][y + 3],board[x][y + 4] };						// ↓ 모양
	int WLArray4[9] = { board[x - 4][y],board[x - 3][y],board[x - 2][y],board[x - 1][y],
		board[x][y],board[x + 1][y],board[x + 2][y],board[x + 3][y],board[x + 4][y] };						// → 모양

	int i, count = 0;
	// ↘ 모양
	for (i = 0; i <= 9; i++)
	{
		if (WLArray1[i] == -1)
			count++;
		else
		{
			if (count == 5)
				return TRUE;
			else
				count = 0;
		}
	}
	count = 0;
	// ↙ 모양
	for (i = 0; i <= 9; i++)
	{
		if (WLArray2[i] == -1)
			count++;
		else
		{
			if (count == 5)
				return TRUE;
			else
				count = 0;
		}
	}
	count = 0;
	// ↓ 모양
	for (i = 0; i <= 9; i++)
	{
		if (WLArray3[i] == -1)
			count++;
		else
		{
			if (count == 5)
				return TRUE;
			else
				count = 0;
		}
	}
	count = 0;
	// → 모양
	for (i = 0; i <= 9; i++)
	{
		if (WLArray4[i] == -1)
			count++;
		else
		{
			if (count == 5)
				return TRUE;
			else
				count = 0;
		}
	}
	return FALSE;
}
