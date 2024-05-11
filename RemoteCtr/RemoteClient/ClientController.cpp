#include "pch.h"
#include "ClientController.h"


//静态的，必须做实现:(构造的时候不能做初始化)
std::map<UINT, CClientController::MSGFUNC>CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;

CClientController* CClientController::getInstance()
{
	if (m_instance == nullptr) {
		m_instance = new CClientController();
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] = {
			{WM_SEND_PACK,&OnSendPack},
			{WM_SEND_DATA,&OnSendData},
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
	return info.result;
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
				pmsg->result - 1;
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

LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
