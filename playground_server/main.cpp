#include <iostream>
#include <WS2tcpip.h>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")
const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

void error_display(const char* msg, int err_no)
{
	

	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);
}


int main()
{
	std::wcout.imbue(std::locale("korean"));

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(s_socket, SOMAXCONN);
	INT addr_size = sizeof(server_addr);
	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
	for (;;) {
		char recv_buf[BUFSIZE];
		WSABUF mybuf;
		mybuf.buf = recv_buf; mybuf.len = BUFSIZE;
		DWORD recv_byte;
		DWORD recv_flag = 0;
		int ret = WSARecv(c_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
		if (ret != 0) {
			int err_num = WSAGetLastError();
			error_display("WSARECV : ", err_num);
		}
		cout << "Client Sent [" << recv_byte << "bytes] : " << recv_buf << endl;
		DWORD sent_byte;
		mybuf.len = recv_byte;
		WSASend(c_socket, &mybuf, 1, &sent_byte, 0, 0, 0);
	}
	closesocket(c_socket);
	closesocket(s_socket);
	WSACleanup();
}
