#include<iostream>
#include "include/lua.hpp"

#pragma comment(lib,"lua54.lib")

using namespace std;



int main() {
	const char lua_pro[] = "print \"hello\"\n";

	//인터프리터를 사용하기 위해서 가상 머신을 사용
	lua_State* L = luaL_newstate();
	//표준 라이브러리 로딩
	luaL_openlibs(L);

	luaL_loadbuffer(L,lua_pro,strlen(lua_pro),"Line");
	
	int err = lua_pcall(L, 0, 0, 0);
	if (err) {
		cout << "ERROR : " << lua_tostring(L,-1) << endl;
	}
	lua_close(L);
}