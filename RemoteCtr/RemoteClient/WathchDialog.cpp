// WathchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WathchDialog.h"
#include "RemoteClientDlg.h"
#include "ClientController.h"
// CWathchDialog 对话框

IMPLEMENT_DYNAMIC(CWathchDialog, CDialog)

CWathchDialog::CWathchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
	m_isFull = false;
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
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWathchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWathchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK,&CWathchDialog::OnSendPackAck)
END_MESSAGE_MAP()


// CWathchDialog 消息处理程序


CPoint CWathchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	
	CRect clientRect;
	if (!isScreen)ClientToScreen(&point);	//转换为屏幕左上角的坐标(屏幕内的绝对坐标)
	m_picture.ScreenToClient(&point);	//全局坐标到客户区坐标；
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
	//SetTimer(0, 50, NULL);
	m_isFull = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWathchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		//CClientController* pParent = (CClientController*)GetParent();
		//if (m_isFull) {
		//	CRect rect;
		//	m_picture.GetWindowRect(rect);
		//	//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
		//		m_nObjWidth = m_image.GetWidth();
		//		m_nObjHeight = m_image.GetHeight();
		//	m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc()
		//		, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
		//	m_picture.InvalidateRect(NULL);
		//	m_image.Destroy();
		//	m_isFull = false;
		//	TRACE(_T("获取图片成功!!\r\n"));
		//}
	}
	CDialog::OnTimer(nIDEvent);
}

LRESULT CWathchDialog::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam ==-1||lParam==-2) {

	}
	else if (lParam == 1) {
		//对方关闭套接字
	}
	else {
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != nullptr) {
			switch (pPacket->sCmd)
			{
			case 6:
			{
				if (m_isFull) {
					CMyTool::Bytes2Image(m_image, pPacket->strData);
					CRect rect;
					m_picture.GetWindowRect(rect);
					//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
					m_nObjWidth = m_image.GetWidth();
					m_nObjHeight = m_image.GetHeight();
					m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc()
						, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
					m_picture.InvalidateRect(NULL);
					m_image.Destroy();
					m_isFull = false;
					TRACE(_T("获取图片成功!!\r\n"));
				}
				break;
			}
			case 5:
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	return 0;
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
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
		CClientController::getInstance()
			->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
}


void CWathchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWathchDialog::OnBnClickedBtnLock()
{
	CClientController::getInstance()
		->SendCommandPacket(GetSafeHwnd(),7);
}


void CWathchDialog::OnBnClickedBtnUnlock()
{
	CClientController::getInstance()
		->SendCommandPacket(GetSafeHwnd(),8);
}
