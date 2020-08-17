#include "stdafx.h"
#include "WinApiProjectClientOMOK.h"
#include <math.h>
#include <WinSock2.h>
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
#define BSIZE 15				// Ŭ�� ������ ���� ���� ������
#define LinetoLineSize 36		// �ٵ��� ���� �� ������ ����
#define LINENUMBER 18			// �ٵ��� ������ 19 �� 19 (0���� �̹Ƿ� 18)

HBITMAP hBlackImage;			// �浹
BITMAP bitBlack;				
HBITMAP hWhiteImage;			// �鵹
BITMAP bitWhite;				

void CreateBitmap();
void DrawBitmap(HWND hWnd, HDC hdc);
void DrawStoneBlack(HWND hWnd, HDC hdc, int x, int y);		// �浹 �׸��� �Լ�
void DrawStoneWhite(HWND hWnd, HDC hdc, int x, int y);		// �鵹 �׸��� �Լ�
void DeleteBitmap();

																												 //	 0 : ���� ���� ����
// �� ���� ����																									 //  1 : �÷��̾� 1�� ���� ����(�浹)
int board[LINENUMBER + 8][LINENUMBER + 8];					// ������ Ŭ���̾�Ʈ�� ���� �ְ� �޴� board �迭	 // -1 : �÷��̾� 2�� ���� ����(�鵹)
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

// ���� ����
BOOL WinLossDecisionBlack(int x, int y);
BOOL WinLossDecisionWhite(int x, int y);

// ���� ����ü(client��������, ���� ���� ��ǥ)
typedef struct PlayerGoxy
{
	int client;
	int x;
	int y;
}PlayerGoxy;
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
	static PlayerGoxy xy;
	// static int x, y;
	static BOOL SelectionBlack;
	static BOOL SelectionWhite;
	int mx, my;
	// ��Ʈ��ũ �κ�
	static WSADATA wsadata;
	static SOCKET server;
	static SOCKADDR_IN	 sddr = { 0 };
    switch (message)
    {
	case WM_CREATE:
	{
		int i, j;
		CreateBitmap();
		// board �迭 �ʱ�ȭ
		for (i = 0; i <= LINENUMBER + 4; i++)
			for (j = 0; j <= LINENUMBER + 4; j++)
				board[i][j] = -2;
		for (i = 4; i <= 4 + LINENUMBER; i++)
			for (j = 4; j <= 4 + LINENUMBER; j++)
				board[i][j] = 0;
		// �� ���� ���� 
		for (i = 0; i <= LINENUMBER; i++)
		{
			for (j = 0; j <= LINENUMBER; j++)
			{
				checkboard[i][j].x = 475 + LinetoLineSize * i;
				checkboard[i][j].y = 30 + LinetoLineSize * j;
			}
		}
		SelectionBlack = FALSE;
		SelectionWhite = FALSE;
		// ��Ʈ��ũ �κ�
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		server = socket(AF_INET, SOCK_STREAM, 0);		
		sddr.sin_family = AF_INET;	sddr.sin_port = 20; sddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		if (connect(server, (LPSOCKADDR)&sddr, sizeof(sddr)) == SOCKET_ERROR)
		{
			MessageBox(NULL, _T("client connect failed"), _T("Error"), MB_OK);
			return 0;
		}
		else
		{
			MessageBox(NULL, _T("client connect success"), _T("Success"), MB_OK);
		}
		WSAAsyncSelect(server, hWnd, WM_ASYNC, FD_READ);
	}
		break;
	// ���콺 ���� ��ư : 1P(�浹)
	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		for (int i = 0; i <= LINENUMBER; i++)
		{
			for (int j = 0; j <= LINENUMBER; j++)
			{
				if (InCircle(checkboard[i][j].x, checkboard[i][j].y, mx, my))
				{	
					if (board[i + 4][j + 4] == 0)
					{
						xy.x = i;
						xy.y = j;
						//int retval;
						//int len = sizeof(xy);
						//retval = send(server, (char *)&len, sizeof(int), 0);
						//retval = send(server, (char *)&xy, sizeof(PlayerGoxy), 0);
						SelectionBlack = TRUE;
						board[i + 4][j + 4] = 1;
						InvalidateRgn(hWnd, NULL, FALSE);
					}
					i = LINENUMBER+1;
					break;
				}
			}
		}
			break;
	// ���콺 ������ ��ư : 2P(�鵹)
	case WM_RBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		for (int i = 0; i <= LINENUMBER; i++)
		{
			for (int j = 0; j <= LINENUMBER; j++)
			{
				if (InCircle(checkboard[i][j].x, checkboard[i][j].y, mx, my))
				{
					if (board[i + 4][j + 4] == 0)
					{
						xy.x = i;
						xy.y = j;
						SelectionWhite = TRUE;
						board[i + 4][j + 4] = -1;
						InvalidateRgn(hWnd, NULL, FALSE);
					}
					i = LINENUMBER + 1;
					break;
				}
			}
		}
		break;
	case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hWnd, hdc);
			if (SelectionBlack)
			{
				DrawStoneBlack(hWnd, hdc, xy.x, xy.y);
				SelectionBlack = FALSE;
			}
			if (SelectionWhite)
			{
				DrawStoneWhite(hWnd, hdc, xy.x, xy.y);
				SelectionWhite = FALSE;
			}
			if (WinLossDecisionBlack(xy.x + 4, xy.y + 4))
			{
				TCHAR Player1WIN[] = _T("1P WIN!!!");
				TextOut(hdc, 20, 20, Player1WIN, _tcslen(Player1WIN));
			}
			if (WinLossDecisionWhite(xy.x + 4, xy.y + 4))
			{
				TCHAR Player2WIN[] = _T("2P WIN!!!");
				TextOut(hdc, 20, 20, Player2WIN, _tcslen(Player2WIN));
			}
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteBitmap();
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
	HDC hMenDC;
	HBITMAP hOldBitmap;
	int bx, by;
	// �⺻ UI
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
		//BitBlt(hdc, 1132, 15, bx, by, hMenDC, 0, 0, SRCCOPY);
		TransparentBlt(hdc, 1152, 30, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
		SelectObject(hMenDC, hOldBitmap);
		DeleteDC(hMenDC);
	}
	{	// �鵹
		hMenDC = CreateCompatibleDC(hdc);
		hOldBitmap = (HBITMAP)SelectObject(hMenDC, hWhiteImage);
		bx = bitWhite.bmWidth;
		by = bitWhite.bmHeight;
		//BitBlt(hdc, 1132, 57, bx, by, hMenDC, 0, 0, SRCCOPY);
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
void DrawStoneBlack(HWND hWnd, HDC hdc, int x, int y)
{
	HDC hMenDC;
	HBITMAP hOldBitmap;
	int bx, by;
	{	// �浹
		hMenDC = CreateCompatibleDC(hdc);
		hOldBitmap = (HBITMAP)SelectObject(hMenDC, hBlackImage);
		bx = bitBlack.bmWidth;
		by = bitBlack.bmHeight;
		//BitBlt(hdc, 1132, 15, bx, by, hMenDC, 0, 0, SRCCOPY);
		TransparentBlt(hdc, 459 + 36 * x, 14 + 36 * y, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
		SelectObject(hMenDC, hOldBitmap);
		DeleteDC(hMenDC);
	}
}
void DrawStoneWhite(HWND hWnd, HDC hdc, int x, int y)
{
	HDC hMenDC;
	HBITMAP hOldBitmap;
	int bx, by;
	{	// �鵹
		hMenDC = CreateCompatibleDC(hdc);
		hOldBitmap = (HBITMAP)SelectObject(hMenDC, hWhiteImage);
		bx = bitWhite.bmWidth;
		by = bitWhite.bmHeight;
		//BitBlt(hdc, 1132, 15, bx, by, hMenDC, 0, 0, SRCCOPY);
		TransparentBlt(hdc, 459 + 36 * x, 14 + 36 * y, bx, by, hMenDC, 0, 0, bx, by, RGB(255, 0, 255));
		SelectObject(hMenDC, hOldBitmap);
		DeleteDC(hMenDC);
	}
}
void DeleteBitmap()
{
	DeleteObject(hWhiteImage);
	DeleteObject(hBlackImage);
}
BOOL WinLossDecisionBlack(int x, int y)
{
	// �¸��Ǵ� 4���� �迭
	int WLArray1[9] = { board[x - 4][y - 4],board[x - 3][y - 3],board[x - 2][y - 2],board[x - 1][y - 1],
		board[x][y],board[x + 1][y + 1],board[x + 2][y + 2],board[x + 3][y + 3],board[x + 4][y + 4] };		// �� ���
	int WLArray2[9] = { board[x - 4][y + 4],board[x - 3][y + 3],board[x - 2][y + 2],board[x - 1][y + 1],
		board[x][y],board[x + 1][y - 1],board[x + 2][y - 2],board[x + 3][y - 3],board[x + 4][y - 4] };		// �� ���
	int WLArray3[9] = { board[x][y - 4],board[x][y - 3],board[x][y - 2],board[x][y - 1],
		board[x][y],board[x][y + 1],board[x][y + 2],board[x][y + 3],board[x][y + 4] };						// �� ���
	int WLArray4[9] = { board[x - 4][y],board[x - 3][y],board[x - 2][y],board[x - 1][y],
		board[x][y],board[x + 1][y],board[x + 2][y],board[x + 3][y],board[x + 4][y] };						// �� ���

	int i, count = 0;
	// �� ���
	for (i = 0; i <= 9; i++)
	{
		if (WLArray1[i] == 1)
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
	// �� ���
	for (i = 0; i <= 9; i++)
	{
		if (WLArray2[i] == 1)
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
	// �� ���
	for (i = 0; i <= 9; i++)
	{
		if (WLArray3[i] == 1)
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
	// �� ���
	for (i = 0; i <= 9; i++)
	{
		if (WLArray4[i] == 1)
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
BOOL WinLossDecisionWhite(int x, int y)
{
	// �¸��Ǵ� 4���� �迭
	int WLArray1[9] = { board[x - 4][y - 4],board[x - 3][y - 3],board[x - 2][y - 2],board[x - 1][y - 1],
		board[x][y],board[x + 1][y + 1],board[x + 2][y + 2],board[x + 3][y + 3],board[x + 4][y + 4] };		// �� ���
	int WLArray2[9] = { board[x - 4][y + 4],board[x - 3][y + 3],board[x - 2][y + 2],board[x - 1][y + 1],
		board[x][y],board[x + 1][y - 1],board[x + 2][y - 2],board[x + 3][y - 3],board[x + 4][y - 4] };		// �� ���
	int WLArray3[9] = { board[x][y - 4],board[x][y - 3],board[x][y - 2],board[x][y - 1],
		board[x][y],board[x][y + 1],board[x][y + 2],board[x][y + 3],board[x][y + 4] };						// �� ���
	int WLArray4[9] = { board[x - 4][y],board[x - 3][y],board[x - 2][y],board[x - 1][y],
		board[x][y],board[x + 1][y],board[x + 2][y],board[x + 3][y],board[x + 4][y] };						// �� ���

	int i, count = 0;
	// �� ���
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
	// �� ���
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
	// �� ���
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
	// �� ���
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