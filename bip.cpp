#line 2 "bip.cpp"
/****************************************************************************
FILE: BIP.CPP

DESCRIPTION: <DESCRIPTION>

COMPONENT: BIP ( Bootloader Interface Protocol Class )

LANGUAGE: C++

CONTENTS:

   File              Procedure                  Description
   ----              ---------                  -----------
   <FILENAME>        <FUNCTION>                 <DESCRIPTION>

NOTES:

        (c) Copyright 2001 Mobiltex Data Ltd.

****************************************************************************/

/****************
 * PREPROCESSOR *
 ****************/

/*****************
 * INCLUDE FILES *
 *****************/
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <float.h>
#include "htype.h"
#include "BIPiprot.hpp"
#include "fcs16.hpp"

/***************************
 * VARIABLES AND CONSTANTS *
 ***************************/

#define BOOTLOADER_KEY 0xA5125A34


/**********************
 * INTERNAL VARIABLES *
 **********************/


/********************************
 * EXTERNALLY VISIBLE VARIABLES *
 ********************************/
#define IP(a,b,c,d) (((UINT32)(a)<<24)|((UINT32)(b)<<16)|((UINT32)(c)<<8)|(UINT32)(d))

const T_PARAM_PARAMETERS param_data_defaults=
{
	PARAM_VERSION, //UINT8 paramVersion;

	CONFIGFLAGS_DCENABLED, //UINT16 configFlags;

	1000, //UINT32 samplingCyclePeriod; /* Number of milliseconds between readings */
	100, //UINT32 samplingCyclePeriodFast; /* Number of milliseconds between fast sampling mode readings, must be 100ms or greater */
	60, //UINT32 samplingFastDuration; /* Number of seconds for a fast sampling mode */
	30*60UL, //UINT32 samplingFastInterval; /* Number of seconds between fast sampling groups */

	10000, // UINT16 interruptionCycleTime; /* Future use for sync to interruption cycles (ms) */
	1000, // UINT16 interruptionOffTime; /* Future use for sync to interruption cycles (ms) */
	100, // UINT16 interruptionOffToOnOffset; /* Future use for sync to interruption cycles (ms) */
	100, //UINT16 interruptionOnToOffOffset; /* Future use for sync to interruption cycles (ms) */
	0, //INT16 interruptionUTCOffset; /* Future use for sync to interruption cycles (ms) */
	0, //UINT8 interruptionOnFirst; /* Future use for sync to interruption cycles (ms) */	

	2, //UINT8 rangeSelect; /*0=low, 1=med, 2=high*/
	{-1000.0, 1000.0}, //FLOAT32 lowLimit[2];
	{-1000.0, 1000.0}, //	FLOAT32 highLimit[2];
	{1.0, 1.0}, //	FLOAT32 scaleFactor[2];
	{0.0, 0.0}, //	FLOAT32 offsetFactor[2];

	3150,	//UINT16 batteryLowThreshold;
	3300,	//UINT16 batteryWarnThreshold;

	"", //UINT8 notes[PARAM_NOTES_SIZE]; //Notes field
	
  0, //UINT16 samplingCycleOffset; /* Number of milliseconds to delay sampling on normal sampling mode */
  
  0, //UINT16 configFlagsExtended; /* Extended config flags */

  0, //INT16 timeZoneOffsetMins; /* Time zone offset minutes from UTC */
  
  80, //UINT16 minimumEHPE; //Minimum EHPE for position recording (fixed point with 0.1m resolution)
  300, //UINT16 minimumEHPEWaitTimeSec; //Maximum number of seconds to wait for a good EHPE

  0, //UINT8 lockedFastSamplingOffsetMs; //Number of milliseconds to offset from 100ms boundary before starting fast sampling (0-99ms)
  
	0, //UINT32 delayStartSec; //Delay before starting acquisition

  {0}, //UINT8 spare8[1]; //For future use so we don't have to bump the parameter version number for minor changes
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //UINT32 spare32[28]; //For future use so we don't have to bump the parameter version number for minor changes

	117, //UINT16 firmwareVerParamWrite; //Used to flag a firmware to update any changed areas in the configuration	
	
	0 //UINT16 crc16; /*patch up on defaults install*/
};


const T_PARAM_MFGDATA param_mfgdata_defaults=
{
	PARAM_MFG_VERSION, //UINT8 paramVersion;

	0, //UINT32 optionFlags;
	
	{
        {{0,0,0},{4.77676E-006,6.47599E-004,5.19762E-003}}, //zeroCal[3], scaleFactor[3]
        {{0,0,0},{2.73466E-006,3.38183E-004,3.02280E-03}},
	}, //T_CHANNEL_CALIBRATION calibration[2]; //DC/AC
	
	{0,0,0}, //UINT16 acFloorCal[3]; //AC floor for zero display

	0, //INT16 calTemperature; //fixed point, LSB = 0.01C
	0, //UINT32 calDate; //C time
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //UINT32 spare[29];

	0 //UINT16 crc16; /*patch up on defaults install*/
};

const char *bip_faultStrings[]=
{
	"Configuration fault", //#define FAULT_CONFIG_INVALID   		0x00000001
	"GPS initialization fault", //#define FAULT_GPS_INIT 				 		0x00000002
	"EEPROM fault", //#define FAULT_EEPROM					 		0x00000004
	"Battery fault", //#define FAULT_BATTERY					 		0x00000008
	"ADC fault", //#define FAULT_ADC							 		0x00000010
	"Dataflash fault", //#define FAULT_DATAFLASH				 		0x00000020

	"Calibration fault", //#define FAULT_CALIBRATION					0x00000040
	"Dataflash write location not blank", //#define FAULT_DATAFLASH_FF		 		0x00000080

	"Dataflash blank locate fault", //#define FAULT_DATAFLASH_FINDSTART 0x00000100
	"Reserved 0x00000200", //Reserved								0x00000200
	"Reserved 0x00000400", //Reserved								0x00000400
	"Reserved 0x00000800", //Reserved								0x00000800

	"Reserved 0x00001000", //Reserved								0x00001000
	"Reserved 0x00002000", //Reserved								0x00002000
	"Reserved 0x00004000", //Reserved								0x00004000
	"Reserved 0x00008000", //Reserved								0x00008000
	
	"Reserved 0x00010000", //Reserved								0x00010000
	"Reserved 0x00020000", //Reserved								0x00020000
	"Reserved 0x00040000", //Reserved								0x00040000
	"Reserved 0x00080000", //Reserved								0x00080000
	
	"Reserved 0x00100000", //Reserved								0x00100000
	"Reserved 0x00200000", //Reserved								0x00200000
	"Reserved 0x00400000", //Reserved								0x00400000
	"Reserved 0x00800000", //Reserved								0x00800000
	
	"Reserved 0x01000000", //Reserved								0x01000000
	"Reserved 0x02000000", //Reserved								0x02000000
	"Reserved 0x04000000", //Reserved								0x04000000
	"Reserved 0x08000000", //Reserved								0x08000000
	
	"Reserved 0x10000000", //Reserved								0x10000000
	"Reserved 0x20000000", //Reserved								0x20000000
	"Reserved 0x40000000", //Reserved								0x40000000
	"Reserved 0x80000000" //Reserved								0x80000000
};


/********************************
 * EXTERNALLY DEFINED VARIABLES *
 ********************************/



/****************************************************************************
FUNCTION: <FUNCTION NAME WITH SCOPE INDICATED>

DESCRIPTION: <FUNCTION DESCRIPTION>

FILE: SMC.CPP

COMPONENT: SMC ( Private Channel )

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


C_BIP::C_BIP()
{
   //C_LOG::Printf(255,__FILE__,__LINE__,TRUE,"Constructing object");
   //MessageBox(NULL,L"Instantiating",L"Status",MB_OK);
	int i;

   m_serPortPtr=NULL;
   m_dwError=0;
   m_lastRxValid=FALSE;
   m_rxState=BIP_STATE_FLAG;
   m_rxBufferCount=0;
   m_rxCallback=NULL;
   strcpy(m_bipName,"X");

   //MessageBox(NULL,L"Instantiating middle",L"Status",MB_OK);

	//
	// Create the message received signal event object
	//
	m_messageReceived = CreateEvent(NULL,		// Security
		  			                    TRUE,		// Manual reset
						                 FALSE,		// Initial State
						                 NULL);		// Name

	m_responseReceived = CreateEvent(NULL,		// Security
		  			                    TRUE,		// Manual reset
						                 FALSE,		// Initial State
						                 NULL);		// Name
   //MessageBox(NULL,L"Instantiating Done",L"Status",MB_OK);
		memset(&m_gpsInfo,0,sizeof(T_GPSDATA_DEF));

		m_faults=0;
}

C_BIP::~C_BIP()
{
   Options(NULL,"");
	 CloseHandle(m_messageReceived);
	 CloseHandle(m_responseReceived);
   //C_LOG::Printf(255,__FILE__,__LINE__,TRUE,"Destructing object");
}

HUINT32 C_BIP::Options(C_SER *serPortPtr_, char *bipName)
{
   m_dwError=0;
   m_rxState=BIP_STATE_FLAG;
   m_lastRxValid=FALSE;

   //Release Callback on prior port
   if (m_serPortPtr)
   {
      m_serPortPtr->ReceiveEnable(FALSE);
      m_serPortPtr->AddReceiveCB(NULL,NULL);
   }

   m_serPortPtr=serPortPtr_;

   //If we have a valid port, enable receive call backs
   if (m_serPortPtr)
   {
      m_serPortPtr->AddReceiveCB( (T_SER_CALLBACK)(&C_BIP::CReceiveCB), (void *)this);
      m_serPortPtr->ReceiveEnable(TRUE);
		//m_serPortPtr->UseXONXOFF(FALSE);
   }

   strcpy(m_bipName,bipName);
   return m_dwError;
}

HUINT32 C_BIP::SetReceiveCallback(void* routine, void *cbData)
{
	m_rxCallback=(T_MBP_CALLBACK)routine;
	m_rxCallbackData=cbData;
	return 0;
}

void C_BIP::CReceiveCB(void *dwUser_, HUINT8 *bufferPtr_, HUINT32 bufferLen_)
{
   ((C_BIP*)dwUser_)->ReceiveCB(bufferPtr_, bufferLen_);
}

void C_BIP::ReceiveCB(HUINT8 *bufferPtr_, HUINT32 bufferLen_)
{
   HUINT8 *nextPtr, *endPtr;
   HUINT8 currChar;
   //unsigned short wstr[100];

   //printf("X");

   nextPtr=bufferPtr_;
   endPtr=bufferPtr_+bufferLen_;

   while(nextPtr<endPtr)
   {
      currChar=*(nextPtr++);
      //wsprintf(wstr,L"[%s%02X]",m_bipName,currChar);
      //OutputDebugString(wstr);
      //printf("[%s,%02X,%c]",m_bipName,currChar,isprint(currChar)?currChar:'.');
			//printf("{%02X}",currChar);

		if (currChar==BIP_FLAG)
		{
         //printf("f");
			//Dispatch if valid length (A+P+M+2CRC) and reset state
			if (m_rxBufferCount>=5)
			{
            if (C_FCS::Verify(m_rxBuffer,m_rxBufferCount))
            {
               //printf("%s: Received packet.\r\n",m_bipName);
					DispatchPacket();
            }
            //else
            //printf("%s: CRC error in receive packet.\r\n",m_bipName);
			}
         else
         {
            //printf("Received packet is too short.\r\n");
         }
			//Stay in the receiving data state, since a closing flag can be an opening flag
			//for the next packet
			m_rxBufferCount=0;
			m_rxState=BIP_STATE_DATA;
		}
      else
		switch(m_rxState)
		{
			case BIP_STATE_FLAG:
            //printf("F");
				//This state should only occur at initialization and after a length error
				if (currChar==BIP_FLAG)
				{
					m_rxBufferCount=0;
					m_rxState=BIP_STATE_DATA;
				}
				break;

			case BIP_STATE_DATA:
            if (currChar==BIP_ESCAPE)
				{
               //printf("e");
					//Discard character and move to ESCAPE state
					m_rxState=BIP_STATE_ESCAPE;
				}
				else
				{
               //printf("d");
					//Verify buffer can hold the data
					if (m_rxBufferCount<(BIP_MAXIMUM_RX_PAYLOAD+4))
					{
						m_rxBuffer[m_rxBufferCount++]=currChar;
					}
					else
					{
						m_rxState=BIP_STATE_FLAG;
					}
				}
				break;

			case BIP_STATE_ESCAPE:
            //printf("E");
				//Verify buffer can hold the data
				if (m_rxBufferCount<(BIP_MAXIMUM_RX_PAYLOAD+4))
				{
					m_rxBuffer[m_rxBufferCount++]=currChar^0x20;
					m_rxState=BIP_STATE_DATA;
				}
				else
				{
					m_rxState=BIP_STATE_FLAG;
				}
				break;

			default:
            //printf("D");
				m_rxState=BIP_STATE_FLAG;
				break;
		}
   }
}



void C_BIP::DispatchPacket(void)
{
   HUINT32 i;
	 INT16 temps16;
   //Verify address
   if ((m_rxBuffer[BIP_ADDRESS]==BIP_PCANT_ADDRESS)&&(m_rxBuffer[BIP_PROTOCOL]==BIP_PCANT_PROTOCOL))
   //if ((m_rxBuffer[BIP_ADDRESS]==BIP_PCANT_ADDRESS))
   {
      //Retain code for use by controlling functions
      m_lastRxCode=m_rxBuffer[BIP_MSGTYPE];
      //m_actionOK=m_rxBuffer[BIP_START_OF_PAYLOAD]>0?TRUE:FALSE;
      //printf("%s(0x%02x)Rx:",m_bipName,m_lastRxCode);
      //printf("%s<%02X><%02X><%02X><%02X><%02X>\n",m_bipName,m_rxBuffer[1],m_rxBuffer[2],m_rxBuffer[3],m_rxBuffer[4],m_rxBuffer[5]);
      //fflush(stdout);
      //for (i=0; i<m_rxBufferCount; i++)
      //{
      //   printf("%02X/",m_rxBuffer[i]);
      //   fflush(stdout);
      //}
      //printf("\n");
      //fflush(stdout);


			if (m_lastRxCode!=MBP_DEBUGINFO_RESP) m_ack=m_rxBuffer[BIP_START_OF_PAYLOAD];
      switch(m_lastRxCode)
      {
			case MBP_PING_RESP:
			case MBP_GETVERSION_RESP:
				memcpy(&m_appVersion,m_rxBuffer+BIP_START_OF_PAYLOAD+1,4);
				memcpy(&m_bootVersion,m_rxBuffer+BIP_START_OF_PAYLOAD+5,4);
				memcpy(&m_serialNumber,m_rxBuffer+BIP_START_OF_PAYLOAD+9,4);
				m_paramVersion=m_rxBuffer[BIP_START_OF_PAYLOAD+13];
				memcpy(&m_storageUsedBytes,m_rxBuffer+BIP_START_OF_PAYLOAD+14,4);
				memcpy(&m_storageTotalBytes,m_rxBuffer+BIP_START_OF_PAYLOAD+18,4);
				if (m_appVersion<115)
				{
					m_boardType=UDL1_BOARDTYPE_G1;
				}
				else
				{
					m_boardType=m_rxBuffer[BIP_START_OF_PAYLOAD+22];
				}
				break;

			case MBP_READEEPROM_RESP:
				memcpy(m_dataBuffer,&m_rxBuffer[BIP_START_OF_PAYLOAD+1],BIP_MAXIMUM_RX_PAYLOAD);
				break;
			case MBP_WRITEEEPROM_RESP:
				break;

			case MBP_ERASEFLASH_RESP:
				break;

			case MBP_READFLASH_RESP:
				memcpy(m_dataBuffer,&m_rxBuffer[BIP_START_OF_PAYLOAD+1],BIP_MAXIMUM_RX_PAYLOAD);
				break;

			case MBP_WRITEFLASH_RESP:
				break;

			case MBP_ERASEDF_RESP:
				break;

			case MBP_READDF_RESP:
				memcpy(m_dataBuffer,&m_rxBuffer[BIP_START_OF_PAYLOAD+1],BIP_MAXIMUM_RX_PAYLOAD);
				break;

			case MBP_WRITEDF_RESP:
				break;

			case MBP_REBOOT_RESP:
				break;

			case MBP_LOCKINBOOT_RESP:
				break;

			case MBP_GETAPPINFO_RESP:
				m_appValid=m_rxBuffer[BIP_START_OF_PAYLOAD+1];
				memcpy(&m_appVersion, m_rxBuffer+BIP_START_OF_PAYLOAD+2, sizeof(HUINT32));
				memcpy(&m_appChecksum,	m_rxBuffer+BIP_START_OF_PAYLOAD+6, sizeof(HUINT32));
				break;

			case MBP_GETGPSINFO_RESP:
				memcpy(&m_gpsInfo, m_rxBuffer+BIP_START_OF_PAYLOAD+1, sizeof(T_GPSDATA_DEF));
				break;

			case MBP_GETBATTERYINFO_RESP:
				memcpy(&m_batteryVolts, m_rxBuffer+BIP_START_OF_PAYLOAD+1, sizeof(m_batteryVolts));
				memcpy(&m_batteryTemperature, m_rxBuffer+BIP_START_OF_PAYLOAD+1+sizeof(m_batteryVolts), sizeof(m_batteryTemperature));
				m_batteryCharging=m_rxBuffer[BIP_START_OF_PAYLOAD+1+sizeof(m_batteryVolts)+sizeof(m_batteryTemperature)]?TRUE:FALSE;
				break;

			case MBP_GETFAULTS_RESP:
				memcpy(&m_faults, m_rxBuffer+BIP_START_OF_PAYLOAD+1, sizeof(m_faults));
				memcpy(&m_faultHistory, m_rxBuffer+BIP_START_OF_PAYLOAD+1+sizeof(m_faults), sizeof(m_faultHistory));
				break;

			case MBP_POWERDOWN_RESP:
				break;

			case MBP_DEBUGINFO_RESP:
				printf("%s",(char*)(m_rxBuffer+BIP_START_OF_PAYLOAD));
				break;

			case MBP_SETRTC_RESP:
				break;

			case MBP_GETRTC_RESP:
				memcpy(&m_rtcTime, m_rxBuffer+BIP_START_OF_PAYLOAD+1, sizeof(HUINT32));
				memcpy(&m_1ppsCount, m_rxBuffer+BIP_START_OF_PAYLOAD+5, sizeof(HUINT32));
				memcpy(&m_1ppsMsCount, m_rxBuffer+BIP_START_OF_PAYLOAD+9, sizeof(HUINT16));
				m_rtcTimeLock=m_rxBuffer[BIP_START_OF_PAYLOAD+11]?TRUE:FALSE;
				break;

			case MBP_CLEARSTORAGESTART_RESP:
				break;

			case MBP_CLEARSTORAGESTATUS_RESP:
				m_storageClearBusy=m_rxBuffer[BIP_START_OF_PAYLOAD+1];
				m_storageClearPercent=m_rxBuffer[BIP_START_OF_PAYLOAD+2];
				m_storageClearFailure=m_rxBuffer[BIP_START_OF_PAYLOAD+3];
				break;

			case MBP_SETSERIAL_RESP:
				break;

			case MBP_SETINPUTRANGE_RESP:
				break;

			case MBP_READRAWANALOG_RESP:
				memcpy(&m_rawMeasurement,m_rxBuffer+BIP_START_OF_PAYLOAD+1,sizeof(HINT32));
				break;

			case MBP_READSCALEDANALOG_RESP:
				memcpy(&m_scaledMeasurement,m_rxBuffer+BIP_START_OF_PAYLOAD+1,sizeof(HINT32));
				break;

			case MBP_CLEARFAULTHISTORY_RESP:
				break;

       default:
          //for (i=0; i<m_rxBufferCount; i++)
          //   printf("[%02X,%c]",m_rxBuffer[i],isprint(m_rxBuffer[i])?m_rxBuffer[i]:'.');
          break;
      }
      m_lastRxValid=TRUE;
      SetEvent(m_messageReceived);
			if ((m_lastMessageSent|0x80)==m_lastRxCode) SetEvent(m_responseReceived);
	  if (m_rxCallback) (*m_rxCallback)(m_rxCallbackData,m_lastRxCode);
   }
}

HUINT8 C_BIP::GetLastRxCode(void)
{
   return m_lastRxCode;
}

HBOOL C_BIP::IsLastRxValid(void)
{
   return m_lastRxValid;
}

void C_BIP::ResetLastRxValid(void)
{
   ResetEvent(m_messageReceived);
	 ResetEvent(m_responseReceived);
   m_lastRxValid=FALSE;
}


void C_BIP::RxDataPrint(void)
{
}


void C_BIP::AddEscape(HUINT8 data_, HUINT8 *bufferPtr_, HUINT32 *bufferCountPtr_)
{
	if ((data_==BIP_ESCAPE)||(data_==BIP_FLAG))
	{
		bufferPtr_[(*bufferCountPtr_)++]=BIP_ESCAPE;
   	bufferPtr_[(*bufferCountPtr_)++]=data_^0x20;
	}
   else
   {
   	bufferPtr_[(*bufferCountPtr_)++]=data_;
   }
}

HBOOL C_BIP::Encapsulate(HUINT8 address_, HUINT8 control_, HUINT8 *payloadPtr_, HUINT32 payloadSize_, HUINT8 protocol_)
{
	HUINT32 i;
   HUINT32 written;
   HUINT16 fcstemp;

	m_txBufferCount=0;
	m_txBuffer[m_txBufferCount++]=BIP_FLAG;

   AddEscape(address_,m_txBuffer,&m_txBufferCount);
   fcstemp=C_FCS::Generate(FCS16_INIT,&address_,1L);

   AddEscape(protocol_,m_txBuffer,&m_txBufferCount); //Protocol Number
   fcstemp=C_FCS::Generate(fcstemp,&protocol_,1L);

   AddEscape(control_,m_txBuffer,&m_txBufferCount);
   fcstemp=C_FCS::Generate(fcstemp,&control_,1L);

   for (i=0; i<payloadSize_; i++)
		AddEscape(payloadPtr_[i], m_txBuffer,&m_txBufferCount);
   fcstemp=C_FCS::Generate(fcstemp,payloadPtr_,payloadSize_);

   fcstemp^=0xFFFF;
	AddEscape((HUINT8)fcstemp,m_txBuffer,&m_txBufferCount);
	AddEscape((HUINT8)(fcstemp>>8),m_txBuffer,&m_txBufferCount);

   m_txBuffer[m_txBufferCount++]=BIP_FLAG;
   m_txBuffer[m_txBufferCount++]=BIP_FLAG;
   //if (!m_serPortPtr->SendData(m_txBuffer,m_txBufferCount))
#if 0
   printf("%s(0x%02x)Tx:",m_bipName,control_);
   for (i=0; i<m_txBufferCount; i++)
   {
      printf("%02X/",m_txBuffer[i]);
      fflush(stdout);
   }
   printf("\n");
   fflush(stdout);
#endif   
	 if (address_==BIP_PCANT_ADDRESS)
	 {
		 m_lastMessageSent=control_;
	 }
	if (!m_serPortPtr->Write(m_txBuffer,m_txBufferCount,(HUINT32*)&written))
	{
			return FALSE;
	}
	 else
	{
			return TRUE;
	}
}



//Commands to uDL1 
HBOOL C_BIP::SendGeneric(UINT8 command)
{
   ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, command, NULL, 0);   
}

HBOOL C_BIP::SendReadEEPROM(HUINT32 address, HUINT32 count)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 memcpy(data,&address,4);
   memcpy(data+4,&count,4);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_READEEPROM, data, 8);
}

HBOOL C_BIP::SendWriteEEPROM(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 if (dataBufferSize>(BIP_MAXIMUM_TX_PAYLOAD-8)) return FALSE;

   memcpy(data,&address,4);
   memcpy(data+4,&dataBufferSize,4);

   memcpy(data+8,dataBuffer,dataBufferSize);
		
	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_WRITEEEPROM, data, 8+dataBufferSize);
}

HBOOL C_BIP::SendEraseFlash(HUINT32 address, HUINT32 count)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 memcpy(data,&address,4);
   memcpy(data+4,&count,4);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_ERASEFLASH, data, 8);
}

HBOOL C_BIP::SendReadFlash(HUINT32 address, HUINT32 count)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 memcpy(data,&address,4);
   memcpy(data+4,&count,4);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_READFLASH, data, 8);
}


HBOOL C_BIP::SendWriteFlash(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 if (dataBufferSize>(BIP_MAXIMUM_TX_PAYLOAD-8)) return FALSE;

   memcpy(data,&address,4);
   memcpy(data+4,&dataBufferSize,4);

   memcpy(data+8,dataBuffer,dataBufferSize);
		
	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_WRITEFLASH, data, 8+dataBufferSize);
}

HBOOL C_BIP::SendEraseDataFlash(HUINT32 address, HUINT32 count, HUINT32 eraseSectorCode)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 memcpy(data,&address,4);
   memcpy(data+4,&count,4);
   memcpy(data+8,&eraseSectorCode,4);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_ERASEDF, data, 12);
}

HBOOL C_BIP::SendReadDataFlash(HUINT32 address, HUINT32 count)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 memcpy(data,&address,4);
   memcpy(data+4,&count,4);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_READDF, data, 8);
}


HBOOL C_BIP::SendWriteDataFlash(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 if (dataBufferSize>(BIP_MAXIMUM_TX_PAYLOAD-8)) return FALSE;

   memcpy(data,&address,4);
   memcpy(data+4,&dataBufferSize,4);

   memcpy(data+8,dataBuffer,dataBufferSize);
		
	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_WRITEDF, data, 8+dataBufferSize);
}

HBOOL C_BIP::SendSetRTC(HUINT32 rtcTime)
{
	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_SETRTC, (HUINT8*)&rtcTime, 4);
}

HBOOL C_BIP::SendSetSerial(HUINT32 serial)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 *((HUINT32*)data)=MBP_SETSERIALKEY;
	 *(((HUINT32*)data)+1)=serial;

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_SETSERIAL, data, 8);
}

HBOOL C_BIP::SendSetInputRange(HUINT8 range)
{
	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_SETINPUTRANGE, &range, 1);
}

HBOOL C_BIP::SendReadRawAnalog(HUINT8 channel, HUINT8 samplesForAverage)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 data[0]=channel;
	 data[1]=samplesForAverage;

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_READRAWANALOG, data, 2);
}

HBOOL C_BIP::SendReadScaledAnalog(HUINT8 channel, HUINT8 samplesForAverage)
{
	HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	data[0]=channel;
	data[1]=samplesForAverage;
	ResetLastRxValid();
  return Encapsulate(BIP_PCANT_ADDRESS, MBP_READSCALEDANALOG, data, 2);
}

HBOOL C_BIP::SendSetPortOutput(HUINT8 port, HUINT8 pinIndex, HUINT16 state)
{
   HUINT8 data[BIP_MAXIMUM_TX_PAYLOAD];

	 data[0]=port;
	 data[1]=pinIndex;
	 memcpy(data+2,&state,2);

	 ResetLastRxValid();
   return Encapsulate(BIP_PCANT_ADDRESS, MBP_SETPORTOUTPUT, data, 4);
}



HBOOL C_BIP::MessageWait(HUINT8 expectedResp_, HUINT32 timeout_)
{
   HBOOL dead;
   dead=FALSE;
   if (WaitForSingleObject(m_messageReceived,timeout_)!=WAIT_OBJECT_0)
   {  //Timed out
      //printf("Timeout Failure.\n");
      dead=TRUE;
   }
   else
   {
      if (m_lastRxCode!=expectedResp_)
      {  //Wrong response
         //printf("Response Failure(%u).\n",m_lastRxCode);
         dead=TRUE;
      }
   }
   return !dead;
}

HBOOL C_BIP::ResponseWait(HUINT32 timeout_)
{
   if (WaitForSingleObject(m_responseReceived,timeout_)!=WAIT_OBJECT_0)
   {  //Timed out
      return FALSE;
   }
   else
   {
		 return TRUE;
   }
}


void C_BIP::PrintBuffer(HUINT8 *buffer_, HUINT32 length_)
{
   HUINT32 i;

   for (i=0; i<length_; i++)
   {
      printf("<%02X>",buffer_[i]);
   }
   printf("\n");

}


