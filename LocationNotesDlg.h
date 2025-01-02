#pragma once


// CLocationNotesDlg dialog

class CSdlLocationNotesDlg : public CDialog
{
	DECLARE_DYNAMIC(CSdlLocationNotesDlg)

public:
	CSdlLocationNotesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSdlLocationNotesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SDLLOCATIONNOTES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	UINT m_sdlChainage;
	UINT m_sdlStnSeries;
	BOOL m_sdlNotesEnabled;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedEnable();
};
