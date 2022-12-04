#include "programSet.h"
#include "fileCtl.h"
#include <atlstr.h>

std::string LPCWSTRTostring(LPCWSTR pWCStrKey)
{
    //第一次调用确认转换后单字节字符串的长度，用于开辟空间
    int pSize = WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey),
        NULL, 0, NULL, NULL);
    char* pCStrKey = new char[pSize + 1];
    //第二次调用将双字节字符串转换成单字节字符串
    WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey),
        pCStrKey, pSize, NULL, NULL);
    
    pCStrKey[pSize] = '\0';
    std::string pKey = pCStrKey;
    
    delete[]pCStrKey;

    return pKey;
}


LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}


//设置当前程序开机启动
static void set_auto_strat()
{
    HKEY hKey;
    //std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    // 
    //1、找到系统的启动项  打开启动项   
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        //2、得到本程序自身的全路径
        TCHAR strExeFullDir[MAX_PATH];
        GetModuleFileName(NULL, strExeFullDir, MAX_PATH);


        //3、判断注册表项是否已经存在
        TCHAR strDir[MAX_PATH] = {};
        DWORD nLength = MAX_PATH;
        long result = RegGetValue(hKey, nullptr, _T("hookQuickBuild"), RRF_RT_REG_SZ, 0, strDir, &nLength);


        //4、已经存在
        if (result != ERROR_SUCCESS || _tcscmp(strExeFullDir, strDir) != 0)
        {
            //5、添加一个子Key,并设置值，"hookQuickBuild"是应用程序名字（不加后缀.exe） 
            LONG judge = RegSetValueEx(hKey, _T("hookQuickBuild"), 0, REG_SZ,
                (LPBYTE)strExeFullDir, (lstrlen(strExeFullDir) + 1) * sizeof(TCHAR));

            if (judge == ERROR_SUCCESS)
            {
                Error_insert_File(F_LOG, LOG_NOTICE,
                    "Setting the boot auto start successed.");
            }
            else
            {
                Error_insert_File(F_LOG, LOG_CRIT,
                    "Failed to set the boot auto start :> add new key failed.");
            }

        }
        //6、关闭注册表
        RegCloseKey(hKey);

    }//end if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    else
    {
        Error_insert_File(F_LOG, LOG_CRIT,
            "Failed to set the boot auto start :> registry open failed.");
    }
}


//取消当前程序开机启动
static void cancel_auto_start()
{
    HKEY hKey;
    //std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    //1、找到系统的启动项  
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        //2、删除值
        LONG judge = RegDeleteValue(hKey, _T("hookQuickBuild"));

        if (judge == ERROR_SUCCESS)
        {
            Error_insert_File(F_LOG, LOG_NOTICE,
                "Canceling the boot auto start successed.");
        }
        else
        {
            Error_insert_File(F_LOG, LOG_CRIT,
                "Failed to cancel the boot auto start :> add new key failed.");
        }

        //3、关闭注册表
        RegCloseKey(hKey);
    }
    else
    {
        Error_insert_File(F_LOG, LOG_CRIT,
            "Failed to cancel the boot auto start :> registry open failed.");
    }
}


void SetStartRunning()
{
	if (stoi(user.GetPath("is_auto_start")))
	{
        set_auto_strat();
	}
    else
    {
        cancel_auto_start();
    }
}


void HotReset()
{

    PROCESS_INFORMATION pi; //启动窗口的信息

    STARTUPINFO si; //进程的信息

    memset(&si, 0, sizeof(si));

    si.cb = sizeof(si);

    si.wShowWindow = SW_SHOW;

    si.dwFlags = STARTF_USESHOWWINDOW;

    //2、得到本程序自身的全路径
    TCHAR strExeFullDir[MAX_PATH];
    GetModuleFileName(NULL, strExeFullDir, MAX_PATH);
    LPCWSTR pa = strExeFullDir;
    bool fRet = CreateProcess(pa, NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &si, &pi);

    if (fRet)
    {
        exit(0);
        Error_insert_File(F_LOG, LOG_INFO, "Hot reset success");
    }
    else
    {
        Error_insert_File(F_LOG, LOG_INFO, "Hot reset failed");
    }

    //不使用的句柄最好关掉
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}


//判断是否有管理员权限
static BOOL IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    
    //判断自己是否有管理员权限和自己是否为服务
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup))
    {
        Error_insert_File(F_LOG, LOG_CRIT, 
            "AllocateAndInitializeSid Get administrator judged failed.");
    }

    //检查原始Token中，管理员pAdministratorsGroup是否被激活，如果被激活，那么说明启动这个
    //程序的账号是管理员账号，否则不是
    //这个CheckTokenMembership函数，结果被保存在fIsRunAsAdmin参数中，
    //而这个函数的返回值只是表示，这个函数是否成功
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        Error_insert_File(F_LOG, LOG_CRIT, "CheckTokenMembership function failed.");
    }


    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    return fIsRunAsAdmin;
}


void GainAdminPrivileges()
{
    if (!IsRunAsAdministrator())
    {
        TCHAR strExeFullDir[MAX_PATH];
        GetModuleFileName(NULL, strExeFullDir, MAX_PATH);

        SHELLEXECUTEINFO execinfo;
        memset(&execinfo, 0, sizeof(execinfo));
        execinfo.lpFile = strExeFullDir;
        execinfo.cbSize = sizeof(execinfo);
        execinfo.lpVerb = _T("runas");
        execinfo.fMask = SEE_MASK_NO_CONSOLE;
        execinfo.nShow = SW_SHOWDEFAULT;

        ShellExecuteEx(&execinfo);
    }
}