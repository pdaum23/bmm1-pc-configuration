#ifndef WINUSBIF_H
#define WINUSBIF_H

#include "windows.h"
#undef _MP 
#include "winusb.h"
#include "usb100.h"

typedef void (*T_WINUSB_CALLBACK)(void *, UINT8 *, UINT32);
#define MAX_DEVPATH_LENGTH MAX_PATH

extern GUID gRDU1GUID;
extern GUID gRMU2SDL1GUID;

class C_WINUSB
{
public:
   C_WINUSB();                        //Constructor
   ~C_WINUSB();                       //Destructor

   UINT32 Open(GUID classGUID);
   UINT32 Close(void);
   UINT32 Write(UINT8 *dataPtr_, UINT32 requestSize_, UINT32 *numWrittenPtr_);
   UINT32 Read(UINT8 *dataPtr_, UINT32 requestSize_, UINT32 *numReadPtr_);
   BOOL IsOpen(void);

   BOOL AddReceiveCB(T_WINUSB_CALLBACK routinePtr_, void *callBackIDPtr_);
   BOOL ReceiveEnable(BOOL state_);
   void Iteration(void);

	BOOL GetDevicePath(LPGUID InterfaceGuid,
                   PCHAR DevicePath,
                   size_t BufLen);
	HANDLE OpenDevice(BOOL bSync);
	void CloseDevice(void);
	BOOL Initialize_Device(void);
	BOOL WriteToDevice(UCHAR *buffer, ULONG bufferSize, ULONG *numWritten);
	BOOL ReadFromDevice(UCHAR *buffer, ULONG bufferSize, ULONG *bytesRead);

private:
   T_WINUSB_CALLBACK m_callBackPtr;
   void *m_callBackID;   

   HANDLE m_threadExitEvent;
   HANDLE m_threadStoppedEvent;
   HANDLE m_receiveDisableEvent;
   HANDLE m_dataMutex;
   TCHAR  m_dataMutexName[12];

   static void CIteration(void * dwUser_);

	WINUSB_INTERFACE_HANDLE m_winUSBHandle;
	WINUSB_INTERFACE_HANDLE m_winUSBHandle2;
	HANDLE m_deviceHandle;
	UCHAR m_deviceSpeed;
	UCHAR m_bulkInPipe;
	UCHAR m_bulkOutPipe;
	UCHAR m_interruptPipe;
	GUID m_classGUID;

	DWORD m_dwError;
};


#endif