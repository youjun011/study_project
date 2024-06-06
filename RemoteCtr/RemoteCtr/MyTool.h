#pragma once
class CMyTool
{
public:
	static void Dump(BYTE* pData, size_t nSize) {
        std::string strout;
        for (size_t i = 0; i < nSize; i++) {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strout += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strout += buf;
        }
        strout += "\n";
        OutputDebugStringA(strout.c_str());
    }
    static bool Init()
    {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr) {
            // TODO: 更改错误代码以符合需要
            wprintf(L"错误: GetModuleHandle 失败\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            return false;
        }
        return true;
    }
    static bool RunAsAdmin() {
        //TODO:获取管理员权限、使用该权限创建进程
        //本地策略组 开启Administrator账户  禁止空密码只能登录本地控制台(修改的地方)
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL,
            LOGON_WITH_PROFILE, NULL, sPath,
            CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {
            ShowError();
            MessageBox(NULL, sPath, _T("进程创建失败"), 0);
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        DWORD dw = GetLastError(); // 获取最后的错误码
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL)) {
            OutputDebugString(lpMessageBuf); // 输出错误消息到调试器
            LocalFree(lpMessageBuf); // 释放分配的缓冲区
        }
        else {
            OutputDebugString(L"Failed to format error message."); // FormatMessage 失败时的处理
        }
    }
    static bool IsAdmin() {
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {
            return eve.TokenIsElevated;
        }
        OutputDebugString(L"length of tokeninformation is\r\n");
        return false;
    }

    static BOOL WriteStartupDir(const CString& strPath) {
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return CopyFile(sPath, strPath, FALSE);
    }

    static bool WriteRegisterTable(const CString& strPath) {
        //通过修改注册表来实现开机启动
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CopyFile(sPath, strPath, FALSE);
        if (ret == FALSE) {
            MessageBox(NULL, _T("复制文件失败！是否权限不足？"),
                _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0,
            KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"),
                _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        //CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0,
            REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"),
                _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }
};

