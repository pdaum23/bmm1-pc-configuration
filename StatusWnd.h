#pragma once
#include "afxwin.h"


// StatusWnd dialog

class StatusWnd : public CDialog
{
	DECLARE_DYNAMIC(StatusWnd)

public:
	StatusWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~StatusWnd();

// Dialog Data
	enum { IDD = IDD_DIALOG_STATUS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_statusTextCtrl;
};
