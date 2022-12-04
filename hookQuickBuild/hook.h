#pragma once
#include <windows.h>
#include <Winuser.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <conio.h>
#include <tchar.h>
using namespace std;

//判断按键是否按下true为按下
#define  IsKeyPressed(nVirtKey)     ((GetKeyState(nVirtKey) & (1<<(sizeof(SHORT)*8-1))) != 0)


//钩子执行函数
LRESULT CALLBACK LowLevelKeyboardProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);

//打开运行进程函数
LRESULT CALLBACK ExecuteProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);