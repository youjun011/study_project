#pragma once
#include "ClientSocket.h"
#include "WathchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include"resource.h"
#include "ClientSocket.h"
#include <map>
#include "MyTool.h"
//#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
//#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)
#define WM_SHOW_WATCH (WM_USER+4)
#define WM_SEND_MESSAGE (WM_USER+5)
class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();
	//��ʼ������
	int ImitController();
	//����
	int Invoke(CWnd* pMainWnd);
	//������Ϣ
	//��������������ĵ�ַ
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}

	//1 �鿴���̷�����
	//2 �鿴ָ��Ŀ¼���ļ�
	//3 ���ļ�
	//4 �����ļ�
	//9 ɾ���ļ�
	//5 ������
	// 6 ������Ļ����
	// 7 ����
	// 8 ����
	// 1981 ��������
	//����ֵ�������
	bool SendCommandPacket(
		HWND hWnd,//���ݰ��յ�����ҪӦ��Ĵ���
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
				AfxMessageBox("����û��Ȩ�ޱ�����ļ��������ļ��޷���������");
				return -1;
			}
			int ret = SendCommandPacket(m_remoteDlg,
				4, false, (BYTE*)(LPCSTR)m_strRemote,
				m_strRemote.GetLength(), (WPARAM)pfile);
			TRACE("%s\r\n", LPCSTR(m_strRemote));
			m_remoteDlg.BeginWaitCursor();
			m_statusDlg.m_info.SetWindowText(_T("��������ִ���У���"));
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
	bool m_isClosed;//����Ƿ�رգ�

	CString m_strRemote;//�����ļ���Զ��·��
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

