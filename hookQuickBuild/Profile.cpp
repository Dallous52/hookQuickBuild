#include "ProFile.h"
#include "fileCtl.h"

ProFile::ProFile()
{
	m_note = { '#','[','\n' };
}


void ProFile::Initialize()
{
	//do nothing;
}


std::vector<std::string> ProFile::GetConfig_Name_Num(char* txt)
{
	std::vector<std::string> ans;

	if (txt == NULL || !strlen(txt))
	{
		return ans;
	}

	std::string txts = txt;

	//删除行间空格
	for (int i = 0; i < txts.size(); i++)
	{
		if (txts[i] == ' ')
		{
			txts.erase(i--, 1);
		}
	}

	//注释等无用行处理
	for (int i = 0; i < m_note.size(); i++)
	{
		if (txts[0] == m_note[i])
		{
			return ans;
		}
	}

	ans.resize(2);

	int mid = txts.find('=');

	ans[0].append(txts, 0, mid);
	ans[1].append(txts, mid + 1, txts.size() - 1);

	return ans;
}


ProFile::~ProFile()
{
	;
}