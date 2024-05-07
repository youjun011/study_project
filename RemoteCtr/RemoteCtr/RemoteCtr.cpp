// RemoteCtr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtr.h"
#include "ServerSocket.h"
#include<direct.h>
#include<atlimage.h>
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
    //Dump((BYTE*)pack.Data(), pack.Size());
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

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
        finfo.hasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有访问权限！！"));
        return -2;
    }
    _finddata_t fdata;
    long long hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件！！"));
        FILEINFO finfo;
        finfo.hasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        return -3;
    }
    int count = 0;
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        TRACE(("finfo.szFileName:%s\r\n"), finfo.szFileName);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        count++;
        //listFileInfos.push_back(finfo);
    } while (!_findnext(hfind, &fdata));
    TRACE(_T("send count:%d\r\n"), count);
    FILEINFO finfo;
    finfo.hasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);

    return 0;
}

int Runfile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

#pragma warning(disable:4996)   //fopen sprintf strstr strcpy
int DownloadFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    FILE* file = NULL;
    errno_t err = fopen_s(&file, strPath.c_str(), "rb");
    long long data = 0;
    if (err !=0) {
        CPacket pack(4, (BYTE*) & data, 8);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        data = _ftelli64(file);
        CPacket head(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(head);
        fseek(file, 0, SEEK_SET);
        char buff[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buff, 1, 1024, file);
            CPacket pack(4, (BYTE*)buff, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(file);
    }
    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int MouseEvent() {
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
        DWORD nFlags = 0;
        switch (mouse.nButton)
        {
        case 0://左键；
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键；
            nFlags = 4;
            break;
        case 4://没有按键，鼠标移动
            nFlags = 8;
            break;
        }
        if (nFlags != 8) {
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        }
        switch (mouse.nAction)
        {
        case 0://单击；1,2,4,8表示4个比特位；
            nFlags |= 0x10;
            break;
        case 1://双击；
            nFlags |= 0x20;
            break;
        case 2://按下；
            nFlags |= 0x40;
            break;
        case 3://放开；
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        switch (nFlags)
        {
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x22:
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12:
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42:
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82:
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x24:
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14:
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44:
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84:
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08:  //移动鼠标；
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }
        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
    }
    else {
        OutputDebugString(_T("获取鼠标操作参数失败！！"));
        return -1;
    }
    return 0;
}

int SendScreen() {
    CImage screen;//GDI
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1060, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL)return -1;
    IStream* pStream = NULL;
    HRESULT ret =  CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (ret == S_OK) {
        //screen.Save(_T("test2024.jpg"), Gdiplus::ImageFormatJPEG);
        screen.Save(pStream, Gdiplus::ImageFormatJPEG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        PBYTE pData = (PBYTE)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, pData, nSize);
        CServerSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);
    }
    //screen.Save(_T("test2020.webp"), Gdiplus::ImageFormatTIFF);
    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}

#include"LockDialog.h"
CLockDialog dlg;
unsigned threadid;
unsigned _stdcall threadLockDlg(void* arg) {
    TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) - 30;
    dlg.MoveWindow(rect);
    //窗口置顶；
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //限制鼠标活动范围
    ShowCursor(false);

    dlg.GetWindowRect(rect);
    ClipCursor(rect);   //限制鼠标活动范围；
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_KEYDOWN) {
            TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
            if (msg.wParam == 0x1B) {   //est才有反应；
                break;
            }

        }
    }
    ShowCursor(true);
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

int LockMachine() {
    if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
       // _beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
        TRACE("threadid=%d\r\n", threadid);
    }
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int UnloakMachine() {
    PostThreadMessage(threadid, WM_KEYDOWN, 0x1b, 0x10001);//根据线程来的；
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect() {
    CPacket pack(1981, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int DeleteLocalFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    std::wstring wPath(strPath.begin(), strPath.end());
    if (DeleteFile(wPath.c_str()) == 0) {
        AfxMessageBox(_T("删除文件失败！！"));
    }
    CPacket pack(9, NULL, 0);
    bool ret = CServerSocket::getInstance()->Send(pack);
    return 0;
}

int ExcuteCommand(int nCmd) {
    //int nCmd = 7;
    int ret = 0;
    switch (nCmd)
    {
    case 1:  //  查看磁盘分区；
        ret = MakeDriverInfo();
        break;
    case 2:  //查看指定目录下的文件；
        ret = MakeDirectoryInfo();
        break;
    case 3: //打开文件；
        ret = Runfile();
        break;
    case 4: //下载文件；
        ret = DownloadFile();
        break;
    case 5:
        ret = MouseEvent();
        break;
    case 6:
        ret = SendScreen();//发送屏幕截图信息；
        break;
    case 7: //锁机；
        ret = LockMachine();
        /* Sleep(50);
         LockMachine();*/
        break;
    case 8: //解锁
        ret = UnloakMachine();
    case 9://删除文件；
        ret = DeleteLocalFile();
        break;
    case 1981:
        ret = TestConnect();
    }
    return ret;
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
            CServerSocket* pserver = CServerSocket::getInstance();
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
                int ret = pserver->DealCommand();
                if (ret > 0) {
                    ret = ExcuteCommand(pserver->GetPacket().sCmd);
                    if (ret != 0) {
                        TRACE(_T("执行命令失败：%d ret=%d\r\n", pserver->GetPacket().sCmd, ret));
                    }
                    pserver->CloseClient();
                }
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
