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


CPoint CWathchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{
	CRect clientRect;
	ScreenToClient(&point);	//全局坐标到客户区坐标；
	//本地到远程坐标
	m_picture.GetWindowRect(clientRect);
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = 1920, height = 1080;
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
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;
	event.nAction = 2;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWathchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
}



void CWathchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	////坐标转换
	//CPoint remote = UserPoint2RemoteScreenPoint(point);
	////封装
	//MOUSEEV event;
	//event.ptXY = remote;
	//event.nButton = 0;
	//event.nAction = 2;
	//CClientSocket* pClient = CClientSocket::getInstance();
	//CPacket pack(5, (BYTE*)&event, sizeof(event));
	//pClient->Send(pack);
	CDialog::OnLButtonUp(nFlags, point);
}

void CWathchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;
	event.nAction = 2;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWathchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;
	event.nAction = 4;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWathchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;
	event.nAction = 3;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWathchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;
	event.nAction = 1;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnMouseMove(nFlags, point);
}


void CWathchDialog::OnStnClickedWatch()
{
	CPoint point;
	GetCursorPos(&point);
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;
	event.nAction = 2;
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
}
