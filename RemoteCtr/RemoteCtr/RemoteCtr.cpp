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
#include <MSWSock.h>
#include "EdoyunServer.h"
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

class COverlapped {
public:
    OVERLAPPED  m_overlapped;
    DWORD m_operator;
    char m_buff[4096];
    COverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        memset(m_buff, 0, sizeof(m_buff));
    }
};

void iocp();

void udp_server();
void udp_client(bool ishost=true);

void initsock() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

void clearsock() {
    WSACleanup();
}

int main(int argc,char *argv[])  //extern声明的全局变量，在main函数之前实现；
{

    if (!CMyTool::Init())return 1;
    initsock();
    if (argc == 1) {//
        char wstrDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, wstrDir);
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        string strCmd = argv[0];
        strCmd += " 1";
        BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE,
           0 , NULL, wstrDir, &si, &pi);
        if (bRet) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TRACE("进程ID:%d\r\n", pi.dwProcessId);
            TRACE("线程ID:%d\r\n", pi.dwThreadId);
            strCmd += " 2";
            bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE,
                0, NULL, wstrDir, &si, &pi);
            if (bRet) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                TRACE("进程ID:%d\r\n", pi.dwProcessId);
                TRACE("线程ID:%d\r\n", pi.dwThreadId);
                udp_server();//服务器代码
            }
        }
    }
    else if (argc == 2) {//主客户端
        udp_client();
    }
    else {
        udp_client(false);
    }
    clearsock();
    //iocp();
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
    return 0;
}

void iocp() {
    EdoyunServer server;
    server.StartService();
    getchar();
}

#include "ESocket.h"
#include "ENetwork.h"

int RecvFromCB(void* arg, const EBuffer& buffer, ESockaddrIn& addr) {
    EServer* server = (EServer*)arg;
    return server->Sendto(addr, buffer);
}

int SendToCB(void* arg, const ESockaddrIn& addr, int ret) {
    EServer* server = (EServer*)arg;
    printf("sendto done!\r\n");
    return 0;
}

void udp_server() {
    std::list<ESockaddrIn>lstclients;
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    EServerParameter param(
        "127.0.0.1", 20000, ETypeUDP, NULL, NULL, NULL, RecvFromCB, SendToCB
    );
    EServer server(param);
    server.Invoke(&server);
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    getchar();
}
void udp_client(bool ishost) {
    
    Sleep(2000);
    sockaddr_in server, client;
    int len = sizeof(client);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(20000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    if (ishost) {
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        EBuffer msg = "hello!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%s  ERROR(%d)!!! ret =%d\r\n", __FILE__,
                __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0) {
                printf("%s(%d):%s ip = %08X port = %d\r\n",__FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__,msg.size());
            }
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%s  ERROR(%d)!!! ret =%d\r\n", __FILE__,
                __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0) {
                printf("%s(%d):%s ip = %08X port = %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg = %s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
            }
        }
    }
    else {
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        std::string msg = "hello world!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("client %s(%d):%s  ERROR(%d)!!! ret =%d\r\n", __FILE__,
                __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0) {
                sockaddr_in addr;
                memcpy(&addr, msg.c_str(), sizeof(addr));
                sockaddr_in* paddr = &addr;
                printf("%s(%d):%s ip = %08X port = %d\r\n",__FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
                printf("%s(%d):%s ip = %08X port = %d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
                msg = "hello (from client)";
                ret = sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)paddr, sizeof(sockaddr_in));
                printf("client %s(%d):%s  ERROR(%d)!!! ret =%d\r\n", __FILE__,__LINE__, __FUNCTION__, WSAGetLastError(), ret);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            }
        }
    }
    closesocket(sock);
}
