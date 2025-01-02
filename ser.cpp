


#line 2 "ser.cpp"
/****************************************************************************
FILE: SER.CPP

DESCRIPTION: <DESCRIPTION>

COMPONENT: SER ( Serial Port Class )

LANGUAGE: C++

CONTENTS:

File              Procedure                  Description
----              ---------                  -----------
<FILENAME>        <FUNCTION>                 <DESCRIPTION>

NOTES:

(c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

/****************
* PREPROCESSOR *
****************/

/*****************
* INCLUDE FILES *
*****************/
#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <process.h>
#include <stdio.h>
#include "htype.h"
#include "SERiprot.hpp"

//using namespace std;
/***************************
* VARIABLES AND CONSTANTS *
***************************/

/**********************
* INTERNAL VARIABLES *
**********************/

/********************************
* EXTERNALLY VISIBLE VARIABLES *
********************************/

/********************************
* EXTERNALLY DEFINED VARIABLES *
********************************/



/****************************************************************************
FUNCTION: <FUNCTION NAME WITH SCOPE INDICATED>

DESCRIPTION: <FUNCTION DESCRIPTION>

FILE: SER.CPP

COMPONENT: SER ( Private Channel )

LANGUAGE: C++

AUTHOR: <AUTHOR> <DATE>

GLOBAL VARIABLES:

File              Variable          Description
----              --------          -----------
<FILENAME>       <VARIABLE>        <DESCRIPTION>

RETURNS: <TYPE> <DESCRIPTION>

NOTES:

IMPLEMENTATION DESCRIPTION: <DESCRIPTION>


(c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/


C_SER::C_SER()
{
	//printf("C_SER: Default constructor creating C_SER object\r\n");
	//Create a unique mutex name
	//sprintf(m_dataMutexName,"SER%08X",(HUINT32)this);
	//printf("Mutex: %s\r\n",m_dataMutexName);

	//Allocate a mutex handle
	m_dataMutex=CreateMutex( NULL,false, NULL);

	m_cbusOutputs=0; //All CBUS outputs low

	m_portNameStr="COM1";
	m_portBaud=115200;
	m_portHandle=NULL;
	m_dwError=ERROR_SUCCESS;

	m_callBackPtr=NULL;

	//
	// Create the exit signal event object
	//
	m_threadExitEvent = CreateEvent(NULL,		// Security
		TRUE,		// Manual reset
		FALSE,		// Initial State
		NULL);		// Name

	//
	// Create the thread Stopped signal event object
	//
	m_threadStoppedEvent = CreateEvent(NULL,		// Security
		TRUE,		// Manual reset
		TRUE,		// Initial State
		NULL);		// Name

	//
	// Create the receive enable signal event object
	//
	m_receiveDisableEvent = CreateEvent(NULL,		// Security
		TRUE,		// Manual reset
		TRUE,		// Initial State
		NULL);		// Name

}

C_SER::~C_SER()
{
	//printf("C_SER: Destructor destroying C_SER object\r\n");
	Close();
	CloseHandle(m_dataMutex);;
	//printf("C_SER: C_SER object destroyed\r\n");
}


HUINT32 C_SER::Open(void)
{
	HANDLE handle;
	BOOL fSuccess, deadFile;
	BOOL comPortType = FALSE;

	deadFile=FALSE;
	m_dwError=ERROR_SUCCESS;

	//m_portNameStr="MBTXUDL1"; //All uDL1 programming tools have the same serial number

	if (m_portHandle!=NULL) Close();

	//printf("Opening: %s\r\n",m_portNameStr);
	if (strstr(m_portNameStr, "COM"))
	{
		comPortType = TRUE;
	}

#define OVERLAPPEDIO
#ifdef OVERLAPPEDIO
	handle = CreateFile(m_portNameStr,
		FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL
	);


#else
	handle = CreateFile(m_portNameStr,
		dwDesiredAccess,
		dwShare,    // comm devices must be opened w/exclusive-access 
		NULL, // no security attributes 
		dwCreateType, //OPEN_EXISTING, // comm devices must use OPEN_EXISTING 
		0,    // overlapped I/O 
		NULL  // hTemplate must be NULL for comm devices 
	);
#endif

	if (handle == INVALID_HANDLE_VALUE)
	{
		m_dwError = GetLastError();
		printf("%s: Didn't accomplish CreateFile(Error=%u)\r\n", m_portNameStr, m_dwError);
		PrintError(m_dwError);
		deadFile = TRUE;
}


	if (comPortType)
	{
		//Set COM port parameters only on COM??????? type file names
		COMMTIMEOUTS hComTO;
		DCB dcb;

		if (!deadFile)
		{
			fSuccess = GetCommState(handle, &dcb);
			if (!fSuccess)
			{
				m_dwError = GetLastError();
				printf("%s: Didn't accomplish GetCommState(Error=%u)\r\n", m_portNameStr, m_dwError);
				//PrintError();
				deadFile = TRUE;
			}
		}

		if (!deadFile)
		{
			dcb.BaudRate = m_portBaud;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;
			dcb.StopBits = ONESTOPBIT;
			dcb.fBinary = 1;
			dcb.fNull = 0;
			dcb.fAbortOnError = 0;
			dcb.EofChar = (HUINT8)0xFF;
			dcb.EvtChar = (HUINT8)0xFF;
			dcb.fOutxCtsFlow = 0;
			dcb.fOutxDsrFlow = 0;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
			dcb.fRtsControl = RTS_CONTROL_ENABLE;

			fSuccess = SetCommState(handle, &dcb);
			fSuccess = GetCommState(handle, &dcb);

			if (!fSuccess)
			{
				m_dwError = GetLastError();
				printf("%s: Didn't accomplish SetCommState(Error=%u)\r\n", m_portNameStr, m_dwError);
				//PrintError();
				deadFile = TRUE;
			}
		}

		if (!deadFile)
		{
			//Set buffer properties
			fSuccess = SetupComm(handle, 16384 * 8, 16384 * 8);

			if (!fSuccess)
			{
				m_dwError = GetLastError();
				printf("%s: Didn't accomplish SetupComm(Error=%u)\r\n", m_portNameStr, m_dwError);
				//PrintError();
				deadFile = TRUE;
			}
		}

		if (!deadFile)
		{
			//No total timeouts for read or write
			//10ms intercharacter timeout

			hComTO.ReadIntervalTimeout = MAXDWORD;
			hComTO.ReadTotalTimeoutMultiplier = MAXDWORD;
			hComTO.ReadTotalTimeoutConstant = 50;

			hComTO.WriteTotalTimeoutMultiplier = 0;
			hComTO.WriteTotalTimeoutConstant = 0;

			fSuccess = SetCommTimeouts(handle, &hComTO);

			if (!fSuccess)
			{
				m_dwError = GetLastError();
				printf("%s: Didn't accomplish SetCommTimeouts(Error=%u)\r\n", m_portNameStr, m_dwError);
				//PrintError();
				deadFile = TRUE;
			}
		}

		if (!deadFile)
		{
			fSuccess = SetCommMask(handle, 0); //Ignore all events 

			if (!fSuccess)
			{
				m_dwError = GetLastError();
				printf("%s: Didn't accomplish SetCommMask(Error=%u)\r\n", m_portNameStr, m_dwError);
				//PrintError();
				deadFile = TRUE;
			}
		}
	}
	if (!deadFile)
	{
		HUINT32 dwClientThread;

		//Reset thread exit prior to running thread
		ResetEvent(m_threadExitEvent);
		//Don't start receiving until specifically asked to do so
		SetEvent(m_receiveDisableEvent);

		//See if thread is already running
		if (WaitForSingleObject(m_threadStoppedEvent, 0) == WAIT_OBJECT_0)
		{
			dwClientThread = _beginthread(CIteration, 0, (void*)this);
			if (dwClientThread == 0xFFFFFFFF)
			{
				printf("SER: Could not create thread!\r\n");
			}
			else
			{
				if (!SetThreadPriority((HANDLE)dwClientThread, THREAD_PRIORITY_HIGHEST))
				{
					HINT32 dwError;
					//HCHAR buffer[100];

					dwError = GetLastError();

					//_snprintf(buffer, 99, "SER: Couln't set thread priority to HIGH (handle=%08X,error=%ld)",dwClientThread,dwError);

					//MessageBeep(0xFFFFFFFF); 
					//MessageBeep(MB_ICONEXCLAMATION); 
					//MessageBox( NULL, buffer, "CMS Proxy Error",MB_ICONEXCLAMATION);  
				}
			}
		}
		else
		{
			printf("C_SER: Thread Already Running\r\n");
		}

		m_portHandle = handle;
		return ERROR_SUCCESS;
	}
	m_portHandle = NULL;
	return m_dwError;
}

HUINT32 C_SER::Close(void)
{
	BOOL fSuccess;

	//printf("Closing C_SER\r\n");

	//Lock out other threads from closing at same time
	WaitForSingleObject( m_dataMutex,INFINITE);

	if (m_portHandle)
	{

		//Stop receiving
		SetEvent(m_receiveDisableEvent);

		//Stop thread if running
		if (WaitForSingleObject(m_threadStoppedEvent,0)==WAIT_TIMEOUT)
		{
			//printf("C_SER: Attempting to shutdown C_SER Thread\r\n");
			SetEvent(m_threadExitEvent);

			if (WaitForSingleObject(m_threadStoppedEvent,10000)==WAIT_TIMEOUT)
			{
				printf("C_SER: Couldn't kill off C_SER thread\r\n");
			}
			else
			{
				//printf("C_SER: Killed off C_SER thread\r\n");
			}
		}

		m_dwError=ERROR_SUCCESS;


		fSuccess = CloseHandle(m_portHandle);

		if (!fSuccess)
		{
			m_dwError = 6;
			printf("C_SER: Didn't accomplish CloseHandle on HANDLE %llu(Error=%u)\r\n", (LONGLONG)m_portHandle,m_dwError);
			//PrintError();
		}

		m_portHandle=NULL;

	}
	else
	{
		//printf("C_SOC: Handle already closed\r\n");
	}

	ReleaseMutex(m_dataMutex);

	m_dwError=ERROR_SUCCESS;
	return m_dwError;
}

HUINT32 C_SER::Options(TCHAR *portNameStr_, DWORD portBaud_)
{
	m_dwError=ERROR_SUCCESS;
	m_portNameStr=portNameStr_;
	m_portBaud=portBaud_;
	if (m_portHandle!=NULL)
	{
		Close();
		Sleep(200L);
	}
	if ((m_dwError = Open()) != ERROR_SUCCESS)
	{
		return m_dwError;
	}
	UseXONXOFF(FALSE);
	Dtr(FALSE);
	Rts(FALSE);
	//Cbus(0,FALSE);
	return m_dwError;
}


HUINT32 C_SER::UseXONXOFF(HBOOL state_)
{
	BOOL fSuccess;

	m_dwError=ERROR_SUCCESS;

	//Check to see if port is open;
	if (m_portHandle==NULL) {m_dwError=1; return m_dwError;};


	//Set COM port parameters only on COM??????? type file names
	DCB dcb;

	fSuccess = GetCommState(m_portHandle, &dcb);
	if (!fSuccess)
	{
		m_dwError = GetLastError();
		printf("%s: Didn't accomplish GetCommState(Error=%u)\r\n",m_portNameStr,m_dwError);
		//PrintError();
		return m_dwError;
	}

	if (state_)
	{
		dcb.XonChar=0x11;
		dcb.XoffChar=0x13;
		dcb.fOutX=TRUE;

	}
	else
	{
		dcb.fOutX=FALSE;

	}

	//dcb.BaudRate = m_portBaud;
	//dcb.ByteSize = 8;
	//dcb.Parity = NOPARITY;
	//dcb.StopBits = ONESTOPBIT;
	//dcb.fBinary=1;
	//dcb.fNull=0;
	//dcb.fAbortOnError=0;
	//dcb.EofChar=(HUINT8)0xFF;
	//dcb.EvtChar=(HUINT8)0xFF;
	//dcb.fOutxCtsFlow = 0;
	//dcb.fOutxDsrFlow = 0;

	fSuccess = SetCommState(m_portHandle, &dcb);
	fSuccess = GetCommState(m_portHandle, &dcb);

	if (!fSuccess) 
	{
		m_dwError = GetLastError();
		printf("%s: Didn't accomplish SetCommState(Error=%u)\r\n",m_portNameStr,m_dwError);
		//PrintError();
	}

	return m_dwError;
}

HUINT32 C_SER::Dtr(HBOOL state_)
{
	m_dwError=ERROR_SUCCESS;
	if (m_portHandle!=NULL)
	{
		if (state_)
		{
			EscapeCommFunction(m_portHandle,SETDTR);
			//FT_SetDtr(m_portHandle);
		}
		else
		{
			EscapeCommFunction(m_portHandle,CLRDTR);
			//FT_ClrDtr(m_portHandle);
		}
	}
	return m_dwError;
}

/*
UINT32 C_SER::Cbus(UINT8 index, HBOOL state_) 
{
	m_dwError=ERROR_SUCCESS;
	if ((m_portHandle!=NULL)&&(index<4))
	{
		if (state_) 
			m_cbusOutputs|=(1<<index);
		else
			m_cbusOutputs&=~(1<<index);
		m_cbusOutputs|=0xF0; //Set all as outputs
		FT_SetBitMode(m_portHandle, m_cbusOutputs, 0x20);
	}
	return m_dwError;
}
*/

HUINT32 C_SER::Rts(HBOOL state_)
{
	m_dwError=ERROR_SUCCESS;
	if (m_portHandle!=NULL)
	{
		if (state_)
		{
			EscapeCommFunction(m_portHandle,SETRTS);
			//FT_SetRts(m_portHandle);
		}
		else
		{
			EscapeCommFunction(m_portHandle,CLRRTS);
			//FT_ClrRts(m_portHandle);
		}
	}
	return m_dwError;
}


HUINT32 C_SER::Break(HBOOL state_)
{
	m_dwError=ERROR_SUCCESS;
	if (m_portHandle!=NULL)
	{
		if (state_)
		{
			EscapeCommFunction(m_portHandle,SETBREAK);
		}
		else
		{
			EscapeCommFunction(m_portHandle,CLRBREAK);
		}
	}
	return m_dwError;
}


HUINT32 C_SER::PortStatus(BYTE *dsr, BYTE *cts, BYTE *ri, BYTE *dcd)
{
	m_dwError=ERROR_SUCCESS;
	DWORD dwCommModemStatus;

	if (m_portHandle != NULL)
	{
		// Retrieve modem control-register values.
		GetCommModemStatus (m_portHandle, &dwCommModemStatus);
	}
	// Set the indicator lights.
	if (dsr) *dsr=0;
	if (cts) *cts=0;
	if (ri) *ri=0;
	if (dcd) *dcd=0;

	if ((dwCommModemStatus&MS_DSR_ON)&&dsr) *dsr=1;
	if ((dwCommModemStatus&MS_CTS_ON)&&cts) *cts=1;
	if ((dwCommModemStatus&MS_RING_ON)&&ri) *ri=1;
	if ((dwCommModemStatus&MS_RLSD_ON)&&dcd) *dcd=1;

	return m_dwError;
}


HUINT32 C_SER::BaudSet(DWORD portBaudRate)
{
	BOOL fSuccess;

	m_dwError=ERROR_SUCCESS;

	//Check to see if port is open;
	if (m_portHandle==NULL) {m_dwError=1; return m_dwError;};


	//Set COM port parameters only on COM??????? type file names
	DCB dcb;

	m_portBaud=portBaudRate;
	if (m_portBaud==0) m_portBaud=9600;

	fSuccess = GetCommState(m_portHandle, &dcb);
	if (!fSuccess)
	{
		m_dwError = GetLastError();
		printf("%s: Didn't accomplish GetCommState(Error=%u)\r\n",m_portNameStr,m_dwError);
		//PrintError();
		return m_dwError;
	}

	dcb.BaudRate = m_portBaud;
	//dcb.ByteSize = 8;
	//dcb.Parity = NOPARITY;
	//dcb.StopBits = ONESTOPBIT;
	//dcb.fBinary=1;
	//dcb.fNull=0;
	//dcb.fAbortOnError=0;
	//dcb.EofChar=(HUINT8)0xFF;
	//dcb.EvtChar=(HUINT8)0xFF;
	//dcb.fOutxCtsFlow = 0;
	//dcb.fOutxDsrFlow = 0;

	fSuccess = SetCommState(m_portHandle, &dcb);
	fSuccess = GetCommState(m_portHandle, &dcb);

	if (!fSuccess) 
	{
		m_dwError = GetLastError();
		printf("%s: Didn't accomplish SetCommState(Error=%u)\r\n",m_portNameStr,m_dwError);
		//PrintError();
	}

	return m_dwError;
}


HUINT32 C_SER::Write(HUINT8 *dataPtr_, HUINT32 requestSize_, HUINT32 *numWrittenPtr_)
{
#ifdef OVERLAPPEDIO
	if (m_portHandle)
	{

		OVERLAPPED	   over;
		HUINT32			dwSent;
		HUINT32			dwRet;
		HANDLE			hEvents[2];
		BOOL	   		fPending;
		HINT32			nRet;

		//
		// Setup the OVERLAPPED structure
		//
		over.hEvent = CreateEvent(NULL,   // Not inherited 
			TRUE,   // Manual reset 
			FALSE,  // Initially reset 
			NULL ); // No name 
		over.Offset = 0;
		over.OffsetHigh = 0;

		if (numWrittenPtr_) *numWrittenPtr_=0;

		fPending = FALSE;
		nRet=WriteFile(m_portHandle,  // handle to file to write to
			dataPtr_,      // pointer to data to write to file
			requestSize_,  // number of bytes to write
			&dwSent,       // pointer to number of bytes written
			&over);        // pointer to structure for overlapped I/O
		if (!nRet)
		{
			if (GetLastError() == ERROR_IO_PENDING)
				fPending = TRUE;
			else
			{
				m_dwError=GetLastError();
				CloseHandle(over.hEvent);
				return m_dwError;
			}
		}

		//
		// If the I/O isn't finished...
		//
		if (fPending)
		{
			//
			// Wait for the request to complete 
			// or the exit event to be signaled
			//
			hEvents[0]  = over.hEvent;
			hEvents[1]  = m_threadExitEvent;
			dwRet = WaitForMultipleObjects(2,
				hEvents,
				FALSE,
				INFINITE);
			//
			// Was the thread exit event signaled?
			//
			if ((dwRet - WAIT_OBJECT_0) == 1)
			{
				CloseHandle(over.hEvent);
				m_dwError=ERROR_SUCCESS;
				return m_dwError;
			}

			//
			// Get I/O result
			//
			if (!GetOverlappedResult(m_portHandle,
				&over,
				&dwSent,
				FALSE))
			{
				CloseHandle(over.hEvent);
				m_dwError=GetLastError();
				return m_dwError;
			}
		}

		if (numWrittenPtr_) *numWrittenPtr_=dwSent;
		CloseHandle(over.hEvent);
		m_dwError=ERROR_SUCCESS;
		return m_dwError;
	}
	else
	{
		if (numWrittenPtr_) *numWrittenPtr_=0;
		m_dwError=ERROR_SUCCESS;
		return m_dwError;
	}
#else
	HINT32			nRet;

	if (m_portHandle)
	{
		nRet=FT_W32_WriteFile(m_portHandle,  // handle to file to write to
			dataPtr_,      // pointer to data to write to file
			requestSize_,  // number of bytes to write
			numWrittenPtr_,       // pointer to number of bytes written
			NULL);        // pointer to structure for overlapped I/O
		if (!nRet)
		{
			m_dwError=GetLastError();
			PrintError(m_dwError);
			return m_dwError;
		}

		m_dwError=ERROR_SUCCESS;
		return m_dwError;
	}
	else
	{
		if (numWrittenPtr_) *numWrittenPtr_=0;
		m_dwError=ERROR_SUCCESS;
		return m_dwError;
	}

#endif
}


HUINT32 C_SER::Read(HUINT8 *dataPtr_, HUINT32 requestSize_, HUINT32 *numReadPtr_)
{
#ifndef OVERLAPPEDIO
	DWORD	fdwCommMask;

	fdwCommMask=EV_RXCHAR;
	FT_W32_SetCommMask (m_portHandle, EV_RXCHAR);
	//FT_W32_WaitCommEvent (m_portHandle, &fdwCommMask, NULL);
	//FT_W32_SetCommMask (m_portHandle, EV_RXCHAR);

	// read the next block of data
	if(!FT_W32_ReadFile(m_portHandle,			// where to read from (the open comm port)
		dataPtr_,					// where to put the character
		requestSize_,						// number of bytes to read
		numReadPtr_,			// how many bytes were actually read
		NULL))					// overlapped I/O not supported
	{
		m_dwError=GetLastError();
		if (m_dwError != ERROR_INVALID_HANDLE) // don't print error if port closed
		{
			PrintError(m_dwError);
			return 0;		// terminate thread on first error or port closed
		}
	}
	// add it to the text box
	else if(*numReadPtr_ > 0)
	{
		return ERROR_SUCCESS;
	}
#else
	if (m_portHandle)
	{
		OVERLAPPED	   over;
		HUINT32			dwRecv;
		HUINT32			dwRet;
		HANDLE   		hEvents[3];
		HBOOL		   	fPending;
		HINT32		   nRet;

		//
		// Setup the OVERLAPPED structure
		//
		over.hEvent = CreateEvent(NULL,   // Not inherited 
			TRUE,   // Manual reset 
			FALSE,  // Initially reset 
			NULL ); // No name 

		over.Offset = 0;
		over.OffsetHigh = 0;

		*numReadPtr_=0;
		fPending = FALSE;

		nRet = ReadFile(m_portHandle, // handle of file to read
			dataPtr_,     // pointer to buffer that receives data
			requestSize_, // number of bytes to read
			&dwRecv,      // pointer to number of bytes read
			&over );      // pointer to structure for data


		if (!nRet)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				CloseHandle(over.hEvent);
				m_dwError=GetLastError();
				return m_dwError;
			}
			else
				fPending = TRUE;
		}

		//
		// If the I/O isn't finished...
		//
		if (fPending)
		{
			//
			// Wait for the request to complete or the exit event 
			//
			hEvents[0]  = over.hEvent;
			hEvents[1]  = m_threadExitEvent;
			//hEvents[2]  = m_receiveDisableEvent;
			dwRet = WaitForMultipleObjects(2,
				hEvents,
				FALSE,
				INFINITE);
			//
			// Was the thread exit event signaled?
			//
			if ((dwRet - WAIT_OBJECT_0) == 1)
			{
				CloseHandle(over.hEvent);
				m_dwError=ERROR_SUCCESS;
				return m_dwError;
			}
			else if ((dwRet - WAIT_OBJECT_0) == 2)
			{
				CloseHandle(over.hEvent);
				m_dwError=ERROR_SUCCESS;
				*numReadPtr_=0;
				return m_dwError;
			}
			else
				if (!GetOverlappedResult(m_portHandle,
					&over,
					&dwRecv,
					FALSE))
				{
					CloseHandle(over.hEvent);
					m_dwError=GetLastError();
					return m_dwError;
				}
		}

		//
		// Recv event is complete
		//
		*numReadPtr_=dwRecv;
		CloseHandle(over.hEvent);
		m_dwError=ERROR_SUCCESS;
		return m_dwError;
	}
	else
	{
		m_dwError=ERROR_SUCCESS;
		*numReadPtr_=0;
		return m_dwError;
	}
#endif
	return ERROR_SUCCESS;
}



HBOOL inline C_SER::IsOpen(void)
{
	if (m_portHandle!=NULL)
		return TRUE;
	else
		return FALSE;
}

void * C_SER::GetCurrentCallBackOwner(void)
{
	return m_callBackID;
}

HBOOL C_SER::AddReceiveCB(T_SER_CALLBACK routinePtr_, void *callBackIDPtr_)
{
	m_callBackPtr=routinePtr_;
	m_callBackID=callBackIDPtr_;
	return TRUE;
}

HBOOL C_SER::ReceiveEnable(HBOOL state_)
{
	if (state_)
	{
		ResetEvent(m_receiveDisableEvent);
	}
	else
	{
		SetEvent(m_receiveDisableEvent);
	}
	return TRUE;
}

void C_SER::CIteration(void * dwUser_)
{
	//printf("\rSER Thread Started.\r\n");
	((C_SER*)dwUser_)->Iteration();
	//printf("\rSER Thread Ended.\r\n");
	//_endthread();
}

void C_SER::Iteration(void)
{
	HUINT8 *localBufferPtr= new HUINT8[8192];
	HUINT32 read;

	ResetEvent(m_threadStoppedEvent);

	while (WaitForSingleObject(m_threadExitEvent,0)==WAIT_TIMEOUT)
	{
		if (WaitForSingleObject(m_receiveDisableEvent,0)==WAIT_TIMEOUT)
		{
			//printf("X");
			Sleep(10);
			m_dwError=Read(localBufferPtr, 8192, &read);
			//printf("<blip>");

			//printf("x");

			if (m_dwError==ERROR_SUCCESS)
			{
				//printf("<blip2>");
				if (WaitForSingleObject(m_receiveDisableEvent,0)==WAIT_TIMEOUT)
					(*m_callBackPtr)(m_callBackID,localBufferPtr,read);
				//m_cttPtr->ReceiveCB(localBufferPtr,read);
			}
			else
			{
				//printf("<blip3>");
				//printf("Reading from a COM port died, Error Code = %lu\r\n",m_dwError);

				//Signal back closure of port
				if (m_callBackPtr)
				{
					(*m_callBackPtr)(m_callBackID, NULL, 0);
				}
				//m_cttPtr->ReceiveCB(NULL,0);
			}
		}
		else
		{
			Sleep(250);
		}
	}
	delete[] localBufferPtr;
	//printf("C_SER Thread Almost Dead\r\n");
	SetEvent(m_threadStoppedEvent);
}


void C_SER::PrintError(DWORD errorCode)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL );// Process any inserts in lpMsgBuf.

	// ...// Display the string.
	printf("Error(%lu): %s\r\n",errorCode,lpMsgBuf);

	// Free the buffer.
	LocalFree( lpMsgBuf );
}



