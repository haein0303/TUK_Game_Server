#include<iostream>
#include "include/lua.hpp"

#pragma comment(lib,"lua54.lib")

using namespace std;



int main() {
	const char lua_pro[] = "print \"hello\"\n";

	//���������͸� ����ϱ� ���ؼ� ���� �ӽ��� ���
	lua_State* L = luaL_newstate();
	//ǥ�� ���̺귯�� �ε�
	luaL_openlibs(L);

	luaL_loadbuffer(L,lua_pro,strlen(lua_pro),"Line");
	
	int err = lua_pcall(L, 0, 0, 0);
	if (err) {
		cout << "ERROR : " << lua_tostring(L,-1) << endl;
	}
	lua_close(L);
}