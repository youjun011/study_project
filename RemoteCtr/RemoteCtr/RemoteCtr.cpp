// RemoteCtr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtr.h"
#include "ServerSocket.h"
#include "MyTool.h"
#include "Command.h"
#include <conio.h>
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

void threadQueueEntry(HANDLE hIOCP) {
    DWORD dwTransferred = 0;
    ULONG_PTR CompletionKet = 0;
    OVERLAPPED* pOverlapped = NULL;
    std::list<std::string>lstString;
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
            std::string* pStr = NULL;
            if (lstString.size() > 0) {
                pStr = new std::string(lstString.front());
                lstString.pop_front();
            }
            if (pParam->cbFunc) {
                pParam->cbFunc(pStr);
            }
        }
        else if (pParam->nOperator == IocpListEmpty) {
            lstString.clear();
        }
        delete pParam;
    }
    _endthread();
}

void func(void* arg)
{
    std::string* pstr = (std::string*)arg;
    if (pstr != NULL) {
        printf("pop from list:%s\r\n", pstr->c_str());
        delete pstr;
    }
    else {
        printf("list is empty, no data");
    }
}


int main()  //extern声明的全局变量，在main函数之前实现；
{
    if (!CMyTool::Init())return 1;

    printf("press any ket to exit ...\r\n");
    HANDLE hIOCP = INVALID_HANDLE_VALUE;

    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
    HANDLE hThread = (HANDLE)_beginthread(threadQueueEntry, 0, hIOCP);
    
    ULONGLONG tick = GetTickCount64();
    while (_kbhit() != 0) {
        if (GetTickCount64() - tick > 1300) {
            PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM),
                (ULONG_PTR)new IOCP_PARAM(IocpListPop, "hello!"),
                NULL);
        }
        if (GetTickCount64() - tick > 2000) {
            PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM),
                (ULONG_PTR)new IOCP_PARAM(IocpListPush, "hello!"),
                NULL);
            tick = GetTickCount64();
        }
        Sleep(1);
    }

    if (hIOCP != NULL) {
        PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
        WaitForSingleObject(hThread, INFINITE);
    }
    CloseHandle(hIOCP);
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
