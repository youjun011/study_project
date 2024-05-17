#include "pch.h"
#include "ClientController.h"


//��̬�ģ�������ʵ��:(�����ʱ��������ʼ��)
std::map<UINT, CClientController::MSGFUNC>CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;
CClientController* CClientController::getInstance()
{
	if (m_instance == nullptr) {
		m_instance = new CClientController();
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] = {
			{WM_SHOW_STATUS,&OnShowStatus},
			{WM_SHOW_WATCH,&OnShowWatch},
			{(UINT) - 1,NULL}
		};
		for (int i = 0; MsgFuncs[i].nMsg != -1; i++) {
			m_mapFunc.insert({ MsgFuncs[i].nMsg,
				MsgFuncs[i].func });
		}
	}
	return m_instance;
}

int CClientController::ImitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,
		&CClientController::threadEntry,
		this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DIG_STATUS,&m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd* pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE,
		(WPARAM)&info
		, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
	return info.result;
}

void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("������ɣ���"), _T("��ɣ�"));
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(CClientController::threadWatchScreenEntry, 0, this);
	m_watchDlg.DoModal();	//������������
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed) {
		if (m_watchDlg.isFull() == false) {
			std::list<CPacket>lstPacks;
			int ret =SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			
			if (ret == 6) {
;				if (CMyTool::Bytes2Image(m_watchDlg.GetImage(), 
	lstPacks.front().strData) == 0) {
					m_watchDlg.SetImageStatus(true);
				}
			}
			else {
				TRACE("��ȡͼƬʧ�ܣ�\r\n");
			}
		}
		else {
			Sleep(1);
		}
	}
}

void CClientController::threadWatchScreenEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}

void CClientController::threadDownloadFile()
{
	FILE* pfile = fopen(m_strLocal, "wb+");
	if (pfile == NULL) {
		AfxMessageBox("����û��Ȩ�ޱ�����ļ��������ļ��޷���������");
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}

	TRACE("%s\r\n", LPCSTR(m_strRemote));
	CClientSocket* pClient = CClientSocket::getInstance();
	do {
		//int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);
		int ret = SendCommandPacket(m_remoteDlg,
			4, false, (BYTE*)(LPCSTR)m_strRemote,
			m_strRemote.GetLength(), (WPARAM)pfile);
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0) {
			AfxMessageBox("�ļ�����Ϊ������޷���ȡ�ļ�����");
			break;
		}
		long long nCount = 0;
		while (nCount < nLength) {
			ret = pClient->DealCommand();
			if (ret < 0) {
				AfxMessageBox("����ʧ�ܣ���");
				TRACE("����ʧ��:ret = %d\r\n", ret);
				break;
			}
			nCount += pClient->GetPacket().strData.size();
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pfile);
		}
	} while (false);
	fclose(pfile);
	pClient->CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("������ɣ���"), _T("��ɣ�"));
}

void __cdecl CClientController::threadDownloadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownloadFile();
	_endthread();
}

void CClientController::threadFunc()
{
	MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end()) {
				pmsg->result = (this->*it->second)(pmsg->msg.message, 
					pmsg->msg.wParam, pmsg->msg.lParam);

			}
			else {
				pmsg->result = - 1;
			}
			SetEvent(hEvent);
		}
		else {
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
    }
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}