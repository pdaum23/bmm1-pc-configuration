#line 2 "bipwu.cpp"
/****************************************************************************
FILE: BIP.CPP

DESCRIPTION: <DESCRIPTION>

COMPONENT: BIPWU ( Binary Interface Protocol WINUSB Class )

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
#include "BIPWUiprot.hpp"
#include "fcs16.hpp"

/***************************
* VARIABLES AND CONSTANTS *
***************************/

#ifdef _DEBUG


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


C_BIPWU::C_BIPWU()
{
	//C_LOG::Printf(255,__FILE__,__LINE__,TRUE,"Constructing object");
	//MessageBox(NULL,L"Instantiating",L"Status",MB_OK);
	m_serPortPtr=NULL;
	m_dwError=0;
	m_lastRxValid=FALSE;
	m_rxState=BIPWU_STATE_FLAG;
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
}

C_BIPWU::~C_BIPWU()
{
	Options(NULL,"");
	 CloseHandle(m_messageReceived);
	 CloseHandle(m_responseReceived);
	//C_LOG::Printf(255,__FILE__,__LINE__,TRUE,"Destructing object");
}

HUINT32 C_BIPWU::Options(C_WINUSB *serPortPtr_, char *bipName)
{
	m_dwError=0;
	m_rxState=BIPWU_STATE_FLAG;
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
		m_serPortPtr->AddReceiveCB( (T_WINUSB_CALLBACK)(&C_BIPWU::CReceiveCB), (void *)this);
		m_serPortPtr->ReceiveEnable(TRUE);
		//m_serPortPtr->UseXONXOFF(FALSE);
	}

	strcpy(m_bipName,bipName);
	return m_dwError;
}

HUINT32 C_BIPWU::SetReceiveCallback(void* routine, void *cbData)
{
	m_rxCallback=(T_MBPWU_CALLBACK)routine;
	m_rxCallbackData=cbData;
	return 0;
}

void C_BIPWU::CReceiveCB(void *dwUser_, HUINT8 *bufferPtr_, HUINT32 bufferLen_)
{
	((C_BIPWU*)dwUser_)->ReceiveCB(bufferPtr_, bufferLen_);
}

void C_BIPWU::ReceiveCB(HUINT8 *bufferPtr_, HUINT32 bufferLen_)
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
		//printf("[%02X]",currChar);

		if (currChar==BIPWU_FLAG)
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
			m_rxState=BIPWU_STATE_DATA;
		}
		else
			switch(m_rxState)
		{
			case BIPWU_STATE_FLAG:
				//printf("F");
				//This state should only occur at initialization and after a length error
				if (currChar==BIPWU_FLAG)
				{
					m_rxBufferCount=0;
					m_rxState=BIPWU_STATE_DATA;
				}
				break;

			case BIPWU_STATE_DATA:
				if (currChar==BIPWU_ESCAPE)
				{
					//printf("e");
					//Discard character and move to ESCAPE state
					m_rxState=BIPWU_STATE_ESCAPE;
				}
				else
				{
					//printf("d");
					//Verify buffer can hold the data
					if (m_rxBufferCount<(BIPWU_MAXIMUM_RX_PAYLOAD+4))
					{
						m_rxBuffer[m_rxBufferCount++]=currChar;
					}
					else
					{
						m_rxState=BIPWU_STATE_FLAG;
					}
				}
				break;

			case BIPWU_STATE_ESCAPE:
				//printf("E");
				//Verify buffer can hold the data
				if (m_rxBufferCount<(BIPWU_MAXIMUM_RX_PAYLOAD+4))
				{
					m_rxBuffer[m_rxBufferCount++]=currChar^0x20;
					m_rxState=BIPWU_STATE_DATA;
				}
				else
				{
					m_rxState=BIPWU_STATE_FLAG;
				}
				break;

			default:
				//printf("D");
				m_rxState=BIPWU_STATE_FLAG;
				break;
		}
	}
}



void C_BIPWU::DispatchPacket(void)
{
	HUINT32 i;
	//Verify address
	if ((m_rxBuffer[BIPWU_ADDRESS]==BIPWU_BROADCAST_ADDRESS)&&(m_rxBuffer[BIPWU_PROTOCOL]==BIPWU_PROTOCOL_NUMBER))
		//if ((m_rxBuffer[BIPWU_ADDRESS]==BIPWU_BROADCAST_ADDRESS))
	{
		//Retain code for use by controlling functions
		m_lastRxCode=m_rxBuffer[BIPWU_MSGTYPE];
		//m_actionOK=m_rxBuffer[BIPWU_START_OF_PAYLOAD]>0?TRUE:FALSE;
#if 0
		printf("%s(0x%02x)Rx:",m_bipName,m_lastRxCode);
		//printf("%s<%02X><%02X><%02X><%02X><%02X>\n",m_bipName,m_rxBuffer[1],m_rxBuffer[2],m_rxBuffer[3],m_rxBuffer[4],m_rxBuffer[5]);
		//fflush(stdout);
		for (i=0; i<m_rxBufferCount; i++)
		{
			printf("%02X/",m_rxBuffer[i]);
			fflush(stdout);
		}
		printf("\n");
		fflush(stdout);
#endif
		switch(m_lastRxCode)
		{
		case MBPWU_PING_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(&m_version,m_rxBuffer+BIPWU_START_OF_PAYLOAD+1,2);
			break;

		case MBPWU_GETVERSION_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(&m_version,m_rxBuffer+BIPWU_START_OF_PAYLOAD+1,2);
			break;

		case MBPWU_READEEPROM_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(m_eeData,&m_rxBuffer[BIPWU_START_OF_PAYLOAD+1],BIPWU_MAXIMUM_RX_PAYLOAD);
			break;

		case MBPWU_WRITEEEPROM_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			//Do nothing
			break;

		case MBPWU_REBOOT_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			break;

		case MBPWU_SETRTC_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			break;

		case MBPWU_GETRTC_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(&m_timeData,m_rxBuffer+BIPWU_START_OF_PAYLOAD+1,sizeof(TIME));
			break;

		case MBPWU_SETANALOGOUT_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			break;

		case MBPWU_GETMETER_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(&m_meterReading,m_rxBuffer+BIPWU_START_OF_PAYLOAD+1,sizeof(double));
			break;

		case MBPWU_SETDIGITALOUT_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			break;

		case MBPWU_READLIGHT_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
			memcpy(&m_lightLevel,m_rxBuffer+BIPWU_START_OF_PAYLOAD+1,sizeof(float));
			break;

		case MBPWU_SETMETERDISPLAY_RESP:
			m_ack=m_rxBuffer[BIPWU_START_OF_PAYLOAD];
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

HUINT8 C_BIPWU::GetLastRxCode(void)
{
	return m_lastRxCode;
}

HBOOL C_BIPWU::IsLastRxValid(void)
{
	return m_lastRxValid;
}

void C_BIPWU::ResetLastRxValid(void)
{
	ResetEvent(m_messageReceived);
  ResetEvent(m_responseReceived);
	m_lastRxValid=FALSE;
}


void C_BIPWU::RxDataPrint(void)
{
}


void C_BIPWU::AddEscape(HUINT8 data_, HUINT8 *bufferPtr_, HUINT32 *bufferCountPtr_)
{
	if ((data_==BIPWU_ESCAPE)||(data_==BIPWU_FLAG))
	{
		bufferPtr_[(*bufferCountPtr_)++]=BIPWU_ESCAPE;
		bufferPtr_[(*bufferCountPtr_)++]=data_^0x20;
	}
	else
	{
		bufferPtr_[(*bufferCountPtr_)++]=data_;
	}
}

HBOOL C_BIPWU::Encapsulate(HUINT8 address_, HUINT8 control_, HUINT8 *payloadPtr_, HUINT32 payloadSize_)
{
	HUINT32 i;
	HUINT32 written;
	HUINT16 fcstemp;
	HUINT8 protocol;

	protocol=BIPWU_PROTOCOL_NUMBER;

	m_txBufferCount=0;
	m_txBuffer[m_txBufferCount++]=BIPWU_FLAG;

	AddEscape(address_,m_txBuffer,&m_txBufferCount);
	fcstemp=C_FCS::Generate(FCS16_INIT,&address_,1L);

	AddEscape(protocol,m_txBuffer,&m_txBufferCount); //Protocol Number
	fcstemp=C_FCS::Generate(fcstemp,&protocol,1L);

	AddEscape(control_,m_txBuffer,&m_txBufferCount);
	fcstemp=C_FCS::Generate(fcstemp,&control_,1L);

	for (i=0; i<payloadSize_; i++)
		AddEscape(payloadPtr_[i], m_txBuffer,&m_txBufferCount);
	fcstemp=C_FCS::Generate(fcstemp,payloadPtr_,payloadSize_);

	fcstemp^=0xFFFF;
	AddEscape((HUINT8)fcstemp,m_txBuffer,&m_txBufferCount);
	AddEscape((HUINT8)(fcstemp>>8),m_txBuffer,&m_txBufferCount);

	m_txBuffer[m_txBufferCount++]=BIPWU_FLAG;
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
	m_lastMessageSent=control_;

	if (!m_serPortPtr->Write(m_txBuffer,m_txBufferCount,(UINT32*)&written))
		return FALSE;
	else
		return TRUE;
}



//Commands to RDU1 
HBOOL C_BIPWU::SendPing(void)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_PING, NULL, 0);   
}

HBOOL C_BIPWU::SendReadVersion(void)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_GETVERSION, NULL, 0);
}


HBOOL C_BIPWU::SendReadEE(HUINT32 address, HUINT32 count)
{
	HUINT8 data[BIPWU_MAXIMUM_TX_PAYLOAD];

	memcpy(data,&address,4);
	memcpy(data+4,&count,4);

	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_READEEPROM, data, 8);
}


HBOOL C_BIPWU::SendWriteEE(HUINT32 address, HUINT8 *dataBuffer, HUINT32 dataBufferSize)
{
	HUINT8 data[BIPWU_MAXIMUM_TX_PAYLOAD];

	if (dataBufferSize>(BIPWU_MAXIMUM_TX_PAYLOAD-8)) return FALSE;

	memcpy(data,&address,4);
	memcpy(data+4,&dataBufferSize,4);

	memcpy(data+8,dataBuffer,dataBufferSize);

	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_WRITEEEPROM, data, 8+dataBufferSize);
}

HBOOL C_BIPWU::SendReboot(void)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_REBOOT, NULL, 0);
}

HBOOL C_BIPWU::SendSetRTC(TIME t)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_SETRTC, (HUINT8*)&t, sizeof(TIME));
}

HBOOL C_BIPWU::SendGetRTC(void)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_GETRTC, NULL, 0);
}

HBOOL C_BIPWU::SendSetAnalogOut(HUINT8 channel, HUINT8 dcBias, HUINT8 acLevel, HUINT8 range)
{
	HUINT8 data[BIPWU_MAXIMUM_TX_PAYLOAD];

	data[0]=channel;
	data[1]=dcBias;
	data[2]=acLevel;
	data[3]=range;

	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_SETANALOGOUT, data, 4);
}

HBOOL C_BIPWU::SendGetMeterReading(HUINT8 ac, char *rangeSettingStr)
{
	HUINT8 data[BIPWU_MAXIMUM_TX_PAYLOAD];

	data[0]=ac;
	strcpy((char*)(data+1),rangeSettingStr);

	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_GETMETER, data, 2+strlen(rangeSettingStr));
}

HBOOL C_BIPWU::SendSetDigitalOut(HUINT8 output, HUINT8 state)
{
	HUINT8 data[BIPWU_MAXIMUM_TX_PAYLOAD];

	data[0]=output;
	data[1]=state;

	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_SETDIGITALOUT, data, 2);
}

HBOOL C_BIPWU::SendReadLightLevel(void)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_READLIGHT, NULL, 0);
}

HBOOL C_BIPWU::SendSetMeterDisplay(char *str)
{
	ResetLastRxValid();
	return Encapsulate(BIPWU_BROADCAST_ADDRESS, MBPWU_SETMETERDISPLAY, (HUINT8*)str, 1+strlen(str));
}




HBOOL C_BIPWU::MessageWait(HUINT8 expectedResp_, HUINT32 timeout_)
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

HBOOL C_BIPWU::ResponseWait(HUINT32 timeout_)
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



void C_BIPWU::PrintBuffer(HUINT8 *buffer_, HUINT32 length_)
{
	HUINT32 i;

	for (i=0; i<length_; i++)
	{
		printf("<%02X>",buffer_[i]);
	}
	printf("\n");

}


#endif

