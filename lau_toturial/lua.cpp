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
	//표준 라이브러리 로딩
	luaL_openlibs(L);

	luaL_loadfile(L, "dragon.lua");
	
	int err = lua_pcall(L, 0, 0, 0);
	if (err) {
		cout << "ERROR : " << lua_tostring(L,-1) << endl;
	}

	//C 함수 등록
	lua_register(L, "c_addnum", c_addnum);
	//루아 함수 부르기
	lua_getglobal(L, "addnum");
	//스텍에 값 넣어두기
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	//함수 호출스
	lua_pcall(L, 2, 1, 0);
	//결과 받고 (가상머신 스텍에 있는거 받아오기)
	int sum = lua_tonumber(L, -1);
	//스텍 깔끔하게 비우기
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