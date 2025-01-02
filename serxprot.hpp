/****************************************************************************
FILE: SERXPROT.HPP

DESCRIPTION: <DESCRIPTION OF CONSTRUCTS>

COMPONENT: SER ( Serial Port Classes )

LANGUAGE: C++

NOTES:

        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

/****************
 * PREPROCESSOR *
 ****************/
#ifndef SERXPROT_HPP
#define SERXPROT_HPP

class C_SER;  //Forward references

/***********
 * INCLUDE *
 ***********/

#include <windows.h>
#include "htype.h"

/***************
 * DEFINITIONS *
 ***************/

typedef void (*T_SER_CALLBACK)(void *, HUINT8 *, HUINT32);

/**********************************
 * NON-MEMBER FUNCTION PROTOTYPES *
 **********************************/

/****************************************************************************
FUNCTION: <FUNCTION NAME>

DESCRIPTION: <FUNCTION DESCRIPTION>

FILENAME: SERXPROT.HPP

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
//ostream &operator<<(ostream &os_, C_SER &source_);

/********************
 * CLASS PROTOTYPES *
 ********************/

/***************************************************************************
CLASS NAME:    C_<COMPONENT>_<CLASS_NAME>

DESCRIPTION: <DESCRIPTION>

INHERITANCE: <LIST CLASSES DERIVED FROM>

FUNCTIONS (NOTE: EVERY ELEMENT UNDER THE DASHED AREA IS OPTIONAL IF NO FUNCTIONS
---------        OF THAT TYPE EXIST)
   PUBLIC
   ------
      <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      FRIEND: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      STATIC: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      OPERATORS: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>

   PROTECTED
   ---------
      <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      FRIEND: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      STATIC: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      OPERATORS: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>

   PRIVATE
   -------
      <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      FRIEND: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      STATIC: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>
      OPERATORS: <CLASS NAME> - <DESCRIPTION>
         INPUTS: <INPUT VARIABLES>
         OUTPUTS: <OUTPUT VARIABLES>
         RETURNS: <RETURNED VARIABLE>

MEMBER VARIABLES
----------------
   PUBLIC
   ------
        TYPE         NAME            DESCRIPTION
        ----         ----            -----------
       <TYPE>       <NAME>          <DESCRIPTION>

   PROTECTED
   ---------
        TYPE         NAME            DESCRIPTION
        ----         ----            -----------
       <TYPE>       <NAME>          <DESCRIPTION>

   PRIVATE
   -------
        TYPE         NAME            DESCRIPTION
        ----         ----            -----------
       <TYPE>       <NAME>          <DESCRIPTION>
        
NOTES: <SPECIAL INFORMATION NOT INDICATED ABOVE>


        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

class C_SER
{
public:
   C_SER();                        //Constructor
   ~C_SER();                       //Destructor

   HUINT32 Options(TCHAR *portNameStr_,  DWORD portBaud_);
	HUINT32 UseXONXOFF(HBOOL state_);
	//UINT32 Cbus(UINT8 index, HBOOL state_);
	HUINT32 Dtr(HBOOL state_);
	HUINT32 Rts(HBOOL state_);
	HUINT32 Break(HBOOL state_);
	HUINT32 PortStatus(BYTE *dsr, BYTE *cts, BYTE *ri, BYTE *dcd);
   HUINT32 BaudSet(DWORD portBaud_);
   HUINT32 Open(void);
   HUINT32 Close(void);
   HUINT32 Write(HUINT8 *dataPtr_, HUINT32 requestSize_, HUINT32 *numWrittenPtr_);
   HUINT32 Read(HUINT8 *dataPtr_, HUINT32 requestSize_, HUINT32 *numReadPtr_);
   HBOOL IsOpen(void);

   void * GetCurrentCallBackOwner(void);
   HBOOL AddReceiveCB(T_SER_CALLBACK routinePtr_, void *callBackIDPtr_);
   HBOOL ReceiveEnable(HBOOL state_);
   void Iteration(void);

	void PrintError(DWORD errorCode);

private:
   HANDLE m_portHandle;
   TCHAR *m_portNameStr;
   DWORD m_portBaud;

   HUINT32 m_dwError;

	 UINT8 m_cbusOutputs;

   T_SER_CALLBACK m_callBackPtr;
   void *m_callBackID;   

   HANDLE m_threadExitEvent;
   HANDLE m_threadStoppedEvent;
   HANDLE m_receiveDisableEvent;
   HANDLE m_dataMutex;
   HCHAR  m_dataMutexName[12];

   static void CIteration(void * dwUser_);
};



#endif
