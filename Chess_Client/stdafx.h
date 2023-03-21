#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <windows.h>
#include <atlimage.h>

#ifdef _DEBUG

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#endif

#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "WS2_32.LIB")

constexpr short SERVER_PORT = 3500;
constexpr int BUF_SIZE = 200;