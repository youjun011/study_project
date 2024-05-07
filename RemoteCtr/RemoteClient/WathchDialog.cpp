// WathchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WathchDialog.h"
#include "RemoteClientDlg.h"

// CWathchDialog 对话框

IMPLEMENT_DYNAMIC(CWathchDialog, CDialog)

CWathchDialog::CWathchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWathchDialog::~CWathchDialog()
{
}

void CWathchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWathchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWathchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWathchDialog 消息处理程序


CPoint CWathchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	
	CRect clientRect;
	if (isScreen)ScreenToClient(&point);	//全局坐标到客户区坐标；
	//本地到远程坐标
	m_picture.GetWindowRect(clientRect);
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = m_nObjWidth, height = m_nObjHeight;
	int x = point.x * width / width0;
	int y = point.y * height / height0;
	return CPoint(x, y);
	
}

BOOL CWathchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWathchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			if (m_nObjWidth == -1) {
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc()
				, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	

	CDialog::OnTimer(nIDEvent);
}


void CWathchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 1;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWathchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		TRACE("x =%d, y=%d\r\n", point.x, point.y);
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x =%d, y=%d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 2;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonDown(nFlags, point);
}



void CWathchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CWathchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 1;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWathchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 3;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWathchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWathchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;
		event.nAction = 0;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	}
	//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialog::OnMouseMove(nFlags, point);
}


void CWathchDialog::OnStnClickedWatch()
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != 1)) {
		CPoint point;
		GetCursorPos(&point);
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 0;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
}


void CWathchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}
