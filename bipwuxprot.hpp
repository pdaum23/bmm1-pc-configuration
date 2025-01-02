/****************************************************************************
FILE: BIPWUXPROT.HPP

DESCRIPTION: <DESCRIPTION OF CONSTRUCTS>

COMPONENT: SMC ( Serial Port Classes )

LANGUAGE: C++

NOTES:

        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

/****************
 * PREPROCESSOR *
 ****************/
#ifndef BIPWUXPROT_HPP
#define BIPWUXPROT_HPP

class C_BIPWU;  //Forward references

/***********
 * INCLUDE *
 ***********/
#ifdef _DEBUG

#include <windows.h>
#include "htype.h"
//#include "serxprot.hpp"
#include "winusbif.hpp"

/***************
 * DEFINITIONS *
 ***************/

typedef enum
{
	BIPWU_STATE_FLAG,
   BIPWU_STATE_DATA,
   BIPWU_STATE_ESCAPE
}T_BIPWU_RXSTATE;

#define BIPWU_FLAG 0x7E
#define BIPWU_ESCAPE 0x7D

#define BIPWU_ADDRESS 0
#define BIPWU_PROTOCOL 1
#define BIPWU_MSGTYPE 2
#define BIPWU_START_OF_PAYLOAD 3

#define BIPWU_BROADCAST_ADDRESS 0xFF
#define BIPWU_PROTOCOL_NUMBER 0xFF

#define BIPWU_MAXIMUM_TX_PAYLOAD 1024
#define BIPWU_MAXIMUM_RX_PAYLOAD 1024

#define BIPWU_TX_BUFFER_SIZE (2*(BIPWU_MAXIMUM_TX_PAYLOAD+5)+2)

#define BIPWU_RX_BUFFER_SIZE (BIPWU_MAXIMUM_RX_PAYLOAD+5)
#define BIPWU_RX_TIMEOUT 600

//Protocol Numbers
//To PC
#define MBPWU_PING_RESP 0x80
#define MBPWU_GETVERSION_RESP 0x81
#define MBPWU_READEEPROM_RESP 0x82
#define MBPWU_WRITEEEPROM_RESP 0x83
#define MBPWU_REBOOT_RESP 0x84
#define MBPWU_SETRTC_RESP 0x85
#define MBPWU_GETRTC_RESP 0x86
#define MBPWU_SETANALOGOUT_RESP 0x87
#define MBPWU_GETMETER_RESP 0x88
#define MBPWU_SETDIGITALOUT_RESP 0x89
#define MBPWU_READLIGHT_RESP 0x8A
#define MBPWU_SETMETERDISPLAY_RESP 0x8B

//From PC 
#define MBPWU_PING 0x00
#define MBPWU_GETVERSION 0x01
#define MBPWU_READEEPROM 0x02
#define MBPWU_WRITEEEPROM 0x03
#define MBPWU_REBOOT 0x04
#define MBPWU_SETRTC 0x05
#define MBPWU_GETRTC 0x06
#define MBPWU_SETANALOGOUT 0x07
#define MBPWU_GETMETER 0x08
#define MBPWU_SETDIGITALOUT 0x09
#define MBPWU_READLIGHT 0x0A
#define MBPWU_SETMETERDISPLAY 0x0B


#pragma pack(1)
typedef struct 
{
  UINT8 second;
  UINT8 minute;
  UINT8 hour;
  UINT8 day;
  UINT8 month;
  UINT16 year;
  UINT8 dayOfWeek;            // day of week, Sun=0, Mon=1, ..., Sat=6
  INT8 TimeZone;
} TIME;
#pragma pack()

typedef void (*T_MBPWU_CALLBACK)(void *, HUINT8);

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
//ostream &operator<<(ostream &os_, C_BIPWU &source_);

/********************
 * CLASS PROTOTYPES *
 ********************/

class C_BIPWU
{
public:
   C_BIPWU();                        //Constructor
   ~C_BIPWU();                       //Destructor

   HUINT32 Options(C_WINUSB *serPortPtr_,char *bipName);
   void ReceiveCB(HUINT8 *bufferPtr_, HUINT32 bufferLen_);
   HUINT32 SetReceiveCallback(void* routine, void *cbData);

   //Commands to test jig
	HBOOL SendPing(void);
	HBOOL SendReadEE(HUINT32 address, HUINT32 count);
	HBOOL SendWriteEE(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize);
	HBOOL SendReadVersion(void);
	HBOOL SendReboot(void);
	HBOOL SendSetRTC(TIME t);
	HBOOL SendGetRTC(void);
	HBOOL SendSetAnalogOut(HUINT8 channel, HUINT8 dcBias, HUINT8 acLevel, HUINT8 range);
	HBOOL SendGetMeterReading(HUINT8 ac, char *rangeSettingStr);
	HBOOL SendSetDigitalOut(HUINT8 output, HUINT8 state);
	HBOOL SendReadLightLevel(void);
	HBOOL SendSetMeterDisplay(char *str);


   //Data from responses
	HUINT8 m_ack;
	HUINT8 m_eeData[BIPWU_MAXIMUM_RX_PAYLOAD];
	HUINT16 m_version;
	TIME m_timeData;
	double m_meterReading;
	float m_lightLevel;

	 HBOOL MessageWait(HUINT8 expectedResp_, HUINT32 timeout_);
   HBOOL ResponseWait(HUINT32 timeout_);

   HUINT8 GetLastRxCode(void);
   HBOOL IsLastRxValid(void);
   void ResetLastRxValid(void);

   void RxDataPrint(void);

   //void Print(ostream *os_);
   //void PrintError(void);
   
   HANDLE m_messageReceived; //Event indicating a response was received.
   HANDLE m_responseReceived; //Event indicating a response was received.
	 UINT8 m_lastMessageSent;


private:
   C_WINUSB *m_serPortPtr;

   char m_bipName[100];


   HUINT32 m_dwError;

   T_BIPWU_RXSTATE m_rxState;

   HUINT8 m_txBuffer[BIPWU_TX_BUFFER_SIZE];
   HUINT32 m_txBufferCount;

   HUINT16 m_rxBufferCount;
   HUINT8 m_rxBuffer[BIPWU_RX_BUFFER_SIZE];
   HUINT16 m_rxCommandLength;
   HUINT16 m_rxChecksum;
   HUINT8 m_rxTimerCount;
   HUINT16 m_rxPacketChecksum;
   //HUINT32 m_rxLastTime;   //When was the last character received?

   HUINT8 m_lastRxCode;
   HBOOL m_lastRxValid;

   void DispatchPacket(void);
   
   static void CReceiveCB(void *dwUser_, HUINT8 *bufferPtr_, HUINT32 bufferLen_);

	T_MBPWU_CALLBACK m_rxCallback;
	void *m_rxCallbackData;

   void PrintBuffer(HUINT8 *buffer_, HUINT32 length_);

   
   void AddEscape(HUINT8 data_, HUINT8 *bufferPtr_, HUINT32 *bufferCountPtr_);
   HBOOL Encapsulate(HUINT8 address_, HUINT8 control_, HUINT8 *payloadPtr_, HUINT32 payloadSize_);

};


#endif
#endif
