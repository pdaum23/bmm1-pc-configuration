// FactoryOverridesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "FactoryOverridesDlg.h"


// CFactoryOverridesDlg dialog

IMPLEMENT_DYNAMIC(CFactoryOverridesDlg, CDialog)

CFactoryOverridesDlg::CFactoryOverridesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFactoryOverridesDlg::IDD, pParent)
	, m_gpsEnabled(FALSE)
	, m_largeMemEnabled(FALSE)
	, m_prototypeUnit(FALSE)
{

}

CFactoryOverridesDlg::~CFactoryOverridesDlg()
{
}

void CFactoryOverridesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_GPS, m_gpsEnabled);
	DDX_Check(pDX, IDC_CHECK_LARGEMEM, m_largeMemEnabled);
	DDX_Check(pDX, IDC_CHECK_PROTOTYPE, m_prototypeUnit);
}


BEGIN_MESSAGE_MAP(CFactoryOverridesDlg, CDialog)
END_MESSAGE_MAP()


// CFactoryOverridesDlg message handlers
