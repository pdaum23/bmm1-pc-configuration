#pragma once


// CTransducerCalcDlg dialog

class CTransducerCalcDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransducerCalcDlg)

public:
	CTransducerCalcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransducerCalcDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_4T020MA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_rext;
	double m_4mATransducerValue;
	double m_20mATransducerValue;
	afx_msg void OnBnClickedOk();
};
