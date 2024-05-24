﻿// RemoteCtr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtr.h"
#include "ServerSocket.h"
#include "MyTool.h"
#include "Command.h"
#include <conio.h>
#include "CEdoyunQueue.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//CServerSocket::CHelper CServerSocket::m_helper;

// 唯一的应用程序对象
//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtr.exe")
#define INVOKE_PATH _T("C:\\Users\\19113\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")

CWinApp theApp;
using namespace std;

bool ChooseAutoInvoke(const CString&strPath) {
    if (PathFileExists(strPath)) {
        return true;
    }
    CString strInfo = _T("该程序只允许用于合法的用途!\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮，退出程序。\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行！\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西!\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        //WriteRegisterTable();
        if (!CMyTool::WriteStartupDir(strPath)) {
            MessageBox(NULL, _T("复制文件失败！是否权限不足？"),
                _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    }
    else if (ret == IDCANCEL) {
        return false;
    }
    return true;
}

#define IOCP_LIST_PUSH 1
#define IOCP_LIST_POP 2

enum
{
    IocpListPush,
    IocpListPop,
    IocpListEmpty
};

typedef struct IocpParam {
    int nOperator;
    std::string strData;
    _beginthread_proc_type cbFunc;
    IocpParam(int op, const char* sData, _beginthread_proc_type cb=NULL) {
        nOperator = op;
        strData = sData;
        cbFunc = cb;
    }
    IocpParam() {
        nOperator = -1;
    }
}IOCP_PARAM;

void threadmain(HANDLE hIOCP) {
    DWORD dwTransferred = 0;
    ULONG_PTR CompletionKet = 0;
    OVERLAPPED* pOverlapped = NULL;
    std::list<std::string>lstString;    //会内存泄漏
    while (GetQueuedCompletionStatus(hIOCP,
        &dwTransferred, &CompletionKet, &pOverlapped, INFINITE)) {
        if ((dwTransferred == 0) || (CompletionKet == NULL)) {
            printf("thread is prepare to exit!\r\n");
            break;
        }
        IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKet;
        if (pParam->nOperator == IocpListPush) {
            lstString.push_back(pParam->strData);
        }
        else if (pParam->nOperator == IocpListPop) {
            std::string str;
            if (lstString.size() > 0) {
                str = lstString.front();
                lstString.pop_front();
            }
            if (pParam->cbFunc) {
                pParam->cbFunc(&str);
            }
        }
        else if (pParam->nOperator == IocpListEmpty) {
            lstString.clear();
        }
        delete pParam;
    }
}


void threadQueueEntry(HANDLE hIOCP) {
    threadmain(hIOCP);  //必须用一个函数，不然会导致局部变量无法释放
    //lstString.clear();
    _endthread(); //这里不会返回，直接结束  //代码到此为止，会导致本地对象无法调用析构，从而导致内存泄漏；
}

void func(void* arg)
{
    std::string* pstr = (std::string*)arg;
    if (pstr != NULL) {
        printf("pop from list:%s\r\n", pstr->c_str());
    }
    else {
        printf("list is empty, no data\r\n");
    }
}


int main()  //extern声明的全局变量，在main函数之前实现；
{

    if (!CMyTool::Init())return 1;

    printf("press any ket to exit ...\r\n");
    ULONGLONG tick = GetTickCount64();
    ULONGLONG tick0 = GetTickCount64();
    CEdoyunQueue<std::string>lstStrings;
    while (_kbhit() == 0) {
        if (GetTickCount64() - tick0 > 1300) {
            lstStrings.PushBack("hello");
            tick0 = GetTickCount64();
        }
        if (GetTickCount64() - tick > 2000) {
            std::string str;
            lstStrings.PopFront(str);
            tick = GetTickCount64();
            printf("pop from queue:%s\r\n", str.c_str());
        }
        Sleep(1);
    }
    printf("list size :%d\r\n", lstStrings.Size());
    lstStrings.Clear();
    printf("list size :%d\r\n", lstStrings.Size());
    printf("exit done!\r\n");
    exit(0);


    /*
    if (CMyTool::IsAdmin()) {
        OutputDebugString(L"current is run as administrator!\r\n");
        if (!CMyTool::Init())return 1;
        if (ChooseAutoInvoke(INVOKE_PATH)) {
            CCommand cmd;
            CServerSocket* pserver = CServerSocket::getInstance();
            int ret = pserver->Run(&CCommand::RunCommand, &cmd);
            switch (ret)
            {
            case -1:
                MessageBox(NULL, _T("网络初始化异常"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                break;
            case -2:
                MessageBox(NULL, _T("多次无法正常接入用户！！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                break;
            }
        }
    }
    else {
        OutputDebugString(L"current is run as normal user!\r\n");
        if (CMyTool::RunAsAdmin() == false) {
            CMyTool::ShowError();
            return 1;
        }
    }
    return 0;
    */
}
