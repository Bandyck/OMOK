#include "stdafx.h"
#include "WinApiProjectClientOMOK.h"
#include <math.h>
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
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
#define BSIZE 15								// Ŭ�� ������ ���� ���� ������
#define LinetoLineSize 36						// �ٵ��� ���� �� ������ ����
#define LINENUMBER 18							// �ٵ��� ������ 19 �� 19 (0���� �̹Ƿ� 18)
typedef struct PlayerGoxy
{
	int x;
	int y;
}PlayerGoxy;	// ���� ����ü(���� ���� ��ǥ)
typedef struct ServerGo
{
	int board[LINENUMBER + 9][LINENUMBER + 9];
	BOOL turn;
	BOOL WinLose;
}ServerGo;		// ���� ����ü(�ٵ���, �ڱ� ������, ��������)
static PlayerGoxy xy;
static ServerGo SG;

// �׸� �κ�
HBITMAP hBlackImage;			// �浹
BITMAP bitBlack;				
HBITMAP hWhiteImage;			// �鵹
BITMAP bitWhite;				
void CreateBitmap();
void DrawBitmap(HWND hWnd, HDC hdc);				// �⺻ UI �׸��� �Լ�
void DrawStone(HWND hWnd, HDC hdc, ServerGo SG);	// �� �׸��� �Լ�
void DeleteBitmap();
																												 //	 0 : ���� ���� ����
// �� ���� ����																									 //  1 : �÷��̾� 1�� ���� ����(�浹)
POINT checkboard[LINENUMBER + 1][LINENUMBER + 1];																 // -2 : �ٵ��� �ܰ�ó��
double LengthPts(int x1, int y1, int x2, int y2)																
{																												
	return (sqrt((float)((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))));
}	
BOOL InCircle(int x, int y, int mx, int my)
{
	if (LengthPts(x, y, mx, my) < BSIZE)
		return TRUE;
	else
		return FALSE;
}

// >> : OMOK END
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPIPROJECTCLIENTOMOK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))
    {	return FALSE;	}
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPIPROJECTCLIENTOMOK));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {   TranslateMessage(&msg);
            DispatchMessage(&msg);      }
    }
    return (int) msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPIPROJECTCLIENTOMOK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPIPROJECTCLIENTOMOK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED,
      100, 100, 1242, 785, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
   {	return FALSE;   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int mx, my;
	// ��Ʈ��ũ �κ�
	static WSADATA wsadata;
	static SOCKET server;
	static SOCKADDR_IN	 sddr = { 0 };
    switch (message)
    {
	case WM_CREATE:
	{
		// �ܼ� ���
		AllocConsole();
		freopen("CONOUT$", "wt", stdout);
		// ��Ʈ�� ����
		CreateBitmap();
		// �� ���� ������ ���� ��ǥ���� �� ���α�
		int i, j;
		for (i = 0; i <= LINENUMBER; i++)
		{
			for (j = 0; j <= LINENUMBER; j++)
			{
				checkboard[i][j].x = 475 + LinetoLineSize * i;
				checkboard[i][j].y = 30 + LinetoLineSize * j;
			}
		}
		// ��Ʈ��ũ �κ�
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		server = socket(AF_INET, SOCK_STREAM, 0);		
		sddr.sin_family = AF_INET;	sddr.sin_port = 20; sddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		WSAAsyncSelect(server, hWnd, WM_ASYNC, FD_READ);
		connect(server, (LPSOCKADDR)&sddr, sizeof(sddr));
	}
		break;
	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		for (int i = 0; i <= LINENUMBER; i++)
		{
			for (int j = 0; j <= LINENUMBER; j++)
			{
				if (InCircle(checkboard[i][j].x, checkboard[i][j].y, mx, my))
				{
					if (SG.board[i + 4][j + 4] == 0)
					{
						xy.x = i;
						xy.y = j;
						send(server, (char *)&xy, sizeof(PlayerGoxy), 0);
					}
					i = LINENUMBER + 1;
					break;
				}
			}
		}
	}
			break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_READ:
			recv(server, (char*)&SG, sizeof(ServerGo), 0);
			InvalidateRgn(hWnd, NULL, TRUE);
			break;
		default:
			break;
		}
	case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hWnd, hdc);
			DrawStone(hWnd, hdc, SG);
			if (SG.WinLose)
				TextOut(hdc, 100, 100, _T("Win!"), 10);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		FreeConsole();
		DeleteBitmap();
		closesocket(server);
		WSACleanup();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void CreateBitmap()
{
	// �浹 �޾ƿ���
		hBlackImage = (HBITMAP)LoadImage(NULL, _TEXT("images/black.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBlackImage, sizeof(BITMAP), &bitBlack);
	// �鵹 �޾ƿ���
		hWhiteImage = (HBITMAP)LoadImage(NULL, _TEXT("images/white.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hWhiteImage, sizeof(BITMAP), &bitWhite);
}
void DrawBitmap(HWND hWnd, HDC hdc)
{
	// �⺻ UI
	HDC hMenDC;
	HBITMAP hOldBitmap;
	int bx, by;
	for (int i = 0; i <= LINENUMBER; i++)
	{	// �ٵ��� ����
		MoveToEx(hdc, 475, 30 + LinetoLineSize * i, NULL);
		LineTo(hdc, 475 + LinetoLineSize * LINENUMBER, 30 + LinetoLineSize * i);
		MoveToEx(hdc, 475 + LinetoLineSize * i, 30, NULL);
		LineTo(hdc, 475 + LinetoLineSize * i, 30 + LinetoLineSize * LINENUMBER);
	}
	{	// �浹
		hMenDC = CreateCompatibleDC(hdc);
		hOldBitmap = (HBITMAP)SelectObject(hMenDC, hBlackImage);
		bx = bitBlack.bmWidth;
		by = bitBlack.bmHeight;
		TransparentBlt(hdc, 1152, 30, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
		SelectObject(hMenDC, hOldBitmap);
		DeleteDC(hMenDC);
	}
	{	// �鵹
		hMenDC = CreateCompatibleDC(hdc);
		hOldBitmap = (HBITMAP)SelectObject(hMenDC, hWhiteImage);
		bx = bitWhite.bmWidth;
		by = bitWhite.bmHeight;
		TransparentBlt(hdc, 1152, 72, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
		SelectObject(hMenDC, hOldBitmap);
		DeleteDC(hMenDC);
	}
	{	// �÷��̾� ���
		TCHAR Player1[] = _T("1P");
		TCHAR Player2[] = _T("2P");
		TextOut(hdc, 1189, 35, Player1, _tcslen(Player1));	// 1P ���
		TextOut(hdc, 1189, 77, Player2, _tcslen(Player2));	// 2P ���
	}
}
void DrawStone(HWND hWnd, HDC hdc, ServerGo SG)
{
	HDC hMenDC;
	HBITMAP hOldBitmap;
	int bx, by;
	int i, j;
	for (i = 0; i <= LINENUMBER + 8; i++)
		for (j = 0; j <= LINENUMBER + 8; j++)
		{
			if (SG.board[i + 4][j + 4] == -1)
			{
				{	// �鵹
					hMenDC = CreateCompatibleDC(hdc);
					hOldBitmap = (HBITMAP)SelectObject(hMenDC, hWhiteImage);
					bx = bitWhite.bmWidth;
					by = bitWhite.bmHeight;
					TransparentBlt(hdc, 459 + 36 * i, 14 + 36 * j, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
					SelectObject(hMenDC, hOldBitmap);
					DeleteDC(hMenDC);
				}
			}
			else if (SG.board[i + 4][j + 4] == 1)
			{
				{	// �浹
					hMenDC = CreateCompatibleDC(hdc);
					hOldBitmap = (HBITMAP)SelectObject(hMenDC, hBlackImage);
					bx = bitBlack.bmWidth;
					by = bitBlack.bmHeight;
					TransparentBlt(hdc, 459 + 36 * i, 14 + 36 * j, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
					SelectObject(hMenDC, hOldBitmap);
					DeleteDC(hMenDC);
				}
			}
		}
}
void DeleteBitmap()
{
	DeleteObject(hWhiteImage);
	DeleteObject(hBlackImage);
}