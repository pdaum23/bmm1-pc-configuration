#pragma once


// CShuntCalcDlg dialog

class CShuntCalcDlg : public CDialog
{
	DECLARE_DYNAMIC(CShuntCalcDlg)

public:
	CShuntCalcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShuntCalcDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SHUNTCALC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_shuntFullScaleAmps;
	double m_shuntFullScaleMillivolts;
	afx_msg void OnBnClickedOk();
};
