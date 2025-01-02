#pragma once
#include "afxwin.h"

#include "bipxprot.hpp"

// C32kHzTestDlg dialog

class C32kHzTestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(C32kHzTestDlg)

public:
	C32kHzTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~C32kHzTestDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_32KHZTEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_1ppsCountCtrl;
	CEdit m_1msCountCtrl;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	CButton m_timeLockCtrl;

	C_BIP *m_bipPtr;
	CButton m_rtcLockCtrl;
};
