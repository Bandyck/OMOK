#include "stdafx.h"
#include "WinApiProjectServerOMOK.h"
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
using namespace std;
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
#define MAXCLIENT 3								// 최대 접속 클라이언트 수
#define LINENUMBER 18							// 바둑판 사이즈 19 × 19 (0부터 이므로 18)

typedef struct ServerGo
{
	int board[LINENUMBER + 9][LINENUMBER + 9];
	int player;	
	BOOL WinLose;
}ServerGo;		// 보낼 구조체(바둑판, 플레이어, 승패판정
typedef struct PlayerGoxy
{
	int x;
	int y;
}PlayerGoxy;	// 받을 구조체(돌을 놓은 좌표)
static ServerGo SG;
static PlayerGoxy xy;

BOOL WinLossDecision(int x, int y);				// 승패 판정
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
	// 네트워크 부분
	static WSADATA wsadata;
	static SOCKET server;
	static SOCKET client[MAXCLIENT];
	static SOCKADDR_IN	sddr = { 0 }, cddr[MAXCLIENT];
	static int clientindex = 0;
	int csize[MAXCLIENT];
	switch (message)
	{
	case WM_CREATE:
	{
		// 콘솔 출력
		AllocConsole();
		freopen("CONOUT$", "wt", stdout);
		// 구조체 초기화
		int i, j;									// 바둑판
		for (i = 0; i <= LINENUMBER + 8; i++)
			for (j = 0; j <= LINENUMBER + 8; j++)
				SG.board[i][j] = -2;
		for (i = 4; i <= LINENUMBER + 4; i++)
			for (j = 4; j <= LINENUMBER + 4; j++)
				SG.board[i][j] = 0;
		SG.player = 1;								// 플레이어
		SG.WinLose = FALSE;							// 승패판단
		// 네트워크 연결
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		server = socket(AF_INET, SOCK_STREAM, 0);
		sddr.sin_family = AF_INET;	sddr.sin_port = 20;	 sddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		bind(server, (LPSOCKADDR)&sddr, sizeof(sddr));
		listen(server, MAXCLIENT);
		WSAAsyncSelect(server, hWnd, WM_ASYNC, FD_ACCEPT);
	}
		break;
	case WM_ASYNC:
	{
		switch (lParam)
		{
		case FD_ACCEPT:
		{
			if (clientindex >= MAXCLIENT - 1)
				break;
			csize[clientindex] = sizeof(cddr[clientindex]);
			client[clientindex] = accept(server, (LPSOCKADDR)&cddr[clientindex], &csize[clientindex]);
			WSAAsyncSelect(client[clientindex], hWnd, WM_ASYNC, FD_READ);
			if (clientindex == 0)
			{
				send(client[clientindex], (char*)&SG, sizeof(ServerGo), 0);
			}
			else
			{
				SG.player = -1;
				send(client[clientindex], (char*)&SG, sizeof(ServerGo), 0);
			}
			clientindex++;
			SG.player = 1;
		}
			break;
		case FD_READ:
		{
			if (SG.player == 1)
			{
				recv(client[0], (char *)&xy, sizeof(PlayerGoxy), 0);
				SG.board[xy.x + 4][xy.y + 4] = 1;
				if (WinLossDecision(xy.x + 4, xy.y + 4))
					SG.WinLose = TRUE;
				SG.player = -1;
			}
			else if (SG.player == -1)
			{
				recv(client[1], (char *)&xy, sizeof(PlayerGoxy), 0);
				SG.board[xy.x + 4][xy.y + 4] = -1;
				if (WinLossDecision(xy.x + 4, xy.y + 4))
					SG.WinLose = TRUE;
				SG.player = 1;
			}
			for (int i = 0; i < clientindex; ++i)
				send(client[i], (char *)&SG, sizeof(ServerGo), 0);
		}
			break;
		default:
			break;
		}
	}
	break;
	case WM_DESTROY:
		FreeConsole();
		closesocket(server);
		for (int i = 0; i < MAXCLIENT; ++i)
			closesocket(client[i]);
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
	int WLArray1[9] = { SG.board[x - 4][y - 4],SG.board[x - 3][y - 3],SG.board[x - 2][y - 2],SG.board[x - 1][y - 1],
		SG.board[x][y],SG.board[x + 1][y + 1],SG.board[x + 2][y + 2],SG.board[x + 3][y + 3],SG.board[x + 4][y + 4] };		// ↘ 모양
	int WLArray2[9] = { SG.board[x - 4][y + 4],SG.board[x - 3][y + 3],SG.board[x - 2][y + 2],SG.board[x - 1][y + 1],
		SG.board[x][y],SG.board[x + 1][y - 1],SG.board[x + 2][y - 2],SG.board[x + 3][y - 3],SG.board[x + 4][y - 4] };		// ↙ 모양
	int WLArray3[9] = { SG.board[x][y - 4],SG.board[x][y - 3],SG.board[x][y - 2],SG.board[x][y - 1],
		SG.board[x][y],SG.board[x][y + 1],SG.board[x][y + 2],SG.board[x][y + 3],SG.board[x][y + 4] };						// ↓ 모양
	int WLArray4[9] = { SG.board[x - 4][y],SG.board[x - 3][y],SG.board[x - 2][y],SG.board[x - 1][y],
		SG.board[x][y],SG.board[x + 1][y],SG.board[x + 2][y],SG.board[x + 3][y],SG.board[x + 4][y] };						// → 모양

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