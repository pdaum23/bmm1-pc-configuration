// zoomscrolldemoDlg.h : header file
//

#if !defined(AFX_ZOOMSCROLLDEMODLG_H__1D289C1E_2C9E_43C6_B6F4_CDE438051F6C__INCLUDED_)
#define AFX_ZOOMSCROLLDEMODLG_H__1D289C1E_2C9E_43C6_B6F4_CDE438051F6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChartViewer.h"
#include <afxmt.h>

/////////////////////////////////////////////////////////////////////////////
// CZoomscrolldemoDlg dialog

#define MAX_WAVE_SEGMENTS 5000
typedef struct 
{
	UINT32 startIndex; 
	UINT32 endIndex;
} T_WAVE_SEGMENT;

const int sampleSize = 240;

class CZoomscrolldemoDlg : public CDialog
{
public:
// Construction
	CZoomscrolldemoDlg(CWnd* pParent = NULL);	// standard constructor
	~CZoomscrolldemoDlg();

// Dialog Data
	//{{AFX_DATA(CZoomscrolldemoDlg)
	enum { IDD = IDD_DIALOG_CHART };
	CDateTimeCtrl	m_StartDate;
	CComboBox	m_Duration;
	CScrollBar	m_VScrollBar;
	CScrollBar	m_HScrollBar;
	CButton	m_XZoomPB;
	CButton	m_PointerPB;
	CChartViewer	m_ChartViewer;
	MemBlock m_bmp;
	//}}AFX_DATA

	CString m_chartTitle;
	CString m_timeZoneStr;

	BOOL m_waveprint;
	UINT32 m_waveSegmentCurrent;
	T_WAVE_SEGMENT *m_waveSegments;
	UINT32 m_waveSegmentsCount;

	BOOL m_multiline;
	CString m_dataTitleA;
	DoubleArray m_timeStampsA;
	DoubleArray m_dataSeriesA;
	double m_dataSeriesAMinTime;
	double m_dataSeriesAMinVal;
	double m_dataSeriesAMaxTime;
	double m_dataSeriesAMaxVal;
	CString m_dataTitleB;
	DoubleArray m_timeStampsB;
	DoubleArray m_dataSeriesB;
	CString m_dataTitleC;
	DoubleArray m_timeStampsC;
	DoubleArray m_dataSeriesC;

	CPrintDialog *m_printDlgPtr;

	CFont m_fontForStatic;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomscrolldemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CZoomscrolldemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPointerPB();
	afx_msg void OnZoomInPB();
	afx_msg void OnZoomOutPB();
	afx_msg void OnXZoomPB();
	afx_msg void OnXYZoomPB();
	afx_msg void OnChartViewer();
	afx_msg void OnViewPortChanged();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDatetimechangeStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeDuration();
	afx_msg void OnKillfocusDuration();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

	// Handles pressing the "Enter" key
	virtual void OnOK();

private:
	
	int m_extBgColor;			// The default background color

	double m_minDate;			// The earliest date (in chartTime unit)
	double m_maxDate;		// The lastest date (in chartTime unit)
	double m_dateRange;			// The overall duration (in seconds)
	double m_maxValue;			// The maximum overall y scale
	double m_minValue;			// The minimum overall y scale
	
	double m_currentDuration;	// The visible duration of the view port (in seconds)
	double m_minDuration;		// The maximum zoom-in limit (in seconds)

	// Draw chart
	void drawChart(CChartViewer *viewer);
	// Update image map 
	void updateImageMap(CChartViewer *viewer);

	// Moves the scroll bar when the user clicks on it
	double moveScrollBar(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	// Validate the duration entered by the user
	void validateDuration(const CString &text);

	// utility to get default background color
	int getDefaultBgColor();
	// utility to load icon resource to a button
	void loadButtonIcon(int buttonId, int iconId, int width, int height);
public:
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonSavechart();
	afx_msg void OnBnClickedButtonZoomall();
	afx_msg void OnBnClickedButtonPrevwave();
	afx_msg void OnBnClickedButtonNextwave();
	afx_msg void OnBnClickedButtonFirstwave();
	afx_msg void OnBnClickedButtonLastwave();

	void UpdateWaveSegment();
	BOOL m_showMinMax;
	afx_msg void OnBnClickedCheckShowminmax();
	afx_msg void OnBnClickedCheckNodecimation();
	BOOL m_noDecimation;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZOOMSCROLLDEMODLG_H__1D289C1E_2C9E_43C6_B6F4_CDE438051F6C__INCLUDED_)
