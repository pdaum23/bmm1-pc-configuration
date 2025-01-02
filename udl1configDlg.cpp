// udl1configDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "udl1configDlg.h"
#include "comenum.h"
#include "bipxprot.hpp"
#include "ftd2xx.h"
#include "interruptersettings.h"
#include "shuntcalcdlg.h"
#include "locationnotesdlg.h"
#include "transducercalcdlg.h"
#include "relaydlg.h"
#include "factoryoverridesdlg.h"
#include "FaultDlg.h"
#include "statuswnd.h"
#include "BusyDialog.h"
#include "zoomscrolldemodlg.h"
#include "FileSplitUtilityDlg.h"
#include "C32kHzTestDlg.h"
#include <CkCsv.h>
#include <math.h>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FIRMWARE_CURRENT_VER 118

#define fabs(x) (((x)<0)?-(x):(x))

// CAboutDlg dialog used for App About

#define DATALOG_MAXSIZE (32*1024UL*1024UL)
UINT8 datalogMemory[DATALOG_MAXSIZE]; //Local storage for datalogger memory
DWORD datalogMemoryCount;

CString datalogReadingsTzStr;
double datalogReadingsDC[DATALOG_MAXSIZE/8];
double datalogReadingsDCTime[DATALOG_MAXSIZE/8];
double datalogReadingsDCType[DATALOG_MAXSIZE/8]; //0=regular, 1=wave, 2=int off, 3=int on
DWORD datalogReadingsDCCount;

double datalogReadingsAC[DATALOG_MAXSIZE/8];
double datalogReadingsACTime[DATALOG_MAXSIZE/8];
double datalogReadingsACType[DATALOG_MAXSIZE/8]; //0=regular, 1=unused, 2=int off, 3=int on
DWORD datalogReadingsACCount;

double datalogReadingsTemp[DATALOG_MAXSIZE/8];
double datalogReadingsTempTime[DATALOG_MAXSIZE/8];
DWORD datalogReadingsTempCount;

T_WAVE_SEGMENT datalogReadingsWaveSegments[MAX_WAVE_SEGMENTS];
UINT32 datalogReadingsWaveSegmentCount;

CPrintDialog printDlg(FALSE);

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Cudl1configDlg dialog


Cudl1configDlg::Cudl1configDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cudl1configDlg::IDD, pParent)
	, m_inputRange(0)
	, m_dcSamplingEnabled(FALSE)
	, m_acSamplingEnabled(FALSE)
	, m_dcScaleFactor(0)
	, m_acScaleFactor(0)
	, m_dcOffset(0)
	, m_acOffset(0)
	, m_tempSamplingEnabled(FALSE)
	, m_temperatureUnits(0)
	, m_dcUnits(0)
	, m_acUnits(0)
	, m_dateFormat(0)
	, m_dcWaveCaptureEnable(FALSE)
	, m_dcWaveCaptureRepetitionInterval(0)
	, m_dcWaveCaptureDuration(0)
	, m_requireGPSToStart(FALSE)
	, m_dcWaveCaptureContinuous(FALSE)
	, m_dcWaveCaptureSampleRate(0)
	, m_noLongTermGPSSync(FALSE)
	, m_timeDisplayEnabled(FALSE)
	, m_timeDisplayFormat(0)
	, m_fastSyncOffsetMs(0)
	, m_fastSyncEnable(FALSE)
	, m_minEHPEEnable(FALSE)
	, m_minEHPEMeters(0)
	, m_minEHPETimeoutMinutes(0)
	, m_interruptionTrackingEnable(FALSE)
	, m_interruptionTrackingOnFirst(FALSE)
	, m_interruptionTrackingOffTimeMs(0)
	, m_interruptionTrackingOnTime(0)
	, m_interruptionTrackingCycleTime(0)
	, m_interruptionTrackingOnToOffOffsetMs(0)
	, m_interruptionTrackingOffToOnOffsetMs(0)
	, m_interruptionTrackingUTCOffsetMs(0)
	, m_acquisitionDelayMins(0)	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_advancedUnit=FALSE;
	m_g2Unit=FALSE;
	m_sdlNotes = SdlNotes();
}

void Cudl1configDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PORTSEL, m_comPortControl);
	DDX_Control(pDX, IDC_RICHEDIT2_LINKSTATUS, m_linkStatusRichCtrl);
	DDX_Control(pDX, IDC_RICHEDIT_CONFIGSTATUS, m_configStatusRichCtrl);
	DDX_Control(pDX, IDC_EDIT_FIRMVER, m_codeVersionCtrl);
	DDX_Control(pDX, IDC_EDIT_ESN, m_serialNumberCtrl);
	DDX_Control(pDX, IDC_EDIT_FACTORYOVERRIDES, m_factoryOverrides);
	DDX_Radio(pDX, IDC_RADIO_RNGLOW, m_inputRange);
	DDX_Check(pDX, IDC_CHECK_DCENABLED, m_dcSamplingEnabled);
	DDX_Check(pDX, IDC_CHECK_ACENABLED, m_acSamplingEnabled);
	DDX_Text(pDX, IDC_EDIT_SCALEFACTORDC, m_dcScaleFactor);
	DDX_Text(pDX, IDC_EDIT_SCALEFACTORAC, m_acScaleFactor);
	DDX_Text(pDX, IDC_EDIT_OFFSETDC, m_dcOffset);
	DDX_Text(pDX, IDC_EDIT_OFFSETAC, m_acOffset);
	DDX_Control(pDX, IDC_PROGRESS_MEMUSE, m_memUseProgress);
	DDX_Check(pDX, IDC_CHECK_TEMPENABLED, m_tempSamplingEnabled);
	DDX_Control(pDX, IDC_EDIT_SAMPLECYCLEPERIODHOURS, m_sampleIntervalHoursCtrl);
	DDX_Control(pDX, IDC_EDIT_SAMPLECYCLEPERIODMINS, m_sampleIntervalMinsCtrl);
	DDX_Control(pDX, IDC_EDIT_SAMPLECYCLEPERIODSECS, m_sampleIntervalSecsCtrl);
	DDX_Radio(pDX, IDC_RADIO_TEMPC, m_temperatureUnits);
	DDX_Radio(pDX, IDC_RADIO_DCV, m_dcUnits);
	DDX_Radio(pDX, IDC_RADIO_ACV, m_acUnits);
	DDX_Radio(pDX, IDC_RADIO_UTC, m_dateFormat);
	DDX_Control(pDX, IDC_RADIO_UTC, m_timeFormatCtrl);
	DDX_Check(pDX, IDC_CHECK_DCWAVECAPEN, m_dcWaveCaptureEnable);
	DDX_Text(pDX, IDC_EDIT_DCWAVEREPINT, m_dcWaveCaptureRepetitionInterval);
	DDX_Text(pDX, IDC_EDIT_DCWAVEDUR, m_dcWaveCaptureDuration);
	DDX_Check(pDX, IDC_CHECK_DCWAVEGPSSYNC, m_requireGPSToStart);
	DDX_Check(pDX, IDC_CHECK_DCWAVECAPCONT, m_dcWaveCaptureContinuous);
	DDX_Text(pDX, IDC_EDIT_DCWAVESAMPLERATE, m_dcWaveCaptureSampleRate);
	DDX_Check(pDX, IDC_CHECK_NOEXTENDEDGPSSYNC, m_noLongTermGPSSync);
	DDX_Control(pDX, IDC_EDIT_SAMPLECYCLEDELAYMS, m_sampleDelayMsCtrl);
	DDX_Check(pDX, IDC_CHECK_TIMEENABLED, m_timeDisplayEnabled);
	DDX_Radio(pDX, IDC_RADIO_TDUTC, m_timeDisplayFormat);
	DDX_Text(pDX, IDC_EDIT_FASTOFFSETMS, m_fastSyncOffsetMs);
	DDX_Check(pDX, IDC_CHECK_SYNCOFFSETENABLE, m_fastSyncEnable);
	DDX_Check(pDX, IDC_CHECK_MINEHPEENABLE, m_minEHPEEnable);
	DDX_Text(pDX, IDC_EDIT_EHPEMIN, m_minEHPEMeters);
	DDX_Text(pDX, IDC_EDIT_EHPEMINTIMEOUT, m_minEHPETimeoutMinutes);
	DDX_Check(pDX, IDC_CHECK_INTTRACKINGENABLE, m_interruptionTrackingEnable);
	DDX_Check(pDX, IDC_CHECK_INTTRACKINGONFIRST, m_interruptionTrackingOnFirst);
	DDX_Text(pDX, IDC_EDIT_INTOFFTIME, m_interruptionTrackingOffTimeMs);
	DDX_Text(pDX, IDC_EDIT_INTONTIME, m_interruptionTrackingOnTime);
	DDX_Text(pDX, IDC_EDIT_INTCYCLETIME, m_interruptionTrackingCycleTime);
	DDX_Text(pDX, IDC_EDIT_INTONTOOFFOFFSET, m_interruptionTrackingOnToOffOffsetMs);
	DDX_Text(pDX, IDC_EDIT_INTOFFTOONOFFSET, m_interruptionTrackingOffToOnOffsetMs);
	DDX_Text(pDX, IDC_EDIT_INTUTCOFFSET, m_interruptionTrackingUTCOffsetMs);
	DDX_Text(pDX, IDC_EDIT_ACQUISITIONDELAYMINS, m_acquisitionDelayMins);
	DDX_Control(pDX, IDC_EDIT_INTCYCLETIME, m_interruptionTrackingCycleTimeMsCtrl);
}

BEGIN_MESSAGE_MAP(Cudl1configDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_PORTSEL, &Cudl1configDlg::OnCbnSelchangeComboPortsel)
	ON_BN_CLICKED(IDC_BUTTON_READRMU, &Cudl1configDlg::OnBnClickedButtonReadrmu)
	ON_BN_CLICKED(IDC_BUTTON_WRITERMU, &Cudl1configDlg::OnBnClickedButtonWritermu)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_DEFAULTS, &Cudl1configDlg::OnBnClickedButtonDefaults)
	ON_BN_CLICKED(IDC_BUTTON_COMMSTATUS, &Cudl1configDlg::OnBnClickedButtonCommstatus)
	ON_BN_CLICKED(IDC_BUTTON_READFAULTS, &Cudl1configDlg::OnBnClickedButtonReadfaults)
	ON_COMMAND(ID_COMMUNICATIONS_READFROMuDL1, &Cudl1configDlg::OnBnClickedButtonReadrmu)
	ON_COMMAND(ID_COMMUNICATIONS_WRITETOuDL1, &Cudl1configDlg::OnBnClickedButtonWritermu)
	ON_COMMAND(ID_FILE_DEFAULTS, &Cudl1configDlg::OnBnClickedButtonDefaults)
	ON_COMMAND(ID_FILE_EXIT, &Cudl1configDlg::OnOK)
	ON_COMMAND(ID_HELP_CONFIGURATIONGUIDE, &Cudl1configDlg::OnHelpConfigurationguide)
	ON_BN_CLICKED(IDC_BUTTON_READFILE, &Cudl1configDlg::OnBnClickedButtonReadfile)
	ON_BN_CLICKED(IDC_BUTTON_WRITEFILE, &Cudl1configDlg::OnBnClickedButtonWritefile)
	ON_COMMAND(ID_FILE_OPEN32772, &Cudl1configDlg::OnBnClickedButtonReadfile)
	ON_COMMAND(ID_FILE_SAVEAS, &Cudl1configDlg::OnBnClickedButtonWritefile)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_REBOOT, &Cudl1configDlg::OnBnClickedButtonReboot)
	ON_BN_CLICKED(IDC_BUTTON_SETRTC, &Cudl1configDlg::OnBnClickedButtonSetrtc)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACTDATA, &Cudl1configDlg::OnBnClickedButtonExtractdata)
	ON_BN_CLICKED(IDC_BUTTON_CLEARDATA, &Cudl1configDlg::OnBnClickedButtonCleardata)
	ON_BN_CLICKED(IDC_BUTTON_SETFACTORY, &Cudl1configDlg::OnBnClickedButtonSetfactory)
	ON_BN_CLICKED(IDC_BUTTON_ANALOGTEST, &Cudl1configDlg::OnBnClickedButtonAnalogtest)
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATE, &Cudl1configDlg::OnBnClickedButtonCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_SETSERIAL, &Cudl1configDlg::OnBnClickedButtonSetserial)
	ON_BN_CLICKED(IDC_BUTTON_SHUNTCALCDC, &Cudl1configDlg::OnBnClickedButtonShuntcalcdc)
	ON_BN_CLICKED(IDC_BUTTON_SHUNTCALCAC, &Cudl1configDlg::OnBnClickedButtonShuntcalcac)
	ON_BN_CLICKED(IDC_BUTTON_CHARTDC, &Cudl1configDlg::OnBnClickedButtonChartdc)
	ON_BN_CLICKED(IDC_BUTTON_CHARTAC, &Cudl1configDlg::OnBnClickedButtonChartac)
	ON_BN_CLICKED(IDC_BUTTON_CHARTTEMP, &Cudl1configDlg::OnBnClickedButtonCharttemp)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_READDATAFILE, &Cudl1configDlg::OnBnClickedButtonReaddatafile)
	ON_BN_CLICKED(IDC_BUTTON_CHARTALL, &Cudl1configDlg::OnBnClickedButtonChartall)
	ON_BN_CLICKED(IDC_BUTTON_ANALOGTEMPTEST, &Cudl1configDlg::OnBnClickedButtonAnalogtemptest)
	ON_BN_CLICKED(IDC_BUTTON_DLGTEST, &Cudl1configDlg::OnBnClickedButtonDlgtest)
	ON_BN_CLICKED(IDC_BUTTON_SPLITDATAFILE, &Cudl1configDlg::OnBnClickedButtonSplitdatafile)
	ON_BN_CLICKED(IDC_BUTTON_CHARTDCWAVE, &Cudl1configDlg::OnBnClickedButtonChartdcwave)
	ON_BN_CLICKED(IDC_BUTTON_32KHZTEST, &Cudl1configDlg::OnBnClickedButton32khztest)
	ON_BN_CLICKED(IDC_BUTTON_BINARYEXTRACT, &Cudl1configDlg::OnBnClickedButtonBinaryextract)
	ON_COMMAND(ID_HELP_BINARYMEMORYEXTRACTIONTOOL, &Cudl1configDlg::OnBnClickedButtonBinaryextract)
	ON_EN_KILLFOCUS(IDC_EDIT_INTONTIME, &Cudl1configDlg::OnEnKillfocusEditIntontime)
	ON_EN_KILLFOCUS(IDC_EDIT_INTOFFTIME, &Cudl1configDlg::OnEnKillfocusEditIntofftime)
	ON_EN_KILLFOCUS(IDC_EDIT_INTONTOOFFOFFSET, &Cudl1configDlg::OnEnKillfocusEditIntontooffoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_INTOFFTOONOFFSET, &Cudl1configDlg::OnEnKillfocusEditIntofftoonoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_INTUTCOFFSET, &Cudl1configDlg::OnEnKillfocusEditIntutcoffset)
	ON_BN_CLICKED(IDC_BUTTON_CHARTINTDC, &Cudl1configDlg::OnBnClickedButtonChartintdc)
	ON_BN_CLICKED(IDC_BUTTON_CHARTINTAC, &Cudl1configDlg::OnBnClickedButtonChartintac)
	ON_BN_CLICKED(IDC_BUTTON_MEMTIME, &Cudl1configDlg::OnBnClickedButtonMemtime)
	ON_COMMAND(ID_HELP_MAXIMUMACQUISITIONTIMEESTIMATE, &Cudl1configDlg::OnBnClickedButtonMemtime)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACTSDLDATA, &Cudl1configDlg::OnBnClickedButtonExtractSdldata)
	ON_BN_CLICKED(IDC_BUTTON_SDLLOCATIONNOTES, &Cudl1configDlg::OnBnClickedButtonSdllocationnotes)
END_MESSAGE_MAP()


// Cudl1configDlg message handlers

BOOL Cudl1configDlg::OnInitDialog()
{
	FT_STATUS status;
	DWORD dwLibraryVer;
	HUINT32 count;

	CWinApp* pApp = AfxGetApp();
	CString strSection;
	CString strIntItem;
	int nValue;
	CDialogEx::OnInitDialog();

	SetWindowText(m_dialogTitle);
	FindAppPath();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Load accelerators 
	m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_ACCELERATOR_MAINMENU)); 
	ASSERT(m_hAccel);


	//Check to see if we should enable factory defaults programming
	strSection       = "AccessControl";
	strIntItem       = "EnableFactoryDefaultsProgramming";
	nValue = pApp->GetProfileInt(strSection, strIntItem, 0);
	pApp->WriteProfileInt(strSection, strIntItem, nValue); //Create the value in the registry to make it easier to edit
	if (nValue==2912770)
	{
		((CButton*)GetDlgItem(IDC_BUTTON_SETFACTORY))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_CALIBRATE))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_SETSERIAL))->ShowWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_ESN))->SetReadOnly(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_ANALOGTEST))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_ANALOGTEMPTEST))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_DLGTEST))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_32KHZTEST))->ShowWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_BINARYEXTRACT))->ShowWindow(TRUE);
	}


	strSection       = "Options";
	strIntItem       = "ExtractionDateType";
	nValue = pApp->GetProfileInt(strSection, strIntItem, 0);
	pApp->WriteProfileInt(strSection, strIntItem, nValue); //Create the value in the registry to make it easier to edit
	m_dateFormat=nValue?1:0;

	//
	// Get FTDI DLL version
	//
	status = FT_GetLibraryVersion(&dwLibraryVer);
	if (status == FT_OK)
		printf("Library version = 0x%x\n",dwLibraryVer);
	else
	{
		printf("error reading library version\n");
		MessageBox("uDL1 programming device drivers not installed.  Re-install before attempting communications with uDL1.","Error",MB_OK);
	}

	DisplayLinkStatus("Disabled",0,128,255); 
	DisplayConfigStatus("No changes", 0, 255, 0);

	m_serialPortOpen=FALSE;
	m_comPortControl.AddString("Disabled");
	m_comPortControl.AddString("COM5");

	m_memUseProgress.SetStep(1);
	m_memUseProgress.SetRange( 0, 100*100);
	SetMemUseBar(0);

	memcpy(&m_params,&param_data_defaults,sizeof(T_PARAM_PARAMETERS));
	count=sizeof(T_PARAM_PARAMETERS)-2;
	C_FCS::Add((UINT8*)&m_params, &count);
	memcpy(&m_params_original,&param_data_defaults,sizeof(T_PARAM_PARAMETERS));
	count=sizeof(T_PARAM_PARAMETERS)-2;
	C_FCS::Add((UINT8*)&m_params_original, &count);

	EEPROMToDialog();

	datalogReadingsDCCount=0;
	datalogReadingsACCount=0;
	datalogReadingsTempCount=0;
	datalogReadingsWaveSegmentCount=0;

	m_lastFWWarningSerial=0;

	m_timerCounter=0;
	SetTimer(100,1000,NULL);

	//Try to open port right away
#if 0
	m_comPortControl.SetCurSel(1);
	OnCbnSelchangeComboPortsel();
#else
	m_comPortControl.SetCurSel(0);
#endif

	printf("param size=%lu\r\n",sizeof(T_PARAM_PARAMETERS));

	CString tzStr;
	int tzOffset;

	GetTZInfo(tzOffset, tzStr);
	((CButton*)GetDlgItem(IDC_RADIO_LOCAL))->SetWindowText("Local ("+tzStr+")");


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cudl1configDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cudl1configDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cudl1configDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cudl1configDlg::FindAppPath()
{
	char appPathStr[MAX_PATH+1];
	char *p, *lastSlashPtr;

	//Find the path that we were executed from
	appPathStr[MAX_PATH]=0;
	GetModuleFileName( NULL, appPathStr, MAX_PATH );
	//Trim off the application name portion
	lastSlashPtr=p=appPathStr;
	while(*p)
	{
		if ((*p=='\\')||(*p=='/')) lastSlashPtr=p;
		p++;
	}
	*lastSlashPtr=0;
	m_appPathStr=appPathStr;
}

void Cudl1configDlg::FixPath(CString &s)
{
	char strTemp[MAX_PATH], *cPtr;
	strcpy(strTemp,s);
	//Convert forward slashes to backslashes
	cPtr=strTemp;
	while (*cPtr)
	{
		if (*cPtr=='/') *cPtr='\\';
		cPtr++;
	}
	//Remove trailing backslash
	if (strlen(strTemp))
	{
		if (strTemp[strlen(strTemp)-1]=='\\') strTemp[strlen(strTemp)-1]=0;
	}
	s=strTemp;
}
void Cudl1configDlg::OnFileOpen32772()
{
	// TODO: Add your command handler code here
}

void Cudl1configDlg::DisplayLinkStatus(CString s, UINT8 red, UINT8 green, UINT8 blue, UINT8 textRed, UINT8 textGreen, UINT8 textBlue) 
{
#if 0
	char strTemp[255];

	sprintf(strTemp,"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033"
		"{\\fonttbl{\\f0\\fswiss\\fcharset0 Tahoma;}}"
		"{\\colortbl ;\\red%lu\\green%lu\\blue%lu;}"
		"\\viewkind4\\uc1\\pard\\cf1\\b\\f0\\fs16 %s\\cf0\\par}",(DWORD)red,(DWORD)green,(DWORD)blue,s);
	m_linkStatusRichCtrl.SetWindowTextA(strTemp);
#else
	CHARFORMAT cf;
	memset(&cf,0,sizeof(CHARFORMAT));
	cf.cbSize=sizeof(CHARFORMAT);
	//cf.dwMask=CFM_COLOR|CFM_BOLD;
	cf.dwMask=CFM_BOLD;
	cf.dwEffects=CFE_BOLD;
	cf.crTextColor=RGB(textRed,textGreen,textBlue);
	m_linkStatusRichCtrl.SetDefaultCharFormat(cf);
	m_linkStatusRichCtrl.SetWindowTextA(s);
	m_linkStatusRichCtrl.SetBackgroundColor(FALSE,RGB(red,green,blue));

#endif
	UpdateWindow();
}


void Cudl1configDlg::DisplayConfigStatus(CString s, UINT8 red, UINT8 green, UINT8 blue) 
{
#if 0
	char strTemp[255];
	sprintf(strTemp,"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033"
		"{\\fonttbl{\\f0\\fswiss\\fcharset0 Tahoma;}}"
		"{\\colortbl ;\\red%lu\\green%lu\\blue%lu;}"
		"\\viewkind4\\uc1\\pard\\cf1\\b\\f0\\fs16 %s\\cf0\\par}",(DWORD)red,(DWORD)green,(DWORD)blue,s);
	m_configStatusRichCtrl.SetWindowTextA(strTemp);
#else
	m_configStatusRichCtrl.SetWindowTextA(s);
	m_configStatusRichCtrl.SetBackgroundColor(FALSE,RGB(red,green,blue));
#endif
	UpdateWindow();
}


void Cudl1configDlg::SetMemUseBar(double pos)
{
	if (pos>90)
	{
		m_memUseProgress.SetBarColor(RGB(255,0,0));
		m_memUseProgress.SetTextBkColor(CLR_DEFAULT);
		m_memUseProgress.SetTextColor(CLR_DEFAULT);
	}
	else
		if (pos>80)
		{
			m_memUseProgress.SetBarColor(RGB(255,255,0));
			m_memUseProgress.SetTextBkColor(RGB(0,0,0));
			m_memUseProgress.SetTextColor(CLR_DEFAULT);
		}
		else
		{
			m_memUseProgress.SetBarColor(RGB(0,255,0));
			m_memUseProgress.SetTextBkColor(CLR_DEFAULT);
			m_memUseProgress.SetTextColor(CLR_DEFAULT);
		}
		m_memUseProgress.SetPos(pos*100);
}

void Cudl1configDlg::OnCbnSelchangeComboPortsel()
{
	char portString[256];
	char lbString[256];
	UINT8 tries;
	int retryCount;
	DWORD startTime;
	StatusWnd dlgStatus;

	if (UpdateData(TRUE))
	{

		if (m_serialPortOpen)
		{
			DisplayLinkStatus("Disabled",0,128,255);
			m_ser1.Close();
			m_serialPortOpen=FALSE;
			Sleep(100);
		}

		m_comPortControl.SetCurSel(1); //Always selected for uDL1

		if (!m_comPortControl.GetCurSel()) return; //Selected no port

		m_comPortControl.GetLBText(m_comPortControl.GetCurSel(),lbString);
		sprintf(portString,"\\\\.\\%s",lbString);
		printf("Com port: %s\r\n",portString);

		DisplayLinkStatus("Opening",0,0,255);
		if (m_ser1.Options(portString,115200)!=ERROR_SUCCESS)
		{
			DisplayLinkStatus("Disabled",0,128,255);
			m_comPortControl.SetCurSel(0);
			UpdateWindow();
			MessageBox("Error opening USB serial port!  Please connect a uDL1 data logger.","Error",MB_ICONERROR|MB_OK);
			return;
		}
		m_bip1.Options(&m_ser1,"BIP1");

		UpdateData(FALSE);
		m_serialPortOpen=TRUE;
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
#if 0
		//Retrieve parameters if uDL1 is attached
		if (m_serialPortOpen)
		{
			//Grab the existing app checksum and version
			tries=0;
retryGetAppVersion:
			m_bip1.SendGeneric(MBP_GETVERSION);
			if (!m_bip1.ResponseWait(1000))
			{
				if (tries++<2) goto retryGetAppVersion;
			}
			else
			{
				SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));
				OnBnClickedButtonReadrmu();
			}
		}
#endif
	}
}

void Cudl1configDlg::OnBnClickedButtonReadrmu()
{
	ReaduDLConfig();
}


BOOL Cudl1configDlg::ReaduDLConfig()
{
	UINT32 address;
	BOOL failure=FALSE;
	DWORD tries;
	DWORD toRead, numLeft;
	T_PARAM_PARAMETERS paramTemp;
	T_PARAM_MFGDATA paramMfgTemp;
	char strTemp[256];
	CString overrides;

	OnCbnSelchangeComboPortsel();
	if (!m_serialPortOpen)
	{
		//MessageBox("Serial port not selected!","Error",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	DisplayLinkStatus("Busy",255,0,0);

	//Grab the existing app checksum and version
	tries=0;
retryGetAppVersion:
	m_bip1.SendGeneric(MBP_GETVERSION);
	if (!m_bip1.ResponseWait(1000))
	{
		if (tries++<7) goto retryGetAppVersion;
		MessageBox("Failed to ping uDL1.","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return FALSE;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));
	sprintf(strTemp,"%lu",m_bip1.m_serialNumber);
	m_serialNumberCtrl.SetWindowTextA(strTemp);
	sprintf(strTemp,"%lu.%02lu G%u",m_bip1.m_appVersion/100,m_bip1.m_appVersion%100,m_bip1.m_boardType+1);
	m_codeVersionCtrl.SetWindowTextA(strTemp);

	if (m_bip1.m_boardType>0) m_g2Unit=TRUE; else m_g2Unit=FALSE;

	address=0;
	numLeft=sizeof(T_PARAM_PARAMETERS);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return FALSE;
		}
		memcpy(((UINT8*)&paramTemp)+address,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	//sprintf(strTemp,"%u",sizeof(T_PARAM_PARAMETERS));
	//MessageBox(strTemp,"Info",MB_OK);

	//Validate version
	if (paramTemp.paramVersion!=PARAM_VERSION)
	{
		MessageBox("The uDL1 parameter set is not compatible with this version of uDL1Config!","Error",MB_OK|MB_ICONEXCLAMATION);
		sprintf(strTemp,"uDL1 parameter set version=%lu, uDL1Config parameter set version=%lu.\r\n",(UINT32)(paramTemp.paramVersion),PARAM_VERSION);
		//DisplayGeneralStatus(strTemp);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return FALSE;
	}

	//Validate CRC
	if (!C_FCS::Verify((UINT8*)&paramTemp, sizeof(T_PARAM_PARAMETERS)))
	{
		MessageBox("CRC error in parameter data!","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);

		return FALSE;
	}

	//Read in manufacturing option data

	address=PARAM_MFG_OFFSET;
	numLeft=sizeof(T_PARAM_MFGDATA);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead2:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead2;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return FALSE;
		}
		memcpy(((UINT8*)&paramMfgTemp)+address-PARAM_MFG_OFFSET,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	//sprintf(strTemp,"%u",sizeof(T_PARAM_PARAMETERS));
	//MessageBox(strTemp,"Info",MB_OK);

	//Validate version
	if (paramMfgTemp.paramVersion!=PARAM_MFG_VERSION)
	{
		MessageBox("The uDL1 mfg parameter set is not compatible with this version of uDL1Config!","Error",MB_OK|MB_ICONEXCLAMATION);
		sprintf(strTemp,"uDL1 mfg parameter set version=%lu, uDL1Config parameter set version=%lu.\r\n",(UINT32)(paramMfgTemp.paramVersion),PARAM_MFG_VERSION);
		//DisplayGeneralStatus(strTemp);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return FALSE;
	}

	//Validate CRC
	if (!C_FCS::Verify((UINT8*)&paramMfgTemp, sizeof(T_PARAM_MFGDATA)))
	{
		MessageBox("CRC error in mfg parameter data!","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);

		return FALSE;
	}

	memcpy(&m_params,&paramTemp,sizeof(T_PARAM_PARAMETERS));
	memcpy(&m_params_original,&paramTemp,sizeof(T_PARAM_PARAMETERS));
	memcpy(&m_paramsmfg,&paramMfgTemp,sizeof(T_PARAM_MFGDATA));

	if (!m_paramsmfg.optionFlags)
	{
		overrides="None";
		m_advancedUnit=FALSE;
	}
	else
	{
		overrides="";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_GPS)
		{
			overrides+="GPS TIMESYNC\r\n";
			m_advancedUnit=TRUE;
		}
		else
		{
			m_advancedUnit=FALSE;
		}
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_EXTMEM) overrides+="LARGE MEM\r\n";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_NEWCAL) overrides+="CAL V2\r\n";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_PROTOTYPE) overrides+="PROTOTYPE\r\n";

	}
	m_factoryOverrides.SetWindowTextA(overrides);

	EEPROMToDialog();


	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	//If we read from an old firmware version, alert the user to update the firmware
	if ((m_bip1.m_appVersion<FIRMWARE_CURRENT_VER)&&(m_lastFWWarningSerial!=m_bip1.m_serialNumber)) //Warn only once per unit attachment
	{
		m_lastFWWarningSerial=m_bip1.m_serialNumber;
		MessageBox("The connected uDL1 is running an obsolete firmware version.  Some features may be unavailable until the firmware is updated in the unit.  To update the firmware, exit the uDL1 configuration application and run the uDL1 Firmware Update application.  Refer to the uDL1 user manual for further information.","Warning",MB_OK);
	}

	return TRUE;
}

BOOL Cudl1configDlg::DialogToEEPROM()
{
	HUINT32 count;
	CString str;
	int offsetSeconds;
	CString label;

	if (UpdateData(TRUE))
	{

		UINT32 hours, mins, secs, totalSecs, delay;
		CString temp;

		m_sampleIntervalHoursCtrl.GetWindowText(temp);
		hours=atoi(temp);
		m_sampleIntervalMinsCtrl.GetWindowText(temp);
		mins=atoi(temp);
		m_sampleIntervalSecsCtrl.GetWindowText(temp);
		secs=atoi(temp);
		totalSecs=hours*3600+mins*60+secs;
		if (totalSecs>43200) totalSecs=43200;
		if (totalSecs<1) totalSecs=1;

		hours=totalSecs/3600;
		mins=(totalSecs-hours*3600)/60;
		secs=(totalSecs-hours*3600-mins*60);

		temp.Format("%u",hours);
		m_sampleIntervalHoursCtrl.SetWindowText(temp);
		temp.Format("%u",mins);
		m_sampleIntervalMinsCtrl.SetWindowText(temp);
		temp.Format("%u",secs);
		m_sampleIntervalSecsCtrl.SetWindowText(temp);

		//The sample delay is artificially compensated for an additional 100ms in the GUI
		// (100-600ms) in the GUI is (0-500ms) in the actual parameter
		m_sampleDelayMsCtrl.GetWindowText(temp);
		delay=atoi(temp);
		if (delay<100) delay=100;
		delay-=100;
		if (delay>500) delay=500;
		temp.Format("%u",delay+100);
		m_sampleDelayMsCtrl.SetWindowText(temp);

		if (!m_dcSamplingEnabled&&!m_acSamplingEnabled&&!m_tempSamplingEnabled)
		{
			m_dcSamplingEnabled=TRUE;
			((CButton*)GetDlgItem(IDC_CHECK_DCENABLED))->SetCheck(TRUE);
		}

		m_params.samplingCyclePeriod=totalSecs*1000;
		m_params.samplingCycleOffset=delay;
		m_params.rangeSelect=m_inputRange;
		m_params.configFlags&=~(CONFIGFLAGS_DCENABLED|
			CONFIGFLAGS_ACENABLED|
			CONFIGFLAGS_TEMPERATURE_ENABLED|
			CONFIGFLAGS_TEMPERATURE_FAHRENHEIT|
			CONFIGFLAGS_DC_AMPS|
			CONFIGFLAGS_AC_AMPS|
			CONFIGFLAGS_REQUIREGPSLOCK_ENABLED|
			CONFIGFLAGS_FASTSAMPLING_ENABLED|
			CONFIGFLAGS_FASTSAMPLINGCONTINUOUS_ENABLED|
			CONFIGFLAGS_NOLONGTERMGPSSYNC_ENABLED|
			CONFIGFLAGS_INTERRUPTIONTRACKING_ENABLED);
		m_params.configFlags|=m_dcSamplingEnabled?CONFIGFLAGS_DCENABLED:0;
		m_params.configFlags|=m_acSamplingEnabled?CONFIGFLAGS_ACENABLED:0;
		m_params.configFlags|=m_tempSamplingEnabled?CONFIGFLAGS_TEMPERATURE_ENABLED:0;
		m_params.configFlags|=m_temperatureUnits?CONFIGFLAGS_TEMPERATURE_FAHRENHEIT:0;
		m_params.configFlags|=m_dcUnits?CONFIGFLAGS_DC_AMPS:0;
		m_params.configFlags|=m_acUnits?CONFIGFLAGS_AC_AMPS:0;
		m_params.configFlags|=m_requireGPSToStart?CONFIGFLAGS_REQUIREGPSLOCK_ENABLED:0;
		m_params.configFlags|=m_dcWaveCaptureEnable?CONFIGFLAGS_FASTSAMPLING_ENABLED:0;
		m_params.configFlags|=m_dcWaveCaptureContinuous?CONFIGFLAGS_FASTSAMPLINGCONTINUOUS_ENABLED:0;
		m_params.configFlags|=m_noLongTermGPSSync?CONFIGFLAGS_NOLONGTERMGPSSYNC_ENABLED:0;
		m_params.configFlags|=m_interruptionTrackingEnable?CONFIGFLAGS_INTERRUPTIONTRACKING_ENABLED:0;

		if (m_dcWaveCaptureDuration<1) m_dcWaveCaptureDuration=1;
		if (m_dcWaveCaptureRepetitionInterval>720) m_dcWaveCaptureRepetitionInterval=720;
		if ((m_dcWaveCaptureRepetitionInterval*60)<m_dcWaveCaptureDuration) m_dcWaveCaptureRepetitionInterval=((m_dcWaveCaptureDuration-1)/60)+1;
		if (m_dcWaveCaptureRepetitionInterval<1) m_dcWaveCaptureRepetitionInterval=1;
		if (m_dcWaveCaptureDuration>(m_dcWaveCaptureRepetitionInterval*60)) m_dcWaveCaptureDuration=m_dcWaveCaptureRepetitionInterval*60;
		m_params.samplingFastDuration=m_dcWaveCaptureDuration; /* Number of seconds for a fast sampling mode */
		m_params.samplingFastInterval=m_dcWaveCaptureRepetitionInterval*60; /* Number of seconds between fast sampling groups */
		m_dcWaveCaptureSampleRate=100; //Fixed sampling rate of 100ms
		m_params.samplingCyclePeriodFast=m_dcWaveCaptureSampleRate;

		if (m_dcScaleFactor==0.0) m_dcScaleFactor=1.0; //Disallow 0 scale factors
		if (m_acScaleFactor==0.0) m_acScaleFactor=1.0; //Disallow 0 scale factors
		m_params.scaleFactor[PARAM_CHANNEL_DC]=m_dcScaleFactor;
		m_params.scaleFactor[PARAM_CHANNEL_AC]=m_acScaleFactor;
		m_params.offsetFactor[PARAM_CHANNEL_DC]=m_dcOffset;
		m_params.offsetFactor[PARAM_CHANNEL_AC]=m_acOffset;

		if (!GetTZInfo(offsetSeconds, label)) {m_timeDisplayFormat=0; offsetSeconds=0;}

		m_params.configFlagsExtended&=~(CONFIGFLAGSEXT_SHOWTIME|CONFIGFLAGSEXT_LOCALTIME|CONFIGFLAGSEXT_EHPEWAIT|CONFIGFLAGSEXT_LOCKFASTSAMPLING_100MS);
		m_params.configFlagsExtended|=m_timeDisplayEnabled?CONFIGFLAGSEXT_SHOWTIME:0;
		m_params.configFlagsExtended|=m_timeDisplayFormat?CONFIGFLAGSEXT_LOCALTIME:0;
		m_params.configFlagsExtended|=m_minEHPEEnable?CONFIGFLAGSEXT_EHPEWAIT:0;
		m_params.configFlagsExtended|=m_fastSyncEnable?CONFIGFLAGSEXT_LOCKFASTSAMPLING_100MS:0;
		m_params.timeZoneOffsetMins=-offsetSeconds/60;

		if (m_fastSyncOffsetMs>99) m_fastSyncOffsetMs=99;
		m_params.lockedFastSamplingOffsetMs=m_fastSyncOffsetMs;

		if (m_minEHPEMeters<2.5) m_minEHPEMeters=2.5;
		if (m_minEHPEMeters>200.0) m_minEHPEMeters=200.0;
		m_params.minimumEHPE=m_minEHPEMeters*10;

		if (m_minEHPETimeoutMinutes<1) m_minEHPETimeoutMinutes=1;
		if (m_minEHPETimeoutMinutes>60) m_minEHPETimeoutMinutes=60;
		m_params.minimumEHPEWaitTimeSec=m_minEHPETimeoutMinutes*60;

		((CEdit*)GetDlgItem(IDC_EDIT_NOTES))->GetWindowText(str);

		m_sdlNotes.notes = str;
		memcpy(m_params.notes, m_sdlNotes.GetSdlNotes(), PARAM_NOTES_SIZE);
		m_params.notes[PARAM_NOTES_SIZE - 1] = 0;

		((CEdit*)GetDlgItem(IDC_EDIT_NOTES))->SetWindowText(m_sdlNotes.notes);

		if (m_acquisitionDelayMins>(28UL*24*60)) m_acquisitionDelayMins=28UL*24*60;
		m_params.delayStartSec=m_acquisitionDelayMins*60;


		//Correct interruption cycle times to be within limits
		ValidateInterruption();

		m_params.interruptionCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;
		m_params.interruptionOffTime=m_interruptionTrackingOffTimeMs;
		m_params.interruptionOffToOnOffset=m_interruptionTrackingOffToOnOffsetMs;
		m_params.interruptionOnToOffOffset=m_interruptionTrackingOnToOffOffsetMs;
		m_params.interruptionUTCOffset=m_interruptionTrackingUTCOffsetMs; 
		m_params.interruptionOnFirst=m_interruptionTrackingOnFirst?1:0;



		//Compute CRC
		count=sizeof(T_PARAM_PARAMETERS)-2;
		C_FCS::Add((UINT8*)&m_params, &count);

		//Push changes back to the dialog
		UpdateData(FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL Cudl1configDlg::EEPROMToDialog()
{
	UINT32 hours, mins, secs, totalSecs;
	CString temp;

	totalSecs=m_params.samplingCyclePeriod/1000;
	hours=totalSecs/3600;
	mins=(totalSecs-hours*3600)/60;
	secs=(totalSecs-hours*3600-mins*60);

	temp.Format("%u",hours);
	m_sampleIntervalHoursCtrl.SetWindowText(temp);
	temp.Format("%u",mins);
	m_sampleIntervalMinsCtrl.SetWindowText(temp);
	temp.Format("%u",secs);
	m_sampleIntervalSecsCtrl.SetWindowText(temp);

	temp.Format("%u",m_params.samplingCycleOffset+100);
	m_sampleDelayMsCtrl.SetWindowTextA(temp);

	m_inputRange=m_params.rangeSelect;
	m_dcSamplingEnabled=m_params.configFlags&CONFIGFLAGS_DCENABLED?TRUE:FALSE;
	m_acSamplingEnabled=m_params.configFlags&CONFIGFLAGS_ACENABLED?TRUE:FALSE;
	m_tempSamplingEnabled=m_params.configFlags&CONFIGFLAGS_TEMPERATURE_ENABLED?TRUE:FALSE;
	m_temperatureUnits=m_params.configFlags&CONFIGFLAGS_TEMPERATURE_FAHRENHEIT?1:0;
	m_dcUnits=m_params.configFlags&CONFIGFLAGS_DC_AMPS?1:0;
	m_acUnits=m_params.configFlags&CONFIGFLAGS_AC_AMPS?1:0;
	m_requireGPSToStart=m_params.configFlags&CONFIGFLAGS_REQUIREGPSLOCK_ENABLED?1:0;
	m_dcWaveCaptureEnable=m_params.configFlags&CONFIGFLAGS_FASTSAMPLING_ENABLED?1:0;
	m_dcWaveCaptureContinuous=m_params.configFlags&CONFIGFLAGS_FASTSAMPLINGCONTINUOUS_ENABLED?1:0;
	m_noLongTermGPSSync=m_params.configFlags&CONFIGFLAGS_NOLONGTERMGPSSYNC_ENABLED?1:0;

	m_dcWaveCaptureDuration=m_params.samplingFastDuration; /* Number of seconds for a fast sampling mode */
	m_dcWaveCaptureRepetitionInterval=m_params.samplingFastInterval/60; /* Number of seconds between fast sampling groups */
	m_dcWaveCaptureSampleRate=m_params.samplingCyclePeriodFast;

	m_dcScaleFactor=m_params.scaleFactor[PARAM_CHANNEL_DC];
	m_acScaleFactor=m_params.scaleFactor[PARAM_CHANNEL_AC];
	m_dcOffset=m_params.offsetFactor[PARAM_CHANNEL_DC];
	m_acOffset=m_params.offsetFactor[PARAM_CHANNEL_AC];

	m_timeDisplayEnabled=m_params.configFlagsExtended&CONFIGFLAGSEXT_SHOWTIME?TRUE:FALSE;
	m_timeDisplayFormat=m_params.configFlagsExtended&CONFIGFLAGSEXT_LOCALTIME?TRUE:FALSE;
	m_fastSyncEnable=m_params.configFlagsExtended&CONFIGFLAGSEXT_LOCKFASTSAMPLING_100MS?TRUE:FALSE;
	m_minEHPEEnable=m_params.configFlagsExtended&CONFIGFLAGSEXT_EHPEWAIT?TRUE:FALSE;;

	m_fastSyncOffsetMs=m_params.lockedFastSamplingOffsetMs;
	if (m_fastSyncOffsetMs>99) m_fastSyncOffsetMs=99;

	m_minEHPEMeters=(float)m_params.minimumEHPE/10.0;
	if (m_minEHPEMeters<3.0) m_minEHPEMeters=2.5;
	if (m_minEHPEMeters>200.0) m_minEHPEMeters=200.0;

	m_minEHPETimeoutMinutes=m_params.minimumEHPEWaitTimeSec/60;
	if (m_minEHPETimeoutMinutes<1) m_minEHPETimeoutMinutes=1;
	if (m_minEHPETimeoutMinutes>60) m_minEHPETimeoutMinutes=60;

	m_acquisitionDelayMins=m_params.delayStartSec/60;
	if (m_acquisitionDelayMins>(28UL*24*60)) m_acquisitionDelayMins=28UL*24*60;

	m_interruptionTrackingCycleTime=m_params.interruptionCycleTime;
	m_interruptionTrackingOffTimeMs=m_params.interruptionOffTime;
	m_interruptionTrackingOnTime=m_interruptionTrackingCycleTime-m_interruptionTrackingOffTimeMs;
	m_interruptionTrackingOffToOnOffsetMs=m_params.interruptionOffToOnOffset;
	m_interruptionTrackingOnToOffOffsetMs=m_params.interruptionOnToOffOffset;
	m_interruptionTrackingUTCOffsetMs=m_params.interruptionUTCOffset; 

	ValidateInterruption();

	m_interruptionTrackingOnFirst=m_params.interruptionOnFirst?TRUE:FALSE;
	m_interruptionTrackingEnable=m_params.configFlags&CONFIGFLAGS_INTERRUPTIONTRACKING_ENABLED?TRUE:FALSE;

	m_sdlNotes = SdlNotes(CString(m_params.notes));

	((CEdit*)GetDlgItem(IDC_EDIT_NOTES))->SetWindowText(m_sdlNotes.notes);

	if (m_advancedUnit)
	{
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVEGPSSYNC))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_CHECK_NOEXTENDEDGPSSYNC))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVECAPEN))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVECAPCONT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->SetReadOnly(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->SetReadOnly(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_CHECK_MINEHPEENABLE))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_EHPEMIN))->SetReadOnly(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_EHPEMINTIMEOUT))->SetReadOnly(FALSE);

		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED1))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED2))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED3))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED4))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED5))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED6))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED7))->EnableWindow(TRUE);

		((CWnd*)GetDlgItem(IDC_EDIT_DCWAVESAMPLERATE))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_CHECK_SYNCOFFSETENABLE))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_EHPEMIN))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_EHPEMINTIMEOUT))->EnableWindow(TRUE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVEGPSSYNC))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_NOEXTENDEDGPSSYNC))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVECAPEN))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_DCWAVECAPCONT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->SetReadOnly(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->SetReadOnly(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_CHECK_MINEHPEENABLE))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_EHPEMIN))->SetReadOnly(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_EHPEMINTIMEOUT))->SetReadOnly(TRUE);

		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED1))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED2))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED3))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED4))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED5))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED6))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_ADVANCED7))->EnableWindow(FALSE);

		((CWnd*)GetDlgItem(IDC_EDIT_DCWAVESAMPLERATE))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_CHECK_SYNCOFFSETENABLE))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_EHPEMIN))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_EHPEMINTIMEOUT))->EnableWindow(FALSE);
	}

	if (m_advancedUnit&&m_g2Unit)
	{
		((CWnd*)GetDlgItem(IDC_CHECK_INTTRACKINGENABLE))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_CHECK_INTTRACKINGONFIRST))->EnableWindow(TRUE);

		((CEdit*)GetDlgItem(IDC_EDIT_INTOFFTIME))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTOFFTIME))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTONTIME))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTONTIME))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTONTOOFFOFFSET))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTONTOOFFOFFSET))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTOFFTOONOFFSET))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTOFFTOONOFFSET))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTUTCOFFSET))->SetReadOnly(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTUTCOFFSET))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTCYCLETIME))->EnableWindow(TRUE);

		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK1))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK2))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK3))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK4))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK5))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK6))->EnableWindow(TRUE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK7))->EnableWindow(TRUE);

	}
	else
	{
		((CWnd*)GetDlgItem(IDC_CHECK_INTTRACKINGENABLE))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_CHECK_INTTRACKINGONFIRST))->EnableWindow(FALSE);

		((CEdit*)GetDlgItem(IDC_EDIT_INTOFFTIME))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTOFFTIME))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTONTIME))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTONTIME))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTONTOOFFOFFSET))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTONTOOFFOFFSET))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTOFFTOONOFFSET))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTOFFTOONOFFSET))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_INTUTCOFFSET))->SetReadOnly(TRUE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTUTCOFFSET))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_EDIT_INTCYCLETIME))->EnableWindow(FALSE);

		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK1))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK2))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK3))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK4))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK5))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK6))->EnableWindow(FALSE);
		((CWnd*)GetDlgItem(IDC_STATIC_INTRACK7))->EnableWindow(FALSE);
	}


	UpdateData(FALSE);
	return TRUE;
}

void Cudl1configDlg::OnBnClickedButtonWritermu()
{
	UINT32 address;
	BOOL failure=FALSE;
	DWORD tries;
	DWORD toWrite, numLeft;
	char strTemp[256];

	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Serial port not selected!","Error",MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	if (!DialogToEEPROM())
	{
		return;
	}

	DisplayLinkStatus("Busy",255,0,0);

	//Grab the existing app checksum and version
	tries=0;
retryGetAppVersion:
	m_bip1.SendGeneric(MBP_GETVERSION);
	if (!m_bip1.ResponseWait(1000))
	{
		if (tries++<7) goto retryGetAppVersion;
		MessageBox("Failed to ping uDL1.","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));
	sprintf(strTemp,"%lu",m_bip1.m_serialNumber);
	m_serialNumberCtrl.SetWindowTextA(strTemp);
	sprintf(strTemp,"%lu.%02lu G%u",m_bip1.m_appVersion/100,m_bip1.m_appVersion%100,m_bip1.m_boardType+1);
	m_codeVersionCtrl.SetWindowTextA(strTemp);

	if (m_bip1.m_paramVersion!=PARAM_VERSION)
	{
		MessageBox("Target uDL1 parameter set version is not compatible with the version of uDL1Config.","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	if ((m_bip1.m_appVersion<FIRMWARE_CURRENT_VER)&&(m_lastFWWarningSerial!=m_bip1.m_serialNumber)) //Warn only once per unit attachment
	{
		m_lastFWWarningSerial=m_bip1.m_serialNumber;
		MessageBox("The connected uDL1 is running an obsolete firmware version.  Some features may be unavailable until the firmware is updated in the unit.  To update the firmware, exit the uDL1 configuration application and run the uDL1 Firmware Update application.  Refer to the uDL1 user manual for further information.","Warning",MB_OK);
	}

	address=0;
	numLeft=sizeof(T_PARAM_PARAMETERS);
	if (numLeft&0x3) numLeft=(numLeft&0xFFFFFFFC)+4; //Make sure we write complete words to flash
	while (numLeft)
	{
		toWrite=1024;
		if (toWrite>numLeft) toWrite=numLeft;
		tries=0;
retryEEWrite:
		m_bip1.SendWriteEEPROM(address,((UINT8*)&m_params)+address,toWrite);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEEWrite;
			MessageBox("Failed to write configuration data to uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		address+=toWrite;
		numLeft-=toWrite;
	}
	memcpy(&m_params_original,&m_params,sizeof(T_PARAM_PARAMETERS));

#if 1
retryReboot:
	m_bip1.SendGeneric(MBP_REBOOT);
	if (!m_bip1.ResponseWait(1000))
	{
		if (tries++<5) goto retryReboot;
		MessageBox("Failed to reboot uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
#endif
	DisplayLinkStatus("Idle",0,255,0,0,0,0);
	EEPROMToDialog();
}




void Cudl1configDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString strTemp;
	int i;
	UINT8 rssiRaw;
	float rssi;
	int rssiPercentInt;
	int rssidB;

	if (nIDEvent==100)
	{
		KillTimer(100);

		UINT32 hours, mins, secs, totalSecs, delay;
		CString temp;

		m_sampleIntervalHoursCtrl.GetWindowText(temp);
		hours=atoi(temp);
		m_sampleIntervalMinsCtrl.GetWindowText(temp);
		mins=atoi(temp);
		m_sampleIntervalSecsCtrl.GetWindowText(temp);
		secs=atoi(temp);

		if (((CButton*)GetDlgItem(IDC_CHECK_DCENABLED))->GetCheck()&&!hours&&!mins&&(secs==1))
		{
			((CWnd*)GetDlgItem(IDC_STATIC_1SDCDELAY))->EnableWindow(TRUE);
			((CWnd*)GetDlgItem(IDC_EDIT_SAMPLECYCLEDELAYMS))->EnableWindow(TRUE);
		}
		else
		{
			((CWnd*)GetDlgItem(IDC_STATIC_1SDCDELAY))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_EDIT_SAMPLECYCLEDELAYMS))->EnableWindow(FALSE);
		}

		if (m_advancedUnit)
		{
			if (((CButton*)GetDlgItem(IDC_CHECK_DCWAVECAPCONT))->GetCheck())
			{
				((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->EnableWindow(FALSE);
				((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->EnableWindow(FALSE);
			}
			else
			{
				((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->EnableWindow(TRUE);
				((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->EnableWindow(TRUE);
			}
			((CWnd*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->EnableWindow(TRUE);
			((CWnd*)GetDlgItem(IDC_CHECK_SYNCOFFSETENABLE))->EnableWindow(TRUE);
			((CWnd*)GetDlgItem(IDC_CHECK_MINEHPEENABLE))->EnableWindow(TRUE);
		}
		else
		{
			((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEREPINT))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_EDIT_DCWAVEDUR))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_EDIT_FASTOFFSETMS))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CHECK_SYNCOFFSETENABLE))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CHECK_MINEHPEENABLE))->EnableWindow(FALSE);
		}

		SetTimer(100,1000,NULL);
	}
	CDialog::OnTimer(nIDEvent);
}

void Cudl1configDlg::OnBnClickedButtonDefaults()
{
	memcpy(&m_params,&param_data_defaults,sizeof(T_PARAM_PARAMETERS));
	EEPROMToDialog();
}

void Cudl1configDlg::CheckForChanges()
{
	DialogToEEPROM();
	if (memcmp(&m_params, &m_params_original,sizeof(T_PARAM_PARAMETERS)))
	{
		DisplayConfigStatus("Configuration changed", 255, 0, 0);
	}
	else
	{
		DisplayConfigStatus("No changes", 0, 255, 0);
	}
}


void Cudl1configDlg::OnBnClickedButtonCommstatus()
{
	DWORD startTime;
	BOOL found;
	int i;
	CString strTemp, strTemp2, satInfo;
	struct tm dateTime, dateTime2;
	char dateTimeStr[30], dateTimeStr2[30];

	UINT32 address;
	BOOL failure=FALSE;
	DWORD tries;
	DWORD toRead, numLeft;

	T_PARAM_MFGDATA mfgData;

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	m_bip1.SendGeneric(MBP_GETGPSINFO);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	m_bip1.SendGeneric(MBP_GETBATTERYINFO);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	m_bip1.SendGeneric(MBP_GETFAULTS);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	m_bip1.SendGeneric(MBP_GETRTC);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}


	address=PARAM_MFG_OFFSET;
	numLeft=sizeof(T_PARAM_MFGDATA);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead2:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead2;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		memcpy(((UINT8*)&mfgData)+address-PARAM_MFG_OFFSET,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	strTemp="";
	if (mfgData.optionFlags&OPTIONFLAGS_GPS)
	{

		satInfo="GPS Status:\r\n";

		strTemp2.Format("\tPosition: Lat %.5f deg, Long %.5f deg , Alt %.2f m\r\n",(float)m_bip1.m_gpsInfo.latitude/1E7,(float)m_bip1.m_gpsInfo.longitude/1E7,(float)m_bip1.m_gpsInfo.altitude/1E2);
		satInfo+=strTemp2;

		strTemp2.Format("\tPosition Valid: %s\r\n",(m_bip1.m_gpsInfo.solValid&&m_bip1.m_gpsInfo.hdopValid)?"Yes":"No");
		satInfo+=strTemp2;

		strTemp2.Format("\tTime: %04u/%02u/%02u %02u:%02u:%02u (UTC)\r\n",
			m_bip1.m_gpsInfo.utcYear,
			m_bip1.m_gpsInfo.utcMonth,
			m_bip1.m_gpsInfo.utcDay,
			m_bip1.m_gpsInfo.utcHours,
			m_bip1.m_gpsInfo.utcMinutes,
			m_bip1.m_gpsInfo.utcSeconds
			);
		satInfo+=strTemp2;


		strTemp2.Format("\tTime Valid: %s\r\n",m_bip1.m_gpsInfo.timeMarkValid?"Yes":"No");
		satInfo+=strTemp2;


		strTemp2.Format("\tSats in Fix: %u\r\n",m_bip1.m_gpsInfo.satsInFix);
		satInfo+=strTemp2;

		strTemp2.Format("\tSats Fully Acquired: %u\r\n",m_bip1.m_gpsInfo.satsTracked);
		satInfo+=strTemp2;

		if (m_bip1.m_appVersion>=111) //v1.11 or higher included EHPE estimate
		{
			strTemp2.Format("\tHDOP: %.1fm\r\n",m_bip1.m_gpsInfo.hdop/10.0);
			satInfo+=strTemp2;
			strTemp2.Format("\tEHPE: %.1fm\r\n",m_bip1.m_gpsInfo.ehpe/10.0);
			satInfo+=strTemp2;
		}

		satInfo+="\tSVID: ";
		for (i=0; i<12; i++) if (m_bip1.m_gpsInfo.snr[i]) {strTemp2.Format("%02u ",m_bip1.m_gpsInfo.svid[i]); satInfo+=strTemp2;};
		satInfo+="\r\n";
		satInfo+="\tC/No: ";
		for (i=0; i<12; i++) if (m_bip1.m_gpsInfo.snr[i]) {strTemp2.Format("%02u ",m_bip1.m_gpsInfo.snr[i]); satInfo+=strTemp2;};
		satInfo+="\r\n\r\n";
		strTemp+=satInfo;
	}

	strTemp2.Format(
		"Battery Status:\r\n"
		"\tCharge State:\t%s\r\n"
		"\tPotential:\t\t%.2f V\r\n"
		"\tTemperature:\t%.2f C  %.2f F\r\n\r\n",
		m_bip1.m_batteryCharging?"Charging":"Charge Done",
		m_bip1.m_batteryVolts,
		m_bip1.m_batteryTemperature,
		m_bip1.m_batteryTemperature*9/5+32);
	strTemp+=strTemp2;

	strTemp2="Fault Status:\r\n";
	if (!m_bip1.m_faults)
		strTemp2+="\tNo faults\r\n";
	else
		for (i=0; i<32; i++)
		{
			if (m_bip1.m_faults&(1<<i))
			{
				strTemp2+="\t";
				strTemp2+=bip_faultStrings[i];
				strTemp2+="\r\n";
			}
		}
		strTemp2+="\r\n";
		strTemp+=strTemp2;

		memcpy(&dateTime,_localtime32((__time32_t *)(&m_bip1.m_rtcTime)),sizeof(struct tm));
		sprintf(dateTimeStr,"%04u/%02u/%02u %02u:%02u:%02u",dateTime.tm_year+1900,dateTime.tm_mon+1,dateTime.tm_mday,dateTime.tm_hour,dateTime.tm_min,dateTime.tm_sec);
		memcpy(&dateTime2,_gmtime32((__time32_t *)(&m_bip1.m_rtcTime)),sizeof(struct tm));
		sprintf(dateTimeStr2,"%04u/%02u/%02u %02u:%02u:%02u",dateTime2.tm_year+1900,dateTime2.tm_mon+1,dateTime2.tm_mday,dateTime2.tm_hour,dateTime2.tm_min,dateTime2.tm_sec);
		//strcpy(dateTimeStr,asctime(&dateTime));
		//dateTimeStr[25]=0; //Get rid of \n
		strTemp2.Format("Real Time Clock:\r\n\t%s (Local)\r\n\t%s (UTC)\r\n",dateTimeStr,dateTimeStr2);
		strTemp+=strTemp2;

		if (mfgData.calDate)
		{
			memcpy(&dateTime,_gmtime32((__time32_t *)(&mfgData.calDate)),sizeof(struct tm));
			sprintf(dateTimeStr,"%04u/%02u/%02u %02u:%02u:%02u",dateTime.tm_year+1900,dateTime.tm_mon+1,dateTime.tm_mday,dateTime.tm_hour,dateTime.tm_min,dateTime.tm_sec);
			strTemp2.Format("\r\nCalibration:\r\n\tDate:\t\t%s UTC\r\n\tTemperature:\t%.2f C  %0.2f F\r\n",dateTimeStr,((float)mfgData.calTemperature)/100,((float)mfgData.calTemperature)/100*9/5+32);
			strTemp+=strTemp2;
		}


		MessageBox(strTemp,"Status",MB_OK);

		DisplayLinkStatus("Idle",0,255,0,0,0,0);
}

void Cudl1configDlg::OnBnClickedButtonReadfaults()
{
	BOOL found;
	DWORD startTime;
	CString faultsStr;
	int i,j, oldestIndex, index, tries;
	DWORD oldestTime;
	BOOL noHistory;

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	m_bip1.SendGeneric(MBP_GETFAULTS);
	if (!m_bip1.ResponseWait(2000))
	{
		MessageBox("Error requesting faults from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	faultsStr="";
	for (i=0; i<31; i++)
	{
		if (m_bip1.m_faults&(1L<<i)) {faultsStr+="\r\n"; faultsStr+="\t"; faultsStr+=bip_faultStrings[i];};
	}
	if (faultsStr.GetLength()==0) faultsStr="\tNo faults";

	faultsStr="Current:\r\n"+faultsStr;

	noHistory=TRUE;
	if (m_bip1.m_appVersion>=105)
	{
		faultsStr+="\r\n\r\nHistory:\r\n";
		oldestTime=m_bip1.m_faultHistory.time[0];
		oldestIndex=0;
		for (j=0; j<FAULT_HISTORY_SIZE; j++)
		{
			if(m_bip1.m_faultHistory.time[j]&&(m_bip1.m_faultHistory.time[j]<oldestTime))
			{
				oldestTime=m_bip1.m_faultHistory.time[j];
				oldestIndex=j;
			}
		}
		for (j=0; j<FAULT_HISTORY_SIZE; j++)
		{
			index=(j+oldestIndex)%FAULT_HISTORY_SIZE;
			if ((m_bip1.m_faultHistory.time[index]>0)&&m_bip1.m_faultHistory.faultCode[index])
			{
				for (i=0; i<31; i++)
				{
					__time64_t t;
					struct tm  when;
					char       buff[80];

					t=m_bip1.m_faultHistory.time[index];
					_localtime64_s( &when, &t );
					asctime_s( buff, sizeof(buff), &when );
					buff[24]=0; //Trim LF

					if (m_bip1.m_faultHistory.faultCode[index]&(1L<<i)) 
					{
						noHistory=FALSE;
						faultsStr+="\r\n"; faultsStr+="\t"; faultsStr+=buff; faultsStr+="  "; faultsStr+=bip_faultStrings[i];
					};
				}
			}
		}
		if (noHistory) faultsStr+="\tNo faults";
	}

	//MessageBox(faultsStr,"Faults",MB_OK);
	CFaultDlg dlg;
	dlg.m_faultsString=faultsStr;
	if ((dlg.DoModal()==IDCANCEL)&&(m_bip1.m_appVersion>=105))
	{
		DisplayLinkStatus("Busy",255,0,0);
		memset(&m_bip1.m_faultHistory,0,sizeof(T_FAULT_HISTORY));
		tries=0;
retryFaultClear:
		m_bip1.SendGeneric(MBP_CLEARFAULTHISTORY);
		if (!m_bip1.ResponseWait(1000))
		{
			if (tries++<5) goto retryFaultClear;
			MessageBox("Failed to clear fault history on uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
		}
		DisplayLinkStatus("Idle",0,255,0);
	}
}

void Cudl1configDlg::OnHelpConfigurationguide()
{
	int retVal;
	char appPathStr[MAX_PATH+1];
	char *p, *lastSlashPtr;

	//Find the path that we were executed from
	appPathStr[MAX_PATH]=0;
	GetModuleFileName( NULL, appPathStr, MAX_PATH );
	//Trim off the application name portion
	lastSlashPtr=p=appPathStr;
	while(*p)
	{
		if ((*p=='\\')||(*p=='/')) lastSlashPtr=p;
		p++;
	}
	*lastSlashPtr=0;
	strcat(appPathStr,"\\");
	strncat(appPathStr,"udl1manual.pdf",MAX_PATH-strlen(appPathStr));

	//system("start manual.pdf");
	retVal=(int) ShellExecute(NULL,"open",appPathStr,NULL,NULL,SW_SHOWDEFAULT);
	if (retVal<=32)
	{
		if (retVal==ERROR_FILE_NOT_FOUND)
		{
			MessageBox("uDL1 manual not found!  uDL1MANUAL.PDF must be in the same directory as uDL1CONFIG.EXE","Error",MB_OK);
		}
		else
			if (retVal==SE_ERR_NOASSOC)
			{
				MessageBox("PDF view not installed! Install Adobe Acrobat Reader.","Error",MB_OK);
			}
			else
			{
				MessageBox("Error opening uDL1 manual!","Error",MB_OK);
			}
	}
}
void Cudl1configDlg::OnBnClickedButtonReadfile()
{
	FILE *inFilePtr;
	UINT32 magicKey, versionKey, serialNumber;
	BOOL failure;
	T_PARAM_PARAMETERS localeeprom;
	size_t numRead;

	CFileDialog fileDialog(TRUE,".cfg","*.cfg",0,"uDL1 Config Files (*.cfg)|*.cfg|All Files (*.*)|*.*||",NULL,0,TRUE);

	if( fileDialog.DoModal()==IDOK )
	{
		SetWindowText(m_dialogTitle+" "+fileDialog.GetPathName());
		inFilePtr=fopen(fileDialog.GetPathName(),"rb");
		if (inFilePtr)
		{
			failure=FALSE;
			if (!fread(&magicKey,sizeof(magicKey),1,inFilePtr)) failure=TRUE;
			if (magicKey!=MAGIC_KEY_FOR_FILES)
			{
				MessageBox("Invalid file!","Error",MB_OK|MB_ICONEXCLAMATION);
				fclose(inFilePtr);
				return;			
			}
			//Since this is version 0 of the file format, we won't do anything right now with the following info.
			if (!fread(&versionKey,sizeof(versionKey),1,inFilePtr)) failure=TRUE;
			if (!fread(&serialNumber,sizeof(serialNumber),1,inFilePtr)) failure=TRUE;
			numRead=fread(&localeeprom,1,sizeof(localeeprom),inFilePtr);
			if (numRead!=sizeof(localeeprom))  failure=TRUE;
			fclose(inFilePtr);

			if (failure)
			{
				MessageBox("Error reading file!","Error",MB_OK|MB_ICONEXCLAMATION);
				return;
			}
			else
			{
				memcpy(&m_params,&localeeprom,sizeof(localeeprom));
				EEPROMToDialog();
			}
		}
		else
		{
			MessageBox("Error reading file!","Error",MB_OK|MB_ICONEXCLAMATION);
		}

	}
	UpdateData(FALSE);
}

void Cudl1configDlg::OnBnClickedButtonWritefile()
{
	DWORD magicKey, versionKey;
	FILE *outFilePtr;
	BOOL failure;
	char fileNameDefault[100];

	if (DialogToEEPROM())
	{

		sprintf(fileNameDefault,"udl1_sn%06u.cfg",m_bip1.m_serialNumber);
		CFileDialog fileDialog(FALSE,".cfg",fileNameDefault,OFN_OVERWRITEPROMPT, "uDL1 Config Files (*.cfg)|*.cfg|All Files (*.*)|*.*||",NULL,0,TRUE);

		if( fileDialog.DoModal()==IDOK )
		{
			SetWindowText(m_dialogTitle+" "+fileDialog.GetPathName());

			//Create the save structures
			magicKey=MAGIC_KEY_FOR_FILES;
			versionKey=VERSION_KEY_FOR_FILES;

			//Save Data
			outFilePtr=fopen(fileDialog.GetPathName(),"wb");
			if (outFilePtr)
			{
				failure=FALSE;

				if (!fwrite(&magicKey,sizeof(magicKey),1,outFilePtr)) failure=TRUE;
				if (!fwrite(&versionKey,sizeof(versionKey),1,outFilePtr)) failure=TRUE;
				if (!fwrite(&m_bip1.m_serialNumber,sizeof(m_bip1.m_serialNumber),1,outFilePtr)) failure=TRUE;
				if (!fwrite(&m_params,sizeof(m_params),1,outFilePtr)) failure=TRUE;

				if (failure) MessageBox("Error saving data to file!","Error",MB_OK|MB_ICONEXCLAMATION);
				fclose(outFilePtr);
			}
			else
			{
				MessageBox("Error saving file!","Error",MB_OK|MB_ICONEXCLAMATION);
			}
			UpdateData(FALSE);
		}
	}
}


BOOL Cudl1configDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && 
		pMsg->message <= WM_KEYLAST) 
	{ 
		HACCEL hAccel = m_hAccel; 
		if (hAccel && 
			::TranslateAccelerator(m_hWnd, hAccel, pMsg)) 
			return TRUE; 
	} 
	return CDialog::PreTranslateMessage(pMsg); 
}

void Cudl1configDlg::OnBnClickedButtonReboot()
{
	BOOL found;
	DWORD startTime;

	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}

	if (!found)
	{
		if (MessageBox("Error pinging uDL1.  Would you like to attempt a hard reset?","Error",MB_ICONERROR|MB_YESNO))
		{
			//m_ser1.Cbus(0,TRUE);
			Sleep(250);
			//m_ser1.Cbus(0,FALSE);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			MessageBox("Hard reset activated on uDL1","Info",MB_OK);
		}
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	m_bip1.SendGeneric(MBP_REBOOT);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error requesting reboot from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	MessageBox("Reboot request submitted to uDL1","Info",MB_OK);

}

HBRUSH Cudl1configDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


void Cudl1configDlg::OnBnClickedButtonSetrtc()
{
	BOOL found;
	DWORD startTime;

	struct tm dateTime, dateTime2;
	char dateTimeStr[30], dateTimeStr2[30];
	CString strout, tzLabel;
	int tzOffset;
	__time32_t timeNow;


#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	m_bip1.SendSetRTC(timeNow=time(NULL));
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error setting clock on uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	GetTZInfo(tzOffset, tzLabel);
	memcpy(&dateTime,_localtime32((__time32_t *)(&timeNow)),sizeof(struct tm));
	sprintf(dateTimeStr,"%04u/%02u/%02u %02u:%02u:%02u",dateTime.tm_year+1900,dateTime.tm_mon+1,dateTime.tm_mday,dateTime.tm_hour,dateTime.tm_min,dateTime.tm_sec);
	memcpy(&dateTime2,_gmtime32((__time32_t *)(&timeNow)),sizeof(struct tm));
	sprintf(dateTimeStr2,"%04u/%02u/%02u %02u:%02u:%02u",dateTime2.tm_year+1900,dateTime2.tm_mon+1,dateTime2.tm_mday,dateTime2.tm_hour,dateTime2.tm_min,dateTime2.tm_sec);
	strout.Format("uDL1 Clock Set\r\n\r\nLocal:\t%s (%s)\r\nUTC:\t%s\r\n",dateTimeStr,tzLabel,dateTimeStr2);

	MessageBox(strout,"Info",MB_OK);
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
	FILETIME ft;

	UnixTimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, pst);
}

double loc2_ConvertLatLong(UINT8 *inPtr)
{
	INT32 temps32;
	temps32=0;
	memcpy(&temps32,inPtr,3);
	if (inPtr[2]&0x80) temps32|=0xFF000000; //Sign extend
	return (double)temps32/32768.0;
}


void Cudl1configDlg::OnBnClickedButtonExtractSdldata()
{
	CString path = "";
	CString tzStr = "";
	int tzOffsetSeconds, tzUTCCompSeconds;
	if (GetDataLog(tzOffsetSeconds, tzUTCCompSeconds, tzStr)) return;
	tzOffsetSeconds = 0;
	tzUTCCompSeconds = 0;

#if 0
	CFileDialog fileDialogIn(TRUE, ".bin", "*.bin", 0, "uDL1 Binary Memory Files (*.bin)|*.bin|All Files (*.*)|*.*||", NULL, 0, TRUE);
	FILE *inFilePtr;
	if (fileDialogIn.DoModal() == IDOK)
	{
		SetWindowText(m_dialogTitle + " " + fileDialogIn.GetPathName());
		inFilePtr = fopen(fileDialogIn.GetPathName(), "rb");
		if (inFilePtr)
		{
			datalogMemoryCount = fread(datalogMemory, 1, DATALOG_MAXSIZE, inFilePtr);
			fclose(inFilePtr);
		}
	}

#endif

	if(!GenerateMasterFile(path, tzOffsetSeconds, tzUTCCompSeconds, tzStr))
	{
		GenerateSDLFile(path, tzOffsetSeconds, tzUTCCompSeconds, tzStr);
	}
}


void Cudl1configDlg::OnBnClickedButtonExtractdata()
{
	CString tzStr, path = "";
	int tzOffsetSeconds, tzUTCCompSeconds;
	if (GetDataLog(tzOffsetSeconds, tzUTCCompSeconds, tzStr)) return;

#if 0
	CFileDialog fileDialogIn(TRUE, ".bin", "*.bin", 0, "uDL1 Binary Memory Files (*.bin)|*.bin|All Files (*.*)|*.*||", NULL, 0, TRUE);
	FILE *inFilePtr;
	if (fileDialogIn.DoModal() == IDOK)
	{
		SetWindowText(m_dialogTitle + " " + fileDialogIn.GetPathName());
		inFilePtr = fopen(fileDialogIn.GetPathName(), "rb");
		if (inFilePtr)
		{
			datalogMemoryCount = fread(datalogMemory, 1, DATALOG_MAXSIZE, inFilePtr);
			fclose(inFilePtr);
		}
	}

#endif
	
	if(!GenerateMasterFile(path, tzOffsetSeconds, tzUTCCompSeconds, tzStr)) 
	{
		((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->SetWindowText(CString("File: ") + path + CString(".csv"));
		MessageBox("uDL1 Data Memory Extracted", "Info", MB_OK);
	}
}

BOOL Cudl1configDlg::GetDataLog(int &tzOffsetSeconds, int &tzUTCCompSeconds, CString &tzStr)
{
	BOOL found;
	DWORD startTime;
	DWORD address, toTransfer, toCopy, tries, triesPing;
	UINT8 percentComplete;

	UpdateData(TRUE);
	GetTZInfo(tzOffsetSeconds, tzStr);
	printf("Tz: %s, offset: %i\r\n", tzStr, tzOffsetSeconds);
	((CButton*)GetDlgItem(IDC_RADIO_LOCAL))->SetWindowText("Local (" + tzStr + ")");
	tzUTCCompSeconds = 0;
	if (!m_dateFormat)
	{
		tzUTCCompSeconds = tzOffsetSeconds;
		tzOffsetSeconds = 0;
		tzStr = "UTC";
	}

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return TRUE;

	DisplayLinkStatus("Busy", 255, 0, 0);

	//Ping the uDL1 first
	found = FALSE;
	startTime = GetTickCount();
	while ((GetTickCount() - startTime) < 6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found = TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.", "Error", MB_ICONERROR | MB_OK);
		DisplayLinkStatus("Idle", 0, 255, 0, 0, 0, 0);
		return  TRUE;
	}
	if (m_bip1.m_storageUsedBytes > m_bip1.m_storageTotalBytes) m_bip1.m_storageUsedBytes = m_bip1.m_storageTotalBytes; //v1.1.1.4: fix for uDL misreporting bytes used when full.
	SetMemUseBar((m_bip1.m_storageUsedBytes + 1)*100.0 / (m_bip1.m_storageTotalBytes + 1));

	address = 0;
	toTransfer = m_bip1.m_storageUsedBytes;

	if (toTransfer > DATALOG_MAXSIZE)
	{
		MessageBox("Datalogger point count exceeds capability of this version of configuration application.  Please update to the latest version.", "Error", MB_OK);
		DisplayLinkStatus("Idle", 0, 255, 0, 0, 0, 0);
		return TRUE;
	}

	percentComplete = 0;
	CBusyDialog dlg;
	dlg.Create(IDD_DIALOG_BUSYSTATUS, this);
	dlg.ShowWindow(SW_SHOW);
	dlg.m_captionTextCtrl.SetWindowTextA("Busy Transferring Data.  Please Wait.");
	dlg.m_progressBarCtrl.SetPos(percentComplete);
	dlg.RedrawWindow(NULL, NULL, RDW_UPDATENOW);

	startTime = GetTickCount();
	datalogMemoryCount = toTransfer;
	while (toTransfer)
	{
		percentComplete = address * 100 / (datalogMemoryCount + 1);
		dlg.m_progressBarCtrl.SetPos(percentComplete);
		dlg.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
		//ProcessMessages();
		toCopy = 2000;
		if (toCopy > toTransfer) toCopy = toTransfer;
		tries = 0;
	retryReadDF:
		m_bip1.SendReadDataFlash(address, toCopy);
		if (!m_bip1.ResponseWait(2000))
		{
			if (tries++ < 3)
			{	//On a retry of a DF read, first ping the device to resynchronize the transfer
				triesPing = 0;
			retryReadDFPing:
				m_bip1.SendGeneric(MBP_PING);
				if (!m_bip1.ResponseWait(1000))
				{
					if (triesPing++ < 3) goto retryReadDFPing;
					dlg.ShowWindow(SW_HIDE);
					MessageBox("Error reading storage memory on uDL1.", "Error", MB_ICONERROR | MB_OK);
					DisplayLinkStatus("Idle", 0, 255, 0, 0, 0, 0);
					return TRUE;
				}
				goto retryReadDF;
			}
			dlg.ShowWindow(SW_HIDE);
			MessageBox("Error reading storage memory on uDL1.", "Error", MB_ICONERROR | MB_OK);
			DisplayLinkStatus("Idle", 0, 255, 0, 0, 0, 0);
			return TRUE;
		}
		memcpy(datalogMemory + address, m_bip1.m_dataBuffer, toCopy);
		address += toCopy;
		toTransfer -= toCopy;
	}
	dlg.ShowWindow(SW_HIDE);

	printf("Transferred: %lu bytes, time: %.3f s, throughput: %.3f KB/s\r\n", datalogMemoryCount, ((GetTickCount() - startTime) / 1000.0), (float)datalogMemoryCount / 1024 / ((GetTickCount() - startTime) / 1000.0));

	DisplayLinkStatus("Idle", 0, 255, 0, 0, 0, 0);

	return FALSE;
}

BOOL Cudl1configDlg::GenerateSDLFile(CString path, int tzOffsetSeconds, int tzUTCCompSeconds, CString tzStr)
{
#define UNIXTIME_JAN_1_2000_OFFSET 946684800
	FILE *outFilePtr = NULL;
	FILE *debugFilePtr = NULL;
	char fileNameDefault[100];
	T_DATALOG_TIME_RECORD *timePtr;
	T_DATALOG_GPS_RECORD *gpsPtr;
	T_DATALOG_GPSACCURACY_RECORD *gpsAccuracyPtr;
	T_DATALOG_READING_RECORD *readingPtr;
	T_DATALOG_NOTES_RECORD *notesPtr;
	T_SDL_DATALOG_GPSPOINT datalogGps;
	UINT32 millisecondOffset;
	UINT64 unix2000Time;
	UINT64 unix2000MsTime;
	double tempC, tempF;
	char notes[PARAM_NOTES_SIZE];
	int notesCount;
	DWORD version;
	UINT8 range;
	double jun1_2011;
	BOOL inWaveSegment;
	double ehpe, ehpeLimit, hdop;
	BOOL ehpeLimitEnabled, ehpeLimitTimeout;
	UINT8 buffer00[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	UINT32 baseTime;
	DWORD address, toCopy;
	UINT32 sdlFileCount = 0;
	CString fileName, filePath, directoryPath, debugFileName, debugFilePath;
	SdlNotes sdlNotes = SdlNotes();
	Datalog datalog = Datalog();
	BOOL rtcGood = FALSE;
	INT tempInt32;
	UINT32 utcOffset;
	BOOL intOn;

	baseTime = 0;
	notesCount = 0;
	range = 0;
	version = 0;
	ehpe = 0.0;
	ehpeLimit = 0.0;
	hdop = 0.0;
	ehpeLimitEnabled = FALSE;
	ehpeLimitTimeout = FALSE;
	inWaveSegment = FALSE;
	utcOffset = 0;


	jun1_2011 = Chart::chartTime(2011, 06, 01, 0, 0, 0); //For detecting bad RTC timestamps

	//sprintf(fileNameDefault, "udl1_sn%06u.sdl", m_bip1.m_serialNumber);
	//CFileDialog fileDialog(FALSE, ".sdl", fileNameDefault, OFN_OVERWRITEPROMPT, "SDL Data Files (*.sdl)|*.sdl|All Files (*.*)|*.*||", NULL, 0, TRUE);
	
	directoryPath = path;
	while(!CreateDirectory(directoryPath, NULL))
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			directoryPath.Format("%s-%d",path, ++sdlFileCount);
		}
		else {
			MessageBox("Error creating output directory!", "Error", MB_OK | MB_ICONERROR);
			return TRUE; // unable 
		}
	}

	sdlFileCount = 0;
	datalogReadingsTzStr = tzStr;

	for (address = 0; address < datalogMemoryCount; address += 8)
	{
		timePtr = (T_DATALOG_TIME_RECORD *)(datalogMemory + address);
		gpsPtr = (T_DATALOG_GPS_RECORD *)(datalogMemory + address);
		gpsAccuracyPtr = (T_DATALOG_GPSACCURACY_RECORD *)(datalogMemory + address);
		readingPtr = (T_DATALOG_READING_RECORD *)(datalogMemory + address);
		notesPtr = (T_DATALOG_NOTES_RECORD *)(datalogMemory + address);

		// Don't log anything while the RTC is bad
		if (!rtcGood && (datalogMemory[address] & 0x1F != E_DATALOG_TIMESTAMP)) continue;

		switch (datalogMemory[address] & 0x1F)
		{
		case E_DATALOG_TIMESTAMP:
			// update our time
			if (!memcmp(datalogMemory + address, buffer00, 8)) break; //Skip over records that are completely zeroed out
			rtcGood = FALSE;
			if (timePtr->seconds > UNIXTIME_JAN_1_2000_OFFSET) {
				baseTime = timePtr->seconds - UNIXTIME_JAN_1_2000_OFFSET;
				rtcGood = TRUE;
			}
			millisecondOffset = 0;
			unix2000Time = baseTime;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			//unixMsTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
			range = timePtr->optionsVerHigh & 0x03;
			if (range == 2) { // range 2 is 150V on uDL1, equals range 3 on SDL
				range = 3;
			}
			version = timePtr->optionsVerHigh;
			version = ((version << 4) & 0xF00) | timePtr->verLow;
			switch (timePtr->reason)
			{
			case DATALOG_TIME_REASON_OFFSET:
				break;

			case DATALOG_TIME_REASON_INIT:
				break;

			case DATALOG_TIME_REASON_CABLEINSERTED:
				// Create a new file
				fileName.Format("MBTX%04u.sdl", sdlFileCount);
				filePath = directoryPath + "\\" + fileName;
				outFilePtr = fopen(filePath, "wb");
				if (outFilePtr == NULL)
				{
					MessageBox("Error opening output file!", "Error", MB_OK | MB_ICONERROR);
					return TRUE;
				}

#ifdef _DEBUG
				debugFileName.Format("MBTX%04u.txt", sdlFileCount);
				debugFilePath = directoryPath + "\\" + debugFileName;
				debugFilePtr = fopen(debugFilePath, "w");
				if (debugFilePtr == NULL)
				{
					MessageBox("Error opening output debug file!", "Error", MB_OK | MB_ICONERROR);
					return TRUE;
				}

				datalog.Open(outFilePtr, debugFilePtr);
#else 
				datalog.Open(outFilePtr);
#endif
				sdlFileCount++;
				break;

			case DATALOG_TIME_REASON_CABLEREMOVED:
				// Close file
				if (outFilePtr != NULL) {
					datalog.WriteHeader(); // write dummy header to start, this correct header at end

					if (ftell(outFilePtr) == sizeof(T_SDL_DATALOG_HEADER)) // empty file, delete
					{
						sdlFileCount--;
						fclose(outFilePtr);
						DeleteFile(filePath);
#ifdef _DEBUG
						fclose(debugFilePtr);
						DeleteFile(debugFilePath);
#endif
					}
					else {
						fclose(outFilePtr);
#ifdef _DEBUG
						fclose(debugFilePtr);
#endif
					}
					outFilePtr = NULL;
				}				
				break;
			}
			break;

		case E_DATALOG_GPS:
			millisecondOffset = 0;
			unix2000Time = baseTime;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			//UnixTimeToSystemTime(unixTime, &st);
			unix2000MsTime = ((UINT64)unix2000Time * 1000 + (millisecondOffset % 1000));
			datalog.WriteGPSData(unix2000MsTime, loc2_ConvertLatLong(gpsPtr->latitude), loc2_ConvertLatLong(gpsPtr->longitude), 0, gpsPtr->satCount, gpsPtr->satCount, 1);

			//Clear out the accuracy information in case someone flashes older firmware before next reading cycle.
			ehpe = 0.0;
			ehpeLimit = 0.0;
			hdop = 0.0;
			ehpeLimitEnabled = FALSE;
			ehpeLimitTimeout = FALSE;
			break;


		case E_DATALOG_GPSACCURACY:
			//A GPS accuracy record always comes before a position record, so let the position record deal with outputing the data
			ehpe = gpsAccuracyPtr->ehpe / 10.0;
			hdop = gpsAccuracyPtr->hdop / 10.0;
			ehpeLimit = gpsAccuracyPtr->ehpeLimit / 10.0;
			ehpeLimitEnabled = (gpsAccuracyPtr->recordType&DATALOG_GPSACCURACY_FLAG_EHPE_LIM_ENABLED) ? TRUE : FALSE;
			ehpeLimitTimeout = (gpsAccuracyPtr->recordType&DATALOG_GPSACCURACY_FLAG_EHPE_LIM_TIMEOUT) ? TRUE : FALSE;
			break;

		case E_DATALOG_READINGDC:
			break; // non interrupt readings ignored

		case E_DATALOG_INTREADINGDC:
			millisecondOffset = 0;
			memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
			unix2000Time = baseTime + millisecondOffset / 1000;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			unix2000MsTime = ((UINT64)unix2000Time * 1000 + (millisecondOffset % 1000));

			// wait for first off transition and skip current readings
			intOn = readingPtr->recordType&DATALOG_READING_FLAG_ON ? TRUE : FALSE;
			//if (readingPtr->recordType&DATALOG_READING_FLAG_AMPS) { break; } // allow Amp readings

			datalog.AddDcValue(unix2000MsTime, readingPtr->value, intOn);

			if (inWaveSegment)
			{
				inWaveSegment = FALSE;
			}
			break;


		case E_DATALOG_READINGDCFAST:
			if (!inWaveSegment)
			{
				inWaveSegment = TRUE;
			}
			//if (readingPtr->recordType&DATALOG_READING_FLAG_AMPS) { break; } // allow Amp readings
			millisecondOffset = 0;
			memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
			unix2000Time = baseTime + millisecondOffset / 1000;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			unix2000MsTime = ((UINT64)unix2000Time * 1000 + (millisecondOffset % 1000));
			datalog.AddWaveformValue(unix2000MsTime, readingPtr->value);
			break;

		case E_DATALOG_READINGAC:
			break; // non interrupt readings ignored

		case E_DATALOG_INTREADINGAC:
			millisecondOffset = 0;
			memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
			unix2000Time = baseTime + millisecondOffset / 1000;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			unix2000MsTime = (double)((UINT64)unix2000Time * 1000 + (millisecondOffset % 1000));

			// wait for first off transition and skip current readings
			intOn = readingPtr->recordType&DATALOG_READING_FLAG_ON?TRUE:FALSE;
			//if (readingPtr->recordType&DATALOG_READING_FLAG_AMPS) { break; } // allow Amp readings
			datalog.AddAcValue(unix2000MsTime, readingPtr->value, intOn);
			break;

		case E_DATALOG_TEMPERATURE:
			break; // ignored

		case E_DATALOG_BATTERYVOLTS:
			break; // ignored

		case E_DATALOG_NOTES:
			millisecondOffset = 0;
			unix2000Time = baseTime + millisecondOffset / 1000;
			if (tzOffsetSeconds > 0) unix2000Time -= (DWORD)(abs(tzOffsetSeconds)); else unix2000Time += (DWORD)(abs(tzOffsetSeconds));
			//UnixTimeToSystemTime(unix2000Time, &st);
			unix2000MsTime = ((UINT64)unix2000Time * 1000 + (millisecondOffset % 1000));
						
			if (notesPtr->recordType&DATALOG_NOTES_FLAG_START) notesCount = 0;

			toCopy = PARAM_NOTES_SIZE - notesCount;
			if (toCopy > (sizeof(T_DATALOG_NOTES_RECORD) - 1)) toCopy = sizeof(T_DATALOG_NOTES_RECORD) - 1;
			memcpy(notes + notesCount, notesPtr->notes, toCopy);
			notesCount += toCopy;

			if (notesPtr->recordType&DATALOG_NOTES_FLAG_END)
			{
				CString notesEsc;
				notes[PARAM_NOTES_SIZE - 1] = 0; //Zero terminate guarantee
				notesEsc = notes;
				notesEsc.Replace("\"", "\"\"");

				sdlNotes = SdlNotes(notesEsc);
				datalog.UpdateHeader(version, sdlNotes.stnSeries, sdlNotes.chainage, range, sdlNotes.notes, m_bip1.m_serialNumber);
			}
			break;

		default:
			break;
		}
	}

	// Close file
	if (outFilePtr != NULL) {
		datalog.WriteHeader(); // write dummy header to start, correct header at end

		if (ftell(outFilePtr) == sizeof(T_SDL_DATALOG_HEADER)) // empty file, delete
		{
			sdlFileCount--;
			fclose(outFilePtr);
			DeleteFile(filePath);
#ifdef _DEBUG
			fclose(debugFilePtr);
			DeleteFile(debugFilePath);
#endif
		}
		else {
			fclose(outFilePtr);
#ifdef _DEBUG
			fclose(debugFilePtr);
#endif
		}
		outFilePtr = NULL;
	}
	return FALSE;
}

BOOL Cudl1configDlg::GenerateMasterFile(CString &path, int tzOffsetSeconds, int tzUTCCompSeconds, CString tzStr)
{
	FILE *outFilePtr;
	char fileNameDefault[100];
	T_DATALOG_TIME_RECORD *timePtr;
	T_DATALOG_GPS_RECORD *gpsPtr;
	T_DATALOG_GPSACCURACY_RECORD *gpsAccuracyPtr;
	T_DATALOG_READING_RECORD *readingPtr;
	T_DATALOG_NOTES_RECORD *notesPtr;
	UINT32 millisecondOffset;
	UINT32 unixTime;
	SYSTEMTIME st;
	double excelTime;
	double tempC, tempF;
#define EXCELJAN_1_2000_OFFSET 36526.0
#define EXCELJAN_1_1970_OFFSET 25569.0
	char notes[PARAM_NOTES_SIZE];
	int notesCount;
	DWORD version;
	UINT8 range;
	double jun1_2011;
	BOOL inWaveSegment;
	double ehpe, ehpeLimit, hdop;
	BOOL ehpeLimitEnabled, ehpeLimitTimeout;
	UINT8 buffer00[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	UINT32 baseTime;
	DWORD address, toCopy;

	baseTime = 0;
	notesCount = 0;
	range = 0;
	version = 0;
	ehpe = 0.0;
	ehpeLimit = 0.0;
	hdop = 0.0;
	ehpeLimitEnabled = FALSE;
	ehpeLimitTimeout = FALSE;
	inWaveSegment = FALSE;

	jun1_2011 = Chart::chartTime(2011, 06, 01, 0, 0, 0); //For detecting bad RTC timestamps

	sprintf(fileNameDefault, "udl1_sn%06u.csv", m_bip1.m_serialNumber);
	CFileDialog fileDialog(FALSE, ".csv", fileNameDefault, OFN_OVERWRITEPROMPT, "uDL1 CSV Data Files (*.csv)|*.csv|All Files (*.*)|*.*||", NULL, 0, TRUE);

	if (fileDialog.DoModal() == IDOK)
	{
		datalogReadingsTzStr = tzStr;

		//Clear out chart memory
		datalogReadingsDCCount = 0;
		datalogReadingsACCount = 0;
		datalogReadingsTempCount = 0;
		datalogReadingsWaveSegmentCount = 0;

		//Save Data
		outFilePtr = fopen(fileDialog.GetPathName(), "wb");
		path = fileDialog.GetFolderPath() + "\\" + fileDialog.GetFileTitle();
		if (outFilePtr)
		{
			fprintf(outFilePtr, "Record Type,\"Date/Time(%s)\",Milliseconds,\"Excel Time(%s)\",DC Reading,DC Units,AC Reading,AC Units,Temperature Reading,Temperature Units,Battery Potential(V),GPS Latitude,GPS Longitude,GPS Sats,Notes,Time Lock,GPS EHPE(m),Limit EHPE(m),Int State On\r\n", tzStr, tzStr);
			for (address = 0; address < datalogMemoryCount; address += 8)
			{
				timePtr = (T_DATALOG_TIME_RECORD *)(datalogMemory + address);
				gpsPtr = (T_DATALOG_GPS_RECORD *)(datalogMemory + address);
				gpsAccuracyPtr = (T_DATALOG_GPSACCURACY_RECORD *)(datalogMemory + address);
				readingPtr = (T_DATALOG_READING_RECORD *)(datalogMemory + address);
				notesPtr = (T_DATALOG_NOTES_RECORD *)(datalogMemory + address);

				switch (datalogMemory[address] & 0x1F)
				{
				case E_DATALOG_TIMESTAMP:
					if (!memcmp(datalogMemory + address, buffer00, 8)) break; //Skip over records that are completely zeroed out
					baseTime = timePtr->seconds;
					millisecondOffset = 0;
					unixTime = baseTime;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					range = timePtr->optionsVerHigh & 0x03;
					version = timePtr->optionsVerHigh;
					version = ((version << 4) & 0xF00) | timePtr->verLow;
					switch (timePtr->reason)
					{
					case DATALOG_TIME_REASON_OFFSET:
						break;

					case DATALOG_TIME_REASON_INIT:
						break;

					case DATALOG_TIME_REASON_CABLEINSERTED:
						fprintf(outFilePtr, "Cable Inserted,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime
						);
						break;

					case DATALOG_TIME_REASON_CABLEREMOVED:
						fprintf(outFilePtr, "Cable Removed,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime
						);
						break;

					case DATALOG_TIME_REASON_GPS:
						break;

					case DATALOG_TIME_REASON_LOWBAT:
						fprintf(outFilePtr, "Low Battery,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime
						);
						break;

					case DATALOG_TIME_REASON_NOTES:
						break;

					case DATALOG_TIME_REASON_MEMFULL:
						fprintf(outFilePtr, "Memory Full,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime
						);
						break;

					case DATALOG_TIME_REASON_WAITSTART:
						fprintf(outFilePtr, "Delayed Start,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime
						);
						break;
					}
					break;

				case E_DATALOG_GPS:
					millisecondOffset = 0;
					unixTime = baseTime;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					fprintf(outFilePtr, "Location,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,%.5f,%.5f,%u,,,%.1f,%.1f,\r\n",
						st.wMonth,
						st.wDay,
						st.wYear,
						st.wHour,
						st.wMinute,
						st.wSecond,
						millisecondOffset % 1000,
						excelTime,
						loc2_ConvertLatLong(gpsPtr->latitude),
						loc2_ConvertLatLong(gpsPtr->longitude),
						gpsPtr->satCount,
						ehpe,
						ehpeLimitEnabled ? ehpeLimit : 0.0
					);
					//Clear out the accuracy information in case someone flashes older firmware before next reading cycle.
					ehpe = 0.0;
					ehpeLimit = 0.0;
					hdop = 0.0;
					ehpeLimitEnabled = FALSE;
					ehpeLimitTimeout = FALSE;
					break;


				case E_DATALOG_GPSACCURACY:
					//A GPS accuracy record always comes before a position record, so let the position record deal with outputing the data
					ehpe = gpsAccuracyPtr->ehpe / 10.0;
					hdop = gpsAccuracyPtr->hdop / 10.0;
					ehpeLimit = gpsAccuracyPtr->ehpeLimit / 10.0;
					ehpeLimitEnabled = (gpsAccuracyPtr->recordType&DATALOG_GPSACCURACY_FLAG_EHPE_LIM_ENABLED) ? TRUE : FALSE;
					ehpeLimitTimeout = (gpsAccuracyPtr->recordType&DATALOG_GPSACCURACY_FLAG_EHPE_LIM_TIMEOUT) ? TRUE : FALSE;
					break;

				case E_DATALOG_READINGDC:
				case E_DATALOG_INTREADINGDC:
					millisecondOffset = 0;
					memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					//Check overrange
					if ((readingPtr->value > 1E10) || (readingPtr->value < -1E10))
						fprintf(outFilePtr, "DC Reading%s,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,\"%s\",\"%s\",,,,,,,,,,%u,,,%u\r\n",
						((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGDC) ? " (Int)" : "",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value < 0 ? "OVER-" : "OVER+",
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0,
							readingPtr->recordType&DATALOG_READING_FLAG_ON ? 1 : 0);
					else
					{
						fprintf(outFilePtr, "DC Reading%s,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,%.6f,\"%s\",,,,,,,,,,%u,,,%u\r\n",
							((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGDC) ? " (Int)" : "",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value,
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0,
							readingPtr->recordType&DATALOG_READING_FLAG_ON ? 1 : 0);

						//Filter out dates with bad RTC values
						if (Chart::chartTime2(unixTime) > jun1_2011)
						{
							datalogReadingsDC[datalogReadingsDCCount] = readingPtr->value;
							datalogReadingsDCType[datalogReadingsDCCount] = ((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGDC) ? ((readingPtr->recordType&DATALOG_READING_FLAG_ON) ? 3 : 2) : 0; //Normal
							datalogReadingsDCTime[datalogReadingsDCCount++] = Chart::chartTime2(unixTime) + (millisecondOffset % 1000) / 1000.0 + tzOffsetSeconds + tzUTCCompSeconds;
							//printf("Excel time: %.8f, chart time: %.8f, diff: %0.f\r\n",excelTime,(Chart::chartTime2(unixTime)+(millisecondOffset%1000)/1000.0+tzOffsetSeconds+tzUTCCompSeconds)/86400,(Chart::chartTime2(unixTime)+(millisecondOffset%1000)/1000.0+tzOffsetSeconds+tzUTCCompSeconds)/86400-excelTime);
						}
					}
					if (inWaveSegment)
					{
						inWaveSegment = FALSE;
					}
					break;


				case E_DATALOG_READINGDCFAST:
					if (!inWaveSegment)
					{
						inWaveSegment = TRUE;
						if (datalogReadingsWaveSegmentCount < MAX_WAVE_SEGMENTS) datalogReadingsWaveSegmentCount++;
						datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount - 1].startIndex = datalogReadingsDCCount;
						datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount - 1].endIndex = datalogReadingsDCCount;
					}
					millisecondOffset = 0;
					memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					//Check overrange
					if ((readingPtr->value > 1E10) || (readingPtr->value < -1E10))
						fprintf(outFilePtr, "DC Reading (Wave Capture),%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,\"%s\",\"%s\",,,,,,,,,,%u,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value < 0 ? "OVER-" : "OVER+",
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0);
					else
					{
						fprintf(outFilePtr, "DC Reading (Wave Capture),%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,%.6f,\"%s\",,,,,,,,,,%u,,,\r\n",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value,
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0);

						//Filter out dates with bad RTC values
						if (Chart::chartTime2(unixTime) > jun1_2011)
						{
							datalogReadingsDC[datalogReadingsDCCount] = readingPtr->value;
							datalogReadingsDCType[datalogReadingsDCCount] = 1; //Wave
							datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount - 1].endIndex = datalogReadingsDCCount;
							datalogReadingsDCTime[datalogReadingsDCCount++] = Chart::chartTime2(unixTime) + (millisecondOffset % 1000) / 1000.0 + tzOffsetSeconds + tzUTCCompSeconds;
						}
					}
					break;

				case E_DATALOG_READINGAC:
				case E_DATALOG_INTREADINGAC:
					millisecondOffset = 0;
					memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					//Check overrange
					if ((readingPtr->value > 1E10) || (readingPtr->value < -1E10))
						fprintf(outFilePtr, "AC Reading%s,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,\"%s\",\"%s\",,,,,,,,%u,,,%u\r\n",
						((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGAC) ? " (Int)" : "",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value < 0 ? "OVER-" : "OVER+",
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0,
							readingPtr->recordType&DATALOG_READING_FLAG_ON ? 1 : 0);
					else
					{
						fprintf(outFilePtr, "AC Reading%s,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,%.6f,\"%s\",,,,,,,,%u,,,%u\r\n",
							((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGAC) ? " (Int)" : "",
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							readingPtr->value,
							readingPtr->recordType&DATALOG_READING_FLAG_AMPS ? "A" : "V",
							readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0,
							readingPtr->recordType&DATALOG_READING_FLAG_ON ? 1 : 0);
						//Filter out dates with bad RTC values
						if (Chart::chartTime2(unixTime) > jun1_2011)
						{
							datalogReadingsAC[datalogReadingsACCount] = readingPtr->value;
							datalogReadingsACType[datalogReadingsACCount] = ((datalogMemory[address] & 0x1F) == E_DATALOG_INTREADINGAC) ? ((readingPtr->recordType&DATALOG_READING_FLAG_ON) ? 3 : 2) : 0; //Normal or interruption
							datalogReadingsACTime[datalogReadingsACCount++] = Chart::chartTime2(unixTime) + (millisecondOffset % 1000) / 1000.0 + tzOffsetSeconds + tzUTCCompSeconds;
						}
					}
					break;

				case E_DATALOG_TEMPERATURE:
					millisecondOffset = 0;
					memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					fprintf(outFilePtr, "Temperature,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,%.1f,\"%s\",,,,,,%u,,,\r\n",
						st.wMonth,
						st.wDay,
						st.wYear,
						st.wHour,
						st.wMinute,
						st.wSecond,
						millisecondOffset % 1000,
						excelTime,
						readingPtr->value,
						readingPtr->recordType&DATALOG_READING_FLAG_FAHRENHEIT ? "F" : "C",
						readingPtr->recordType&DATALOG_READING_FLAG_TIMELOCK ? 1 : 0);
					//Filter out dates with bad RTC values
					if (Chart::chartTime2(unixTime) > jun1_2011)
					{
						datalogReadingsTemp[datalogReadingsTempCount] = readingPtr->value;
						datalogReadingsTempTime[datalogReadingsTempCount++] = Chart::chartTime2(unixTime) + (millisecondOffset % 1000) / 1000.0 + tzOffsetSeconds + tzUTCCompSeconds;
					}
					break;

				case E_DATALOG_BATTERYVOLTS:
					millisecondOffset = 0;
					memcpy(&millisecondOffset, readingPtr->timeOffsetMs, 3);
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;
					fprintf(outFilePtr, "Battery,%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,%.2f,,,,,,,,\r\n",
						st.wMonth,
						st.wDay,
						st.wYear,
						st.wHour,
						st.wMinute,
						st.wSecond,
						millisecondOffset % 1000,
						excelTime,
						readingPtr->value);
					break;

				case E_DATALOG_NOTES:
					millisecondOffset = 0;
					unixTime = baseTime + millisecondOffset / 1000;
					if (tzOffsetSeconds > 0) unixTime -= (DWORD)(abs(tzOffsetSeconds)); else unixTime += (DWORD)(abs(tzOffsetSeconds));
					UnixTimeToSystemTime(unixTime, &st);
					excelTime = (double)((UINT64)unixTime * 1000 + (millisecondOffset % 1000));
					excelTime = (excelTime / 86400000.0) + EXCELJAN_1_1970_OFFSET;

					if (notesPtr->recordType&DATALOG_NOTES_FLAG_START) notesCount = 0;

					toCopy = PARAM_NOTES_SIZE - notesCount;
					if (toCopy > (sizeof(T_DATALOG_NOTES_RECORD) - 1)) toCopy = sizeof(T_DATALOG_NOTES_RECORD) - 1;
					memcpy(notes + notesCount, notesPtr->notes, toCopy);
					notesCount += toCopy;

					if (notesPtr->recordType&DATALOG_NOTES_FLAG_END)
					{
						CString notesEsc;
						notes[PARAM_NOTES_SIZE - 1] = 0; //Zero terminate guarantee
						notesEsc = notes;
						notesEsc.Replace("\"", "\"\"");
						fprintf(outFilePtr, "Notes(%lu:%u.%02u:%s:%s),%02u-%02u-%04u %02u:%02u:%02u,%lu,%#.8f,,,,,,,,,,,\"%s\",,,,\r\n",
							m_bip1.m_serialNumber,
							version / 100,
							version % 100,
							(range == 0) ? "Low" : (range == 1) ? "Med" : "High",
							m_appVerStr,
							st.wMonth,
							st.wDay,
							st.wYear,
							st.wHour,
							st.wMinute,
							st.wSecond,
							millisecondOffset % 1000,
							excelTime,
							notesEsc);
					}
					break;

				default:
					break;
				}
			}
			fclose(outFilePtr);
		}
		else
		{
			MessageBox("Error opening output file!", "Error", MB_OK | MB_ICONERROR);
			return TRUE;
		}
	}
	 else { // return error if dialog cancelled
		return TRUE;
	}
	return FALSE;
}

void Cudl1configDlg::OnBnClickedButtonCleardata()
{
	BOOL found;
	DWORD startTime, tries;

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	if (MessageBox("Are you sure you want to clear the data memory on the uDL1?","Query",MB_YESNO)==IDYES)
	{
		m_bip1.SendGeneric(MBP_CLEARSTORAGESTART);
		if (!m_bip1.ResponseWait(1000))
		{
			MessageBox("Error clearing data memory on uDL1.","Error",MB_ICONERROR|MB_OK);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}

		CBusyDialog dlg;
		dlg.Create(IDD_DIALOG_BUSYSTATUS,this);
		dlg.ShowWindow(SW_SHOW);
		dlg.m_progressBarCtrl.SetPos(0);
		dlg.m_captionTextCtrl.SetWindowTextA("Busy Clearing Data Memory.  Please Wait.");
		dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);

		do
		{
			tries=0;
retryClearProgress:
			m_bip1.SendGeneric(MBP_CLEARSTORAGESTATUS);
			if (!m_bip1.ResponseWait(1000))
			{
				if (tries++<3) goto retryClearProgress;
				dlg.ShowWindow(SW_HIDE);
				MessageBox("Error reading storage memory clear status on uDL1.","Error",MB_ICONERROR|MB_OK);
				DisplayLinkStatus("Idle",0,255,0,0,0,0);
				return;
			}
			dlg.m_progressBarCtrl.SetPos(m_bip1.m_storageClearPercent);
			dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);
			//ProcessMessages();
		}
		while (m_bip1.m_storageClearBusy);
		dlg.ShowWindow(SW_HIDE);

		SetMemUseBar(0);
		if (m_bip1.m_storageClearFailure)
			MessageBox("Error detected while clearing uDL1 data memory!","Info",MB_OK|MB_ICONERROR);
		else
			MessageBox("uDL1 data memory successfully cleared.","Info",MB_OK);
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);
}

void Cudl1configDlg::OnBnClickedButtonSetfactory()
{
	UINT32 address;
	BOOL failure=FALSE;
	DWORD tries;
	DWORD toRead, toWrite, numLeft, count;
	T_PARAM_MFGDATA paramMfgTemp;
	char strTemp[256];
	CString overrides;
	CFactoryOverridesDlg dlg;

#if 1
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	//if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Grab the existing app checksum and version
	tries=0;
retryGetAppVersion:
	m_bip1.SendGeneric(MBP_GETVERSION);
	if (!m_bip1.ResponseWait(1000))
	{
		if (tries++<7) goto retryGetAppVersion;
		MessageBox("Failed to ping uDL1.","Error",MB_OK|MB_ICONEXCLAMATION);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	sprintf(strTemp,"%lu",m_bip1.m_serialNumber);
	m_serialNumberCtrl.SetWindowTextA(strTemp);
	sprintf(strTemp,"%lu.%02lu G%u",m_bip1.m_appVersion/100,m_bip1.m_appVersion%100,m_bip1.m_boardType+1);
	m_codeVersionCtrl.SetWindowTextA(strTemp);

	if (m_bip1.m_boardType>0) m_g2Unit=TRUE; else m_g2Unit=FALSE;

	//Read in manufacturing option data
	address=PARAM_MFG_OFFSET;
	numLeft=sizeof(T_PARAM_MFGDATA);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead2:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead2;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		memcpy(((UINT8*)&paramMfgTemp)+address-PARAM_MFG_OFFSET,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	//Validate version
	if (paramMfgTemp.paramVersion!=PARAM_MFG_VERSION)
	{
		MessageBox("The uDL1 mfg parameter set is not compatible with this version of uDL1Config!","Error",MB_OK|MB_ICONEXCLAMATION);
		sprintf(strTemp,"uDL1 mfg parameter set version=%lu, uDL1Config parameter set version=%lu.\r\n",(UINT32)(paramMfgTemp.paramVersion),PARAM_MFG_VERSION);
		//DisplayGeneralStatus(strTemp);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	//Validate CRC
	if (!C_FCS::Verify((UINT8*)&paramMfgTemp, sizeof(T_PARAM_MFGDATA)))
	{
		MessageBox("CRC error in parameter data!","Error",MB_OK|MB_ICONEXCLAMATION);
		//DisplayLinkStatus("Idle",0,255,0,0,0,0);

		//return;
		memcpy(&m_paramsmfg,&param_mfgdata_defaults,sizeof(T_PARAM_MFGDATA));
	}
	else
	{
		memcpy(&m_paramsmfg,&paramMfgTemp,sizeof(T_PARAM_MFGDATA));
	}

	dlg.m_gpsEnabled=(m_paramsmfg.optionFlags&OPTIONFLAGS_GPS)?TRUE:FALSE;
	dlg.m_largeMemEnabled=(m_paramsmfg.optionFlags&OPTIONFLAGS_EXTMEM)?TRUE:FALSE;
	dlg.m_prototypeUnit=(m_paramsmfg.optionFlags&OPTIONFLAGS_PROTOTYPE)?TRUE:FALSE;

	if (dlg.DoModal()==IDOK)
	{
		m_paramsmfg.optionFlags&=~(OPTIONFLAGS_GPS);
		m_paramsmfg.optionFlags&=~(OPTIONFLAGS_EXTMEM);
		m_paramsmfg.optionFlags&=~(OPTIONFLAGS_PROTOTYPE);

		if (dlg.m_gpsEnabled) m_paramsmfg.optionFlags|=OPTIONFLAGS_GPS;
		if (dlg.m_largeMemEnabled) m_paramsmfg.optionFlags|=OPTIONFLAGS_EXTMEM;
		if (dlg.m_prototypeUnit) m_paramsmfg.optionFlags|=OPTIONFLAGS_PROTOTYPE;

		//Compute CRC
		count=sizeof(T_PARAM_MFGDATA)-2;
		C_FCS::Add((UINT8*)&m_paramsmfg, &count);

		address=0;
		numLeft=sizeof(T_PARAM_MFGDATA);
		while (numLeft)
		{
			toWrite=1024;
			if (toWrite>numLeft) toWrite=numLeft;
			tries=0;
retryEEWrite:
			m_bip1.SendWriteEEPROM(address+PARAM_MFG_OFFSET,((UINT8*)&m_paramsmfg)+address,toWrite);
			if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
			{
				if (tries++<5) goto retryEEWrite;
				MessageBox("Failed to write mfg data to RMU3!","Error",MB_OK|MB_ICONEXCLAMATION);
				DisplayLinkStatus("Idle",0,255,0,0,0,0);
				return;
			}
			address+=toWrite;
			numLeft-=toWrite;
		}
	}

	if (!m_paramsmfg.optionFlags)
	{
		overrides="None";
		m_advancedUnit=FALSE;
	}
	else
	{
		overrides="";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_GPS)
		{
			overrides+="GPS TIMESYNC\r\n";
			m_advancedUnit=TRUE;
		}
		else
		{
			m_advancedUnit=FALSE;
		}
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_EXTMEM) overrides+="LARGE MEM\r\n";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_NEWCAL) overrides+="CAL V2\r\n";
		if (m_paramsmfg.optionFlags&OPTIONFLAGS_PROTOTYPE) overrides+="PROTOTYPE\r\n";
	}
	m_factoryOverrides.SetWindowTextA(overrides);

	EEPROMToDialog();

	DisplayLinkStatus("Idle",0,255,0,0,0,0);
}

void Cudl1configDlg::OnBnClickedButtonAnalogtest()
{
	BOOL found;
	DWORD startTime;
	CString str;

	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	m_bip1.SendSetInputRange(MBP_RANGE_LOW);
	if (!m_bip1.ResponseWait(500))
	{
		MessageBox("Error setting input range on uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	m_bip1.SendReadScaledAnalog(MBP_ANALOG_AC,1);
	if (!m_bip1.ResponseWait(12000))
	{
		MessageBox("Error requesting scaled reading from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	str.Format("DC Reading: %.6f",m_bip1.m_scaledMeasurement);
	MessageBox(str,"Info",MB_OK);
}

#define SetMeterDisplay(str) {m_bip2.SendSetMeterDisplay(str); if (!m_bip2.ResponseWait(1000)) printf("Error setting meter display.\r\n");} 

BOOL Cudl1configDlg::GetMeterReference(C_BIPWU *m_bipPtr, double *refVal, BOOL ac, char *range)
{
#ifdef _DEBUG
	UINT8 tries;

	tries=0;
retryGetMeterVal:
	m_bipPtr->SendGetMeterReading(ac, range);
	if(!m_bipPtr->ResponseWait(ac?10000:1000))
	{
		printf("No response to get meter value request!\r\n");
		if (tries++<5) goto retryGetMeterVal;
		printf("Couldn't talk to calibration jig!\r\n");
		return FALSE;
	}
	if (refVal) *refVal=m_bipPtr->m_meterReading;
#endif
	return TRUE;
}

#define ENDCAL() {if (MessageBox("Failed test, exit?","Query",MB_YESNO|MB_ICONQUESTION)==IDYES) goto cleanup;}

void Cudl1configDlg::OnBnClickedButtonCalibrate()
{
#ifdef _DEBUG
	C_WINUSB m_ser2;
	BOOL m_testerConnected;
	C_BIPWU m_bip2;
	UINT8 tries, range;
	double reference;
	char strRange[256], strTemp[256];
	double scaleFactor;
	double acOffsetReference[3];
	INT32 acOffsetReading[3];
	double slope, intercept;
	INT32 offset;
	T_CHANNEL_CALIBRATION calTable[2];
	UINT16 acFloor[3];
	DWORD toRead, toWrite, numLeft, count;
	T_PARAM_MFGDATA paramMfgTemp;
	int i, j;
	UINT32 address;
	BOOL calPassed;

	calPassed=FALSE;

#if 1
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	//if (!ReaduDLConfig()) return;

	//Read in manufacturing option data
	address=PARAM_MFG_OFFSET;
	numLeft=sizeof(T_PARAM_MFGDATA);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead2:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead2;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		memcpy(((UINT8*)&paramMfgTemp)+address-PARAM_MFG_OFFSET,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	//Validate version
	if (paramMfgTemp.paramVersion!=PARAM_MFG_VERSION)
	{
		MessageBox("The uDL1 mfg parameter set is not compatible with this version of uDL1Config!","Error",MB_OK|MB_ICONEXCLAMATION);
		sprintf(strTemp,"uDL1 mfg parameter set version=%lu, uDL1Config parameter set version=%lu.\r\n",(UINT32)(paramMfgTemp.paramVersion),PARAM_MFG_VERSION);
		//DisplayGeneralStatus(strTemp);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	//Validate CRC
	if (!C_FCS::Verify((UINT8*)&paramMfgTemp, sizeof(T_PARAM_MFGDATA)))
	{
		//MessageBox("CRC error in parameter data!","Error",MB_OK|MB_ICONEXCLAMATION);
		//DisplayLinkStatus("Idle",0,255,0,0,0,0);

		//return;
		memcpy(&m_paramsmfg,&param_mfgdata_defaults,sizeof(T_PARAM_MFGDATA));
	}
	else
	{
		memcpy(&m_paramsmfg,&paramMfgTemp,sizeof(T_PARAM_MFGDATA));
	}

	//Connect to tester
	m_testerConnected=FALSE;
	if (m_ser2.Open(gRMU2SDL1GUID)==ERROR_SUCCESS)
	{
		if (m_bip2.Options(&m_ser2,"BIPWU")==ERROR_SUCCESS)
		{
			printf("Tester connected.\r\n");
			m_testerConnected=TRUE;
		}
	}
	else
	{
		if (m_ser2.Open(gRDU1GUID)==ERROR_SUCCESS)
		{
			if (m_bip2.Options(&m_ser2,"BIPWU")==ERROR_SUCCESS)
			{
				printf("Tester connected.\r\n");
				m_testerConnected=TRUE;
			}
		}
	}
	if (m_testerConnected)
	{
		printf("Enabling 3V3A and RMS converter.\r\n");
		m_bip1.SendSetPortOutput(MBP_PORT_GPIOA,11,1);
		if (!m_bip1.ResponseWait(5000))
		{
			printf("Error enabling 3V3A.\r\n");
			ENDCAL();
		}
		m_bip1.SendSetPortOutput(MBP_PORT_ADC,0,0);
		if (!m_bip1.ResponseWait(5000))
		{
			printf("Error enabling RMS converter.\r\n");
			ENDCAL();
		}
		Sleep(5000);

#if 1
		//Set the test jig to all channels disconnected, to allow us to perform zero offset measurements
#define DCZEROCAL_HIGH_LIMIT 100
#define DCZEROCAL_LOW_LIMIT -100
#define ACFLOORCAL_HIGH_LIMIT 1300
#define ACFLOORCAL_LOW_LIMIT 0
		printf("Performing DC zero offset and AC floor calibration\r\n");
		SetMeterDisplay("Test#1");
		tries=0;
retryDisconnect:
		m_bip2.SendSetAnalogOut(0, 128, 0, 0); 
		if(!m_bip2.ResponseWait(500))
		{
			printf("No response to set analog out request!\r\n");
			if (tries++<5) goto retryDisconnect;
			printf("Couldn't talk to calibration jig!\r\n");
			ENDCAL();
		}

		for (range=0; range<3; range++)
		{
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			Sleep(1000); //Set DC zero offsets

			m_bip1.SendReadRawAnalog(MBP_ANALOG_DC,8);
			if (!m_bip1.ResponseWait(5000))
			{
				printf("Error requesting raw reading from uDL1.\r\n");
				ENDCAL();
			}
			printf("Range %lu, DC Offset: %li\r\n",(DWORD)range,m_bip1.m_rawMeasurement);
			calTable[0].zeroCal[range]=m_bip1.m_rawMeasurement;
			if ((m_bip1.m_rawMeasurement>DCZEROCAL_HIGH_LIMIT)||(m_bip1.m_rawMeasurement<DCZEROCAL_LOW_LIMIT))
			{
				printf("Out of tolerance.\r\n");
				ENDCAL();
			}

			m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,8);
			if (!m_bip1.ResponseWait(5000))
			{
				printf("Error requesting raw reading from uDL1.\r\n");
				ENDCAL();
			}
			printf("Range %lu, AC Floor: %li\r\n",(DWORD)range,m_bip1.m_rawMeasurement);
			acFloor[range]=m_bip1.m_rawMeasurement;
			if ((m_bip1.m_rawMeasurement>ACFLOORCAL_HIGH_LIMIT)||(m_bip1.m_rawMeasurement<ACFLOORCAL_LOW_LIMIT))
			{
				printf("Out of tolerance.\r\n");
				ENDCAL();
			}
		}

		printf("Performing DC scale factor calibration.\r\n");
		SetMeterDisplay("Test#2");
#define DCSCALE_TOLERANCE 0.05
		for (range=0; range<3; range++)
		{
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			//Route cal signal to all inputs
			tries=0;
retryDCcalset:
			if (range==0)
			{
				m_bip2.SendSetAnalogOut(0xF, 128+14, 0, 0);
				strcpy(strRange,"1");
			}
			else
				if (range==1)
				{
					m_bip2.SendSetAnalogOut(0xF, 128+19, 0, 2);
					strcpy(strRange,"100");
				}
				else
				{
					m_bip2.SendSetAnalogOut(0xF, 128+55, 0, 2);
					strcpy(strRange,"100");
				}
				if(!m_bip2.ResponseWait(500))
				{
					printf("No response to set analog out request!\r\n");
					if (tries++<5) goto retryDCcalset;
					printf("Couldn't talk to calibration jig!\r\n");
					ENDCAL();
				}

				Sleep(1000); 

				if (!GetMeterReference(&m_bip2,&reference, 0, strRange))
				{
					printf("Couldn't read meter reference value!\r\n");
					ENDCAL();
				}

				m_bip1.SendReadRawAnalog(MBP_ANALOG_DC,8);
				if (!m_bip1.ResponseWait(5000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				offset=calTable[0].zeroCal[range];
				if (m_bip1.m_rawMeasurement==offset) m_bip1.m_rawMeasurement++; //Avoid /0
				scaleFactor=reference/(m_bip1.m_rawMeasurement-offset);
				calTable[0].scaleFactor[range]=scaleFactor;
				printf("Range %lu, Raw %li, Reference %.5f, ScaleFactor: %.5E\r\n",(DWORD)range,m_bip1.m_rawMeasurement,reference,scaleFactor);
				if ((scaleFactor>(param_mfgdata_defaults.calibration[0].scaleFactor[range]*(1+DCSCALE_TOLERANCE)))
					||(scaleFactor<(param_mfgdata_defaults.calibration[0].scaleFactor[range]*(1-DCSCALE_TOLERANCE))))
				{
					printf("Out of tolerance!\r\n");
					ENDCAL();
				}
		}
#endif	
#if 1
		printf("Performing AC low reference calibration.\r\n");
		SetMeterDisplay("Test#3");

		for (range=0; range<3; range++)
		{
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			tries=0;
retryACcalset:
			if (range==0)
			{
				m_bip2.SendSetAnalogOut(0xF, 128, 5, 0);
				strcpy(strRange,"1");
			}
			else
				if (range==1)
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 255, 0);
					strcpy(strRange,"1");
				}
				else
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 150, 1);
					strcpy(strRange,"10");
				}
				if(!m_bip2.ResponseWait(500))
				{
					printf("No response to set analog out request!\r\n");
					if (tries++<5) goto retryACcalset;
					printf("Couldn't talk to calibration jig!\r\n");
					ENDCAL();
				}

				Sleep(7000); //Let the signal stabilize

				if (!GetMeterReference(&m_bip2,&reference, 1, strRange))
				{
					printf("Couldn't read meter reference value!\r\n");
					ENDCAL();
				}

				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,4);
				if (!m_bip1.ResponseWait(5000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}

				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,16);
				if (!m_bip1.ResponseWait(8000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				acOffsetReference[range]=reference;
				acOffsetReading[range]=m_bip1.m_rawMeasurement;
				printf("AC Range %lu, Raw %li, Reference %.5f\r\n",(DWORD)range,m_bip1.m_rawMeasurement,reference);
		}

		printf("Performing AC scaleFactor calibration.\r\n");
		SetMeterDisplay("Test#4");
#define ACZEROCAL_HIGH_LIMIT 2000
#define ACZEROCAL_LOW_LIMIT -2000
#define ACSCALE_TOLERANCE 0.5
		for (range=0; range<3; range++)
		{
			//Reset all channels to proper range
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			tries=0;
retryACcalset2:
			if (range==0)
			{
				m_bip2.SendSetAnalogOut(0xF, 128, 60, 0);
				strcpy(strRange,"1");
			}
			else
				if (range==1)
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 80, 2);
					strcpy(strRange,"100");
				}
				else
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 240, 2);
					strcpy(strRange,"100");
				}
				if(!m_bip2.ResponseWait(500))
				{
					printf("No response to set analog out request!\r\n");
					if (tries++<5) goto retryACcalset2;
					printf("Couldn't talk to calibration jig!\r\n");
					ENDCAL();
				}

				Sleep(8000); //Let the signal stabilize

				if (!GetMeterReference(&m_bip2,&reference, 1, strRange))
				{
					printf("Couldn't read meter reference value!\r\n");
					ENDCAL();
				}
				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,4);
				if (!m_bip1.ResponseWait(5000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,16);
				if (!m_bip1.ResponseWait(8000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				if (m_bip1.m_rawMeasurement==acOffsetReading[range]) m_bip1.m_rawMeasurement++; //Avoid /0
				slope=(reference-acOffsetReference[range])/(m_bip1.m_rawMeasurement-acOffsetReading[range]);
				intercept=reference-slope*m_bip1.m_rawMeasurement;
				offset=-intercept/slope;
				calTable[1].scaleFactor[range]=slope;
				calTable[1].zeroCal[range]=offset;

				printf("AC Range %lu, Raw %li, Reference %.5f,\r\n Offset=%li, ScaleFactor=%.5E\r\n",(DWORD)range,m_bip1.m_rawMeasurement,reference,(INT32)offset,slope);
				if ((offset>ACZEROCAL_HIGH_LIMIT)||(offset<ACZEROCAL_LOW_LIMIT))
				{
					if (range) //Ignore low range AC cal restrictions for now.
					{
						printf("Out of tolerance zero!\r\n");
						ENDCAL();
					}
				}
#if 1
				if ((slope>(param_mfgdata_defaults.calibration[1].scaleFactor[range]*(1+ACSCALE_TOLERANCE)))
					||(slope<(param_mfgdata_defaults.calibration[1].scaleFactor[range]*(1-ACSCALE_TOLERANCE))))
				{
					if (range) //Ignore low range AC cal restrictions for now.
					{
						printf("Out of tolerance slope!\r\n");
						ENDCAL();
					}
				}
#endif
		}
#endif

#if 1
		//Update manufacturing info area
		for (i=0; i<2; i++)
			for (j=0; j<3; j++)
			{
				m_paramsmfg.calibration[i].zeroCal[j]=calTable[i].zeroCal[j];
				m_paramsmfg.calibration[i].scaleFactor[j]=calTable[i].scaleFactor[j];
			}
			for (i=0; i<3; i++)
				m_paramsmfg.acFloorCal[i]=acFloor[i];
#else
		m_paramsmfg.calibration[1].zeroCal[2]=calTable[1].zeroCal[2];
		m_paramsmfg.calibration[1].scaleFactor[2]=calTable[1].scaleFactor[2];
#endif
		//memcpy(&m_paramsmfg.calibration,&param_mfgdata_defaults.calibration,sizeof(param_mfgdata_defaults.calibration));

		//Write calibration back out
		//Compute CRC
		count=sizeof(T_PARAM_MFGDATA)-2;
		C_FCS::Add((UINT8*)&m_paramsmfg, &count);

		address=0;
		numLeft=sizeof(T_PARAM_MFGDATA);
		while (numLeft)
		{
			toWrite=1024;
			if (toWrite>numLeft) toWrite=numLeft;
			tries=0;
retryEEWrite:
			m_bip1.SendWriteEEPROM(address+PARAM_MFG_OFFSET,((UINT8*)&m_paramsmfg)+address,toWrite);
			if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
			{
				if (tries++<5) goto retryEEWrite;
				printf("Failed to write mfg data to RMU3!\r\n");
				ENDCAL();
			}
			address+=toWrite;
			numLeft-=toWrite;
		}
		calPassed=TRUE;


cleanup:
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		m_ser2.ReceiveEnable(FALSE);
		m_ser2.Close();
		m_bip2.Options(NULL,"");

		if (calPassed)
			MessageBox("Calibration completed successfully.","Info",MB_OK);
		else
			MessageBox("Calibration failed!","Error",MB_OK|MB_ICONHAND);
	}
#else
	MessageBox("Release version of app does not support calibration.","Error",MB_OK);
#endif
}


void Cudl1configDlg::OnBnClickedButtonSetserial()
{
	BOOL found;
	DWORD startTime;
	CString str;

	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	((CEdit*)GetDlgItem(IDC_EDIT_ESN))->GetWindowText(str);
	m_bip1.SendSetSerial(atoi(str));
	if (!m_bip1.ResponseWait(5000))
	{
		MessageBox("Error setting serial number on uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	DisplayLinkStatus("Idle",0,255,0,0,0,0);
	MessageBox("Serial number set.\r\n","Info",MB_OK);
}

void Cudl1configDlg::OnBnClickedButtonShuntcalcdc()
{
	CShuntCalcDlg dlg;
	if (UpdateData(TRUE))
	{
		dlg.m_shuntFullScaleMillivolts=50;
		dlg.m_shuntFullScaleAmps=dlg.m_shuntFullScaleMillivolts/1000*m_dcScaleFactor;

		if (dlg.DoModal()==IDOK)
		{
			m_inputRange=0;
			m_dcUnits=1;
			m_dcScaleFactor=dlg.m_shuntFullScaleAmps/dlg.m_shuntFullScaleMillivolts*1000;
			UpdateData(FALSE);
		}
	}
}

void Cudl1configDlg::OnBnClickedButtonShuntcalcac()
{
	CShuntCalcDlg dlg;
	if (UpdateData(TRUE))
	{
		dlg.m_shuntFullScaleMillivolts=50;
		dlg.m_shuntFullScaleAmps=dlg.m_shuntFullScaleMillivolts/1000*m_acScaleFactor;

		if (dlg.DoModal()==IDOK)
		{
			m_inputRange=0;
			m_acUnits=1;
			m_acScaleFactor=dlg.m_shuntFullScaleAmps/dlg.m_shuntFullScaleMillivolts*1000;
			UpdateData(FALSE);
		}
	}
}


void Cudl1configDlg::OnBnClickedButtonChartdc()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;

	if (!datalogReadingsDCCount)
	{
		MessageBox("No DC readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsDCCount; i++)
	{
		if (datalogReadingsDCTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}

	dlg.m_timeStampsA=DoubleArray(datalogReadingsDCTime,datalogReadingsDCCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsDC,datalogReadingsDCCount);
	dlg.m_dataTitleA="DC";
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="DC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}

void Cudl1configDlg::OnBnClickedButtonChartac()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;

	if (!datalogReadingsACCount)
	{
		MessageBox("No AC readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsACCount; i++)
	{
		if (datalogReadingsACTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}

	dlg.m_timeStampsA=DoubleArray(datalogReadingsACTime,datalogReadingsACCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsAC,datalogReadingsACCount);
	dlg.m_dataTitleA="AC";
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="AC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}

void Cudl1configDlg::OnBnClickedButtonCharttemp()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;

	if (!datalogReadingsTempCount)
	{
		MessageBox("No temperature readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsTempCount; i++)
	{
		if (datalogReadingsTempTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}

	dlg.m_timeStampsA=DoubleArray(datalogReadingsTempTime,datalogReadingsTempCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsTemp,datalogReadingsTempCount);
	dlg.m_dataTitleA="Temperature";
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="Temperature Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}

void Cudl1configDlg::OnDestroy()
{
	CWinApp* pApp = AfxGetApp();
	CString strSection;
	CString strIntItem;
	int nValue;

	UpdateData(TRUE);

	strSection       = "Options";
	strIntItem       = "ExtractionDateType";
	nValue = m_dateFormat;
	pApp->WriteProfileInt(strSection, strIntItem, nValue); //Create the value in the registry to make it easier to edit

	CDialog::OnDestroy();

}

BOOL Cudl1configDlg::GetTZInfo(int &offsetSeconds, CString &label)
{
	DWORD daylight;
	DWORD absOffsetMinutes;
	TIME_ZONE_INFORMATION tzInfo;

	daylight=GetTimeZoneInformation(&tzInfo);
	offsetSeconds=tzInfo.Bias*60; //Convert to minutes
	if (daylight==TIME_ZONE_ID_DAYLIGHT) offsetSeconds+=tzInfo.DaylightBias*60;
	if (daylight==TIME_ZONE_ID_STANDARD) offsetSeconds+=tzInfo.StandardBias*60;
	absOffsetMinutes=abs(offsetSeconds/60);

	if (daylight==TIME_ZONE_ID_DAYLIGHT)
	{
		if (strlen(CString(tzInfo.DaylightName)))
		{
			label.Format("%s UTC%c%u:%02u",CString(tzInfo.DaylightName),(offsetSeconds>0)?'-':'+',absOffsetMinutes/60,absOffsetMinutes%60);
		}
		else
		{
			if (offsetSeconds)
			{
				label.Format("UTC%c%u:%02u",(offsetSeconds>0)?'-':'+',absOffsetMinutes/60,absOffsetMinutes%60);
			}
			else
			{
				label="UTC";
			}
		}
	}
	else
	{
		if (strlen(CString(tzInfo.StandardName)))
		{
			label=tzInfo.StandardName;
		}
		else
		{
			if (offsetSeconds)
			{
				label.Format("%s UTC%c%u:%02u",CString(tzInfo.StandardName),(offsetSeconds>0)?'-':'+',absOffsetMinutes/60,absOffsetMinutes%60);
			}
			else
			{
				label="UTC";
			}
		}
	}
	return TRUE;
}
void Cudl1configDlg::OnBnClickedButtonReaddatafile()
{
	int i,dataType;
	int typeIndex, timeIndex, dcIndex, acIndex, temperatureIndex, intStateIndex;
	int rowCount, columnCount;
	const char *cellDataPtr, *dataValuePtr;
	CString msg;
	CkCsv csv;
	bool success;
	double excelOffset, jun1_2011, sampledate;
	char *cPtr, *dPtr;
	char buffer[256];
	UINT8 percentComplete;
	BOOL inWaveSegment;

	CFileDialog fileDialog(TRUE,".csv","*.csv",0,"uDL1 Data Files (*.csv)|*.csv||",NULL,0,TRUE);

	if( fileDialog.DoModal()==IDOK )
	{
		percentComplete=0;

		CBusyDialog dlg;
		dlg.Create(IDD_DIALOG_BUSYSTATUS,this);
		dlg.ShowWindow(SW_SHOW);
		dlg.m_captionTextCtrl.SetWindowTextA("Busy Reading Data.  Please Wait.");
		dlg.m_progressBarCtrl.SetPos(percentComplete);
		dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);

		csv.put_HasColumnNames(true);

		success = csv.LoadFile(fileDialog.GetPathName());
		if (success != true) 
		{
			dlg.ShowWindow(SW_HIDE);
			msg.Format("Error loading CSV fields: %s\n",csv.lastErrorText());
			MessageBox(msg,"Error",MB_OK|MB_ICONERROR);
			return;
		}
		rowCount = csv.get_NumRows();
		columnCount = csv.get_NumColumns();

		//Find the Record Type, Excel Time, DC Reading, AC Reading, and Temperature Reading column indices
		intStateIndex=typeIndex=timeIndex=dcIndex=acIndex=temperatureIndex=-1;

		for (i=0; i<columnCount; i++)
		{
			if (strstr(csv.getColumnName(i),"Record Type")) typeIndex=i;
			if (strstr(csv.getColumnName(i),"Excel Time")) timeIndex=i;
			if (strstr(csv.getColumnName(i),"DC Reading")) dcIndex=i;
			if (strstr(csv.getColumnName(i),"AC Reading")) acIndex=i;
			if (strstr(csv.getColumnName(i),"Temperature Reading")) temperatureIndex=i;
			if (strstr(csv.getColumnName(i),"Int State On")) intStateIndex=i;
		}

		if ((typeIndex==-1)||(timeIndex==-1)||(dcIndex==-1)||(acIndex==-1)||(temperatureIndex==-1)) //Don't check for int state as that was not present in files prior to v1.1.6.0
		{
			dlg.ShowWindow(SW_HIDE);
			MessageBox("Unable to parse data file.  Has it been modified after uDL1Config saved it?","Error",MB_OK);
			return;
		}

		//Find the time zone string
		memcpy(buffer,csv.getColumnName(timeIndex),256);
		buffer[255]=0;
		cPtr=strchr(buffer,'(');
		dPtr=strchr(buffer,')');
		if (cPtr&&dPtr)
		{
			cPtr++;
			*dPtr=0;
			datalogReadingsTzStr=cPtr;
		}
		else
		{
			datalogReadingsTzStr="TZ Unknown";
		}

		//Clear out chart memory
		datalogReadingsDCCount=0;
		datalogReadingsACCount=0;
		datalogReadingsTempCount=0;
		datalogReadingsWaveSegmentCount=0;
		inWaveSegment=FALSE;

		excelOffset=Chart::chartTime(1899,12,31,0,0,0)-86400;
		//int excelYMD = Chart::getChartYMD(excelOffset);
		//int excelHMS = (int)fmod(excelOffset, 86400);

		jun1_2011=Chart::chartTime(2011,06,01,0,0,0); //For detecting bad RTC timestamps

		//Parse data
		for (i=0; i<rowCount; i++)
		{
			percentComplete=(i*100)/rowCount;
			dlg.m_progressBarCtrl.SetPos(percentComplete);

			cellDataPtr=csv.getCell(i,typeIndex);
			if (strstr(cellDataPtr,"DC Reading (Wave Capture)"))
			{
				if (!inWaveSegment)
				{
					inWaveSegment=TRUE;
					if (datalogReadingsWaveSegmentCount<MAX_WAVE_SEGMENTS) datalogReadingsWaveSegmentCount++;
					datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount-1].startIndex=datalogReadingsDCCount;
					datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount-1].endIndex=datalogReadingsDCCount;
				}
				//printf("DC reading\r\n");
				dataValuePtr=csv.getCell(i,dcIndex);
				if (strncmp(dataValuePtr,"OVER",4))
				{
					sampledate=atof(csv.getCell(i,timeIndex))*86400+excelOffset;
					if (sampledate>jun1_2011)
					{
						datalogReadingsDC[datalogReadingsDCCount]=atof(dataValuePtr);
						datalogReadingsDCType[datalogReadingsDCCount]=1; //Wave capture
						datalogReadingsDCTime[datalogReadingsDCCount]=sampledate;
						datalogReadingsWaveSegments[datalogReadingsWaveSegmentCount-1].endIndex=datalogReadingsDCCount;
						datalogReadingsDCCount++;
					}
				}
			}
			else
				if (strstr(cellDataPtr,"DC Reading"))
				{
					//printf("DC reading\r\n");
					dataValuePtr=csv.getCell(i,dcIndex);
					if (strstr(cellDataPtr,"(Int)"))
					{
						if (intStateIndex>=0)
						{
								dataType=atoi(csv.getCell(i,intStateIndex)); //1=on, 0=off
								if (dataType) dataType=3; else dataType=2; //2=off, 3=on
						}
						else 
						{
								dataType=0; //Normal read if int type column not present
						}
					}
					else
					{
						dataType=0; //Normal read
					}
					if (strncmp(dataValuePtr,"OVER",4))
					{
						sampledate=atof(csv.getCell(i,timeIndex))*86400+excelOffset;
						if (sampledate>jun1_2011)
						{
							datalogReadingsDC[datalogReadingsDCCount]=atof(dataValuePtr);
							datalogReadingsDCType[datalogReadingsDCCount]=dataType;
							datalogReadingsDCTime[datalogReadingsDCCount]=sampledate;
							datalogReadingsDCCount++;
						}
					}
					if (inWaveSegment)
					{
						inWaveSegment=FALSE;
					}
				}
				else
					if (strstr(cellDataPtr,"AC Reading"))
					{
						//printf("AC reading\r\n");
						dataValuePtr=csv.getCell(i,acIndex);
						if (strstr(cellDataPtr,"(Int)"))
						{
							if (intStateIndex>=0)
							{
									dataType=atoi(csv.getCell(i,intStateIndex)); //1=on, 0=off
									if (dataType) dataType=3; else dataType=2; //2=off, 3=on
							}
							else 
							{
									dataType=0; //Normal read if int type column not present
							}
						}
						else
						{
							dataType=0; //Normal read
						}
						if (strncmp(dataValuePtr,"OVER",4))
						{
							sampledate=atof(csv.getCell(i,timeIndex))*86400+excelOffset;
							if (sampledate>jun1_2011)
							{
								datalogReadingsAC[datalogReadingsACCount]=atof(dataValuePtr);
								datalogReadingsACType[datalogReadingsACCount]=dataType;
								datalogReadingsACTime[datalogReadingsACCount]=sampledate;
								datalogReadingsACCount++;
							}
						}
					}
					else
						if (strstr(cellDataPtr,"Temperature"))
						{
							//printf("Temperature reading\r\n");
							dataValuePtr=csv.getCell(i,temperatureIndex);
							if (strncmp(dataValuePtr,"OVER",4))
							{
								sampledate=atof(csv.getCell(i,timeIndex))*86400+excelOffset;
								if (sampledate>jun1_2011)
								{
									datalogReadingsTemp[datalogReadingsTempCount]=atof(dataValuePtr);
									datalogReadingsTempTime[datalogReadingsTempCount]=sampledate;
									datalogReadingsTempCount++;
								}
							}
						}
		}

		dlg.ShowWindow(SW_HIDE);
		((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->SetWindowText(CString("File: ")+fileDialog.GetFileName());
		msg.Format("Data file successfully loaded.\n%lu DC readings\n%lu AC readings\n%lu temperature readings\n",datalogReadingsDCCount,datalogReadingsACCount,datalogReadingsTempCount);
		MessageBox(msg,"Info",MB_OK);
	}
}

void Cudl1configDlg::OnBnClickedButtonChartall()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;

	if (!datalogReadingsACCount&&!datalogReadingsDCCount&&!datalogReadingsTempCount)
	{
		MessageBox("No readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsDCCount; i++)
	{
		if (datalogReadingsDCTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}
	for (i=0; i<datalogReadingsACCount; i++)
	{
		if (datalogReadingsACTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}
	for (i=0; i<datalogReadingsTempCount; i++)
	{
		if (datalogReadingsTempTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}

	dlg.m_timeStampsA=DoubleArray(datalogReadingsDCTime,datalogReadingsDCCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsDC,datalogReadingsDCCount);
	dlg.m_dataTitleA="DC";

	dlg.m_timeStampsB=DoubleArray(datalogReadingsACTime,datalogReadingsACCount);
	dlg.m_dataSeriesB=DoubleArray(datalogReadingsAC,datalogReadingsACCount);
	dlg.m_dataTitleB="AC";

	dlg.m_timeStampsC=DoubleArray(datalogReadingsTempTime,datalogReadingsTempCount);
	dlg.m_dataSeriesC=DoubleArray(datalogReadingsTemp,datalogReadingsTempCount);
	dlg.m_dataTitleC="Temperature";

	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="AC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);

	dlg.m_multiline=TRUE;
	dlg.m_printDlgPtr=&printDlg;

	int nResponse = (int)dlg.DoModal();
}

void Cudl1configDlg::OnBnClickedButtonAnalogtemptest()
{
#ifdef _DEBUG
	FILE *outFilePtr;
	C_WINUSB m_ser2;
	BOOL m_testerConnected;
	C_BIPWU m_bip2;
	UINT8 tries, range;
	char strRange[256], strTemp[256];

	double reading,reference;
	INT32 dcOffset[3];
	UINT16 acFloor[3];
	double dcReading[3];
	double dcRef[3];
	double acReading[3];
	double acRef[3];

	DWORD toRead, numLeft;
	T_PARAM_MFGDATA paramMfgTemp;
	UINT32 address;
	BOOL calPassed;
	BOOL needsheader;
	BOOL found;
	DWORD startTime;

	SYSTEMTIME st;

	calPassed=FALSE;

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		return;
	}


	//Read in the current temperature
	m_bip1.SendGeneric(MBP_GETBATTERYINFO);
	if (!m_bip1.ResponseWait(1000))
	{
		MessageBox("Error retrieving communications info from uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	//Read in manufacturing option data
	address=PARAM_MFG_OFFSET;
	numLeft=sizeof(T_PARAM_MFGDATA);
	while (numLeft)
	{
		toRead=1024;
		if (toRead>numLeft) toRead=numLeft;
		tries=0;
retryEERead2:
		m_bip1.SendReadEEPROM(address,toRead);
		if (!m_bip1.ResponseWait(1000)||!m_bip1.m_ack)
		{
			if (tries++<5) goto retryEERead2;
			MessageBox("Failed to read configuration data from uDL1!","Error",MB_OK|MB_ICONEXCLAMATION);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		memcpy(((UINT8*)&paramMfgTemp)+address-PARAM_MFG_OFFSET,m_bip1.m_dataBuffer,toRead);
		address+=toRead;
		numLeft-=toRead;
	}

	//Validate version
	if (paramMfgTemp.paramVersion!=PARAM_MFG_VERSION)
	{
		MessageBox("The uDL1 mfg parameter set is not compatible with this version of uDL1Config!","Error",MB_OK|MB_ICONEXCLAMATION);
		sprintf(strTemp,"uDL1 mfg parameter set version=%lu, uDL1Config parameter set version=%lu.\r\n",(UINT32)(paramMfgTemp.paramVersion),PARAM_MFG_VERSION);
		//DisplayGeneralStatus(strTemp);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	//Validate CRC
	if (!C_FCS::Verify((UINT8*)&paramMfgTemp, sizeof(T_PARAM_MFGDATA)))
	{
		//MessageBox("CRC error in parameter data!","Error",MB_OK|MB_ICONEXCLAMATION);
		//DisplayLinkStatus("Idle",0,255,0,0,0,0);

		//return;
		memcpy(&m_paramsmfg,&param_mfgdata_defaults,sizeof(T_PARAM_MFGDATA));
	}
	else
	{
		memcpy(&m_paramsmfg,&paramMfgTemp,sizeof(T_PARAM_MFGDATA));
	}

	//Connect to tester
	m_testerConnected=FALSE;
	if (m_ser2.Open(gRMU2SDL1GUID)==ERROR_SUCCESS)
	{
		if (m_bip2.Options(&m_ser2,"BIPWU")==ERROR_SUCCESS)
		{
			printf("Tester connected.\r\n");
			m_testerConnected=TRUE;
		}
	}
	else
	{
		if (m_ser2.Open(gRDU1GUID)==ERROR_SUCCESS)
		{
			if (m_bip2.Options(&m_ser2,"BIPWU")==ERROR_SUCCESS)
			{
				printf("Tester connected.\r\n");
				m_testerConnected=TRUE;
			}
		}
	}
	if (m_testerConnected)
	{
		printf("Enabling 3V3A and RMS converter.\r\n");
		m_bip1.SendSetPortOutput(MBP_PORT_GPIOA,11,1);
		if (!m_bip1.ResponseWait(5000))
		{
			printf("Error enabling 3V3A.\r\n");
			ENDCAL();
		}
		m_bip1.SendSetPortOutput(MBP_PORT_ADC,0,0);
		if (!m_bip1.ResponseWait(5000))
		{
			printf("Error enabling RMS converter.\r\n");
			ENDCAL();
		}
		Sleep(5000);


		//Set the test jig to all channels disconnected, to allow us to perform zero offset measurements
		printf("Performing DC zero offset and AC floor test\r\n");
		SetMeterDisplay("Test#1");
		tries=0;
retryDisconnect:
		m_bip2.SendSetAnalogOut(0, 128, 0, 0); 
		if(!m_bip2.ResponseWait(500))
		{
			printf("No response to set analog out request!\r\n");
			if (tries++<5) goto retryDisconnect;
			printf("Couldn't talk to calibration jig!\r\n");
			ENDCAL();
		}

		for (range=0; range<3; range++)
		{
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			Sleep(1000); //Set DC zero offsets

			m_bip1.SendReadRawAnalog(MBP_ANALOG_DC,8);
			if (!m_bip1.ResponseWait(5000))
			{
				printf("Error requesting raw reading from uDL1.\r\n");
				ENDCAL();
			}
			printf("Range %lu, DC Offset: %li\r\n",(DWORD)range,m_bip1.m_rawMeasurement);
			dcOffset[range]=m_bip1.m_rawMeasurement;

			m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,8);
			if (!m_bip1.ResponseWait(5000))
			{
				printf("Error requesting raw reading from uDL1.\r\n");
				ENDCAL();
			}
			printf("Range %lu, AC Floor: %li\r\n",(DWORD)range,m_bip1.m_rawMeasurement);
			acFloor[range]=m_bip1.m_rawMeasurement;
		}

		printf("Performing DC scale factor test.\r\n");
		SetMeterDisplay("Test#2");
		for (range=0; range<3; range++)
		{
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			//Route cal signal to all inputs
			tries=0;
retryDCcalset:
			if (range==0)
			{
				m_bip2.SendSetAnalogOut(0xF, 128+14, 0, 0);
				strcpy(strRange,"1");
			}
			else
				if (range==1)
				{
					m_bip2.SendSetAnalogOut(0xF, 128+19, 0, 2);
					strcpy(strRange,"100");
				}
				else
				{
					m_bip2.SendSetAnalogOut(0xF, 128+55, 0, 2);
					strcpy(strRange,"100");
				}
				if(!m_bip2.ResponseWait(500))
				{
					printf("No response to set analog out request!\r\n");
					if (tries++<5) goto retryDCcalset;
					printf("Couldn't talk to calibration jig!\r\n");
					ENDCAL();
				}

				Sleep(1000); 

				if (!GetMeterReference(&m_bip2,&reference, 0, strRange))
				{
					printf("Couldn't read meter reference value!\r\n");
					ENDCAL();
				}

				m_bip1.SendReadRawAnalog(MBP_ANALOG_DC,8);
				if (!m_bip1.ResponseWait(5000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				reading=(m_bip1.m_rawMeasurement-m_paramsmfg.calibration[0].zeroCal[range])*m_paramsmfg.calibration[0].scaleFactor[range];
				printf("Range %lu, Raw %li, Reference %.5f, Reading: %.5f\r\n",(DWORD)range,m_bip1.m_rawMeasurement,reference,reading);
				dcRef[range]=reference;
				dcReading[range]=reading;
		}



		printf("Performing AC scaleFactor test.\r\n");
		SetMeterDisplay("Test#4");
		for (range=0; range<3; range++)
		{
			//Reset all channels to proper range
			m_bip1.SendSetInputRange(range);
			if (!m_bip1.ResponseWait(500))
			{
				printf("Error setting input range on uDL1.\r\n");
				ENDCAL();
			}

			tries=0;
retryACcalset2:
			if (range==0)
			{
				m_bip2.SendSetAnalogOut(0xF, 128, 60, 0);
				strcpy(strRange,"1");
			}
			else
				if (range==1)
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 80, 2);
					strcpy(strRange,"100");
				}
				else
				{
					m_bip2.SendSetAnalogOut(0xF, 128, 240, 2);
					strcpy(strRange,"100");
				}
				if(!m_bip2.ResponseWait(500))
				{
					printf("No response to set analog out request!\r\n");
					if (tries++<5) goto retryACcalset2;
					printf("Couldn't talk to calibration jig!\r\n");
					ENDCAL();
				}

				Sleep(8000); //Let the signal stabilize

				if (!GetMeterReference(&m_bip2,&reference, 1, strRange))
				{
					printf("Couldn't read meter reference value!\r\n");
					ENDCAL();
				}
				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,4);
				if (!m_bip1.ResponseWait(5000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				m_bip1.SendReadRawAnalog(MBP_ANALOG_AC,16);
				if (!m_bip1.ResponseWait(8000))
				{
					printf("Error requesting raw reading from uDL1.\r\n");
					ENDCAL();
				}
				reading=(m_bip1.m_rawMeasurement-m_paramsmfg.calibration[1].zeroCal[range])*m_paramsmfg.calibration[1].scaleFactor[range];
				printf("AC Range %lu, Raw %li, Reference %.5f, Reading=%.5f\r\n",(DWORD)range,m_bip1.m_rawMeasurement,reference,reading);
				acRef[range]=reference;
				acReading[range]=reading;
		}

		calPassed=TRUE;


cleanup:
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		m_ser2.ReceiveEnable(FALSE);
		m_ser2.Close();
		m_bip2.Options(NULL,"");

		if (calPassed)
		{
			needsheader=TRUE;
			outFilePtr=fopen("c:\\udl1temptest.csv","rb");
			if (outFilePtr)
			{
				fclose(outFilePtr);
				needsheader=FALSE;
			}

			outFilePtr=fopen("c:\\udl1temptest.csv","ab");
			if (!outFilePtr)
			{
				MessageBox("Failed to open output data file.","Error",MB_OK|MB_ICONERROR);
				return;
			}

			if (needsheader)
				fprintf(outFilePtr,"Time(Local),Serial,Temperature(C),"
				"DC Offset Cal(L),DC Offset Meas(L),"
				"DC Offset Cal(M),DC Offset Meas(M),"
				"DC Offset Cal(H),DC Offset Meas(H),"
				"AC Floor Cal(L),AC Floor Meas(L),"
				"AC Floor Cal(M),AC Floor Meas(M),"
				"AC Floor Cal(H),AC Floor Meas(H),"
				"DC Ref(L),DC Reading(L),"
				"DC Ref(M),DC Reading(M),"
				"DC Ref(H),DC Reading(H),"
				"AC Ref(L),AC Reading(L),"
				"AC Ref(M),AC Reading(M),"
				"AC Ref(H),AC Reading(H)\r\n");


			GetLocalTime(&st);

			fprintf(outFilePtr,"%04u/%02u/%02u %02u:%02u:%02u,",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
			fprintf(outFilePtr,"%lu,",m_bip1.m_serialNumber);
			fprintf(outFilePtr,"%.1f,",m_bip1.m_batteryTemperature);
			for (range=0; range<3; range++)
				fprintf(outFilePtr,"%i,%li,",m_paramsmfg.calibration[0].zeroCal[range],dcOffset[range]);
			for (range=0; range<3; range++)
				fprintf(outFilePtr,"%u,%u,",m_paramsmfg.acFloorCal[range],acFloor[range]);
			for (range=0; range<3; range++)
				fprintf(outFilePtr,"%.6f,%.6f,",dcRef[range],dcReading[range]);
			for (range=0; range<3; range++)
				fprintf(outFilePtr,"%.6f,%.6f,",acRef[range],acReading[range]);
			fprintf(outFilePtr,"\r\n");
			fclose(outFilePtr);

			MessageBox("Analog temperature test completed successfully.","Info",MB_OK);
		}
		else
			MessageBox("Analog temperature test failed!","Error",MB_OK|MB_ICONERROR);
	}
#else
	MessageBox("Release version of app does not support analog temperature testing.","Error",MB_OK);
#endif
}


void Cudl1configDlg::OnBnClickedButtonDlgtest()
{
	CFileDialog fileDialog(FALSE,".cfg","test.cfg",OFN_OVERWRITEPROMPT, "uDL1 Config Files (*.cfg)|*.cfg|All Files (*.*)|*.*||",NULL,0,TRUE);

	if( fileDialog.DoModal()==IDOK )
	{
		MessageBox("Ok","Ok",MB_OK);
	}
	else
	{
		MessageBox("Cancel","Cancel",MB_OK);
	}
}


void Cudl1configDlg::OnBnClickedButtonSplitdatafile()
{
	CFileSplitUtilityDlg dlg;

	CWinApp* pApp = AfxGetApp();

	//Pull in last dialog options from registry
	dlg.m_notesInAllOutput= pApp->GetProfileInt("ConversionOptions", "IncludeNotes", 1)?TRUE:FALSE;
	dlg.m_verbalDates= pApp->GetProfileInt("ConversionOptions", "VerbalDates", 1)?TRUE:FALSE;
	dlg.m_excelDates= pApp->GetProfileInt("ConversionOptions", "ExcelDates", 1)?TRUE:FALSE;
	dlg.m_dcReadings= pApp->GetProfileInt("ConversionOptions", "dcReadings", 1)?TRUE:FALSE;
	dlg.m_acReadings= pApp->GetProfileInt("ConversionOptions", "acReadings", 1)?TRUE:FALSE;
	dlg.m_tempReadings= pApp->GetProfileInt("ConversionOptions", "tempReadings", 1)?TRUE:FALSE;
	dlg.m_dcWaveReadings= pApp->GetProfileInt("ConversionOptions", "dcWaveReadings", 1)?TRUE:FALSE;
	dlg.m_separateSessionFiles= pApp->GetProfileInt("ConversionOptions", "SeparateSessionFiles", 1)?TRUE:FALSE;
	dlg.m_includeUnits= pApp->GetProfileInt("ConversionOptions", "IncludeUnits", 1)?TRUE:FALSE;

	dlg.DoModal();

	//Save selected options to registry
	pApp->WriteProfileInt("ConversionOptions", "IncludeNotes", dlg.m_notesInAllOutput?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "VerbalDates", dlg.m_verbalDates?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "ExcelDates", dlg.m_excelDates?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "dcReadings", dlg.m_dcReadings?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "acReadings", dlg.m_acReadings?1:0);
	pApp->WriteProfileInt("ConversionOptions", "tempReadings", dlg.m_tempReadings?1:0);
	pApp->WriteProfileInt("ConversionOptions", "dcWaveReadings", dlg.m_dcWaveReadings?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "SeparateSessionFiles", dlg.m_separateSessionFiles?1:0); 
	pApp->WriteProfileInt("ConversionOptions", "IncludeUnits", dlg.m_includeUnits?1:0); 
}


void Cudl1configDlg::OnBnClickedButtonChartdcwave()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1;
	CString tempStr;

	if (!datalogReadingsWaveSegmentCount)
	{
		MessageBox("No DC wave readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsDCCount; i++)
	{
		if (datalogReadingsDCTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
	}

	dlg.m_timeStampsB=DoubleArray(datalogReadingsDCTime,datalogReadingsDCCount); //Save data in dataset B--dialog will copy to dataset A as segment is selected
	dlg.m_dataSeriesB=DoubleArray(datalogReadingsDC,datalogReadingsDCCount); //Save data in dataset B
	dlg.m_dataTitleA.Format("DC Wave");
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	dlg.m_waveprint=TRUE;
	dlg.m_waveSegments=&datalogReadingsWaveSegments[0];
	dlg.m_waveSegmentsCount=datalogReadingsWaveSegmentCount;

	//dlg.m_chartTitle="DC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}

void Cudl1configDlg::OnBnClickedButton32khztest()
{
	C32kHzTestDlg dlg;
	BOOL found;
	DWORD startTime;

	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	dlg.m_bipPtr=&m_bip1;
	dlg.DoModal();

	DisplayLinkStatus("Idle",0,255,0,0,0,0);
}

void Cudl1configDlg::ProcessMessages()
{
	CWinApp* pApp = AfxGetApp();
	MSG msg;

	while ( PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE ))
		pApp->PumpMessage();
}

void Cudl1configDlg::OnBnClickedButtonBinaryextract()
{
	BOOL found;
	DWORD startTime;
	DWORD address, toTransfer, toCopy, tries, triesPing;
	UINT8 percentComplete;
	UINT32 baseTime;

	CString tzStr;
	int tzOffsetSeconds, tzUTCCompSeconds;


	UpdateData(TRUE);
	GetTZInfo(tzOffsetSeconds, tzStr);
	printf("Tz: %s, offset: %i\r\n",tzStr,tzOffsetSeconds);
	((CButton*)GetDlgItem(IDC_RADIO_LOCAL))->SetWindowText("Local ("+tzStr+")");
	tzUTCCompSeconds=0;
	if (!m_dateFormat)
	{
		tzUTCCompSeconds=tzOffsetSeconds;
		tzOffsetSeconds=0;
		tzStr="UTC";
	}

#if 0
	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}
#endif
	if (!ReaduDLConfig()) return;

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL1 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	if (m_bip1.m_storageUsedBytes>m_bip1.m_storageTotalBytes) m_bip1.m_storageUsedBytes=m_bip1.m_storageTotalBytes; //v1.1.1.4: fix for uDL misreporting bytes used when full.
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	address=0;
	//toTransfer=m_bip1.m_storageUsedBytes;
	toTransfer=m_bip1.m_storageTotalBytes; //Do a full memory dump

	if (toTransfer>DATALOG_MAXSIZE)
	{	
		MessageBox("Datalogger point count exceeds capability of this version of configuration application.  Please update to the latest version.","Error",MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	memset(datalogMemory,0xFF,DATALOG_MAXSIZE);
	percentComplete=0;
	CBusyDialog dlg;
	dlg.Create(IDD_DIALOG_BUSYSTATUS,this);
	dlg.ShowWindow(SW_SHOW);
	dlg.m_captionTextCtrl.SetWindowTextA("Busy Transferring Data.  Please Wait.");
	dlg.m_progressBarCtrl.SetPos(percentComplete);
	dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);

	startTime=GetTickCount();
	datalogMemoryCount=toTransfer;
	while(toTransfer)
	{
		percentComplete=address*100/(datalogMemoryCount+1);
		dlg.m_progressBarCtrl.SetPos(percentComplete);
		dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);
		//ProcessMessages();
		toCopy=2000;
		if (toCopy>toTransfer) toCopy=toTransfer;
		tries=0;
retryReadDF:
		m_bip1.SendReadDataFlash(address, toCopy);
		if (!m_bip1.ResponseWait(2000))
		{
			if (tries++<3)
			{	//On a retry of a DF read, first ping the device to resynchronize the transfer
				triesPing=0;
retryReadDFPing:
				m_bip1.SendGeneric(MBP_PING);
				if (!m_bip1.ResponseWait(1000))
				{
					if (triesPing++<3) goto retryReadDFPing;
					dlg.ShowWindow(SW_HIDE);
					MessageBox("Error reading storage memory on uDL1.","Error",MB_ICONERROR|MB_OK);
					DisplayLinkStatus("Idle",0,255,0,0,0,0);
					return;
				}
				goto retryReadDF;
			}
			dlg.ShowWindow(SW_HIDE);
			MessageBox("Error reading storage memory on uDL1.","Error",MB_ICONERROR|MB_OK);
			DisplayLinkStatus("Idle",0,255,0,0,0,0);
			return;
		}
		memcpy(datalogMemory+address,m_bip1.m_dataBuffer,toCopy);
		address+=toCopy;
		toTransfer-=toCopy;
	}
	dlg.ShowWindow(SW_HIDE);

	printf("Transferred: %lu bytes, time: %.3f s, throughput: %.3f KB/s\r\n",datalogMemoryCount,((GetTickCount()-startTime)/1000.0),(float)datalogMemoryCount/1024/((GetTickCount()-startTime)/1000.0));

	DisplayLinkStatus("Idle",0,255,0,0,0,0);

	FILE *outFilePtr;
	char fileNameDefault[100];

	sprintf(fileNameDefault,"udl1_sn%06u.bin",m_bip1.m_serialNumber);
	CFileDialog fileDialog(FALSE,".bin",fileNameDefault,OFN_OVERWRITEPROMPT, "uDL1 Binary Data Files (*.bin)|*.bin|All Files (*.*)|*.*||",NULL,0,TRUE);

	if( fileDialog.DoModal()==IDOK )
	{
		//Save Data
		outFilePtr=fopen(fileDialog.GetPathName(),"wb");
		if (outFilePtr)
		{
			if (fwrite(datalogMemory,1,m_bip1.m_storageTotalBytes,outFilePtr)!=m_bip1.m_storageTotalBytes)
			{
				fclose(outFilePtr);
				MessageBox("Error writing to output file!","Error",MB_OK|MB_ICONERROR);
			}
			fclose(outFilePtr);
			MessageBox("uDL1 Data Memory Extracted","Info",MB_OK);
		}
		else
		{
			MessageBox("Error opening output file!","Error",MB_OK|MB_ICONERROR);
		}
	}
}


void Cudl1configDlg::OnEnKillfocusEditIntontime()
{
	if (UpdateData(TRUE))
	{
		ValidateInterruption();
		m_interruptionTrackingCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;

		UpdateData(FALSE);
	}
}


void Cudl1configDlg::OnEnKillfocusEditIntofftime()
{
	if (UpdateData(TRUE))
	{
		ValidateInterruption();
		m_interruptionTrackingCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;

		UpdateData(FALSE);
	}
}

void Cudl1configDlg::OnEnKillfocusEditIntontooffoffset()
{
	if (UpdateData(TRUE))
	{
		ValidateInterruption();
		m_interruptionTrackingCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;

		UpdateData(FALSE);
	}
}


void Cudl1configDlg::OnEnKillfocusEditIntofftoonoffset()
{
	if (UpdateData(TRUE))
	{
		ValidateInterruption();
		m_interruptionTrackingCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;

		UpdateData(FALSE);
	}
}


void Cudl1configDlg::OnEnKillfocusEditIntutcoffset()
{
	if (UpdateData(TRUE))
	{
		ValidateInterruption();
		m_interruptionTrackingCycleTime=m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime;

		UpdateData(FALSE);
	}
}

void Cudl1configDlg::ValidateInterruption()
{
	UINT16 minOnOff;

	minOnOff=(m_acSamplingEnabled&&m_dcSamplingEnabled)?250:150;

	//Correct interruption cycle times to be within limits
	if (m_interruptionTrackingOffTimeMs<minOnOff) m_interruptionTrackingOffTimeMs=minOnOff;
	if (m_interruptionTrackingOffTimeMs>(60000-minOnOff)) m_interruptionTrackingOffTimeMs=(60000-minOnOff);
	if (m_interruptionTrackingOnTime<minOnOff) m_interruptionTrackingOnTime=minOnOff;
	if (m_interruptionTrackingOnTime>(60000-minOnOff)) m_interruptionTrackingOnTime=(60000-minOnOff);
	if ((m_interruptionTrackingOffTimeMs+m_interruptionTrackingOnTime)>60000) m_interruptionTrackingOnTime=60000-m_interruptionTrackingOffTimeMs;
	if (m_interruptionTrackingOffToOnOffsetMs>(m_interruptionTrackingOnTime-minOnOff)) m_interruptionTrackingOffToOnOffsetMs=m_interruptionTrackingOnTime-minOnOff;
	if (m_interruptionTrackingOnToOffOffsetMs>(m_interruptionTrackingOffTimeMs-minOnOff)) m_interruptionTrackingOnToOffOffsetMs=m_interruptionTrackingOffTimeMs-minOnOff;
	if (m_interruptionTrackingUTCOffsetMs>5000) m_interruptionTrackingUTCOffsetMs=5000;
	if (m_interruptionTrackingUTCOffsetMs<-5000) m_interruptionTrackingUTCOffsetMs=-5000;
}





void Cudl1configDlg::OnBnClickedButtonChartintdc()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;
	BOOL intReadingsFound;

	intReadingsFound=FALSE;


	if (!datalogReadingsDCCount)
	{
		MessageBox("No DC readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsDCCount; i++)
	{
		if (datalogReadingsDCTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
		if (datalogReadingsDCType[i]>=2) intReadingsFound=TRUE;
	}

	if (!intReadingsFound)
	{
		MessageBox("No DC interruption tracking readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	dlg.m_pointTypes=TRUE;
	dlg.m_timeStampsA=DoubleArray(datalogReadingsDCTime,datalogReadingsDCCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsDC,datalogReadingsDCCount);
	dlg.m_pointTypesA=DoubleArray(datalogReadingsDCType,datalogReadingsDCCount);
	dlg.m_dataTitleA="DC";
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="DC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}


void Cudl1configDlg::OnBnClickedButtonChartintac()
{
	CZoomscrolldemoDlg dlg;
	DWORD i,j;
	double temp1, temp2;
	BOOL intReadingsFound;

	intReadingsFound=FALSE;

	if (!datalogReadingsACCount)
	{
		MessageBox("No AC readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	//Validate times
	temp1=Chart::chartTime(2011,01,01,0,0,0);
	for (i=0; i<datalogReadingsACCount; i++)
	{
		if (datalogReadingsACTime[i]<temp1)
		{
			MessageBox("Bad time stamp found in data.  Can not preview data chart.","Error",MB_OK|MB_ICONERROR);
			return;
		}
		if (datalogReadingsACType[i]>=2) intReadingsFound=TRUE;
	}

	if (!intReadingsFound)
	{
		MessageBox("No AC interruption tracking readings found.","Error",MB_OK|MB_ICONERROR);
		return;
	}

	dlg.m_pointTypes=TRUE;
	dlg.m_timeStampsA=DoubleArray(datalogReadingsACTime,datalogReadingsACCount);
	dlg.m_dataSeriesA=DoubleArray(datalogReadingsAC,datalogReadingsACCount);
	dlg.m_pointTypesA=DoubleArray(datalogReadingsACType,datalogReadingsACCount);
	dlg.m_dataTitleA="AC";
	dlg.m_timeZoneStr=datalogReadingsTzStr;
	//dlg.m_chartTitle="AC Data Preview Chart("+datalogReadingsTzStr+")";
	((CEdit*)GetDlgItem(IDC_EDIT_CHARTTITLE))->GetWindowText(dlg.m_chartTitle);
	dlg.m_printDlgPtr=&printDlg;
	int nResponse = (int)dlg.DoModal();
}


void Cudl1configDlg::OnBnClickedButtonMemtime()
{
	UINT32 startTime, acqTimeSec;
	double acqRate;
	UINT32 acqSecs, acqMins, acqHours, acqDays;
	BOOL found;
	UINT8 channelCount;
	CString strOut;

	DialogToEEPROM();

	OnCbnSelchangeComboPortsel();

	if (!m_serialPortOpen)
	{
		//MessageBox("Error--no serial port selected.","Error",MB_ICONERROR|MB_OK);
		return;
	}

	DisplayLinkStatus("Busy",255,0,0);

	//Ping the uDL2 first
	found=FALSE;
	startTime=GetTickCount();
	while ((GetTickCount()-startTime)<6000)
	{
		m_bip1.SendGeneric(MBP_PING);
		if (m_bip1.ResponseWait(1000))
		{
			found=TRUE;
			break;
		}
	}
	if (!found)
	{
		MessageBox("Error pinging uDL1.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	if (m_bip1.m_storageUsedBytes>m_bip1.m_storageTotalBytes) m_bip1.m_storageUsedBytes=m_bip1.m_storageTotalBytes; //v1.0.9.2: fix for full memory error 
	SetMemUseBar((m_bip1.m_storageUsedBytes+1)*100.0/(m_bip1.m_storageTotalBytes+1));

	channelCount=0;
	if (m_params.configFlags&CONFIGFLAGS_DCENABLED) channelCount++;
	if (m_params.configFlags&CONFIGFLAGS_ACENABLED) channelCount++;
	if (m_params.configFlags&CONFIGFLAGS_TEMPERATURE_ENABLED) channelCount++;

	printf("Channel count: %u\r\n",channelCount);

	if (!channelCount)
	{
		MessageBox("Configuration error.  No channels are enabled.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}

	acqRate=0.0;
	if (m_params.configFlags&CONFIGFLAGS_INTERRUPTIONTRACKING_ENABLED)
	{
		if (m_params.interruptionCycleTime)
		{
			acqRate+=2/(m_params.interruptionCycleTime/1000.0)*sizeof(T_DATALOG_READING_RECORD)*channelCount;
		}
	}
	else
	{
		if (m_params.samplingCyclePeriod)
		{
			acqRate+=1/(m_params.samplingCyclePeriod/1000.0)*sizeof(T_DATALOG_READING_RECORD)*channelCount;
		}
	}
	if ((m_params.configFlags&CONFIGFLAGS_FASTSAMPLING_ENABLED)&&(m_params.configFlags&CONFIGFLAGS_DCENABLED)&&m_params.samplingFastDuration&&m_params.samplingFastInterval)
	{
		if (m_params.configFlags&CONFIGFLAGS_FASTSAMPLINGCONTINUOUS_ENABLED)
		{
			acqRate=0.0; //Normal sampling won't run in this case
			acqRate+=1.0/(m_params.samplingCyclePeriodFast/1000.0)*sizeof(T_DATALOG_READING_RECORD);
		}
		else
		{
			acqRate*=(m_params.samplingFastInterval-m_params.samplingFastDuration)/(double)m_params.samplingFastInterval; //Compensate for time fast sampling is occurring
			acqRate+=1.0/(m_params.samplingCyclePeriodFast/1000.0)*m_params.samplingFastDuration/m_params.samplingFastInterval*sizeof(T_DATALOG_READING_RECORD);
		}
	}

	if (acqRate==0.0)
	{
		MessageBox("Configuration error.  All sampling intervals disabled.","Error",MB_ICONERROR|MB_OK);
		DisplayLinkStatus("Idle",0,255,0,0,0,0);
		return;
	}
	acqTimeSec=(m_bip1.m_storageTotalBytes-m_bip1.m_storageUsedBytes)/acqRate;

	printf("Acq rate: %.5f  Acq timesec: %lu\r\n",acqRate,acqTimeSec);

	acqDays=acqTimeSec/(24UL*60*60);
	acqTimeSec=acqTimeSec%(24UL*60*60);
	acqHours=acqTimeSec/(60UL*60);
	acqTimeSec=acqTimeSec%(60UL*60);
	acqMins=acqTimeSec/(60UL);
	acqSecs=acqTimeSec%(60UL);

	strOut.Format("***Note that actual maximum acquisition time will be limited by battery capacity and charge levels.***\r\n\r\nWith currently unused memory, a fully charged battery with full capacity and the configuration parameters shown in the editor window, maximum acquisition time is estimated to be approximately\r\n\r\n%u day%s and %u hour%s\r\n\r\nbefore memory becomes full.",acqDays,((acqDays>1)||(!acqDays))?"s":"",acqHours,((acqHours>1)||(!acqHours))?"s":"");
	MessageBox(strOut,"Maximum Acquisition Time Estimate",MB_OK);

	DisplayLinkStatus("Idle",0,255,0,0,0,0);
	return;
}

void Cudl1configDlg::OnBnClickedButtonSdllocationnotes()
{
	CSdlLocationNotesDlg dlg;
	if (UpdateData(TRUE))
	{
		dlg.m_sdlNotesEnabled = m_sdlNotes.enabled;
		dlg.m_sdlChainage = m_sdlNotes.chainage;
		dlg.m_sdlStnSeries = m_sdlNotes.stnSeries;

		if (dlg.DoModal() == IDOK)
		{
			m_sdlNotes.enabled = dlg.m_sdlNotesEnabled;
			m_sdlNotes.chainage = dlg.m_sdlChainage;
			m_sdlNotes.stnSeries = dlg.m_sdlStnSeries;
			UpdateData(FALSE);
		}
	}
}
