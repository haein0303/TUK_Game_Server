#include<iostream>
#include "include/lua.hpp"

#pragma comment(lib,"lua54.lib")

using namespace std;

int c_addnum(lua_State* L) {
	int a = lua_tonumber(L, -2);
	int b = lua_tonumber(L, -1);
	lua_pop(L, 3);

	lua_pushnumber(L, a + b);
	return 1;
}

int main() {
	
	lua_State* L = luaL_newstate();
	//ǥ�� ���̺귯�� �ε�
	luaL_openlibs(L);

	luaL_loadfile(L, "dragon.lua");
	
	int err = lua_pcall(L, 0, 0, 0);
	if (err) {
		cout << "ERROR : " << lua_tostring(L,-1) << endl;
	}

	//C �Լ� ���
	lua_register(L, "c_addnum", c_addnum);
	//��� �Լ� �θ���
	lua_getglobal(L, "addnum");
	//���ؿ� �� �־�α�
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	//�Լ� ȣ�⽺
	lua_pcall(L, 2, 1, 0);
	//��� �ް� (����ӽ� ���ؿ� �ִ°� �޾ƿ���)
	int sum = lua_tonumber(L, -1);
	//���� ����ϰ� ����
	lua_pop(L, 1);

	cout << "sum : " << sum << endl;
	/*lua_getglobal(L, "plustwo");


	lua_pushnumber(L, 100);
	lua_pcall(L, 1, 1, 0);
	int result = lua_tonumber(L, -1);
	lua_pop(L,1);

	cout <<"result : " << result << endl;
	lua_getglobal(L, "pos_x");
	lua_getglobal(L, "pos_y");
	int pos_x = lua_tonumber(L, -2);
	int pos_y = lua_tonumber(L, -1);
	lua_pop(L, 2);

	cout << "X : " << pos_x << " Y : " << pos_y << endl;*/
	lua_close(L);
}