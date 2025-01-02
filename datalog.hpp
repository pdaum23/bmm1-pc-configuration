/****************************************************************************
FILE: DATALOG.HPP

DESCRIPTION: <DESCRIPTION OF CONSTRUCTS>

COMPONENT: SMC ( Serial Port Classes )

LANGUAGE: C++

NOTES:

		(c) Copyright 2021 Mobiltex Data Ltd.

****************************************************************************/


/****************
 * PREPROCESSOR *
 ****************/
#pragma once

/***********
 * INCLUDE *
 ***********/

#include <windows.h>
#include "htype.h"
#include "serxprot.hpp"
#include <stdio.h>



class SdlNotes {
public:
	CString notes = "";
	UINT stnSeries = 0;
	UINT chainage = 0;
	BOOL enabled = FALSE;

	SdlNotes();
	SdlNotes(CString notes);
	CString GetSdlNotes();
};


#define SDL_DATALOG_RECORD_TYPE_HEADER 0
#define SDL_DATALOG_RECORD_TYPE_WAVEFORM 1
#define SDL_DATALOG_RECORD_TYPE_DATALOG 2
#define SDL_DATALOG_RECORD_TYPE_GPSDATA 3

#define SDL_DATALOG_HEADER_FLAGS_DIGITALFILTER	0x00000001
#define SDL_DATALOG_HEADER_FLAGS_NOGPS	0x00000002
#define SDL_DATALOG_HEADER_FLAGS_EXTENDEDHIZ	0x00000004
#define SDL_DATALOG_HEADER_FLAGS_ONFIRST	0x00000008

#pragma pack(1)
typedef struct
{
	UINT8 type;
	UINT32 signature;
	UINT8 fileType;
	UINT16 firmware;
	UINT8 recordVersion;
	UINT32 stationSeries;
	UINT32 chainage;
	UINT64 fileStartDate;
	UINT16 offDuration;
	UINT16 cycleDuration;
	UINT16 wavePrintInterval;
	UINT8 meterRange;
	UINT16 onOffset;
	UINT16 offOffset;
	INT32 latitude;
	INT32 longitude;
	UINT16 altitude;
	char notes[256];
	UINT32 serialNum;
	UINT32 flags;
	UINT32 utcOffset;
	UINT16 spiFirmware;
	UINT8 gpsType;
	char pad[195];
} T_SDL_DATALOG_HEADER;

typedef struct
{
	UINT8 type;
	UINT64 timeMilliseconds;
	INT32 acValue;
	INT32 dcValue;

} T_SDL_DATALOG_DATAPOINT;

typedef struct
{
	UINT8 type;
	UINT64 timeMilliseconds;
	INT32 latitude;
	INT32 longitude;
	UINT16 altitude;
	UINT8 satsVisible;
	UINT8 satsSolution;
	UINT8 solutionValid;
} T_SDL_DATALOG_GPSPOINT;
#pragma pack()

#pragma once
class Datalog
{
public:
	Datalog();
	void Open(FILE *newOutFilePtr);
	void Open(FILE *newOutFilePtr, FILE *debugFilePtr);
	BOOL WriteHeader();
	void UpdateHeader(UINT16 firmware, UINT32 stnSeries, UINT32 chainage, UINT8 meterRange, CString notes, UINT32 serialNum);
	BOOL WriteGPSData(UINT64 time, float latitude, float longitude, UINT16 altitude, UINT8 satsVisible, UINT8 satsUsed, UINT8 solValid);

	void AddDcValue(UINT64 time, FLOAT32 value, BOOL intOn);
	void AddAcValue(UINT64 time, FLOAT32 value, BOOL intOn);
	void AddWaveformValue(UINT64 time, FLOAT32 value);

private:
	BOOL acValid;
	BOOL dcValid;
	BOOL inWaveSegment;
	BOOL lastIntState;
	BOOL waitForFirstOffReading;
	UINT64 lastCycleTime;
	UINT64 lastIntTime;
	UINT64 lastWaveTime;
	UINT32 offIntOffset;
	UINT32 onIntOffset;
	BOOL noGps;
	BOOL onFirstInt;
	T_SDL_DATALOG_DATAPOINT dataPoint;
	T_SDL_DATALOG_HEADER header;

	void Reset();
	void ResetHeader();
	void ResetDatapoint();

	void UpdateIntTimes(UINT64 time, BOOL intOn);

	FILE *outFilePtr, *debugFilePtr;
	BOOL WriteWaveformData(T_SDL_DATALOG_DATAPOINT data);
	BOOL WriteDatalogData(T_SDL_DATALOG_DATAPOINT data);
	BOOL WriteGPSData(T_SDL_DATALOG_GPSPOINT data);
};
