#include <iostream>
#include <WS2tcpip.h>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")

#include "protocol.h"

const short SERVER_PORT = 9000;
const int BUFSIZE = 40;

char client_pos_x = 0;
char client_pos_y = 0;


int main()
{
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
	char send_buf[2];
	WSABUF s_mybuf;
	DWORD sent_byte;

	cout << "클라 접속 완료" << endl;

	send_buf[0] = client_pos_x;
	send_buf[1] = client_pos_y;
	s_mybuf.buf = send_buf;
	s_mybuf.len = 2;
	WSASend(c_socket, &s_mybuf, 1, &sent_byte, 0, 0, 0);
	
	cout << "최초 정보 전달" << endl;

	for (;;) {
		char recv_buf[BUFSIZE];
		WSABUF mybuf;
		mybuf.buf = recv_buf; 
		mybuf.len = BUFSIZE;
		DWORD recv_byte;
		DWORD recv_flag = 0;
		WSARecv(c_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
		cout << "Client X : " << (int)recv_buf[0] << "Y : " << (int)recv_buf[1] << endl;


		switch (recv_buf[0]) {
		case 1://왼쪽키
			if (client_pos_x > 0) {
				client_pos_x--;
			}
			break;
		case 2://오른쪽키
			if (client_pos_x < 7) {
				client_pos_x++;
			}
			break;
		case 3://위키
			if (client_pos_y > 0) {
				client_pos_y--;
			}
			break;
		case 4:
			if (client_pos_y < 7) {
				client_pos_y++;
			}
			break;
		}

		send_buf[0] = client_pos_x;
		send_buf[1] = client_pos_y;
		s_mybuf.buf = send_buf;
		s_mybuf.len = 2;


		//mybuf.len = recv_byte;
		WSASend(c_socket, &s_mybuf, 1, &sent_byte, 0, 0, 0);
	}
	closesocket(c_socket);
	closesocket(s_socket);
	WSACleanup();
}
