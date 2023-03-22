
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <WS2tcpip.h>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")


#include <windows.h>
#include <atlimage.h>




#ifdef _DEBUG

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#endif


#include "../Chess_Server/protocol.h"



#define screenW 815
#define screenH 835
#define board_div 8


using namespace std;

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//이미지 선언부


CImage POWN;

int SQUARE_SIZE = (screenW-15)/ board_div;

const COLORREF WHITE_SQUARE_COLOR = RGB(255, 206, 158);
const COLORREF BLACK_SQUARE_COLOR = RGB(209, 139, 71);


//클라이언트의 크기를 저장합니다.
RECT R_client;

POINT position = { 0,0 };


//통신정보

constexpr short SERVER_PORT = 9000;
constexpr int BUF_SIZE = 200;

WSAOVERLAPPED s_over;

SOCKET s_socket;
WSABUF s_wsabuf[1];
char s_buf[BUF_SIZE];

char recv_buf[BUF_SIZE];
WSABUF mybuf;
DWORD recv_byte;
DWORD recv_flag = 0;



void recv_thread() {
	while (1) {
		mybuf.buf = recv_buf;
		mybuf.len = BUF_SIZE;
		WSARecv(s_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
		cout << "정보 수신" << endl;
		cout << (int)recv_buf[0] << endl;
		cout << (int)recv_buf[1] << endl;

		position.x = recv_buf[0];
		position.y = recv_buf[1];
	}
}


//타임 프로시저
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


	// 윈도우 클래스 구조체 값 설정
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
	// 윈도우 클래스 등록
	RegisterClassEx(&WndClass);
	// 윈도우 생성
	hWnd = CreateWindow(lpszClass, _T("CHESS"), WS_OVERLAPPEDWINDOW, 0, 0, screenW, screenH, NULL, (HMENU)NULL, hInstance, NULL);

	//통신
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);
	WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);
	
	cout << "서버 접속 완료" << endl;

	thread t_recv(recv_thread);
	// 윈도우 출력
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	// 이벤트 루프 처리
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	t_recv.join();
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//화면 출력용
	HDC hdc;
	//더블 버퍼링용
	HDC memDC;
	//비트맵 이미지 출력용
	//memDC와 호환된다
	HDC hMemDC;

	static HBITMAP hBitmap;
	PAINTSTRUCT ps;


	static HBRUSH hBrush;
	static HBRUSH B_INGAME_BG;


	RECT block[3];

	//초기화해주자
	POINT msLocation;
	msLocation.x = 0;
	msLocation.y = 0;

	static int BlockSize = 0;


	mybuf.buf = recv_buf;
	mybuf.len = BUF_SIZE;


	switch (uMsg) {
	case WM_CREATE:
		//타임프로시저 소환
		SetTimer(hWnd, 1, 200, (TIMERPROC)TimerProc);		
		
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

		//생성한 비트맵을 선택해준다. 앞으로 이 비트맵 위에 그릴것이다.
		//더블버퍼링을 위해서이다.
		SelectObject(memDC, (HBITMAP)hBitmap);

		SelectObject(memDC, hBrush);
		Rectangle(memDC, 0, 0, screenW, screenH);

		//cout << "RECV 대기중" << endl;
		//WSARecv(s_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
		

		

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
		char send_buf[1] = { 0 };
		//키입력 있으면은 send
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			send_buf[0] = 1;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			send_buf[0] = 2;
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			send_buf[0] = 3;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			send_buf[0] = 4;
		}
		if (send_buf[0] != 0) {
			
			WSABUF s_buf;

			s_buf.buf = send_buf;
			s_buf.len = 1;

			DWORD sent_byte;
			cout << "SEND가 일을 하니?" << endl;
			WSASend(s_socket, &s_buf, 1, &sent_byte, 0, 0, 0);
		}

		BitBlt(hdc, 0, 0, screenW, screenH, memDC, 0, 0, SRCCOPY);


		EndPaint(hWnd, &ps);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); // 위의 세 메시지 외의 나머지 메시지는 OS로
}