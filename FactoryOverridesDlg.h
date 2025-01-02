#pragma once


// CFactoryOverridesDlg dialog

class CFactoryOverridesDlg : public CDialog
{
	DECLARE_DYNAMIC(CFactoryOverridesDlg)

public:
	CFactoryOverridesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFactoryOverridesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FACTORYOVERRIDES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_gpsEnabled;
	BOOL m_largeMemEnabled;
	BOOL m_prototypeUnit;
};
