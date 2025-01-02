#pragma once


// CInterrupterSettings dialog

class CInterrupterSettings : public CDialog
{
	DECLARE_DYNAMIC(CInterrupterSettings)

public:
	CInterrupterSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInterrupterSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_INTERRUPTERSETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_cycleTime;
	DWORD m_offTime;
	DWORD m_duration;
	afx_msg void OnBnClickedOk();
};
