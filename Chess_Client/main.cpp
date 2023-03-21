#include"stdafx.h"

#include "../Chess_Server/protocol.h"

#define screenW 815
#define screenH 835
#define board_div 8

using namespace std;


HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//�̹��� �����


CImage POWN;

int SQUARE_SIZE = (screenW-15)/ board_div;

const COLORREF WHITE_SQUARE_COLOR = RGB(255, 206, 158);
const COLORREF BLACK_SQUARE_COLOR = RGB(209, 139, 71);


//Ŭ���̾�Ʈ�� ũ�⸦ �����մϴ�.
RECT R_client;

POINT position = { 0,0 };

WSAOVERLAPPED s_over;

SOCKET s_socket;
WSABUF s_wsabuf[1];
char s_buf[BUF_SIZE];


//Ÿ�� ���ν���
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	switch (idEvent) {
	case 1:
		break;
	case 2:
		break;
	}
	InvalidateRect(hWnd, NULL, false);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow){
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;


	// ������ Ŭ���� ����ü �� ����
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	// ������ Ŭ���� ���
	RegisterClassEx(&WndClass);
	// ������ ����
	hWnd = CreateWindow(lpszClass, _T("CHESS"), WS_OVERLAPPEDWINDOW, 0, 0, screenW, screenH, NULL, (HMENU)NULL, hInstance, NULL);

	//���
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);
	WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);
	

	// ������ ���
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	// �̺�Ʈ ���� ó��
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//ȭ�� ��¿�
	HDC hdc;
	//���� ���۸���
	HDC memDC;
	//��Ʈ�� �̹��� ��¿�
	//memDC�� ȣȯ�ȴ�
	HDC hMemDC;

	static HBITMAP hBitmap;
	PAINTSTRUCT ps;


	static HBRUSH hBrush;
	static HBRUSH B_INGAME_BG;


	RECT block[3];

	//�ʱ�ȭ������
	POINT msLocation;
	msLocation.x = 0;
	msLocation.y = 0;

	static int BlockSize = 0;

	switch (uMsg) {
	case WM_CREATE:
		//Ÿ�����ν��� ��ȯ
		SetTimer(hWnd, 1, 200, (TIMERPROC)TimerProc);
		SetTimer(hWnd, 2, 100, (TIMERPROC)TimerProc);

		
		
		POWN.Load(L"B_King.png");

	

		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);

		closesocket(s_socket);
		WSACleanup();
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		memDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, screenW, screenH);
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
		B_INGAME_BG = CreateSolidBrush(RGB(183, 240, 177));

		//������ ��Ʈ���� �������ش�. ������ �� ��Ʈ�� ���� �׸����̴�.
		//������۸��� ���ؼ��̴�.
		SelectObject(memDC, (HBITMAP)hBitmap);

		SelectObject(memDC, hBrush);
		Rectangle(memDC, 0, 0, screenW, screenH);

		

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				
				int x = i * SQUARE_SIZE;
				int y = j * SQUARE_SIZE;
				COLORREF color = ((i + j) % 2 == 0) ? WHITE_SQUARE_COLOR : BLACK_SQUARE_COLOR;

				
				HBRUSH hBrush = CreateSolidBrush(color);

				
				HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hBrush);

				Rectangle(memDC, x, y, x + SQUARE_SIZE, y + SQUARE_SIZE);

				
				SelectObject(memDC, hOldBrush);

				
				DeleteObject(hBrush);

				if (i == position.x && j == position.y)
				{
					POWN.Draw(memDC, x, y, SQUARE_SIZE, SQUARE_SIZE);
				}
			}
		}

		

		BitBlt(hdc, 0, 0, screenW, screenH, memDC, 0, 0, SRCCOPY);


		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RIGHT:
			if (position.x < board_div-1) {
				position.x++;
			}			
			break;
		case VK_LEFT:
			if (position.x > 0) {
				position.x--;
			}
			break;
		case VK_DOWN:
			if (position.y < board_div - 1) {
				position.y++;
			}
			
			break;
		case VK_UP:
			if (position.y > 0) {
				position.y--;
			}
			break;
		}		

		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_MOUSEMOVE:
		msLocation.x = LOWORD(lParam);
		msLocation.y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		

		InvalidateRect(hWnd, NULL, false);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); // ���� �� �޽��� ���� ������ �޽����� OS��
}