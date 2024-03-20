// RemoteCtr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtr.h"
#include "ServerSocket.h"
#include<direct.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//CServerSocket::CHelper CServerSocket::m_helper;

// 唯一的应用程序对象

CWinApp theApp;
  
using namespace std;
void Dump(BYTE* pData, size_t nSize) {
    std::string strout;
    for (size_t i = 0;i < nSize;i++) {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))strout += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strout += buf;
    }
    strout += "\n";
    OutputDebugStringA(strout.c_str());
}


int MakeDriverInfo() {
    std::string result;
    for (int i = 1;i <= 26;i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)
                result += ',';
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//打包用的；
    Dump((BYTE*)pack.Data(), pack.Size());
    //CServerSocket::getInstance()->Send(pack);
    return 0;
}
typedef struct file_info {
    file_info() {
        IsInvalid = FALSE;  //是否无效的文件
        IsDirectory = -1;
        hasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid;
    BOOL IsDirectory;
    BOOL hasNext;
    char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;
#include<io.h>
#include<stdio.h>
#include<list>
int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO>listFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前命令不是获取文件列表信息，解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) == -1) {
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.hasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //listFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有访问权限！！"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件！！"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //listFileInfos.push_back(finfo);
    } while (!_findnext(hfind, &fdata));
    FILEINFO finfo;
    finfo.hasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);

    return 0;
}

int main()  //extern声明的全局变量，在main函数之前实现；
{
    int nRetCode = 0;
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
            /*CServerSocket* pserver = CServerSocket::getInstance();
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T("网络初始化异常"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            int count = 0;
            while (CServerSocket::getInstance()!=NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("多次无法正常接入用户！！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }
                pserver->DealCommand();*/
            int nCmd = 1;
            switch (nCmd)
            {
            case1:  //  查看磁盘分区；
                MakeDriverInfo();
                break;
            case2:  //查看指定目录下的文件；
                MakeDirectoryInfo();
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
