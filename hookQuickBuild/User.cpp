#pragma once
#include "User.h"
#include "fileCtl.h"

User::User()
{
	;
}


void User::Initialize()
{
	std::ifstream file;

	file.open(F_USER, std::ios::in);

	if (!file.is_open())
	{
		Error_insert_File(F_LOG, LOG_FATAL,
			"profile init:> open file failed, %s", strerror(errno));
		exit(0);
	}

	//配置文件变量初始化
	char* test = new char[401];
	while (file.getline(test, 400))
	{
		std::vector<std::string> ansed = GetConfig_Name_Num(test);

		if (!ansed.empty())
		{
			size_t n;
			while ((n = ansed[1].find('&')) != std::string::npos)
			{
				ansed[1][n] = ' ';
			}
			m_userDefine[ansed[0]] = ansed[1];
		}
	}

	file.close();
}


void User::TextPrint()
{
	if (m_userDefine.empty())
	{
		return;
	}

	for (auto tmp : m_userDefine)
	{
		cout << tmp.first << "  " << tmp.second << endl;
	}

	return;
}


std::string User::GetPath(std::string command)
{
	return m_userDefine[command];
}


User::~User()
{

}


