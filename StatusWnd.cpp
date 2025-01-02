// StatusWnd.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "StatusWnd.h"


// StatusWnd dialog

IMPLEMENT_DYNAMIC(StatusWnd, CDialog)

StatusWnd::StatusWnd(CWnd* pParent /*=NULL*/)
	: CDialog(StatusWnd::IDD, pParent)
{

}

StatusWnd::~StatusWnd()
{
}

void StatusWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STATUSTEXT, m_statusTextCtrl);
}


BEGIN_MESSAGE_MAP(StatusWnd, CDialog)
END_MESSAGE_MAP()


// StatusWnd message handlers
