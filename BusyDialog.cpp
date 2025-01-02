// BusyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "BusyDialog.h"


// CBusyDialog dialog

IMPLEMENT_DYNAMIC(CBusyDialog, CDialog)

CBusyDialog::CBusyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBusyDialog::IDD, pParent)
{
	m_cancel=FALSE;
}

CBusyDialog::~CBusyDialog()
{
}

void CBusyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BUSYDLG, m_progressBarCtrl);
	DDX_Control(pDX, IDC_STATIC_CAPTION, m_captionTextCtrl);
	DDX_Control(pDX, IDC_BUTTON_PROGRESSABORT, m_abortButtonCtrl);
}


BEGIN_MESSAGE_MAP(CBusyDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CBusyDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_PROGRESSABORT, &CBusyDialog::OnBnClickedButtonProgressabort)
END_MESSAGE_MAP()


// CBusyDialog message handlers


void CBusyDialog::OnBnClickedOk()
{
	m_cancel=TRUE;
	//CDialog::OnOK();
}


void CBusyDialog::OnBnClickedButtonProgressabort()
{
	m_cancel=TRUE;
	// TODO: Add your control notification handler code here
}
