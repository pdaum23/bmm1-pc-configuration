// udl1configDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "serxprot.hpp"
#include "bipxprot.hpp"
#include "winusbif.hpp"
#include "datalog.hpp"
#include "bipwuxprot.hpp"
#include "ChartViewer.h"
#include "afxcmn.h"
#include "fcs16.hpp"
#include "TextProgressCtrl.h"

typedef unsigned __int32 UINT32;
typedef unsigned __int16 UINT16;
typedef unsigned __int8 UINT8;
typedef __int32 INT32;
typedef __int16 INT16;
//typedef __int8 INT8;

#define MAGIC_KEY_FOR_FILES 0x33554D52 /* "uDL1" */
#define VERSION_KEY_FOR_FILES 0x00000000

// Cudl1configDlg dialog
class Cudl1configDlg : public CDialogEx
{
// Construction
public:
	Cudl1configDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_uDL1CONFIG_DIALOG };

	CString m_dialogTitle;
	CString m_appPathStr;
	CString m_appVerStr;
	CChartViewer	m_ChartViewer;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HACCEL m_hAccel;

	void FindAppPath();
	void FixPath(CString &s);

	C_SER m_ser1;
	BOOL m_serialPortOpen;
	C_BIP m_bip1;

	T_PARAM_PARAMETERS m_params;
	T_PARAM_PARAMETERS m_params_original;
	T_PARAM_MFGDATA m_paramsmfg;

	void DisplayLinkStatus(CString s, UINT8 red, UINT8 green, UINT8 blue, UINT8 textRed=255, UINT8 textGreen=255, UINT8 textBlue=255);
	void DisplayConfigStatus(CString s, UINT8 red, UINT8 green, UINT8 blue);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	SdlNotes NotesParseSDLInfo(CString notes);
	CString NotesGenerateSDLInfo(SdlNotes sdlNotes);

public:
	afx_msg void OnFileOpen32772();
	CComboBox m_comPortControl;
	void SetMemUseBar(double pos);
	afx_msg void OnCbnSelchangeComboPortsel();
	CRichEditCtrl m_linkStatusRichCtrl;
	CRichEditCtrl m_configStatusRichCtrl;
	afx_msg void OnBnClickedButtonReadrmu();
	BOOL ReaduDLConfig();
	BOOL EEPROMToDialog();
	afx_msg void OnBnClickedButtonWritermu();
	BOOL DialogToEEPROM();
	CEdit m_codeVersionCtrl;
	CEdit m_serialNumberCtrl;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonDefaults();
	void CheckForChanges();
	afx_msg void OnBnClickedButtonCommstatus();
	afx_msg void OnBnClickedButtonReadfaults();
	afx_msg void OnHelpConfigurationguide();
	afx_msg void OnBnClickedButtonReadfile();
	afx_msg void OnBnClickedButtonWritefile();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnBnClickedButtonReboot();
	CEdit m_factoryOverrides;

	DWORD m_timerCounter;
	int m_inputRange;
	BOOL m_dcSamplingEnabled;
	BOOL m_acSamplingEnabled;
	float m_dcScaleFactor;
	float m_acScaleFactor;
	float m_dcOffset;
	float m_acOffset;
	afx_msg void OnBnClickedButtonSetrtc();
	CTextProgressCtrl m_memUseProgress;
	afx_msg void OnBnClickedButtonExtractdata();
	afx_msg void OnBnClickedButtonCleardata();
	afx_msg void OnBnClickedButtonSetfactory();
	afx_msg void OnBnClickedButtonAnalogtest();
	BOOL GetMeterReference(C_BIPWU *m_bip2Ptr, double *refVal, BOOL ac, char *range);
	afx_msg void OnBnClickedButtonCalibrate();
	BOOL m_tempSamplingEnabled;
	CEdit m_sampleIntervalHoursCtrl;
	CEdit m_sampleIntervalMinsCtrl;
	CEdit m_sampleIntervalSecsCtrl;

	int m_temperatureUnits;
	afx_msg void OnBnClickedButtonSetserial();
	int m_dcUnits;
	int m_acUnits;
	afx_msg void OnBnClickedButtonShuntcalcdc();
	afx_msg void OnBnClickedButtonShuntcalcac();
	afx_msg void OnBnClickedButtonCharttest();
	afx_msg void OnBnClickedButtonChartdc();
	afx_msg void OnBnClickedButtonChartac();
	afx_msg void OnBnClickedButtonCharttemp();
	int m_dateFormat;
	CButton m_timeFormatCtrl;
	afx_msg void OnDestroy();
	BOOL GetTZInfo(int &offsetSeconds, CString &label);
	BOOL GetDataLog(int &tzOffsetSeconds, int &tzUTCCompSeconds, CString &txStr);
	BOOL GenerateMasterFile(CString &filePath, int tzOffsetSeconds, int tzUTCCompSeconds, CString tzStr);
	BOOL GenerateSDLFile(CString filePath, int tzOffsetSeconds, int tzUTCCompSeconds, CString tzStr);
	afx_msg void OnBnClickedButtonReaddatafile();
	afx_msg void OnBnClickedButtonChartall();

	afx_msg void OnBnClickedButtonAnalogtemptest();
	BOOL m_dcWaveCaptureEnable;
	DWORD m_dcWaveCaptureRepetitionInterval;
	DWORD m_dcWaveCaptureDuration;
	BOOL m_requireGPSToStart;
	BOOL m_dcWaveCaptureContinuous;
	DWORD m_dcWaveCaptureSampleRate;
	BOOL m_noLongTermGPSSync;
	afx_msg void OnBnClickedButtonDlgtest();
	afx_msg void OnBnClickedButtonSplitdatafile();

	BOOL m_advancedUnit;
	BOOL m_g2Unit;
	CEdit m_sampleDelayMsCtrl;
	afx_msg void OnBnClickedButtonChartdcwave();
	afx_msg void OnBnClickedButton32khztest();
	BOOL m_timeDisplayEnabled;
	int m_timeDisplayFormat;
	void ProcessMessages();
	DWORD m_fastSyncOffsetMs;
	BOOL m_fastSyncEnable;
	BOOL m_minEHPEEnable;
	float m_minEHPEMeters;
	DWORD m_minEHPETimeoutMinutes;
	afx_msg void OnBnClickedButtonBinaryextract();
	DWORD m_lastFWWarningSerial;
	BOOL m_interruptionTrackingEnable;
	BOOL m_interruptionTrackingOnFirst;
	DWORD m_interruptionTrackingOffTimeMs;
	DWORD m_interruptionTrackingOnTime;
	DWORD m_interruptionTrackingCycleTime;
	DWORD m_interruptionTrackingOnToOffOffsetMs;
	DWORD m_interruptionTrackingOffToOnOffsetMs;
	int m_interruptionTrackingUTCOffsetMs;
	DWORD m_acquisitionDelayMins;
	CEdit m_interruptionTrackingCycleTimeMsCtrl;
	SdlNotes m_sdlNotes;
	afx_msg void OnEnKillfocusEditIntontime();
	afx_msg void OnEnKillfocusEditIntofftime();
	void ValidateInterruption();
	afx_msg void OnEnKillfocusEditIntontooffoffset();
	afx_msg void OnEnKillfocusEditIntofftoonoffset();
	afx_msg void OnEnKillfocusEditIntutcoffset();
	afx_msg void OnEnKillfocusEditIntStnSeries();
	afx_msg void OnEnKillfocusEditIntChainage();
	afx_msg void OnBnClickedButtonChartintdc();
	afx_msg void OnBnClickedButtonChartintac();
	afx_msg void OnBnClickedButtonMemtime();
	afx_msg void OnBnClickedButtonExtractSdldata();
	afx_msg void OnBnClickedButtonShuntcalcdc2();
	afx_msg void OnBnClickedButtonSdllocationnotes();
};
