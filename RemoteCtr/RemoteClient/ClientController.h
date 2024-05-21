#pragma once
#include "ClientSocket.h"
#include "WathchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include"resource.h"
#include "ClientSocket.h"
#include <map>
#include "MyTool.h"
//#define WM_SEND_PACK (WM_USER+1)//发送包数据
//#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)
#define WM_SHOW_WATCH (WM_USER+4)
#define WM_SEND_MESSAGE (WM_USER+5)
class CClientController
{
public:
	//获取全局唯一对象
	static CClientController* getInstance();
	//初始化操作
	int ImitController();
	//启动
	int Invoke(CWnd* pMainWnd);
	//发送消息
	//更新网络服务器的地址
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}

	//1 查看磁盘分区；
	//2 查看指定目录下文件
	//3 打开文件
	//4 下载文件
	//9 删除文件
	//5 鼠标操作
	// 6 发送屏幕内容
	// 7 锁机
	// 8 解锁
	// 1981 测试连接
	//返回值是命令号
	bool SendCommandPacket(
		HWND hWnd,//数据包收到后需要应答的窗口
		int nCmd,
		bool bAutoClose=true, 
		BYTE* pData=NULL, 
		size_t nLength=0,
		WPARAM wParam=0)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		CPacket pack(nCmd, pData, nLength);
		bool ret = pClient->SendPacket(hWnd, pack, bAutoClose, wParam);
		return ret;
	}
	void DownloadEnd();
	int DownFile(CString strPath) {
		CFileDialog dlg(FALSE, NULL, strPath,
			OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, 
			NULL, &m_remoteDlg);
		if (dlg.DoModal() == IDOK) {
			m_strRemote = strPath;
			m_strLocal = dlg.GetPathName();
			FILE* pfile = fopen(m_strLocal, "wb+");
			if (pfile == NULL) {
				AfxMessageBox("本地没有权限保存该文件，或者文件无法创建！！");
				return -1;
			}
			int ret = SendCommandPacket(m_remoteDlg,
				4, false, (BYTE*)(LPCSTR)m_strRemote,
				m_strRemote.GetLength(), (WPARAM)pfile);
			TRACE("%s\r\n", LPCSTR(m_strRemote));
			m_remoteDlg.BeginWaitCursor();
			m_statusDlg.m_info.SetWindowText(_T("命令正在执行中！！"));
			m_statusDlg.ShowWindow(SW_SHOW);
			m_statusDlg.CenterWindow(&m_remoteDlg);
			m_statusDlg.SetActiveWindow();
		}
		return 0;
	}

	void StartWatchScreen();

protected:
	void threadWatchScreen();
	static void threadWatchScreenEntry(void* arg);

	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
		m_isClosed = true;
	}

	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != nullptr) {
			delete m_instance;
			m_instance = NULL;
		}
	}
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam,
		LPARAM lParam);
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam,
		LPARAM lParam);

private: 
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo() {
			result = 0;
			memset(&msg, 0, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const  MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;

	typedef LRESULT(CClientController::* MSGFUNC)(UINT
		nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>m_mapFunc;

	CRemoteClientDlg m_remoteDlg;
	CWathchDialog m_watchDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//监控是否关闭；

	CString m_strRemote;//下载文件的远程路径
	CString m_strLocal;
	unsigned int m_nThreadID;
	static CClientController* m_instance;
	class CHelper {
	public:
		CHelper() {
			//getInstance();
		}
		~CHelper() {
			releaseInstance();
		}
	};
	static CHelper m_helper;
};

