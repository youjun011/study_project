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
	LRESULT SendMessage(MSG msg);
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
	int SendCommandPacket(
		int nCmd,
		bool bAutoClose=true, 
		BYTE* pData=NULL, 
		size_t nLength=0,
		std::list<CPacket>*plstPacks=NULL)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//TODO:不应该直接发送，而是投入队列
		CPacket pack(nCmd, pData, nLength, hEvent);
		//应答结果包
		std::list<CPacket> lstPacks;
		if (plstPacks == NULL) {
			plstPacks = &lstPacks;
		}
		pClient->SendPacket(pack, *plstPacks);
		CloseHandle(hEvent);
		if (plstPacks->size() > 0) {
			TRACE(_T("SendCommand 成功！！,cmd =%d\r\n"), plstPacks->front().sCmd);
			return plstPacks->front().sCmd;
		}
		
		return -1;
	}

	int DownFile(CString strPath) {
		CFileDialog dlg(FALSE, NULL, strPath,
			OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, 
			NULL, &m_remoteDlg);
		if (dlg.DoModal() == IDOK) {
			m_strRemote = strPath;
			m_strLocal = dlg.GetPathName();
			m_nThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
			if (WaitForSingleObject(m_nThreadDownload, 0) != WAIT_TIMEOUT) {
				return -1;
			}
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
	void threadDownloadFile();
	static void __cdecl threadDownloadEntry(void* arg);

	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadDownload = INVALID_HANDLE_VALUE;
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
	HANDLE m_nThreadDownload;
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

