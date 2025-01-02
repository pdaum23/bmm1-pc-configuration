#define _CRT_SECURE_NO_DEPRECATE 
#define _CRT_NON_CONFORMING_SWPRINTFS
//#define _USE_32BIT_TIME_T
//#define _WIN32_WINDOWS 0x0500
//#define NTDDI_VERSION NTDDI_WINXP 

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#ifdef _DEBUG

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <tchar.h>
#include <WinIoCtl.h>
#include <setupapi.h>
#include <dbt.h>
#include <strsafe.h>
#include <process.h>

#include "winusbif.hpp"

GUID gRDU1GUID=    {0xE6603915, 0x7FEF, 0x4768, {0x98, 0xB6, 0x7C, 0xAC, 0x53, 0x01, 0xAF, 0x93}};
GUID gRMU2SDL1GUID={0x697BE4AB, 0xA0A1, 0x4834, {0xB7, 0x9E, 0xAA, 0x65, 0x21, 0x90, 0xA5, 0x04}};

C_WINUSB::C_WINUSB()
{
	//Allocate a mutex handle
	m_dataMutex=CreateMutex( NULL,false, NULL);

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


	m_deviceHandle = INVALID_HANDLE_VALUE;
	m_winUSBHandle = NULL;
	m_winUSBHandle2 = NULL;

	//memcpy(&m_classGUID,&gRDU1GUID,sizeof(GUID));
	memcpy(&m_classGUID,&gRMU2SDL1GUID,sizeof(GUID));
}

C_WINUSB::~C_WINUSB()
{
	Close();
	CloseHandle(m_dataMutex);;
}


UINT32 C_WINUSB::Open(GUID classGUID)
{
	BOOL deadFile;
	DWORD dwError=ERROR_SUCCESS;

	m_classGUID=classGUID;

	if (m_deviceHandle)
	{
		Close();
	}

	deadFile=!Initialize_Device();

	if (!deadFile)
	{
		uintptr_t dwClientThread;

		//Reset thread exit prior to running thread
		ResetEvent(m_threadExitEvent);
		//Don't start receiving until specifically asked to do so
		SetEvent(m_receiveDisableEvent);

		//See if thread is already running
		if (WaitForSingleObject(m_threadStoppedEvent,0)==WAIT_OBJECT_0)
		{
			dwClientThread=_beginthread( CIteration, 0, (void *) this);  
			if (dwClientThread==0xFFFFFFFF)
			{
				printf("WINUSB: Could not create thread!\r\n");
		 }
			else
			{
				if (!SetThreadPriority((HANDLE)dwClientThread,THREAD_PRIORITY_HIGHEST))
				{
					UINT32 dwError;
					dwError=GetLastError();
				}
			}
		}
		else
		{
			//printf("C_WINUSB: Thread Already Running\r\n"); 
		}
		return ERROR_SUCCESS;
	}
	else
	{
		m_dwError=ERROR_GEN_FAILURE;
	}
	return m_dwError;
}

UINT32 C_WINUSB::Close(void)
{
	BOOL fSuccess;

	//printf("Closing C_SER\r\n");

	//Lock out other threads from closing at same time
	WaitForSingleObject( m_dataMutex,INFINITE);

	if (m_deviceHandle!=INVALID_HANDLE_VALUE)
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
				printf("C_WINUSB: Couldn't kill off C_SER thread\r\n");
			}
			else
			{
				//printf("C_SER: Killed off C_SER thread\r\n");
			}
		}

		CloseDevice();

		m_dwError=ERROR_SUCCESS;
	}
	else
	{
		//printf("C_WINUSB: Handle already closed\r\n");
	}

	ReleaseMutex(m_dataMutex);

	m_dwError=ERROR_SUCCESS;
	return m_dwError;
}

UINT32 C_WINUSB::Write(UINT8 *dataPtr_, UINT32 requestSize_, UINT32 *numWrittenPtr_)
{
	if (WriteToDevice(dataPtr_, requestSize_, (ULONG *)numWrittenPtr_))
		return ERROR_SUCCESS;
	else
		return ERROR_GEN_FAILURE;
}

UINT32 C_WINUSB::Read(UINT8 *dataPtr_, UINT32 requestSize_, UINT32 *numReadPtr_)
{
	if (ReadFromDevice(dataPtr_, requestSize_, (ULONG *)numReadPtr_))
		return ERROR_SUCCESS;
	else
		return ERROR_GEN_FAILURE;
}

BOOL C_WINUSB::IsOpen(void)
{
	if (m_deviceHandle!=INVALID_HANDLE_VALUE) return TRUE; else return FALSE;
}

BOOL C_WINUSB::AddReceiveCB(T_WINUSB_CALLBACK routinePtr_, void *callBackIDPtr_)
{
	m_callBackPtr=routinePtr_;
	m_callBackID=callBackIDPtr_;
	return TRUE;
}

BOOL C_WINUSB::ReceiveEnable(BOOL state_)
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


void C_WINUSB::CIteration(void * dwUser_)
{
	((C_WINUSB *)dwUser_)->Iteration();
}

void C_WINUSB::Iteration(void)
{
	UINT8 localBufferPtr[64];
	UINT32 read;

	ResetEvent(m_threadStoppedEvent);

	while (WaitForSingleObject(m_threadExitEvent,0)==WAIT_TIMEOUT)
	{
		if (WaitForSingleObject(m_receiveDisableEvent,0)==WAIT_TIMEOUT)
		{
			//Sleep(10); //Rely on the read function time out to pace us
			m_dwError=Read(localBufferPtr, 64, &read);

			if (m_dwError==ERROR_SUCCESS)
			{
				if (WaitForSingleObject(m_receiveDisableEvent,0)==WAIT_TIMEOUT)
					(*m_callBackPtr)(m_callBackID,localBufferPtr,read);
			}
			else
			{
				//Signal back closure of port
				if (m_callBackPtr) (*m_callBackPtr)(m_callBackID,NULL,0);
				//m_cttPtr->ReceiveCB(NULL,0);
			}
		}
		else
		{
			Sleep(250);
		}
	}
	SetEvent(m_threadStoppedEvent);
}


BOOL C_WINUSB::GetDevicePath(LPGUID InterfaceGuid,
														 PCHAR DevicePath,
														 size_t BufLen)
{
	BOOL bResult = FALSE;
	HDEVINFO deviceInfo;
	SP_DEVICE_INTERFACE_DATA interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	ULONG length;
	ULONG requiredLength=0;
	HRESULT hr;
	DWORD dwError;

	deviceInfo = SetupDiGetClassDevs(InterfaceGuid,
		NULL, NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfo==INVALID_HANDLE_VALUE)
		return FALSE;

	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	bResult = SetupDiEnumDeviceInterfaces(deviceInfo,
		NULL,
		InterfaceGuid,
		0,
		&interfaceData);
	if (!bResult)
	{
		dwError=GetLastError();
		printf("SetupDiEnumDeviceInterfaces failed (%08lu)\r\n",dwError);
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return FALSE;
	}

	bResult=SetupDiGetDeviceInterfaceDetail(deviceInfo,
		&interfaceData,
		NULL, 0,
		&requiredLength,
		NULL);
	if (!bResult)
	{
		dwError=GetLastError();
		if (dwError!=ERROR_INSUFFICIENT_BUFFER)
		{
			printf("SetupDiGetDeviceInterfaceDetail failed (%08lu)\r\n",dwError);
			SetupDiDestroyDeviceInfoList(deviceInfo);
			return FALSE;
		}
	}

	detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
		LocalAlloc(LMEM_FIXED, requiredLength);

	if(NULL == detailData)
	{
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return FALSE;
	}

	detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	length = requiredLength;

	bResult = SetupDiGetDeviceInterfaceDetail(deviceInfo,
		&interfaceData,
		detailData,
		length,
		&requiredLength,
		NULL);

	if(FALSE == bResult)
	{
		SetupDiDestroyDeviceInfoList(deviceInfo);
		LocalFree(detailData);
		return FALSE;
	}

	hr = StringCchCopy(DevicePath,
		BufLen,
		detailData->DevicePath);
	if(FAILED(hr))
	{
		SetupDiDestroyDeviceInfoList(deviceInfo);
		LocalFree(detailData);
		return FALSE;
	}

	LocalFree(detailData);

	return bResult;
}

HANDLE C_WINUSB::OpenDevice(BOOL bSync)
{
	HANDLE hDev = NULL;
	char devicePath[MAX_DEVPATH_LENGTH];
	BOOL retVal;

	retVal = GetDevicePath( (LPGUID) &m_classGUID,
		devicePath,
		sizeof(devicePath));

	if (!retVal) return FALSE;


	hDev = CreateFile(devicePath,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);


	return hDev;
}

void C_WINUSB::CloseDevice(void)
{
	if ( m_winUSBHandle )
	{
		WinUsb_Free( m_winUSBHandle );
	}
	if ( m_winUSBHandle2 )
	{
		WinUsb_Free( m_winUSBHandle2 );
	}

	if ( m_deviceHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_deviceHandle );
	}

	m_deviceHandle = INVALID_HANDLE_VALUE;
	m_winUSBHandle = NULL;
	m_winUSBHandle2 = NULL;
}

BOOL C_WINUSB::Initialize_Device()
{
	BOOL bResult;

	USB_INTERFACE_DESCRIPTOR ifaceDescriptor;
	WINUSB_PIPE_INFORMATION pipeInfo;
	UCHAR speed;
	ULONG length;

	m_deviceHandle = OpenDevice(TRUE);
	if (m_deviceHandle==INVALID_HANDLE_VALUE) return FALSE;

	bResult = WinUsb_Initialize(m_deviceHandle, &m_winUSBHandle2);
	if (!bResult)
	{
		CloseDevice();
		return FALSE;
	}

	bResult = WinUsb_GetAssociatedInterface (m_winUSBHandle2, 0, &m_winUSBHandle);
	if (!bResult)
	{
		CloseDevice();
		return FALSE;
	}

	if(bResult)
	{
		length = sizeof(UCHAR);
		bResult = WinUsb_QueryDeviceInformation(m_winUSBHandle,
			DEVICE_SPEED,
			&length,
			&speed);
	}


	if(bResult)
	{
		m_deviceSpeed = speed;
		bResult = WinUsb_QueryInterfaceSettings(m_winUSBHandle,
			0,
			&ifaceDescriptor);
	}
	else
	{
		CloseDevice();
		return FALSE;
	}




	if(bResult)
	{
		for(int i=0;i<ifaceDescriptor.bNumEndpoints;i++)
		{
			bResult = WinUsb_QueryPipe(m_winUSBHandle,
				0,
				(UCHAR) i,
				&pipeInfo);
			if (!bResult)
			{
				CloseDevice();
				return FALSE;
			}

			if(pipeInfo.PipeType == UsbdPipeTypeBulk &&
				USB_ENDPOINT_DIRECTION_IN(pipeInfo.PipeId))
			{
				m_bulkInPipe = pipeInfo.PipeId;
			}
			else if(pipeInfo.PipeType == UsbdPipeTypeBulk &&
				USB_ENDPOINT_DIRECTION_OUT(pipeInfo.PipeId))
			{
				m_bulkOutPipe = pipeInfo.PipeId;
			}
			else if(pipeInfo.PipeType == UsbdPipeTypeInterrupt)
			{
				m_interruptPipe = pipeInfo.PipeId;
			}
			else
			{
				bResult = FALSE;
				break;
			}
		}
	}

	return bResult;
}

BOOL C_WINUSB::WriteToDevice(UCHAR *buffer, ULONG bufferSize, ULONG *numWritten)
{
	BOOL bResult;
	ULONG bytesWritten;
	UCHAR shortTerm=1;
	ULONG TimeoutMS=250;

	if (!m_winUSBHandle) return FALSE;

	bResult=WinUsb_SetPipePolicy(m_winUSBHandle,m_bulkOutPipe,SHORT_PACKET_TERMINATE,sizeof(shortTerm),&shortTerm);

	bResult =
		WinUsb_SetPipePolicy(
		m_winUSBHandle,
		m_bulkOutPipe,
		PIPE_TRANSFER_TIMEOUT,
		sizeof( TimeoutMS ),
		&TimeoutMS );

	//[2]
	bResult = WinUsb_WritePipe(m_winUSBHandle,
		m_bulkOutPipe,
		buffer,
		bufferSize,
		&bytesWritten,
		NULL);

	if (numWritten)
	{
		if (bResult) *numWritten=bytesWritten; else *numWritten=0;
	}
	return bResult;
}

BOOL C_WINUSB::ReadFromDevice(UCHAR *buffer, ULONG bufferSize, ULONG *bytesRead)
{
	BOOL bResult;
	ULONG bytesWritten;
	UCHAR shortTerm=1;
	ULONG TimeoutMS=250;
	ULONG numRead;

	if (!m_winUSBHandle) return FALSE;
	*bytesRead=0;
	bResult=FALSE;

	//__try 
	//{
		bResult =
			WinUsb_SetPipePolicy(
			m_winUSBHandle,
			m_bulkInPipe,
			PIPE_TRANSFER_TIMEOUT,
			sizeof( TimeoutMS ),
			&TimeoutMS );

		bResult = WinUsb_ReadPipe(m_winUSBHandle,
			m_bulkInPipe,
			buffer,
			bufferSize,
			&numRead,
			NULL);
	//}
#if 0
	__finally
	{
		*bytesRead=0;
		bResult=FALSE;
    printf("Exception\r\n");
  }
#endif
	if (bytesRead)
	{
		if (bResult) *bytesRead=numRead; else *bytesRead=0;
	}
	return bResult;
}

#endif