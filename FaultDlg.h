#pragma once


// CFaultDlg dialog

class CFaultDlg : public CDialog
{
	DECLARE_DYNAMIC(CFaultDlg)

public:
	CFaultDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFaultDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FAULTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_faultsString;
};
