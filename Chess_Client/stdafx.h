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