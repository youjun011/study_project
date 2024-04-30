﻿#pragma once
#include "afxdialogex.h"


// CWathchDialog 对话框

class CWathchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWathchDialog)

public:
	CWathchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWathchDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_WATCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
};
