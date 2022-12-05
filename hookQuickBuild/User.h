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
	void Initialize(std::unordered_map<std::string, uint32_t>& dic);

	//获取参数
	std::string GetPath(std::string command);

	//热重载相关函数
	void HotReset(std::unordered_map<std::string, uint32_t>& dic);

	//组合指令
	std::string CommandSplicing(std::vector<uint32_t> command);

	//文本指令拆分组合
	std::string TextCommandToNumber(const std::string& command, 
		std::unordered_map<std::string, uint32_t>& dic);

	//测试打印函数
	void TextPrint();

private:

	std::unordered_map<std::string, std::string> m_userDefine;
	
};