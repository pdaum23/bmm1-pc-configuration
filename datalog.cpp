
#include "stdafx.h"
#include "datalog.hpp"
#include "bipxprot.hpp"

static INT RoundInt(UINT64 number, UINT32 multiple)
{
	float fresult = (float)number / (float)multiple;
	INT result = (INT)roundf(fresult) * multiple;
	return result;
}

static INT RoundInt(INT number, UINT32 multiple)
{
	float fresult = (float)number / (float)multiple;
	INT result = (INT)roundf(fresult) * multiple;
	return result;
}

SdlNotes::SdlNotes()
{
}

SdlNotes::SdlNotes(CString newNotes)
{
	notes = newNotes;
	CString sdlInfo;
	UINT tempSeries, tempChainage;
	int index = notes.Find('\r', 0);
	while (index != -1)
	{
		sdlInfo = notes.Right(notes.GetLength() - index);
		notes = notes.Left(index);
		if (sdlInfo.GetLength()) { // parse the current 
			if (sscanf(sdlInfo, "\rS%uC%u", &tempSeries, &tempChainage) == 2) {
				enabled = TRUE;
				chainage = tempChainage;
				stnSeries = tempSeries;
			}
		}
		index = notes.Find('\r', 0);
	}
}

CString SdlNotes::GetSdlNotes()
{
	CString cleanStr = notes;
	CString sdlAddendum = "";
	if (enabled) {
		sdlAddendum.AppendFormat("\rS%uC%u", stnSeries, chainage); // max 15 bytes
		if (notes.GetLength() + sdlAddendum.GetLength() >= PARAM_NOTES_SIZE) {
			cleanStr = cleanStr.Left(PARAM_NOTES_SIZE - sdlAddendum.GetLength() - 1);
		}
		cleanStr.Append(sdlAddendum);
	}
	return cleanStr;
}


void Datalog::ResetDatapoint()
{
	acValid = FALSE;
	dcValid = FALSE;
	memset(&dataPoint, 0, sizeof(T_SDL_DATALOG_DATAPOINT));
}

void Datalog::UpdateIntTimes(UINT64 time, BOOL intOn)
{
	if (inWaveSegment) return;

	// valid cycletime & last OFF && now ON
	if (lastIntTime && !lastIntState && intOn) {
		header.offDuration = RoundInt(time - lastIntTime, 20); // round to the nearest 20ms
	}
	// valid cycletime and last != current and now OFF
	if (lastCycleTime && !intOn && (lastIntState != intOn)) {
		header.cycleDuration = RoundInt(time - lastCycleTime, 20); // round to the nearest 20ms
		lastCycleTime = time;
	}
	if(!lastCycleTime) lastCycleTime = time;
	lastIntTime = time;
	lastIntState = intOn;
}


void Datalog::AddDcValue(UINT64 time, FLOAT32 value, BOOL intOn)
{
	INT32 rounded = floor((value * 1000.0) + 0.5);
	if (value < -1E10) rounded = -1E10; // under
	if (value > 1E10) rounded = 1E10; // over
	if (waitForFirstOffReading && intOn) return;
	waitForFirstOffReading = FALSE;

	if (intOn) {
		if (onIntOffset > time % 60000)
		{
			onIntOffset = time % 60000;
		}
	}
	else {
		if (offIntOffset > time % 60000)
		{
			offIntOffset = time % 60000;
		}
	}
	// no AC measurement between readings (AC disabled?)
	if (dcValid) {
		UpdateIntTimes(time, intOn);
		WriteDatalogData(dataPoint);
		ResetDatapoint();
	}
	dataPoint.timeMilliseconds = time;
	dataPoint.dcValue = rounded;
	dcValid = TRUE;
	inWaveSegment = FALSE;
}

void Datalog::AddAcValue(UINT64 time, FLOAT32 value, BOOL intOn)
{
	INT32 rounded = floor((value * 1000.0) + 0.5);
	if (value < -1E10) rounded = -1E10; // under
	if (value > 1E10) rounded = 1E10; // over
	if (waitForFirstOffReading && intOn) return;
	waitForFirstOffReading = FALSE;

	if (intOn) {
		if (onIntOffset > time % 60000)
		{
			onIntOffset = time % 60000;
		}
	}
	else {
		if (offIntOffset > time % 60000)
		{
			offIntOffset = time % 60000;
		}
	}

	// no DC measurement between readings (DC disabled?)
	if (acValid) {
		UpdateIntTimes(time, intOn);
		WriteDatalogData(dataPoint);
		ResetDatapoint();
	}

	// DC measurement already present, add AC, but use DC's time
	if (dcValid) {
		UpdateIntTimes(time, intOn);
		dataPoint.acValue = rounded;
		WriteDatalogData(dataPoint);
		ResetDatapoint();
	}
	// empty struct, add AC
	else {
		dataPoint.timeMilliseconds = time;
		dataPoint.acValue = rounded;
		acValid = TRUE;
	}
	inWaveSegment = FALSE;
}

void Datalog::AddWaveformValue(UINT64 time, FLOAT32 value)
{
	INT32 rounded = floor((value * 1000.0) + 0.5);
	if (value < -1E10) rounded = -1E10; // under
	if (value > 1E10) rounded = 1E10; // over
	if (waitForFirstOffReading) return;

	// write previous datalog point if present
	if (dcValid || acValid) {
		WriteDatalogData(dataPoint);
		ResetDatapoint();
	}

	if (!inWaveSegment) {
		if (lastWaveTime) {
			header.wavePrintInterval = RoundInt(time - lastWaveTime, 60000) / 60000;
		}
		lastWaveTime = time;
	}

	// Always DC, write straight through and reset
	dataPoint.dcValue = rounded;
	dataPoint.timeMilliseconds = time;
	WriteWaveformData(dataPoint);
	ResetDatapoint();
	inWaveSegment = TRUE;
}

Datalog::Datalog()
{
	Reset();
	ResetHeader();
}

void Datalog::Open(FILE *newOutFilePtr)
{
	outFilePtr = newOutFilePtr;
	Reset();
	WriteHeader();
}

void Datalog::Open(FILE *newOutFilePtr, FILE *debugFilePtr)
{
	this->outFilePtr = newOutFilePtr;
	this->debugFilePtr = debugFilePtr;
	Reset();
	WriteHeader();
}

BOOL Datalog::WriteHeader()
{
	BOOL result;
	CString dbgStr;
	rewind(outFilePtr); // always write the header to the start of the file
	result = fwrite(&header, sizeof(T_SDL_DATALOG_HEADER), 1, outFilePtr);

#ifdef _DEBUG
	fprintf(debugFilePtr,"type: %u, ", header.type);
	fprintf(debugFilePtr,"fileType: %u, ", header.fileType);
	fprintf(debugFilePtr,"firmware: %u, ", header.firmware);
	fprintf(debugFilePtr,"recordVersion: %u, ", header.recordVersion);
	fprintf(debugFilePtr,"stationSeries: %u, ", header.stationSeries);
	fprintf(debugFilePtr,"chainage: %u, ", header.chainage);
	fprintf(debugFilePtr,"fileStartDate: %llu, ", header.fileStartDate);
	fprintf(debugFilePtr,"offDuration: %u, ", header.offDuration);
	fprintf(debugFilePtr,"cycleDuration: %u, ", header.cycleDuration);
	fprintf(debugFilePtr,"wavePrintInterval: %u, ", header.wavePrintInterval);
	fprintf(debugFilePtr,"meterRange: %u, ", header.meterRange);
	fprintf(debugFilePtr,"onOffset: %u, ", header.onOffset);
	fprintf(debugFilePtr,"offOffset: %u, ", header.offOffset);
	fprintf(debugFilePtr,"latitude: %i, ", header.latitude);
	fprintf(debugFilePtr,"longitude: %i, ", header.longitude);
	fprintf(debugFilePtr,"altitude: %u, ", header.altitude);
	fprintf(debugFilePtr,"notes: %s, ", header.notes);
	fprintf(debugFilePtr,"serialNum: %u, ", header.serialNum);
	fprintf(debugFilePtr,"flags: 0x%02x, ", header.flags);
	fprintf(debugFilePtr,"utcOffset: %u, ", header.utcOffset);
	fprintf(debugFilePtr,"spiFirmware: %u, ", header.spiFirmware);
	fprintf(debugFilePtr,"gpsType: %u", header.gpsType);
	fprintf(debugFilePtr,"\n");
#endif

	Reset();
	fseek(outFilePtr, 0L, SEEK_END); // jump back to the end
	return result;
}

void Datalog::UpdateHeader(UINT16 firmware, UINT32 stnSeries, UINT32 chainage, UINT8 meterRange, CString notes, UINT32 serialNum)
{
	header.type = 0; // header
	header.signature = 0xF0615D82;
	header.fileType = 2;
	header.firmware = firmware;
	header.recordVersion = 0;
	header.stationSeries = stnSeries;
	header.chainage = chainage;
	//header.fileStartDate // set in WriteDatalogData or WriteGPSData
	//header.offDuration // set in UpdateOffTime
	//header.cycleDuration; // set in UpdateCycleTime
	//header.wavePrintInterval; // set in UpdateWaveformTime
	header.meterRange = meterRange;
	header.onOffset = 0; // measurement values are absolute
	header.offOffset = 0; // measurement values are absolute
	//header.latitude; // set WriteGPSData
	//header.longitude; // set WriteGPSData
	//header.altitude; // set WriteGPSData
	snprintf(header.notes, 256, "%s", notes.GetString());
	header.notes[255] = 0;
	header.serialNum = serialNum;
	header.flags = 0x00000004; // digital avg off | low range 12V max
	if (noGps) header.flags |= 0x00000002;
	if (onIntOffset < offIntOffset) header.flags |= 0x00000008;
	header.utcOffset = 2000; // match the SDL for UTC 0ms, all values are absolute
	header.spiFirmware = header.firmware;
	header.gpsType = 5;
	memset(header.pad, 0, 195);
}

void Datalog::ResetHeader()
{
	memset(&header, 0, sizeof(T_SDL_DATALOG_HEADER));
}

void Datalog::Reset()
{
	ResetDatapoint();
	lastCycleTime = 0;
	lastIntTime = 0;
	lastWaveTime = 0;
	offIntOffset = 60000;
	onIntOffset = 60000;
	noGps = TRUE;
	onFirstInt = FALSE;
	inWaveSegment = FALSE;
	lastIntState = FALSE;
	waitForFirstOffReading = TRUE;
}

BOOL Datalog::WriteGPSData(UINT64 time, float latitude, float longitude, UINT16 altitude, UINT8 satsVisible, UINT8 satsUsed, UINT8 solValid)
{
	T_SDL_DATALOG_GPSPOINT data;
	data.timeMilliseconds = time;
	data.latitude = latitude * 10000000.0; // 7 decimal places
	data.longitude = longitude * 10000000.0;
	data.altitude = altitude;
	data.satsVisible = satsVisible;
	data.satsSolution = satsUsed;
	data.solutionValid = solValid;
	// update the header
	header.altitude = data.altitude;
	header.latitude = data.latitude;
	header.longitude = data.longitude;
	return WriteGPSData(data);
}


BOOL Datalog::WriteWaveformData(T_SDL_DATALOG_DATAPOINT data)
{
	data.type = 1;
#ifdef _DEBUG
	CString dbgStr;
	fprintf(debugFilePtr,"type: %u,", data.type);
	fprintf(debugFilePtr,"ms: %llu,", data.timeMilliseconds);
	fprintf(debugFilePtr,"dcWave: %i", data.dcValue);
	fprintf(debugFilePtr,"\n");
#endif
	return fwrite(&data, sizeof(T_SDL_DATALOG_DATAPOINT), 1, outFilePtr);
}

BOOL Datalog::WriteDatalogData(T_SDL_DATALOG_DATAPOINT data)
{
	if (header.fileStartDate == 0) {
		header.fileStartDate = data.timeMilliseconds;
	}
	data.type = 2;
#ifdef _DEBUG
	CString dbgStr;
	fprintf(debugFilePtr,"type: %u,", data.type);
	fprintf(debugFilePtr,"ms: %llu,", data.timeMilliseconds);
	fprintf(debugFilePtr,"ac: %i,", data.acValue);
	fprintf(debugFilePtr,"dc: %i", data.dcValue);
	fprintf(debugFilePtr,"\n");
#endif
	return fwrite(&data, sizeof(T_SDL_DATALOG_DATAPOINT), 1, outFilePtr);
}

BOOL Datalog::WriteGPSData(T_SDL_DATALOG_GPSPOINT data)
{
	noGps = FALSE;
	if (header.fileStartDate == 0) {
		header.fileStartDate = data.timeMilliseconds;
	}
	data.type = 3;
#ifdef _DEBUG
	CString dbgStr;
	fprintf(debugFilePtr,"type: %u,", data.type);
	fprintf(debugFilePtr,"ms: %llu,", data.timeMilliseconds);
	fprintf(debugFilePtr,"latitude: %i,", data.latitude);
	fprintf(debugFilePtr,"longitude: %i,", data.longitude);
	fprintf(debugFilePtr,"altitude: %u,", data.altitude);
	fprintf(debugFilePtr,"satsVisible: %u,", data.satsVisible);
	fprintf(debugFilePtr,"satsSolution: %u,", data.satsSolution);
	fprintf(debugFilePtr,"solutionValid: %u,", data.solutionValid);
	fprintf(debugFilePtr,"\n");
#endif
	return fwrite(&data, sizeof(T_SDL_DATALOG_GPSPOINT), 1, outFilePtr);
}