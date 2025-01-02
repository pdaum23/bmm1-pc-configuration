#pragma once


// CRelayDlg dialog

class CRelayDlg : public CDialog
{
	DECLARE_DYNAMIC(CRelayDlg)

public:
	CRelayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRelayDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RELAYSELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_relayIndex;
};
