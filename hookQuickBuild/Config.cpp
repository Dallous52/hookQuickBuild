#include "Config.h"
#include "fileCtl.h"

Config::Config()
{

}


void Config::Initialize()
{
	std::ifstream file;

	file.open(File_Profile_Ctl, std::ios::in);

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
			m_parameter[std::stoul(ansed[1])] = ansed[0];
			m_reverse[ansed[0]] = std::stoul(ansed[1]);
		}
	}

	for (uint32_t i = 65; i <= 90; ++i)
	{
		string tmp;
		tmp.push_back((char)i);
		
		m_parameter[i] = tmp;
	
		m_reverse[tmp] = i;
	}

	file.close();
}


string Config::GetKeyName(uint32_t code)
{
	return m_parameter[code];
}


void Config::TextPrint()
{
	if (m_parameter.empty())
	{
		return;
	}

	for (auto tmp : m_parameter)
	{
		cout << tmp.second << "  " << tmp.first << endl;
	}

	return;
}



void Config::HotReset()
{
	m_parameter.clear();
	m_reverse.clear();
	Initialize();
}




Config::~Config()
{

}