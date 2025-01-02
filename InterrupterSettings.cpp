// InterrupterSettings.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "InterrupterSettings.h"


// CInterrupterSettings dialog

IMPLEMENT_DYNAMIC(CInterrupterSettings, CDialog)

CInterrupterSettings::CInterrupterSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CInterrupterSettings::IDD, pParent)
	, m_cycleTime(1000)
	, m_offTime(500)
	, m_duration(60)
{

}

CInterrupterSettings::~CInterrupterSettings()
{
}

void CInterrupterSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CYCLETIME, m_cycleTime);
	DDV_MinMaxUInt(pDX, m_cycleTime, 1, 60000);
	DDX_Text(pDX, IDC_EDIT_OFFTIME, m_offTime);
	DDV_MinMaxUInt(pDX, m_offTime, 1, 59999);
	DDX_Text(pDX, IDC_EDIT_DURATION, m_duration);
	DDV_MinMaxUInt(pDX, m_duration, 1, 525600);
}


BEGIN_MESSAGE_MAP(CInterrupterSettings, CDialog)
	ON_BN_CLICKED(IDOK, &CInterrupterSettings::OnBnClickedOk)
END_MESSAGE_MAP()


// CInterrupterSettings message handlers

void CInterrupterSettings::OnBnClickedOk()
{
	if (UpdateData(TRUE))
	{
		if (m_offTime>m_cycleTime) m_offTime=m_cycleTime;
		UpdateData(FALSE);
		OnOK();
	}
}
