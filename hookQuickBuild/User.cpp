#pragma once
#include "User.h"
#include "fileCtl.h"
#include <algorithm>

User::User()
{
	;
}


void User::Initialize(std::unordered_map<std::string, uint32_t>& dic)
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
			string temp;

			while ((n = ansed[1].find('&')) != std::string::npos)
			{
				ansed[1][n] = ' ';
			}

			if ((n = ansed[0].find('+')) != std::string::npos)
			{
				
				temp = TextCommandToNumber(ansed[0], dic);

				m_userDefine[temp] = ansed[1];
			}
			else
			{
				m_userDefine[ansed[0]] = ansed[1];
			}
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


void User::HotReset(std::unordered_map<std::string, uint32_t>& dic)
{
	m_userDefine.clear();

	Initialize(dic);
}


std::string User::TextCommandToNumber(const std::string& command,
	std::unordered_map<std::string, uint32_t>& dic)
{
	std::vector<uint32_t> comm;
	std::string ret;

	for (int i = 0; i < command.size(); ++i)
	{
		if (command[i] == '+')
		{
			comm.emplace_back(dic[ret]);
			ret.clear();
		}
		else
		{
			ret.push_back(command[i]);
		}
	}

	comm.emplace_back(dic[ret]);
	ret.clear();

	return CommandSplicing(comm);
}


std::string User::CommandSplicing(std::vector<uint32_t> command)
{
	std::string ret = "";

	std::sort(command.begin(), command.end());

	for (int i = 0; i < command.size(); ++i)
	{
		ret += std::to_string(command[i]);
	}

	return ret;
}


User::~User()
{

}


