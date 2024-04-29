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
}


BEGIN_MESSAGE_MAP(CWathchDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWathchDialog 消息处理程序


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

		}
	}
	

	CDialog::OnTimer(nIDEvent);
}
