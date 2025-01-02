// FaultDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "FaultDlg.h"


// CFaultDlg dialog

IMPLEMENT_DYNAMIC(CFaultDlg, CDialog)

CFaultDlg::CFaultDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFaultDlg::IDD, pParent)
	, m_faultsString(_T(""))
{

}

CFaultDlg::~CFaultDlg()
{
}

void CFaultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FAULTS, m_faultsString);
}


BEGIN_MESSAGE_MAP(CFaultDlg, CDialog)
END_MESSAGE_MAP()


// CFaultDlg message handlers
