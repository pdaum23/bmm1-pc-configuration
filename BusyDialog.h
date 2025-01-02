#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CBusyDialog dialog

class CBusyDialog : public CDialog
{
	DECLARE_DYNAMIC(CBusyDialog)

public:
	CBusyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBusyDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_BUSYSTATUS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_progressBarCtrl;
	CStatic m_captionTextCtrl;
	BOOL m_cancel;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonProgressabort();
	CButton m_abortButtonCtrl;
};
