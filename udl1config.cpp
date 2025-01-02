// udl1config.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "udl1config.h"
#include "udl1configDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cudl1configApp

BEGIN_MESSAGE_MAP(Cudl1configApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Cudl1configApp construction

Cudl1configApp::Cudl1configApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Cudl1configApp object

Cudl1configApp theApp;


// Cudl1configApp initialization

BOOL Cudl1configApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	AfxEnableControlContainer();
	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Mobiltex Data Ltd."));

	CString strSection       = "AccessControl";
	CString strIntItem       = "EnableDebugConsole";
	CWinApp* pApp = AfxGetApp();
	int nValue;
	nValue = pApp->GetProfileInt(strSection, strIntItem, 0);
	pApp->WriteProfileInt(strSection, strIntItem, nValue); //Create the value in the registry to make it easier to edit
	if (nValue)
	{
		AllocConsole();
		freopen("CONIN$","rb",stdin);   // reopen stdin handle as console window input
		freopen("CONOUT$","wb",stdout);  // reopen stout handle as console window output
		freopen("CONOUT$","wb",stderr); // reopen stderr handle as console window output
		printf("Debug console enabled.\r\n");
	}

	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (!::GetVersionEx(&vi)) 
	{
		MessageBox(NULL,"Error reading OS version.","Error",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	// Handle windows 9x and NT4 specially
	if (vi.dwMajorVersion < 5) 
	{
		MessageBox(NULL,"This application requires Windows 2000 or higher to operate.","Error",MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	Cudl1configDlg dlg;
	m_pMainWnd = &dlg;

	// Get version information from the application
	char szVer[256], szCopyright[256];
	*szVer=*szCopyright=0;
	GetAppVersion(szVer,szCopyright,255);

	dlg.m_appVerStr=szVer;

	if (strlen(szVer))
		dlg.m_dialogTitle=CString("Mobiltex corTalk® uDL1 Configuration Editor v") + CString(szVer) + CString(" ©2011-2021");
	else
		dlg.m_dialogTitle=CString("Mobiltex corTalk® uDL1 Configuration Editor v(Fix Me) ©2011-2021");

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void Cudl1configApp::GetAppVersion(char *lpszVer, char *lpszCopyright, int n)
// This function writes the program version and copyright to the passed strings. 
{
  // Get version information from the application
  char szFullPath[200];
  GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
  DWORD dwVerHnd;
  DWORD dwVerInfoSize=GetFileVersionInfoSize(szFullPath, &dwVerHnd);
  if (dwVerInfoSize) 
    {
      LPSTR lpstrVffInfo,lpVer;
      UINT dwBytes;
      HANDLE hMem;
      hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
      if (hMem)
        {
          if (lpstrVffInfo=(char*)GlobalLock(hMem))
            {
              GetFileVersionInfo(szFullPath,dwVerHnd,dwVerInfoSize,lpstrVffInfo);
              if (VerQueryValue((LPVOID)lpstrVffInfo,TEXT("\\StringFileInfo\\040904B0\\ProductVersion"),(void**)&lpVer,&dwBytes))
                {
                  if (lpVer && dwBytes)
                    {
                      if (lpszVer) 
                        {
                          strncat(lpszVer,lpVer,n);
                          lpszVer[n-1]=0;
                        }
                    }
                }
              if (VerQueryValue((LPVOID)lpstrVffInfo,TEXT("\\StringFileInfo\\040904B0\\LegalCopyright"),(void**)&lpVer,&dwBytes))
                {
                  if (lpVer && dwBytes)
                    {
                      if (lpszCopyright)
                        {
                          strncat(lpszCopyright,lpVer,n);
                          lpszCopyright[n-1]=0;
                        }
                    }
                }
              GlobalUnlock(hMem);
            }
          GlobalFree(hMem);
        }
    }
}