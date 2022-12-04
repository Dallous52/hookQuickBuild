#include "hook.h"
#include "fileCtl.h"
#include "programSet.h"

Config config;
User user;

HHOOK keyboardHook = 0;		// 钩子句柄

LRESULT CALLBACK ExecuteProc(
	_In_ int nCode,		// 规定钩子如何处理消息，小于 0 则直接 CallNextHookEx
	_In_ WPARAM wParam,	// 消息类型
	_In_ LPARAM lParam)	// 指向某个结构体的指针，这里是 KBDLLHOOKSTRUCT（低级键盘输入事件)
{
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息
	/*
	typedef struct tagKBDLLHOOKSTRUCT {
		DWORD     vkCode;		// 按键代号
		DWORD     scanCode;		// 硬件扫描代号，同 vkCode 也可以作为按键的代号。
		DWORD     flags;		// 事件类型，一般按键按下为 0 抬起为 128。
		DWORD     time;			// 消息时间戳
		ULONG_PTR dwExtraInfo;	// 消息附加信息，一般为 0。
	}KBDLLHOOKSTRUCT,*LPKBDLLHOOKSTRUCT,*PKBDLLHOOKSTRUCT;
	*/

	string command;

	for (auto tmp : config.m_parameter)
	{
		if (IsKeyPressed(tmp.first))
		{
			if (command.empty())
			{
				command = tmp.second;
			}
			else
			{
				command += "+" + tmp.second;
			}
		}
	}
		
	string programExit = user.GetPath("key_to_exit");
	if (command == programExit)
	{
		exit(0);
	}

	string programReset = user.GetPath("key_to_reset");
	if (command == programReset)
	{
		HotReset();
	}

	string becomeAdmin = user.GetPath("become_admin");
	if (command == becomeAdmin)
	{
		GainAdminPrivileges();
	}

	string path = user.GetPath(command);
	if (!path.empty())
	{
		PROCESS_INFORMATION pi; //启动窗口的信息

		STARTUPINFO si; //进程的信息

		memset(&si, 0, sizeof(si));

		si.cb = sizeof(si);

		si.wShowWindow = SW_SHOW;

		si.dwFlags = STARTF_USESHOWWINDOW;

		LPCWSTR pa = stringToLPCWSTR(path);
		bool fRet = CreateProcess(pa, NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &si, &pi);

		if (fRet)
		{
			cout << command << " :> " << path << endl;
		}

		//不使用的句柄最好关掉
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	// 将消息传递给钩子链中的下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int _tmain(int argc, _TCHAR* argv[])
{
	config.Initialize();
	user.Initialize();

	if (stoi(user.GetPath("is_print_proc")))
	{
		config.TextPrint();
		user.TextPrint();
	}
	
	SetStartRunning();

	bool iskeyboard = stoi(user.GetPath("is_keyboard_monitor"));

	//获取最前端的窗口的句柄 
	HWND hwnd = GetForegroundWindow();

	//隐藏窗口
	ShowWindow(hwnd, iskeyboard);

	//不可漏掉消息处理，不然程序会卡死
	MSG msg;
	while (true)
	{
		if (GetKeyState(VK_SCROLL))
		{
			if (keyboardHook == 0)
			{
				if (iskeyboard)
				{
					// 安装钩子
					keyboardHook = SetWindowsHookEx(
						WH_KEYBOARD_LL,			// 钩子类型，WH_KEYBOARD_LL 为键盘钩子
						LowLevelKeyboardProc,	// 指向钩子函数的指针
						GetModuleHandleA(NULL),	// Dll 句柄
						NULL);
				}
				else
				{
					// 安装钩子
					keyboardHook = SetWindowsHookEx(
						WH_KEYBOARD_LL,			// 钩子类型，WH_KEYBOARD_LL 为键盘钩子
						ExecuteProc,			// 指向钩子函数的指针
						GetModuleHandleA(NULL),	// Dll 句柄
						NULL);
				}

				if (keyboardHook == 0)
				{
					//cout << "挂钩键盘失败" << endl;
					return -1;
				}
			}

			// 如果消息队列中有消息
			if (PeekMessageA(
				&msg,		// MSG 接收这个消息
				NULL,		// 检测消息的窗口句柄，NULL：检索当前线程所有窗口消息
				NULL,		// 检查消息范围中第一个消息的值，NULL：检查所有消息（必须和下面的同时为NULL）
				NULL,		// 检查消息范围中最后一个消息的值，NULL：检查所有消息（必须和上面的同时为NULL）
				PM_REMOVE	// 处理消息的方式，PM_REMOVE：处理后将消息从队列中删除
			)) {
				// 把按键消息传递给字符消息
				TranslateMessage(&msg);

				// 将消息分派给窗口程序
				DispatchMessageW(&msg);
			}
			else
				Sleep(0);    //避免CPU全负载运行
		}
		else
		{
			if (keyboardHook != 0)
			{
				// 删除钩子
				UnhookWindowsHookEx(keyboardHook);
				keyboardHook = 0;
			}
			Sleep(1000);
		}
	}	

	if (keyboardHook != 0)
	{
		// 删除钩子
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = 0;
	}

	return 0;
}


//以下为键盘记录器代码

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int nCode,		// 规定钩子如何处理消息，小于 0 则直接 CallNextHookEx
	_In_ WPARAM wParam,	// 消息类型
	_In_ LPARAM lParam)	// 指向某个结构体的指针，这里是 KBDLLHOOKSTRUCT（低级键盘输入事件)
{
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;			// 包含低级键盘输入事件信息
	/*
	//bool   IS_SHIFT = IsKeyPressed(0x10);
	//string revalue = "";
	////判断键盘中间的特殊符号

	//if (ks->flags == 0 || ks->flags == 1)
	//{
	//	if (ks->vkCode >= 186 && ks->vkCode <= 222)
	//		switch (ks->vkCode)
	//		{
	//		case 186:
	//			if (IS_SHIFT)
	//				revalue = ":";
	//			else
	//				revalue = ";";
	//			break;
	//		case 187:
	//			if (IS_SHIFT)
	//				revalue = "+";
	//			else
	//				revalue = "=";
	//			break;
	//		case 188:
	//			if (IS_SHIFT)
	//				revalue = "<";
	//			else
	//				revalue = ",";
	//			break;
	//		case 189:
	//			if (IS_SHIFT)
	//				revalue = "_";
	//			else
	//				revalue = "-";
	//			break;
	//		case 190:
	//			if (IS_SHIFT)
	//				revalue = ">";
	//			else
	//				revalue = ".";
	//			break;
	//		case 191:
	//			if (IS_SHIFT)
	//				revalue = "?";
	//			else
	//				revalue = "/";
	//			break;
	//		case 192:
	//			if (IS_SHIFT)
	//				revalue = "~";
	//			else
	//				revalue = "`";
	//			break;
	//		case 219:
	//			if (IS_SHIFT)
	//				revalue = "{";
	//			else
	//				revalue = "[";
	//			break;
	//		case 220:
	//			if (IS_SHIFT)
	//				revalue = "|";
	//			else
	//				revalue = "\\";
	//			break;
	//		case 221:
	//			if (IS_SHIFT)
	//				revalue = "}";
	//			else
	//				revalue = "]";
	//			break;
	//		case 222:
	//			if (IS_SHIFT)
	//				revalue = '"';
	//			else
	//				revalue = ",";
	//		default:
	//			revalue = "error";
	//			break;
	//		}


	//	if (ks->vkCode == VK_ESCAPE) // 退出
	//		revalue = "[Esc]";
	//	else if (ks->vkCode == VK_F1) // F1至F12
	//		revalue = "[F1]";
	//	else if (ks->vkCode == VK_F2)
	//		revalue = "[F2]";
	//	else if (ks->vkCode == VK_F3)
	//		revalue = "[F3]";
	//	else if (ks->vkCode == VK_F4)
	//		revalue = "[F4]";
	//	else if (ks->vkCode == VK_F5)
	//		revalue = "[F5]";
	//	else if (ks->vkCode == VK_F6)
	//		revalue = "[F6]";
	//	else if (ks->vkCode == VK_F7)
	//		revalue = "[F7]";
	//	else if (ks->vkCode == VK_F8)
	//		revalue = "[F8]";
	//	else if (ks->vkCode == VK_F9)
	//		revalue = "[F9]";
	//	else if (ks->vkCode == VK_F10)
	//		revalue = "[F10]";
	//	else if (ks->vkCode == VK_F11)
	//		revalue = "[F11]";
	//	else if (ks->vkCode == VK_F12)
	//		revalue = "[F12]";
	//	else if (ks->vkCode == VK_SNAPSHOT) // 打印屏幕
	//		revalue = "[PrScrn]";
	//	else if (ks->vkCode == VK_SCROLL) // 滚动锁定
	//		revalue = "[Scroll Lock]";
	//	else if (ks->vkCode == VK_PAUSE) // 暂停、中断
	//		revalue = "[Pause]";
	//	else if (ks->vkCode == VK_CAPITAL) // 大写锁定
	//		revalue = "[Caps Lock]";
	//	else if (ks->vkCode == 8) //<- 回格键
	//		revalue = "[Backspace]";
	//	else if (ks->vkCode == VK_RETURN) // 回车键、换行
	//		revalue = "[Enter]\n";
	//	else if (ks->vkCode == VK_SPACE) // 空格
	//		revalue = "[Space]";
	//	else if (ks->vkCode == VK_TAB) // 制表键
	//		revalue = "[Tab]";
	//	else if (ks->vkCode == VK_RSHIFT) // 右SHIFT键
	//		revalue = "[Shift]";
	//	else if (ks->vkCode == VK_LSHIFT) // 左SHIFT键
	//		revalue = "[Shift]";
	//	else if (ks->vkCode == VK_LCONTROL) // 左控制键
	//		revalue = "[Ctrl]";
	//	else if (ks->vkCode == VK_RCONTROL) // 右控制键
	//		revalue = "[CTRL]";
	//	else if (ks->vkCode == VK_LMENU) // 左换档键
	//		revalue = "[Alt]";
	//	else if (ks->vkCode == VK_RMENU) // 右换档键
	//		revalue = "[ALT]";
	//	else if (ks->vkCode == VK_LWIN) // 右 WINDOWS 键
	//		revalue = "[Win]";
	//	else if (ks->vkCode == VK_RWIN) // 右 WINDOWS 键
	//		revalue = "[WIN]";
	//	else if (ks->vkCode == VK_APPS) // 键盘上 右键
	//		revalue = "[RightK]";
	//	else if (ks->vkCode == VK_INSERT) // 插入
	//		revalue = "[Insert]";
	//	else if (ks->vkCode == VK_DELETE) // 删除
	//		revalue = "[Delete]";
	//	else if (ks->vkCode == VK_HOME) // 起始
	//		revalue = "[Home]";
	//	else if (ks->vkCode == VK_END) // 结束
	//		revalue = "[End]";
	//	else if (ks->vkCode == VK_PRIOR) // 上一页
	//		revalue = "[PgUp]";
	//	else if (ks->vkCode == VK_NEXT) // 下一页
	//		revalue = "[PgDown]";
	//	// 不常用的几个键:一般键盘没有
	//	else if (ks->vkCode == VK_CANCEL) // Cancel
	//		revalue = "[Cancel]";
	//	else if (ks->vkCode == VK_CLEAR) // Clear
	//		revalue = "[Clear]";
	//	else if (ks->vkCode == VK_SELECT) //Select
	//		revalue = "[Select]";
	//	else if (ks->vkCode == VK_PRINT) //Print
	//		revalue = "[Print]";
	//	else if (ks->vkCode == VK_EXECUTE) //Execute
	//		revalue = "[Execute]";

	//	//----------------------------------------//
	//	else if (ks->vkCode == VK_LEFT) //上、下、左、右键
	//		revalue = "[Left]";
	//	else if (ks->vkCode == VK_RIGHT)
	//		revalue = "[Right]";
	//	else if (ks->vkCode == VK_UP)
	//		revalue = "[Up]";
	//	else if (ks->vkCode == VK_DOWN)
	//		revalue = "[]";
	//	else if (ks->vkCode == VK_NUMLOCK)//小键盘数码锁定
	//		revalue = "[NumLock]";
	//	else if (ks->vkCode == VK_ADD) // 加、减、乘、除
	//		revalue = "+";
	//	else if (ks->vkCode == VK_SUBTRACT)
	//		revalue = "-";
	//	else if (ks->vkCode == VK_MULTIPLY)
	//		revalue = "*";
	//	else if (ks->vkCode == VK_DIVIDE)
	//		revalue = "/";
	//	else if (ks->vkCode == 190 || ks->vkCode == 110) // 小键盘 . 及键盘 .
	//		revalue = ".";
	//	//小键盘数字键:0-9
	//	else if (ks->vkCode == VK_NUMPAD0)
	//		revalue = "0";
	//	else if (ks->vkCode == VK_NUMPAD1)
	//		revalue = "1";
	//	else if (ks->vkCode == VK_NUMPAD2)
	//		revalue = "2";
	//	else if (ks->vkCode == VK_NUMPAD3)
	//		revalue = "3";
	//	else if (ks->vkCode == VK_NUMPAD4)
	//		revalue = "4";
	//	else if (ks->vkCode == VK_NUMPAD5)
	//		revalue = "5";
	//	else if (ks->vkCode == VK_NUMPAD6)
	//		revalue = "6";
	//	else if (ks->vkCode == VK_NUMPAD7)
	//		revalue = "7";
	//	else if (ks->vkCode == VK_NUMPAD8)
	//		revalue = "8";
	//	else if (ks->vkCode == VK_NUMPAD9)
	//		revalue = "9";
	//	//----------------------------上述代码判断键盘上除了字母之外的功能键--------------------------------//
	//	else if (ks->vkCode >= 65 && ks->vkCode <= 90)
	//	{
	//		if (GetKeyState(VK_CAPITAL))
	//		{
	//			if (IS_SHIFT)
	//				revalue = ks->vkCode + 32;
	//			else
	//				revalue = ks->vkCode;
	//		}
	//		else
	//		{
	//			if (IS_SHIFT)
	//				revalue = ks->vkCode;
	//			else
	//				revalue = ks->vkCode + 32;
	//		}
	//	}
	//	//---------------------------上面的部分判断键盘上的字母----------------------------------------------//
	//	else if (ks->vkCode >= 48 && ks->vkCode <= 57)
	//	{
	//		if (IS_SHIFT)
	//		{
	//			switch (ks->vkCode)
	//			{
	//			case 48:revalue = ")"; break;
	//			case 49:revalue = "!"; break;
	//			case 50:revalue = "@"; break;
	//			case 51:revalue = "#"; break;
	//			case 52:revalue = "$"; break;
	//			case 53:revalue = "%"; break;
	//			case 54:revalue = "^"; break;
	//			case 55:revalue = "&"; break;
	//			case 56:revalue = "*"; break;
	//			case 57:revalue = "("; break;
	//			}
	//		}
	//		else
	//		{
	//			switch (ks->vkCode)
	//			{
	//			case 48:revalue = "0"; break;
	//			case 49:revalue = "1"; break;
	//			case 50:revalue = "2"; break;
	//			case 51:revalue = "3"; break;
	//			case 52:revalue = "4"; break;
	//			case 53:revalue = "5"; break;
	//			case 54:revalue = "6"; break;
	//			case 55:revalue = "7"; break;
	//			case 56:revalue = "8"; break;
	//			case 57:revalue = "9"; break;
	//			}
	//		}
	//	}
	*/
	string command;

	if (ks->flags == 128 || ks->flags == 129)
	{
		for (auto tmp : config.m_parameter)
		{
			if (IsKeyPressed(tmp.first))
			{
				if (command.empty())
				{
					command = tmp.second;
				}
				else
				{
					command += "+" + tmp.second;
				}
			}
		}

		string programExit = user.GetPath("key_to_exit");
		if (command == programExit)
		{
			exit(0);
		}

		string programReset = user.GetPath("key_to_reset");
		if (command == programReset)
		{
			HotReset();
		}

		string path = user.GetPath(command);

		cout << command << " :> " << path << endl;
	}
	

	// 将消息传递给钩子链中的下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}