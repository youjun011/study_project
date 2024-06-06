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
            // TODO: ���Ĵ�������Է�����Ҫ
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }
    static bool RunAsAdmin() {
        //TODO:��ȡ����ԱȨ�ޡ�ʹ�ø�Ȩ�޴�������
        //���ز����� ����Administrator�˻�  ��ֹ������ֻ�ܵ�¼���ؿ���̨(�޸ĵĵط�)
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL,
            LOGON_WITH_PROFILE, NULL, sPath,
            CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {
            ShowError();
            MessageBox(NULL, sPath, _T("���̴���ʧ��"), 0);
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        DWORD dw = GetLastError(); // ��ȡ���Ĵ�����
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL)) {
            OutputDebugString(lpMessageBuf); // ���������Ϣ��������
            LocalFree(lpMessageBuf); // �ͷŷ���Ļ�����
        }
        else {
            OutputDebugString(L"Failed to format error message."); // FormatMessage ʧ��ʱ�Ĵ���
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
        //ͨ���޸�ע�����ʵ�ֿ�������
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CopyFile(sPath, strPath, FALSE);
        if (ret == FALSE) {
            MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲��㣿"),
                _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0,
            KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿"),
                _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        //CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0,
            REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿"),
                _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }
};

