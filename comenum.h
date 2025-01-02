#ifndef COMENUM_H
#define COMENUM_H
HANDLE  BeginEnumeratePorts(VOID);
BOOL EnumeratePortsNext(HANDLE DeviceInfoSet, LPTSTR lpBuffer);
BOOL  EndEnumeratePorts(HANDLE DeviceInfoSet);
#endif