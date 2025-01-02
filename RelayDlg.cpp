// RelayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "RelayDlg.h"


// CRelayDlg dialog

IMPLEMENT_DYNAMIC(CRelayDlg, CDialog)

CRelayDlg::CRelayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRelayDlg::IDD, pParent)
	, m_relayIndex(0)
{

}

CRelayDlg::~CRelayDlg()
{
}

void CRelayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_SOLIDSTATERELAY, m_relayIndex);
}


BEGIN_MESSAGE_MAP(CRelayDlg, CDialog)
END_MESSAGE_MAP()


// CRelayDlg message handlers
