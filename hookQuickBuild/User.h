#pragma once
#include "Profile.h"
#include <unordered_map>

#define F_USER "E:\\product\\c_c++\\hookQuickBuild\\x64\\Release\\userdefine.proc"

class User : private ProFile
{
public:
	User();
	~User();

	//初始化
	void Initialize();

	std::string GetPath(std::string command);

	//测试打印函数
	void TextPrint();

private:
	std::unordered_map<std::string, std::string> m_userDefine;
};