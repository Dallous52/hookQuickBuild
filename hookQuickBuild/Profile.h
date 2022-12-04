#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <string>
#include <vector>


class ProFile
{
public:
	ProFile();
	~ProFile();

	//初始化
	virtual void Initialize();


protected:
	//处理配置文件信息
	std::vector<std::string> GetConfig_Name_Num(char* txt);

private:
	std::vector<char> m_note;					//用于注释的字符
};