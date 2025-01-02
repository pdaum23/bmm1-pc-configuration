// LocationNotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "LocationNotesDlg.h"


// LocationNotesDlg dialog

IMPLEMENT_DYNAMIC(CSdlLocationNotesDlg, CDialog)

CSdlLocationNotesDlg::CSdlLocationNotesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSdlLocationNotesDlg::IDD, pParent)
	, m_sdlNotesEnabled(FALSE)
	, m_sdlChainage(0)
	, m_sdlStnSeries(0)
{
}

CSdlLocationNotesDlg::~CSdlLocationNotesDlg()
{
}

BOOL CSdlLocationNotesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	((CButton*)GetDlgItem(IDC_EDIT_SDLCHAINAGE))->EnableWindow(m_sdlNotesEnabled);
	((CEdit*)GetDlgItem(IDC_EDIT_SDLCHAINAGE))->SetReadOnly(!m_sdlNotesEnabled);
	((CButton*)GetDlgItem(IDC_EDIT_SDLSTNSERIES))->EnableWindow(m_sdlNotesEnabled);
	((CEdit*)GetDlgItem(IDC_EDIT_SDLSTNSERIES))->SetReadOnly(!m_sdlNotesEnabled);

	((CWnd*)GetDlgItem(IDC_STATIC_SDLCHAINAGE))->EnableWindow(m_sdlNotesEnabled);
	((CWnd*)GetDlgItem(IDC_STATIC_SDLSTNSERIES))->EnableWindow(m_sdlNotesEnabled);
	return TRUE;
}

void CSdlLocationNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SDLLOCATIONENNOTESABLE, m_sdlNotesEnabled);
	DDX_Text(pDX, IDC_EDIT_SDLCHAINAGE, m_sdlChainage);
	DDV_MinMaxUInt(pDX, m_sdlChainage, 0, 999999);
	DDX_Text(pDX, IDC_EDIT_SDLSTNSERIES, m_sdlStnSeries);
	DDV_MinMaxUInt(pDX, m_sdlStnSeries, 0, 9999999);
}


BEGIN_MESSAGE_MAP(CSdlLocationNotesDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSdlLocationNotesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_SDLLOCATIONENNOTESABLE, &CSdlLocationNotesDlg::OnBnClickedEnable)
END_MESSAGE_MAP()

// LocationNotesDlg message handlers

void CSdlLocationNotesDlg::OnBnClickedOk()
{
	if (UpdateData(TRUE))
	{
		OnOK();
	}
}

void CSdlLocationNotesDlg::OnBnClickedEnable()
{
	UpdateData(TRUE);
	((CButton*)GetDlgItem(IDC_EDIT_SDLCHAINAGE))->EnableWindow(m_sdlNotesEnabled);
	((CEdit*)GetDlgItem(IDC_EDIT_SDLCHAINAGE))->SetReadOnly(!m_sdlNotesEnabled);
	((CButton*)GetDlgItem(IDC_EDIT_SDLSTNSERIES))->EnableWindow(m_sdlNotesEnabled);
	((CEdit*)GetDlgItem(IDC_EDIT_SDLSTNSERIES))->SetReadOnly(!m_sdlNotesEnabled);

	((CWnd*)GetDlgItem(IDC_STATIC_SDLCHAINAGE))->EnableWindow(m_sdlNotesEnabled);
	((CWnd*)GetDlgItem(IDC_STATIC_SDLSTNSERIES))->EnableWindow(m_sdlNotesEnabled);
}
