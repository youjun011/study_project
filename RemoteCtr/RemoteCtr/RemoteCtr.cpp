﻿// RemoteCtr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtr.h"
#include "ServerSocket.h"
#include "MyTool.h"
#include "Command.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//CServerSocket::CHelper CServerSocket::m_helper;

// 唯一的应用程序对象

CWinApp theApp;
using namespace std;

void WriteRegisterTable() {
    CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    char sPath[MAX_PATH] = "";
    char sSys[MAX_PATH] = "";
    std::string strExe = "\\RemoteCtrl.exe ";
    GetCurrentDirectoryA(MAX_PATH, sPath);
    GetSystemDirectoryA(sSys, sizeof(sSys));
    std::string strCmd = "mklink " + std::string(sSys) +
        strExe + std::string(sPath) + strExe;
    system(strCmd.c_str());
    HKEY hKey = NULL;
    int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
    if (ret != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"),
            _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
    CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0,
        REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
    if (ret != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"),
            _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
    RegCloseKey(hKey);
}

void WriteStartupDir() {
    CString strPath = _T("C:\\Users\\19113\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
    CString strCmd = GetCommandLine();
    strCmd.Replace(_T("\""), _T(""));
    BOOL ret = CopyFile(strCmd,strPath , FALSE);
    if (ret == FALSE) {
        MessageBox(NULL, _T("复制文件失败！是否权限不足？"),
            _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
}

void ChooseAutoInvoke() {
    CString strPath =  CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    if (PathFileExists(strPath)) {
        return;
    }
    CString strInfo = _T("该程序只允许用于合法的用途!\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮，退出程序。\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行！\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西!\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        //WriteRegisterTable();
        WriteStartupDir();
    }
    else if (ret == IDCANCEL) {
        exit(0);
    }
    return;
}

void ShowError() {
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

bool IsAdmin() {
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

void RunAsAdmin() {
    //TODO:获取管理员权限、使用该权限创建进程
    HANDLE hToken = NULL;
    BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_BATCH,
        LOGON32_PROVIDER_DEFAULT, &hToken);
    if (!ret) {
        ShowError();
        MessageBox(NULL, _T("登录错误！"), _T("程序错误"), 0);
        exit(0);
    }
    OutputDebugString(L"Logon administrator sucess!\r\n");
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    TCHAR sPath[MAX_PATH] = _T("");
    GetCurrentDirectory(MAX_PATH, sPath);
    CString strCmd = sPath;
    strCmd += _T("\\RemoteCtrl.exe");
   /* ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, (LPWSTR)strCmd.GetBuffer(),
        CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);*/
    ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL,
        LOGON_WITH_PROFILE, NULL, (LPWSTR)strCmd.GetBuffer(),
        CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
    CloseHandle(hToken);
    if (!ret) {
        ShowError();
        MessageBox(NULL, _T("进程创建失败！"), _T("程序错误"), 0);
        exit(0);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main()  //extern声明的全局变量，在main函数之前实现；
{
    int nRetCode = 0;
    if (IsAdmin()) {
        OutputDebugString(L"current is run as administrator!\r\n");
        //MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
    }
    else {
        OutputDebugString(L"current is run as normal user!\r\n");
        RunAsAdmin();
        //MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
        return nRetCode;
    }
    
    //test1
    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            CCommand cmd;
            //ChooseAutoInvoke();
            CServerSocket* pserver = CServerSocket::getInstance();
            int ret = pserver->Run(&CCommand::RunCommand, &cmd);
            switch (ret)
            {
            case -1:
                MessageBox(NULL, _T("网络初始化异常"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            case -2:
                MessageBox(NULL, _T("多次无法正常接入用户！！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            }

        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
