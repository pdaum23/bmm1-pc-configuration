// TransducerCalcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "TransducerCalcDlg.h"


// CTransducerCalcDlg dialog

IMPLEMENT_DYNAMIC(CTransducerCalcDlg, CDialog)

CTransducerCalcDlg::CTransducerCalcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransducerCalcDlg::IDD, pParent)
	, m_rext(250)
	, m_4mATransducerValue(0)
	, m_20mATransducerValue(0)
{

}

CTransducerCalcDlg::~CTransducerCalcDlg()
{
}

void CTransducerCalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_REXT, m_rext);
	DDV_MinMaxDouble(pDX, m_rext, 0, 100000);
	DDX_Text(pDX, IDC_EDIT_4MAVALUE, m_4mATransducerValue);
	DDX_Text(pDX, IDC_EDIT_20MAVALUE, m_20mATransducerValue);
}


BEGIN_MESSAGE_MAP(CTransducerCalcDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTransducerCalcDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTransducerCalcDlg message handlers

void CTransducerCalcDlg::OnBnClickedOk()
{
	if (UpdateData(TRUE))
	{
		OnOK();
	}
}
