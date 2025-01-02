// ShuntCalcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "ShuntCalcDlg.h"


// CShuntCalcDlg dialog

IMPLEMENT_DYNAMIC(CShuntCalcDlg, CDialog)

CShuntCalcDlg::CShuntCalcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShuntCalcDlg::IDD, pParent)
	, m_shuntFullScaleAmps(0)
	, m_shuntFullScaleMillivolts(50)
{

}

CShuntCalcDlg::~CShuntCalcDlg()
{
}

void CShuntCalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_AMPS, m_shuntFullScaleAmps);
	DDV_MinMaxDouble(pDX, m_shuntFullScaleAmps, 0, 1000000);
	DDX_Text(pDX, IDC_EDIT_MILLIVOLTS, m_shuntFullScaleMillivolts);
	DDV_MinMaxDouble(pDX, m_shuntFullScaleMillivolts, 1, 150);
}


BEGIN_MESSAGE_MAP(CShuntCalcDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CShuntCalcDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CShuntCalcDlg message handlers

void CShuntCalcDlg::OnBnClickedOk()
{
	if (UpdateData(TRUE))
	{
		OnOK();
	}
}
