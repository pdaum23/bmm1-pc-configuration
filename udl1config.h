// udl1config.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cudl1configApp:
// See udl1config.cpp for the implementation of this class
//

class Cudl1configApp : public CWinApp
{
public:
	Cudl1configApp();

// Overrides
	public:
	virtual BOOL InitInstance();

	void GetAppVersion(char *lpszVer, char *lpszCopyright, int n);

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cudl1configApp theApp;