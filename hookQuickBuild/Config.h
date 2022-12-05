#pragma once
#include "Profile.h"
#include <unordered_map>

#define File_Profile_Ctl "E:\\product\\c_c++\\hookQuickBuild\\x64\\Release\\config.proc"

class Config : private ProFile
{
public:
	Config();
	~Config();

	//初始化设置参数
	void Initialize();
	
	//获取键名
	std::string GetKeyName(uint32_t code);

	//热重载相关函数
	void HotReset();

	//测试打印函数
	void TextPrint();
	
	//键盘按键与对应键值
	std::unordered_map<uint32_t, std::string>	m_parameter;

	std::unordered_map<std::string, uint32_t>	m_reverse;

};