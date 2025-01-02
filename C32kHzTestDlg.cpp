// C32kHzTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "C32kHzTestDlg.h"
#include "afxdialogex.h"


// C32kHzTestDlg dialog

IMPLEMENT_DYNAMIC(C32kHzTestDlg, CDialogEx)

C32kHzTestDlg::C32kHzTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(C32kHzTestDlg::IDD, pParent)
{

}

C32kHzTestDlg::~C32kHzTestDlg()
{
}

void C32kHzTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_1PPSCOUNT, m_1ppsCountCtrl);
	DDX_Control(pDX, IDC_EDIT_MSCOUNT, m_1msCountCtrl);
	DDX_Control(pDX, IDC_CHECK_TIMELOCK, m_timeLockCtrl);
	DDX_Control(pDX, IDC_CHECK_RTCLOCK, m_rtcLockCtrl);
}


BEGIN_MESSAGE_MAP(C32kHzTestDlg, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// C32kHzTestDlg message handlers


void C32kHzTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString displayStr;
	UINT8 retry;

	CDialogEx::OnTimer(nIDEvent);
	if (nIDEvent==500)
	{
		KillTimer(500);

		retry=0;
retryRTC:
		m_bipPtr->SendGeneric(MBP_GETRTC);
		if (!m_bipPtr->ResponseWait(1000))
		{
			if (retry++<3) goto retryRTC;
			MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
			OnOK();
			return;
		}

		retry=0;
retryGPS:
		m_bipPtr->SendGeneric(MBP_GETGPSINFO);
		if (!m_bipPtr->ResponseWait(1000))
		{			
			if (retry++<3) goto retryGPS;
			MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
			OnOK();
			return;
		}

		displayStr.Format("%lu",m_bipPtr->m_1ppsCount);
		m_1ppsCountCtrl.SetWindowTextA(displayStr);

		displayStr.Format("%lu",m_bipPtr->m_1ppsMsCount);
		m_1msCountCtrl.SetWindowTextA(displayStr);

	  m_timeLockCtrl.SetCheck(m_bipPtr->m_gpsInfo.timeMarkValid?1:0);
	  m_rtcLockCtrl.SetCheck(m_bipPtr->m_rtcTimeLock?1:0);

		SetTimer(500,1000,NULL);
	}
}


BOOL C32kHzTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_1ppsCountCtrl.SetWindowTextA("Acquiring");
	m_1msCountCtrl.SetWindowTextA("Acquiring");

	SetTimer(500,1000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL C32kHzTestDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(500);
	return CDialogEx::DestroyWindow();
}
