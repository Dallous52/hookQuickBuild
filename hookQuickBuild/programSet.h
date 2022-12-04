#pragma once
#include "Config.h"
#include "User.h"
#include <Windows.h>

extern Config config;
extern User user;

//string转换lpcwstr
LPCWSTR stringToLPCWSTR(std::string orig);

//是否设置开机自启动
void SetStartRunning();

//热重载函数
void HotReset();

//获取管理员权限
void GainAdminPrivileges();
