#pragma once

#include "BusyDialog.h"

// CFileSplitUtilityDlg dialog

class CFileSplitUtilityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileSplitUtilityDlg)

public:
	CFileSplitUtilityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileSplitUtilityDlg();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_SPLITDATAFILES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSelectfiles();
	BOOL m_busy;
	BOOL m_notesInAllOutput;
	BOOL m_verbalDates;
	BOOL m_excelDates;
	BOOL m_dcReadings;
	BOOL m_acReadings;
	BOOL m_tempReadings;
	BOOL m_dcWaveReadings;
	BOOL m_separateSessionFiles;
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonConvertfiles();
	BOOL ProcessFile(CString filename, CBusyDialog &busyDlg);
	void ProcessMessages();
	BOOL m_includeUnits;
};
