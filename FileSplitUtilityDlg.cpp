// FileSplitUtilityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "udl1config.h"
#include "FileSplitUtilityDlg.h"
#include "afxdialogex.h"
#include "BusyDialog.h"
#include <CkCsv.h>


// CFileSplitUtilityDlg dialog

IMPLEMENT_DYNAMIC(CFileSplitUtilityDlg, CDialogEx)


void CFileSplitUtilityDlg::ProcessMessages()
{
	CWinApp* pApp = AfxGetApp();
	MSG msg;

		while ( PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE ))
				pApp->PumpMessage();
}

CFileSplitUtilityDlg::CFileSplitUtilityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileSplitUtilityDlg::IDD, pParent)
	, m_notesInAllOutput(TRUE)
	, m_verbalDates(TRUE)
	, m_excelDates(TRUE)
	, m_dcReadings(TRUE)
	, m_acReadings(TRUE)
	, m_tempReadings(TRUE)
	, m_dcWaveReadings(TRUE)
	, m_separateSessionFiles(TRUE)
	, m_includeUnits(TRUE)
{
	m_busy=FALSE;
}

CFileSplitUtilityDlg::~CFileSplitUtilityDlg()
{
}

void CFileSplitUtilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_INCLUDENOTES, m_notesInAllOutput);
	DDX_Check(pDX, IDC_CHECK_INCLUDEVERBALTIMEDATE, m_verbalDates);
	DDX_Check(pDX, IDC_CHECK_INCLUDEEXCELTIME, m_excelDates);
	DDX_Check(pDX, IDC_CHECK_INCLUDEDCREADINGS, m_dcReadings);
	DDX_Check(pDX, IDC_CHECK_INCLUDEACREADINGS, m_acReadings);
	DDX_Check(pDX, IDC_CHECK_INCLUDETEMPREADINGS, m_tempReadings);
	DDX_Check(pDX, IDC_CHECK_INCLUDETEMPREADINGS2, m_dcWaveReadings);
	DDX_Check(pDX, IDC_CHECK_INCLUDETEMPREADINGS3, m_separateSessionFiles);
	DDX_Check(pDX, IDC_CHECK_INCLUDEUNITS, m_includeUnits);
}


BEGIN_MESSAGE_MAP(CFileSplitUtilityDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SELECTFILES, &CFileSplitUtilityDlg::OnBnClickedButtonSelectfiles)
	ON_BN_CLICKED(IDC_BUTTON_CONVERTFILES, &CFileSplitUtilityDlg::OnBnClickedButtonConvertfiles)
END_MESSAGE_MAP()


BOOL CFileSplitUtilityDlg::OnInitDialog()
{
	UpdateData(FALSE);
	return FALSE;
}

// CFileSplitUtilityDlg message handlers


void CFileSplitUtilityDlg::OnBnClickedButtonSelectfiles()
{
	if (m_busy) return;

	CFileDialog dlgFile(TRUE);
	CString fileName;
	CString displayFiles="";
	POSITION fpos;
	
	const int c_cMaxFiles = 100;
	const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
	dlgFile.GetOFN().Flags|=OFN_ALLOWMULTISELECT;
	dlgFile.GetOFN().lpstrFile = fileName.GetBuffer(c_cbBuffSize);
	dlgFile.GetOFN().nMaxFile = c_cbBuffSize;
	dlgFile.GetOFN().lpstrFilter = "uDL1 Data Files (*.csv)\0*.csv\0";
	dlgFile.GetOFN().lpstrDefExt = ".csv";

	if( dlgFile.DoModal()==IDOK )
	{
		((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->ResetContent();
		fpos=dlgFile.GetStartPosition();
		if (fpos)
		{
			do 
			{
				((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->AddString(dlgFile.GetNextPathName(fpos));
			}
			while(fpos);
		}
		else
		{
			((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->ResetContent();
		}
	}
	else
	{
		((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->ResetContent();
	}
	fileName.ReleaseBuffer();
}


void CFileSplitUtilityDlg::OnOK()
{
	if (m_busy) return;
	CDialogEx::OnOK();
}


void CFileSplitUtilityDlg::OnCancel()
{
	if (m_busy) return;
	UpdateData(TRUE);
	CDialogEx::OnCancel();
}


void CFileSplitUtilityDlg::OnBnClickedButtonConvertfiles()
{
	if (m_busy) return;

	int fileCount,i;
	CString currentFileName;
	CString tempstr;

	m_busy=TRUE;

	UpdateData(TRUE);

	fileCount=((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->GetCount();

	if (!fileCount)
	{
		MessageBox("No files selected for conversion!","Error",MB_OK|MB_ICONERROR);
		m_busy=FALSE;
		return;
	}

	CBusyDialog dlg;
	dlg.Create(IDD_DIALOG_BUSYSTATUS,this);
	dlg.ShowWindow(SW_SHOW);
	dlg.m_captionTextCtrl.SetWindowTextA("Busy Processing Data.  Please Wait.");
	dlg.m_progressBarCtrl.SetPos(0);
	dlg.m_abortButtonCtrl.ShowWindow(TRUE);
	dlg.RedrawWindow(NULL,NULL,RDW_UPDATENOW);

	for (i=0; i<fileCount; i++)
	{
		tempstr.Format("Progress (File %li of %li)",i+1,fileCount);
		dlg.SetWindowTextA(tempstr);
		((CListBox*)GetDlgItem(IDC_LIST_FILELIST))->GetText(i,currentFileName);
		ProcessFile(currentFileName,dlg);
		//dlg.m_progressBarCtrl.SetPos((i*100)/fileCount);
		if (dlg.m_cancel)
		{
			dlg.ShowWindow(SW_HIDE);
			MessageBox("Processing Aborted","Info",MB_OK);
			m_busy=FALSE;
			return;
		}
	}

	dlg.ShowWindow(SW_HIDE);
	MessageBox("Done","Info",MB_OK);
	m_busy=FALSE;
}


#define COLUMN_TYPE_COUNT 19

#define COLUMN_RECORD_TYPE 0
#define COLUMN_VERBAL_DATE 1
#define COLUMN_VERBAL_MS 2
#define COLUMN_EXCEL_DATE 3
#define COLUMN_DC_READING 4
#define COLUMN_DC_UNITS 5
#define COLUMN_AC_READING 6
#define COLUMN_AC_UNITS 7
#define COLUMN_TEMP_READING 8
#define COLUMN_TEMP_UNITS 9
#define COLUMN_BATTERY 10
#define COLUMN_GPS_LAT 11
#define COLUMN_GPS_LONG 12
#define COLUMN_GPS_SATS 13
#define COLUMN_NOTES 14
#define COLUMN_TIMELOCK 15
#define COLUMN_EHPE 16
#define COLUMN_EHPELIMIT 17
#define COLUMN_INTSTATEON 18

typedef struct
{
	const char *name;
	BOOL required;
} T_COLUMN_NAMES;

const T_COLUMN_NAMES columnNames[COLUMN_TYPE_COUNT]=
{
	{"Record Type",1},
	{"Date/Time",1},
	{"Milliseconds",1},
	{"Excel Time",1},
	{"DC Reading",1},
	{"DC Units",1},
	{"AC Reading",1},
	{"AC Units",1},
	{"Temperature Reading",1},
	{"Temperature Units",1},
	{"Battery Potential",1},
	{"GPS Latitude",1},
	{"GPS Longitude",1},
	{"GPS Sats",1},
	{"Notes",1},
	{"Time Lock",1},
	{"GPS EHPE(m)",0},
	{"Limit EHPE(m)",0},
	{"Int State On",0}
};


#define OUTPUT_TYPE_COUNT 4
#define OUTPUT_TYPE_DC 0
#define OUTPUT_TYPE_AC 1
#define OUTPUT_TYPE_TEMP 2
#define OUTPUT_TYPE_DCWAVE 3

static char *outputTypeNames[OUTPUT_TYPE_COUNT]=
{
	"DC",
	"AC",
	"Temperature",
	"DCWave"
};

static char *outputTypeLabels[OUTPUT_TYPE_COUNT][2]=
{
	{"DC Reading","DC Reading (Int)"},
	{"AC Reading","AC Reading (Int)"},
	{"Temperature","Temperature"},
	{"DC Reading (Wave Capture)","DC Reading (Wave Capture)"}
};

BOOL CFileSplitUtilityDlg::ProcessFile(CString filename, CBusyDialog &busyDlg)
{
	  CkCsv csvin, csvout;
		CString outputFilename;
		bool success;
		int rowCount, columnCount;
		int columnInIndex[COLUMN_TYPE_COUNT];
		int columnInIndex2[COLUMN_TYPE_COUNT];
		int columnOutIndex[COLUMN_TYPE_COUNT];
		int columnOutIndex2[COLUMN_TYPE_COUNT];
		int i,j,k,session,outputType;
		int outputRowCount, outputColumnCount;
		BOOL dataInRow;
		CkString data, data2;
		BOOL newnote=FALSE;
		CString notes="";
		CString tempstring;
		DWORD lastMessageProcTime;

		busyDlg.m_progressBarCtrl.SetPos(0);
		tempstring.Format("Busy Processing Data From %s.\nPlease Wait.",filename.GetString());
		busyDlg.m_captionTextCtrl.SetWindowTextA(tempstring);

		lastMessageProcTime=GetTickCount();

    csvin.put_HasColumnNames(true);
		csvout.put_HasColumnNames(true);

		success = csvin.LoadFile(filename);
		if (!success)
		{
			MessageBox("Error loading CSV file " + filename,"Error",MB_OK|MB_ICONERROR);
			return FALSE;
		}

    rowCount = csvin.get_NumRows();
		columnCount = csvin.get_NumColumns();

		//Find the various column indices
		for (i=0; i<COLUMN_TYPE_COUNT; i++) columnInIndex[i]=-1;
		for (i=0; i<columnCount; i++)
		{
			for (j=0; j<COLUMN_TYPE_COUNT; j++)
			{
				if (strstr(csvin.getColumnName(i),columnNames[j].name)) columnInIndex[j]=i;
			}
		}
		//Verify that we have found all of the column headers
		for (i=0; i<COLUMN_TYPE_COUNT; i++)
		{
			if ((columnInIndex[j]==-1)&&columnNames[j].required)
			{
				MessageBox("Unable to parse data file.  Has it been modified after uDL1Config saved it?","Error",MB_OK);
				return FALSE;
			}
		}

		for (outputType=0; outputType<OUTPUT_TYPE_COUNT; outputType++)
		{ //DC,AC,TEMP,DCWAVE

			csvout.LoadFromString(""); //Wipe out the output buffer
			session=0;

			if ((outputType==OUTPUT_TYPE_DC)&&!m_dcReadings) continue; 
			if ((outputType==OUTPUT_TYPE_AC)&&!m_acReadings) continue; 
			if ((outputType==OUTPUT_TYPE_TEMP)&&!m_tempReadings) continue; 
			if ((outputType==OUTPUT_TYPE_DCWAVE)&&!m_dcWaveReadings) continue; 

			//Build an output map for the columns
			i=0;
			columnOutIndex[COLUMN_RECORD_TYPE]=-1;
			columnOutIndex[COLUMN_VERBAL_DATE]=m_verbalDates?i++:-1;
			columnOutIndex[COLUMN_VERBAL_MS]=m_verbalDates?i++:-1;
			columnOutIndex[COLUMN_EXCEL_DATE]=m_excelDates?i++:-1;
			columnOutIndex[COLUMN_DC_READING]=((outputType==OUTPUT_TYPE_DC)||(outputType==OUTPUT_TYPE_DCWAVE))?i++:-1;
			columnOutIndex[COLUMN_DC_UNITS]=m_includeUnits?((outputType==OUTPUT_TYPE_DC)||(outputType==OUTPUT_TYPE_DCWAVE))?i++:-1:-1;
			columnOutIndex[COLUMN_AC_READING]=(outputType==OUTPUT_TYPE_AC)?i++:-1;
			columnOutIndex[COLUMN_AC_UNITS]=m_includeUnits?(outputType==OUTPUT_TYPE_AC)?i++:-1:-1;
			columnOutIndex[COLUMN_TEMP_READING]=(outputType==OUTPUT_TYPE_TEMP)?i++:-1;
			columnOutIndex[COLUMN_TEMP_UNITS]=m_includeUnits?(outputType==OUTPUT_TYPE_TEMP)?i++:-1:-1;
			columnOutIndex[COLUMN_BATTERY]=-1;
			columnOutIndex[COLUMN_GPS_LAT]=-1;
			columnOutIndex[COLUMN_GPS_LONG]=-1;
			columnOutIndex[COLUMN_GPS_SATS]=-1;
			columnOutIndex[COLUMN_NOTES]=m_notesInAllOutput?i++:-1;
			columnOutIndex[COLUMN_TIMELOCK]=-1;
			columnOutIndex[COLUMN_EHPE]=-1;
			columnOutIndex[COLUMN_EHPELIMIT]=-1;
			columnOutIndex[COLUMN_INTSTATEON]=((outputType==OUTPUT_TYPE_DC)||(outputType==OUTPUT_TYPE_AC))?i++:-1;

			//Now add the column headers to the output CSV
			outputColumnCount=0;
			for (i=0; i<COLUMN_TYPE_COUNT; i++)
			{
				if (columnOutIndex[i]>=0)
				{
					columnOutIndex2[outputColumnCount]=columnOutIndex[i];
					columnInIndex2[outputColumnCount++]=columnInIndex[i];
					csvout.SetColumnName(columnOutIndex[i],columnNames[i].name);
				}
			}
			 
			outputRowCount=0;
			//Now build the output CSV file
			for(i=0; i<rowCount; i++)
			{
				//Pump the message loop and set the progress bar every 0.25s
				if ((GetTickCount()-lastMessageProcTime)>250)
				{
					lastMessageProcTime=GetTickCount();
					busyDlg.m_progressBarCtrl.SetPos(outputType*25+(i*25)/rowCount);
					ProcessMessages();
					if (busyDlg.m_cancel) return FALSE;
				}
				if (!(i%10000))
				{
					printf("Line: %li\r\n",i);
				}
				//Get the record type for this line
				csvin.GetCell(i,columnInIndex[COLUMN_RECORD_TYPE],data);
				if (strstr(data.getString(),"Notes"))
				{
					//csvin.GetCell(i,columnInIndex[COLUMN_NOTES],data2);
					//printf("Notes: %s\r\n",data2.getString());
					notes=csvin.getCell(i,columnInIndex[COLUMN_NOTES]);
					newnote=TRUE;
				}
				if (strstr(data.getString(),"Cable Inserted")&&outputRowCount&&m_separateSessionFiles)
				{
					printf("Rows: %lu, columns: %lu\r\n",csvout.get_NumRows(),csvout.get_NumColumns());
					//printf("CSV:\r\n%s",csvout.saveToString());
					outputFilename.Format("%s_session%li_%s.csv",filename,session,outputTypeNames[outputType]);
					DeleteFile(outputFilename);
					success = csvout.SaveFile(outputFilename);
					if (!success)
					{
						printf("%s\n",csvout.lastErrorText());
						MessageBox("Error saving CSV file " + filename,"Error",MB_OK|MB_ICONERROR);
						return FALSE;
					}
					csvout.LoadFromString(""); //Wipe out the output buffer
					csvout.put_HasColumnNames(true);
					//Set the column headers again after wiping the output buffer
					for (j=0; j<COLUMN_TYPE_COUNT; j++)
					{
						if (columnOutIndex[j]>=0)
						{
							csvout.SetColumnName(columnOutIndex[j],columnNames[j].name);
						}
					}
					outputRowCount=0;
					session++;
				}
				if (strcmp(data.getString(),outputTypeLabels[outputType][0])&&strcmp(data.getString(),outputTypeLabels[outputType][1])) continue; //Not the record type we're looking for
				for(j=0; j<outputColumnCount; j++)
				{
						if (columnInIndex2[j]>=0) //If no source, then don't set it in the output
						{
							if (!csvout.SetCell(outputRowCount,columnOutIndex2[j],csvin.getCell(i,columnInIndex2[j])))
							{
								printf("Error setting cell contents.(%li,%li)\r\n%s\r\n",outputRowCount,columnOutIndex2[j],csvout.lastErrorText());
							}
						}
				}
				//If it's the first line, output the last known notes value
				if (((!outputRowCount)||newnote)&&m_notesInAllOutput)
				{
					csvout.SetCell(outputRowCount,columnOutIndex[COLUMN_NOTES],notes);
					newnote=FALSE;
				}
				outputRowCount++;
			}
#if 0
			for (i=0; i<outputRowCount; i++)
			{
				for (j=0; j<csvout.get_NumColumns(); j++)
				{
					
					printf("%s,",csvout.getCell(i,columnInIndex[j]));
				}
				printf("\r\n");
			}
#endif
			if (outputRowCount)
			{
				printf("Rows: %lu, columns: %lu\r\n",csvout.get_NumRows(),csvout.get_NumColumns());
				//printf("CSV:\r\n%s",csvout.saveToString());
				if (m_separateSessionFiles)
					outputFilename.Format("%s_session%li_%s.csv",filename,session,outputTypeNames[outputType]);
				else
					outputFilename.Format("%s_%s.csv",filename,outputTypeNames[outputType]);
				DeleteFile(outputFilename);
				success = csvout.SaveFile(outputFilename);
				if (!success)
				{
					printf("%s\n",csvout.lastErrorText());
					MessageBox("Error saving CSV file " + filename,"Error",MB_OK|MB_ICONERROR);
					return FALSE;
				}
			}
		}

		return TRUE;
}