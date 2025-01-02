/****************************************************************************
FILE: BIPXPROT.HPP

DESCRIPTION: <DESCRIPTION OF CONSTRUCTS>

COMPONENT: SMC ( Serial Port Classes )

LANGUAGE: C++

NOTES:

        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

/****************
 * PREPROCESSOR *
 ****************/
#ifndef BIPXPROT_HPP
#define BIPXPROT_HPP

class C_BIP;  //Forward references

/***********
 * INCLUDE *
 ***********/

#include <windows.h>
#include "htype.h"
#include "serxprot.hpp"

/***************
 * DEFINITIONS *
 ***************/

typedef enum
{
	BIP_STATE_FLAG,
   BIP_STATE_DATA,
   BIP_STATE_ESCAPE
}T_BIP_RXSTATE;

#define BIP_FLAG 0x7E
#define BIP_ESCAPE 0x7D

#define BIP_ADDRESS 0
#define BIP_PROTOCOL 1
#define BIP_MSGTYPE 2
#define BIP_START_OF_PAYLOAD 3

#define BIP_PCANT_ADDRESS 0x15 /* BMM1 */
#define BIP_PCANT_PROTOCOL 0x01

#define BIP_MAXIMUM_TX_PAYLOAD 2048
#define BIP_MAXIMUM_RX_PAYLOAD 2048

#define BIP_TX_BUFFER_SIZE (2*(BIP_MAXIMUM_TX_PAYLOAD+5)+2)

#define BIP_RX_BUFFER_SIZE (BIP_MAXIMUM_RX_PAYLOAD+5)
#define BIP_RX_TIMEOUT 600


#define MBP_SETSERIALKEY	0xAA551234

//uDL1 Protocol Numbers
//To PC
#define MBP_PING_RESP 0x80
#define MBP_GETVERSION_RESP 0x81
#define MBP_READEEPROM_RESP 0x82
#define MBP_WRITEEEPROM_RESP 0x83
#define MBP_ERASEFLASH_RESP 0x84
#define MBP_READFLASH_RESP 0x85
#define MBP_WRITEFLASH_RESP 0x86
#define MBP_ERASEDF_RESP 0x87
#define MBP_READDF_RESP 0x88
#define MBP_WRITEDF_RESP 0x89
#define MBP_REBOOT_RESP 0x8a
#define MBP_LOCKINBOOT_RESP 0x8b
#define MBP_GETAPPINFO_RESP 0x8c
#define MBP_GETGPSINFO_RESP 0x8d
#define MBP_GETBATTERYINFO_RESP 0x8e
#define MBP_GETFAULTS_RESP 0x8f
#define MBP_POWERDOWN_RESP 0x90
#define MBP_SETTESTMODE_RESP 0x91
#define MBP_GETALLINPUTS_RESP 0x92
#define MBP_SETPORTOUTPUT_RESP 0x93
#define MBP_SETGPSPWR_RESP 0x94
#define MBP_SETRTC_RESP 0x95
#define MBP_GETRTC_RESP 0x96
#define MBP_CLEARSTORAGESTART_RESP 0x97
#define MBP_CLEARSTORAGESTATUS_RESP 0x98
#define MBP_SETSERIAL_RESP 0x99
#define MBP_SETINPUTRANGE_RESP 0x9A
#define MBP_READRAWANALOG_RESP 0x9B
#define MBP_READSCALEDANALOG_RESP 0x9C
#define MBP_CLEARFAULTHISTORY_RESP 0x9D
#define MBP_DEBUGINFO_RESP 0xFF

//From PC 
#define MBP_PING 0x00
#define MBP_GETVERSION 0x01
#define MBP_READEEPROM 0x02
#define MBP_WRITEEEPROM 0x03
#define MBP_ERASEFLASH 0x04
#define MBP_READFLASH 0x05
#define MBP_WRITEFLASH 0x06
#define MBP_ERASEDF 0x07
#define MBP_READDF 0x08
#define MBP_WRITEDF 0x09
#define MBP_REBOOT 0x0a
#define MBP_LOCKINBOOT 0x0b
#define MBP_GETAPPINFO 0x0c
#define MBP_GETGPSINFO 0x0d
#define MBP_GETBATTERYINFO 0x0e
#define MBP_GETFAULTS 0x0f
#define MBP_POWERDOWN 0x10
#define MBP_SETTESTMODE 0x11
#define MBP_GETALLINPUTS 0x12
#define MBP_SETPORTOUTPUT 0x13
#define MBP_SETGPSPWR 0x14
#define MBP_SETRTC 0x15
#define MBP_GETRTC 0x16
#define MBP_CLEARSTORAGESTART 0x17
#define MBP_CLEARSTORAGESTATUS 0x18
#define MBP_SETSERIAL 0x19
#define MBP_SETINPUTRANGE 0x1A
#define MBP_READRAWANALOG 0x1B
#define MBP_READSCALEDANALOG 0x1C
#define MBP_CLEARFAULTHISTORY 0x1D

#define MBP_PORT_GPIOA 0
#define MBP_PORT_GPIOB 1
#define MBP_PORT_GPIOC 2
#define MBP_PORT_ADC 3

#define MBP_ANALOG_DC 0
#define MBP_ANALOG_AC 1
#define MBP_ANALOG_TEMP 2
#define MBP_ANALOG_2V8 3

#define MBP_RANGE_LOW 0
#define MBP_RANGE_MED 1
#define MBP_RANGE_HIGH 2

/* Dataflash Sector erase sizes */
#define DATAFLASH_SECTOR_4K 0
#define DATAFLASH_SECTOR_32K 1
#define DATAFLASH_SECTOR_64K 2
#define DATAFLASH_SECTOR_ALL 3

//EEPROM locations
#define EEPROM_SIZE	0x1000
#define EEPROM_MFG_OFFSET 0x800 /*2K offset from start of EEPROM*/
#define EEPROM_SERIAL_NUM	(EEPROM_SIZE-4)
#define EEPROM_TEST_COMPLETE (EEPROM_SIZE-8)
#define EEPROM_TEST_VERSION (EEPROM_SIZE-12)

//Datalogger structures
typedef enum
{
	E_DATALOG_TIMESTAMP, //0
	E_DATALOG_GPS, //1
	E_DATALOG_READINGDC, //2
	E_DATALOG_READINGAC, //3
	E_DATALOG_TEMPERATURE, //4
	E_DATALOG_BATTERYVOLTS, //5
	E_DATALOG_NOTES, //6
	E_DATALOG_READINGDCFAST, //7
	E_DATALOG_GPSACCURACY, //8
	E_DATALOG_INTREADINGDC, //9
	E_DATALOG_INTREADINGAC //10
} T_DATALOG_RECORDTYPES;

#pragma pack(1)
#define DATALOG_TIME_REASON_OFFSET 0
#define DATALOG_TIME_REASON_INIT	1
#define DATALOG_TIME_REASON_CABLEINSERTED 2
#define DATALOG_TIME_REASON_CABLEREMOVED 3
#define DATALOG_TIME_REASON_GPS 4
#define DATALOG_TIME_REASON_LOWBAT 5
#define DATALOG_TIME_REASON_NOTES 6
#define DATALOG_TIME_REASON_MEMFULL 7
#define DATALOG_TIME_REASON_WAITSTART 8

#define DATALOG_OPTIONS_RANGELOW 0x00
#define DATALOG_OPTIONS_RANGEMED 0x01
#define DATALOG_OPTIONS_RANGEHI 0x02
typedef struct
{
	UINT8 recordType; /* D7-D5=record specific flags, D4-D0=record type(0)	*/
	UINT32 seconds; /* Seconds from Jan 1, 1970, unix time format */
	UINT8 reason;	/* Reason why timestamp record was inserted */
	UINT8 optionsVerHigh; /* Pertinent config options, D1-D0=range, D2,D3=spare, D4-D7=version high nibble*/
	UINT8 verLow; /* Low version bytes */
} T_DATALOG_TIME_RECORD;


typedef struct
{
	UINT8 recordType; /* D7-D5=record specific flags, D4-D0=record type(1)	*/
	UINT8 latitude[3]; /* 3-byte fixed point encoded latitude */
	UINT8 longitude[3]; /* 3-byte fixed point encoded longitude */
	UINT8 satCount; /* Number of satellites in fix */
} T_DATALOG_GPS_RECORD;

#define DATALOG_GPSACCURACY_FLAG_EHPE_LIM_ENABLED 0x20
#define DATALOG_GPSACCURACY_FLAG_EHPE_LIM_TIMEOUT 0x40
typedef struct
{
	UINT8 recordType; /* D7-D5=record specific flags, D4-D0=record type(8)	*/
	UINT16 hdop; /* fixed point 0.1m resolution */
	UINT16 ehpe; /* fixed point 0.1m resolution */
	UINT16 ehpeLimit; /* fixed point 0.1m resolution */
  UINT8 spare; /* future use */
} T_DATALOG_GPSACCURACY_RECORD;

#define DATALOG_READING_FLAG_ON 0x80
#define DATALOG_READING_FLAG_FAHRENHEIT 0x40
#define DATALOG_READING_FLAG_AMPS 0x40
#define DATALOG_READING_FLAG_TIMELOCK 0x20
typedef struct
{ /* D7 indicates an on point reading for future use */
	UINT8 recordType; /* D7-D5=record specific flags, D4-D0=record type(2-4)	*/
	UINT8 timeOffsetMs[3]; /* 3-byte time offset from last timestamp record */
	FLOAT32 value; /* reading value */
} T_DATALOG_READING_RECORD;

#define DATALOG_NOTES_FLAG_START 0x80
#define DATALOG_NOTES_FLAG_END 0x40
typedef struct
{
	UINT8 recordType; /* D7-D5=record specific flags, D4-D0=record type(6)	*/
	UINT8 notes[7];
} T_DATALOG_NOTES_RECORD;
#pragma pack()









typedef void (*T_MBP_CALLBACK)(void *, HUINT8);

//////////////////////////////////////////
#pragma pack(1)


typedef struct
{
#define PARAM_VERSION 0
	UINT8 paramVersion;
	
#define CONFIGFLAGS_DCENABLED 0x0001
#define CONFIGFLAGS_ACENABLED 0x0002
#define CONFIGFLAGS_4TO20MA 0x0004
#define CONFIGFLAGS_LOWLIMITDC_ENABLED 0x0008
#define CONFIGFLAGS_HIGHLIMITDC_ENABLED 0x0010
#define CONFIGFLAGS_LOWLIMITAC_ENABLED 0x0020
#define CONFIGFLAGS_HIGHLIMITAC_ENABLED 0x0040
#define CONFIGFLAGS_FASTSAMPLING_ENABLED 0x0080
#define CONFIGFLAGS_INTERRUPTIONTRACKING_ENABLED 0x0100
#define CONFIGFLAGS_TEMPERATURE_ENABLED 0x0200
#define CONFIGFLAGS_TEMPERATURE_FAHRENHEIT 0x0400
#define CONFIGFLAGS_DC_AMPS 0x0800
#define CONFIGFLAGS_AC_AMPS 0x1000
#define CONFIGFLAGS_REQUIREGPSLOCK_ENABLED 0x2000
#define CONFIGFLAGS_FASTSAMPLINGCONTINUOUS_ENABLED 0x4000
#define CONFIGFLAGS_NOLONGTERMGPSSYNC_ENABLED 0x8000
	UINT16 configFlags;
	
	UINT32 samplingCyclePeriod; /* Number of milliseconds between readings */
	UINT32 samplingCyclePeriodFast; /* Number of milliseconds between fast sampling mode readings, must be 100ms or greater */
	UINT32 samplingFastDuration; /* Number of seconds for a fast sampling mode*/
	UINT32 samplingFastInterval; /* Number of seconds between fast sampling groups */
	
	
	UINT16 interruptionCycleTime; /* Future use for sync to interruption cycles (ms) */
	UINT16 interruptionOffTime; /* Future use for sync to interruption cycles (ms) */
	UINT16 interruptionOffToOnOffset; /* Future use for sync to interruption cycles (ms) */
	UINT16 interruptionOnToOffOffset; /* Future use for sync to interruption cycles (ms) */
	INT16 interruptionUTCOffset; /* Future use for sync to interruption cycles (ms) */
	UINT8 interruptionOnFirst; /* Future use for sync to interruption cycles (ms) */	
	
	UINT8 rangeSelect; /*0=low, 1=med, 2=high*/
#define PARAM_CHANNEL_DC 0
#define PARAM_CHANNEL_AC 1
	FLOAT32 lowLimit[2];
	FLOAT32 highLimit[2];
	FLOAT32 scaleFactor[2];
	FLOAT32 offsetFactor[2];

	UINT16 batteryLowThreshold;
	UINT16 batteryWarnThreshold;

#define PARAM_NOTES_SIZE 105
	UINT8 notes[PARAM_NOTES_SIZE]; //Notes field
	
  UINT16 samplingCycleOffset; /* Number of milliseconds to delay sampling on normal sampling mode (v1.06 addition)*/
  
#define CONFIGFLAGSEXT_SHOWTIME 0x0001  
#define CONFIGFLAGSEXT_LOCALTIME  0x0002
#define CONFIGFLAGSEXT_EHPEWAIT  0x0004
#define CONFIGFLAGSEXT_LOCKFASTSAMPLING_100MS 0x0008 /*Start fast sampling relative to a 100ms boundary*/  
  UINT16 configFlagsExtended; /* Extended config flags */
  
  INT16 timeZoneOffsetMins; /* Time zone offset minutes from UTC */
  
  UINT16 minimumEHPE; //Minimum EHPE for position recording (fixed point with 0.1m resolution), 2.5 to 200.0 range
  UINT16 minimumEHPEWaitTimeSec; //Maximum number of seconds to wait for a good EHPE (3600s max, 300s nom)
  
  UINT8 lockedFastSamplingOffsetMs; //Number of milliseconds to offset from 100ms boundary before starting fast sampling (0-95ms)
  
  UINT32 delayStartSec; //Delay before starting acquisition

  UINT8 spare8[1]; //For future use so we don't have to bump the parameter version number for minor changes
	UINT32 spare32[28]; //For future use so we don't have to bump the parameter version number for minor changes
	
	UINT16 firmwareVerParamWrite; //Used to flag a firmware to update any changed areas in the configuration
	
	UINT16 crc16;
} T_PARAM_PARAMETERS;


#define PARAM_MFG_OFFSET 0x800 /*2K offset from start of EEPROM*/

typedef struct
{
	INT16 zeroCal[3];
	FLOAT32 scaleFactor[3];
} T_CHANNEL_CALIBRATION;

typedef struct
{
#define PARAM_MFG_VERSION 0
	UINT8 paramVersion;

#define OPTIONFLAGS_GPS 0x01
#define	OPTIONFLAGS_EXTMEM 0x02
#define OPTIONFLAGS_NEWCAL 0x40000000
#define OPTIONFLAGS_PROTOTYPE 0x80000000
	UINT32 optionFlags;

	T_CHANNEL_CALIBRATION calibration[2]; //DC/AC
	UINT16 acFloorCal[3]; //AC floor for zero display

	INT16 calTemperature; //fixed point, LSB = 0.01C
	UINT32 calDate; //C time
	UINT32 spare32[29]; //For future use so we don't have to bump the parameter version number for minor changes
	
	UINT16 crc16;
} T_PARAM_MFGDATA;

#pragma pack()
extern const T_PARAM_PARAMETERS param_data_defaults;
extern const T_PARAM_MFGDATA param_mfgdata_defaults;


#pragma pack(1)   
typedef struct GPSDATA_DEF
{
	UINT16 utcYear;
	UINT8 utcMonth;
	UINT8 utcDay;
	UINT8 utcHours;
	UINT8 utcMinutes;
	UINT8 utcSeconds;
  UINT16 utcMilliseconds;
	
	INT32 latitude;
	INT32 longitude;
	INT32 altitude;
	
	UINT8 satsInFix;
	UINT8 satsTracked;

	UINT16 solValid;
	UINT16 hdopValid;
	
	
	UINT8 timeMarkSeconds;
	UINT8 timeMarkValid;
	UINT8 timeMarkNew;
	
	UINT16 gpsWeek;
	
	UINT8 svid[12];
	UINT8 snr[12];

	UINT16 hdop; //fixed point, 0.1m resolution
  UINT16 ehpe; //fixed point, 0.1m resolution
} T_GPSDATA_DEF;

#pragma pack()
typedef struct GPSFixDef T_GPSFIXDEF;   

#define FAULT_CONFIG_INVALID   		0x00000001
#define FAULT_GPS_INIT 				 		0x00000002
#define FAULT_EEPROM					 		0x00000004
#define FAULT_BATTERY					 		0x00000008
#define FAULT_ADC							 		0x00000010
#define FAULT_DATAFLASH				 		0x00000020
#define FAULT_CALIBRATION					0x00000040
#define FAULT_DATAFLASH_FF		 		0x00000080
#define FAULT_DATAFLASH_FINDSTART 0x00000100
#define FAULT_ADC1								0x40000000
#define FAULT_ADC2								0x80000000

#define FAULT_HISTORY_SIZE 8	 
#pragma pack(1)	 
typedef	struct
{
	UINT32 time[FAULT_HISTORY_SIZE];
	UINT32 faultCode[FAULT_HISTORY_SIZE];
} T_FAULT_HISTORY;
#pragma pack()

extern const char *bip_faultStrings[];


#define UDL1_BOARDTYPE_G1 0
#define UDL1_BOARDTYPE_G2 1
#define UDL1_BOARDTYPE_UNKNOWN 255

/**********************************
 * NON-MEMBER FUNCTION PROTOTYPES *
 **********************************/

/****************************************************************************
FUNCTION: <FUNCTION NAME>

DESCRIPTION: <FUNCTION DESCRIPTION>

FILENAME: BIPXPROT.HPP

INPUT ARGUMENTS:

      Parameter     Description
      ---------     -----------
    <TYPE> <NAME>  <DESCRIPTION>

OUTPUT ARGUMENT:

      Parameter     Description
      ---------     -----------
    <TYPE> <NAME>  <DESCRIPTION>

RETURNS: <TYPE> <DESCRIPTION>

        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/
//ostream &operator<<(ostream &os_, C_BIP &source_);

/********************
 * CLASS PROTOTYPES *
 ********************/

class C_BIP
{
public:
   C_BIP();                        //Constructor
   ~C_BIP();                       //Destructor

   HUINT32 Options(C_SER *serPortPtr_,char *bipName);
   void ReceiveCB(HUINT8 *bufferPtr_, HUINT32 bufferLen_);
   HUINT32 SetReceiveCallback(void* routine, void *cbData);

  //Commands to uDL1
  HBOOL SendGeneric(UINT8 command);
	HBOOL SendWriteEEPROM(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize);
	HBOOL SendReadEEPROM(HUINT32 address, HUINT32 count);
	HBOOL SendEraseFlash(HUINT32 address, HUINT32 count);
	HBOOL SendReadFlash(HUINT32 address, HUINT32 count);
	HBOOL SendWriteFlash(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize);
	HBOOL SendEraseDataFlash(HUINT32 address, HUINT32 count, HUINT32 eraseSectorCode);
	HBOOL SendReadDataFlash(HUINT32 address, HUINT32 count);
	HBOOL SendWriteDataFlash(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize);
	HBOOL SendSetRTC(HUINT32 rtcTime);
	HBOOL SendSetSerial(HUINT32 serial);
	HBOOL SendSetInputRange(HUINT8 range);
	HBOOL SendReadRawAnalog(HUINT8 channel, HUINT8 samplesForAverage);
	HBOOL SendReadScaledAnalog(HUINT8 channel, HUINT8 samplesForAverage);
	HBOOL SendSetPortOutput(HUINT8 port, HUINT8 pinIndex, HUINT16 state);

   //Data from responses on RMU
	HUINT8 m_ack;
	HUINT8 m_dataBuffer[BIP_MAXIMUM_RX_PAYLOAD];
	HUINT32 m_bootVersion;
	HUINT8 m_paramVersion;
	HUINT8 m_appValid;
	HUINT32 m_appChecksum;
	HUINT32 m_appVersion;
	HUINT32 m_serialNumber;
	HUINT32 m_boardType;
	T_GPSDATA_DEF m_gpsInfo;
	float m_batteryVolts;
	float	m_batteryTemperature;
	BOOL m_batteryCharging;
	HUINT32 m_faults;
	T_FAULT_HISTORY m_faultHistory;
	HUINT32 m_rtcTime;
	HUINT32 m_1ppsCount;
	HUINT16 m_1ppsMsCount;
	BOOL m_rtcTimeLock;
	HUINT32 m_storageUsedBytes;
	HUINT32 m_storageTotalBytes;
	HUINT8 m_storageClearBusy;
	HUINT8 m_storageClearPercent;
	HUINT8 m_storageClearFailure;
	HINT32 m_rawMeasurement;
	float m_scaledMeasurement;

	HBOOL MessageWait(HUINT8 expectedResp_, HUINT32 timeout_);
	HBOOL ResponseWait(HUINT32 timeout_);
   HUINT8 GetLastRxCode(void);
   HBOOL IsLastRxValid(void);
   void ResetLastRxValid(void);

   void RxDataPrint(void);
   //void Print(ostream *os_);
   //void PrintError(void);
   
   HANDLE m_messageReceived; //Event indicating a response was received.
	 UINT8 m_lastMessageSent;
	 HANDLE m_responseReceived; //Event indicating a response to a query.

private:
   C_SER *m_serPortPtr;

   char m_bipName[100];


   HUINT32 m_dwError;

   T_BIP_RXSTATE m_rxState;

   HUINT8 m_txBuffer[BIP_TX_BUFFER_SIZE];
   HUINT32 m_txBufferCount;

   HUINT16 m_rxBufferCount;
   HUINT8 m_rxBuffer[BIP_RX_BUFFER_SIZE];
   HUINT16 m_rxCommandLength;
   HUINT16 m_rxChecksum;
   HUINT8 m_rxTimerCount;
   HUINT16 m_rxPacketChecksum;
   //HUINT32 m_rxLastTime;   //When was the last character received?

   HUINT8 m_lastRxCode;
   HBOOL m_lastRxValid;

	 HUINT8 m_pgmLastRxCode;
   HBOOL m_pgmLastRxValid;

   void DispatchPacket(void);
   
   static void CReceiveCB(void *dwUser_, HUINT8 *bufferPtr_, HUINT32 bufferLen_);

	T_MBP_CALLBACK m_rxCallback;
	void *m_rxCallbackData;

   void PrintBuffer(HUINT8 *buffer_, HUINT32 length_);

   
   void AddEscape(HUINT8 data_, HUINT8 *bufferPtr_, HUINT32 *bufferCountPtr_);
   HBOOL Encapsulate(HUINT8 address_, HUINT8 control_, HUINT8 *payloadPtr_, HUINT32 payloadSize_, HUINT8 protocol_=BIP_PCANT_PROTOCOL);

};



#endif
