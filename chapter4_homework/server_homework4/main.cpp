#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <random>
#include <queue>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

constexpr int VIEW_RANGE = 4;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_AI };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	EVENT_TYPE _event_type;

	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		::ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		::ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
	OVER_EXP _recv_over;

public:
	mutex _s_lock;
	S_STATE _state;
	int _id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	int		_last_move_time;

	unordered_set <int> _view_list;
	unordered_set <int> _zone_list;
	mutex _zl;
	mutex _vl;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		::memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_remove_player_packet(int c_id)
	{
		_vl.lock();
		if (_view_list.count(c_id) == 0) {
			_vl.unlock();
			return;
		}
		_view_list.erase(c_id);
		_vl.unlock();
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
		
	}
};

class ZONE {
public:
	int x_min_, x_max_, y_min_, y_max_;
	int my_num = -1;
	std::unordered_set<int> user_list;
	mutex zl;
public:
	ZONE(const ZONE& zone) 
		: x_min_(zone.x_min_), x_max_(zone.x_max_), y_min_(zone.y_min_), y_max_(zone.y_max_), my_num(zone.my_num) {

	}
	ZONE(int x_min, int x_max, int y_min, int y_max,int num)
		: x_min_(x_min), x_max_(x_max), y_min_(y_min), y_max_(y_max),my_num(num)
	{
		
	}
	bool contains(SESSION& player) {
		
		bool is_in = false;
		//내부에 있는지 검사
		if (player.x >= x_min_ && player.x <= x_max_ && player.y >= y_min_ && player.y <= y_max_) 
		{
			is_in = true;
		};

		if (is_in) {
			//없을때 처리
			//없을때만 검사하고 동작시켜야죵
			zl.lock();
			if (user_list.count(player._id) == 0) {
				//유저 zonelist에 추가
				//내꺼도 추가
				
				user_list.insert(player._id);
				zl.unlock();
				player._zl.lock();
				player._zone_list.insert(my_num);
				player._zl.unlock();
				return is_in;
			}
			zl.unlock();
		}
		else {
			//있을때 처리
			//있을때만 검사하고 동작시켜야죵
			zl.lock();
			if (user_list.count(player._id) != 0) {
				//유저 zonelist에서 빼줘야됨
				//내꺼에서도 빼야됨

				
				user_list.erase(player._id);
				zl.unlock();
				player._zl.lock();
				player._zone_list.erase(my_num);
				player._zl.unlock();
				return is_in;
			}
			zl.unlock();
		}

		return is_in;
	}

};

class MAP {
public:
	vector<ZONE> zone_list;
public:
	//존 크기 오토로 만들기
	void make_zone_auto(int sep_count) {
		/*전체 크기 나누기 몇개의 존 생성할지 
		  버퍼존은 양쪽에 적용되서 실질적 2배
		  맨 끝에 있는 영역의 초과는 이동에서 막아줌으로 별도의 처리는 하지 않는걸로
		*/
		int counter = 0;
		
		int x = W_WIDTH / sep_count;
		int y = W_HEIGHT / sep_count;
		int buf_size = (x + y) / 6; // X랑 Y의 평균의 1/3정도
		for (int i = 0; i < sep_count; ++i) {
			for (int j = 0; j < sep_count; ++j) {
				ZONE tmp(
					x * j - buf_size, 
					x * (j + 1) + buf_size, 
					y * i - buf_size,
					y * (i + 1) + buf_size,
					counter
				);
				tmp.my_num = counter++;
				zone_list.emplace_back(tmp);
			}
		}
	}

	
};

enum EVENT_TYPE{EV_RANDOM_MOVE,EV_ATTACK,EV_HEAL};
class EVENT {
public:
	int _oid;
	EVENT_TYPE _type;
	chrono::system_clock::time_point _exec_time;

public:
	EVENT(int id, EVENT_TYPE t, chrono::system_clock::time_point tmp) :
		_oid(id),
		_type(t),
		_exec_time(tmp) 
	{
		
	}
	constexpr bool operator< (const EVENT& _Left) const {
		return _exec_time > _Left._exec_time;
	}
};
array<SESSION, MAX_USER+MAX_NPC> clients;
MAP g_map;
SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
HANDLE h_iocp;



priority_queue<EVENT> timer_queue;

void SESSION::send_move_packet(int c_id)
{
	_vl.lock();
	if (_view_list.count(c_id) != 0) {
		_vl.unlock();
		SC_MOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(SC_MOVE_PLAYER_PACKET);
		p.type = SC_MOVE_PLAYER;
		p.x = clients[c_id].x;
		p.y = clients[c_id].y;
		p.move_time = clients[c_id]._last_move_time;
		do_send(&p);
	}
	else {
		_vl.unlock();
		send_add_player_packet(c_id);
	}
}

void SESSION::send_add_player_packet(int c_id)
{
	_vl.lock();
	if (_view_list.count(c_id) != 0) {
		_vl.unlock();
		send_move_packet(c_id);
		return;
	}
	_view_list.insert(c_id);
	_vl.unlock();

	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.x = clients[c_id].x;
	add_packet.y = clients[c_id].y;
	do_send(&add_packet);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

bool can_see(int p1, int p2)
{
	// return VIEW_RANGE <= SQRT((p1.x - p2.x) ^ 2 + (p1.y - p2.y) ^ 2);
	if (abs(clients[p1].x - clients[p2].x) > VIEW_RANGE) return false;
	if (abs(clients[p1].y - clients[p2].y) > VIEW_RANGE) return false;
	return true;
}

void do_npc_ai(int c_id);

//자꾸 생성하는건 좀 힘들어 하네
default_random_engine engine;

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		
		engine.seed(c_id);
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].x = engine() % W_WIDTH;
		clients[c_id].y = engine() % W_HEIGHT;
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id]._s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		/*for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			if (can_see(c_id, pl._id) == false) continue;
			pl.send_add_player_packet(c_id);
			clients[c_id].send_add_player_packet(pl._id);
		}*/
		for (auto& m : g_map.zone_list) {
			m.contains(clients[c_id]);
		}
		for (auto& vl : clients[c_id]._zone_list) {
			for (auto& p : g_map.zone_list[vl].user_list) {
				if (p == c_id) continue;
				if (can_see(c_id, p) == false) continue;
				clients[p].send_add_player_packet(c_id);
				clients[c_id].send_add_player_packet(p);
			}
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]._last_move_time = p->move_time;
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;

		//cout << clients[c_id].x << ":" << clients[c_id].y << endl;

		clients[c_id]._vl.lock();
		auto old_vl = clients[c_id]._view_list;
		clients[c_id]._vl.unlock();


		//Todo : 모든 클라이언트 검사가 아니라 zone에서 검사하자

		for (auto& m : g_map.zone_list) {
			m.contains(clients[c_id]);
		}

		unordered_set <int> new_vl;

		for (auto& vl : clients[c_id]._zone_list) {
			for (auto& p : g_map.zone_list[vl].user_list) {
				if (p == c_id) continue;
				if (can_see(c_id, p) == false) continue;
				new_vl.insert(p);
			}
		}

		for (auto& o : new_vl) {
			if (old_vl.count(o) == 0) {
				clients[o].send_add_player_packet(c_id);
				clients[c_id].send_add_player_packet(o);
			}
			else {
				clients[o].send_move_packet(c_id);
				clients[c_id].send_move_packet(o);
			}
		}
		clients[c_id].send_move_packet(c_id);

		}
	}
}

void disconnect(int c_id)
{
	for (auto& pl : clients) {
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
	clients[c_id]._zl.lock();
	clients[c_id]._view_list.clear();
	clients[c_id]._zl.unlock();
	for (auto& m : g_map.zone_list) {
		if (m.user_list.count(c_id) != 0) {
			m.zl.lock();
			m.user_list.erase(c_id);
			m.zl.unlock();
		}
	}

	closesocket(clients[c_id]._socket);

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	clients[c_id]._state = ST_FREE;
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			::ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		case OP_NPC_AI:

			EVENT_TYPE evt = ex_over->_event_type;
			do_npc_ai(key);
			delete ex_over;
			break;
		}
	}
}

void do_random_move(int c_id) {
	//cout << c_id << endl;
	
	short x = clients[c_id].x;
	short y = clients[c_id].y;

	switch (rand()%4) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}
	clients[c_id].x = x;
	clients[c_id].y = y;

	clients[c_id]._vl.lock();
	auto old_vl = clients[c_id]._view_list;
	clients[c_id]._vl.unlock();


	//Todo : 모든 클라이언트 검사가 아니라 zone에서 검사하자

	for (auto& m : g_map.zone_list) {
		m.contains(clients[c_id]);
	}

	unordered_set <int> new_vl;

	for (auto& vl : clients[c_id]._zone_list) {
		for (auto& p : g_map.zone_list[vl].user_list) {
			if (p > MAX_USER) continue;
			if (can_see(c_id, p) == false) continue;			
			new_vl.insert(p);
		}
	}
	

	for (auto& o : new_vl) {
		if (old_vl.count(o) == 0) {
			clients[o].send_add_player_packet(c_id);
			//clients[c_id].send_add_player_packet(o);
		}
		else {
			clients[o].send_move_packet(c_id);
			//clients[c_id].send_move_packet(o);
		}
	}
}

void do_npc_ai(int c_id) {
	do_random_move(c_id);
}

void init_npc() {
	for (int i = 0; i < MAX_NPC; ++i) {
		int npc_id = i + MAX_USER;
		
		clients[npc_id].x = rand() % W_WIDTH;
		clients[npc_id].y = rand() % W_HEIGHT;
		clients[npc_id]._state = ST_INGAME;
		clients[npc_id]._id = npc_id;
		sprintf_s(clients[npc_id]._name, "NPC%d", npc_id);
	}

}

void do_ai() {
	
	while (1) {
		for (int i = 0; i < MAX_NPC; ++i) {
			int npc_id = i + MAX_USER;
			OVER_EXP* ov = new OVER_EXP();
			ov->_comp_type = OP_NPC_AI;
			PostQueuedCompletionStatus(h_iocp, 1, npc_id, &ov->_over);			
		}
	}
	
}

void do_timer() {
	while (1) {
		auto ev = timer_queue.top();
		if (ev._exec_time > chrono::system_clock::now()) {
			this_thread::sleep_for(10ms);
			continue;
		}
		timer_queue.pop();

		int npc_id = ev._oid;
		OVER_EXP* ov = new OVER_EXP();
		ov->_comp_type = OP_NPC_AI;
		ov->_event_type = ev._type;
		PostQueuedCompletionStatus(h_iocp, 1, npc_id, &ov->_over);
		
	}
}

int main()
{
	g_map.make_zone_auto(20);

	

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
	init_npc();
	//thread ai_thread{ do_ai };
	thread timer_thread{ do_timer };
	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();

	//ai_thread.join();
	timer_thread.join();
	closesocket(g_s_socket);
	WSACleanup();
}
