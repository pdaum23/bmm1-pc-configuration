#include "stdafx.h"
#include <setupapi.h>
#include <windows.h>
#include "comenum.h"

#define MAX_NAME_PORTS 30
#define RegDisposition_OpenExisting (0x00000001) // open key only if exists
#define CM_REGISTRY_HARDWARE        (0x00000000)

typedef DWORD ( WINAPI * CM_Open_DevNode_Key)(DWORD, DWORD, DWORD, DWORD, ::PHKEY, DWORD);

HANDLE  BeginEnumeratePorts(VOID)
{
  BOOL guidTest=FALSE;
  DWORD RequiredSize=0;
  int j;
  HDEVINFO DeviceInfoSet;
  char* buf;

 guidTest=SetupDiClassGuidsFromNameA(
       "Ports",0,0,&RequiredSize);
 if(RequiredSize < 1)return (HANDLE)-1;

 buf=(char *)malloc(RequiredSize*sizeof(GUID));

 guidTest=SetupDiClassGuidsFromNameA("Ports",(LPGUID)buf,RequiredSize*sizeof(GUID),&RequiredSize);

 if(!guidTest)return (HANDLE)-1;


 DeviceInfoSet=SetupDiGetClassDevs((const GUID *)buf,NULL,NULL,DIGCF_PRESENT);
 if(DeviceInfoSet == INVALID_HANDLE_VALUE)return (HANDLE)-1;

 free(buf);

 return DeviceInfoSet;
}

BOOL EnumeratePortsNext(HANDLE DeviceInfoSet, LPTSTR lpBuffer)
{
  static CM_Open_DevNode_Key OpenDevNodeKey=NULL;
  static HINSTANCE CfgMan;

  int res1;
  char DevName[MAX_NAME_PORTS]={0};
  static int numDev=0;
  int numport;

  SP_DEVINFO_DATA DeviceInfoData={0};
  DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA);


  if(!DeviceInfoSet || !lpBuffer)return -1;
  if (!OpenDevNodeKey) 
  {
	CfgMan = LoadLibrary("cfgmgr32");
	if (!CfgMan)
		return FALSE;
	OpenDevNodeKey = (CM_Open_DevNode_Key)GetProcAddress(CfgMan,"CM_Open_DevNode_Key");
	if(!OpenDevNodeKey) {
		FreeLibrary(CfgMan);
		return FALSE;
	}
	numDev=0;
  }

  while(TRUE)
  {

    HKEY KeyDevice;
    DWORD len;
    res1=SetupDiEnumDeviceInfo(
        DeviceInfoSet,numDev,&DeviceInfoData);

    if(!res1)
    {
      SetupDiDestroyDeviceInfoList(DeviceInfoSet);
      FreeLibrary(CfgMan);
      OpenDevNodeKey=NULL;
      return FALSE;
    }


    res1=OpenDevNodeKey(DeviceInfoData.DevInst,KEY_QUERY_VALUE,0,
      RegDisposition_OpenExisting,&KeyDevice,CM_REGISTRY_HARDWARE);
    if(res1 != ERROR_SUCCESS)return NULL;
    len=MAX_NAME_PORTS;

    res1=RegQueryValueEx(
      KeyDevice,    // handle of key to query
      "portname",    // address of name of value to query
      NULL,    // reserved
      NULL,    // address of buffer for value type
      (LPBYTE)DevName,    // address of data buffer
      &len     // address of data buffer size
    );

    RegCloseKey(KeyDevice);
    if(res1 != ERROR_SUCCESS)return NULL;
    numDev++;
    if(memicmp(DevName, "com", 3))continue;
    numport=atoi(DevName+3);
    if(numport > 0 && numport <= 256)
    {
      strcpy(lpBuffer,DevName);
      return TRUE;
    }

    FreeLibrary(CfgMan);
    OpenDevNodeKey=NULL;
    return FALSE;
  }
}

BOOL  EndEnumeratePorts(HANDLE DeviceInfoSet)
{
  if(SetupDiDestroyDeviceInfoList(DeviceInfoSet))return TRUE;
  else return FALSE;
}