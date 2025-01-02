/****************************************************************************
FILE: HType.H

DESCRIPTION: This file contains the standard 'Higher' types.
 

COMPONENT: Used by all components.

LANGUAGE: C++

NOTES:

        (c) Copyright 1999 Mobiltex Data Ltd.

****************************************************************************/

/****************
 * PREPROCESSOR *
 ****************/

#ifndef HTYPE_H
#define HTYPE_H

/***********
 * INCLUDE *
 ***********/

/************** 
 * DEFINITION * 
 **************/

#ifndef _ASMLANGUAGE       /* asm modules don't understand typedef */
typedef char                 HCHAR;
typedef unsigned char        HUCHAR;
typedef char                 HINT8;
typedef unsigned char        HUINT8;
typedef __int16                HINT16;
typedef unsigned __int16       HUINT16;
typedef long           HINT32;
typedef DWORD        HUINT32;
typedef float                HFLOAT32;
typedef double               HFLOAT64;

typedef char                 CHAR;
typedef unsigned char        UCHAR;
//typedef char                 INT8;
typedef unsigned char        UINT8;
typedef __int16                INT16;
typedef unsigned __int16       UINT16;
//typedef long                 INT32;
//typedef unsigned long        UINT32;
typedef float                FLOAT32;
typedef double               FLOAT64;

/*
 * WARNING
 *
 * The following definitions for HBOOL must both compile to 1 byte types
 * in order for C and C++ code to interoperate.  This has been verified
 * with the following compilers:
 *
 * - VxWorks GNU C++ "CC386" compiler 
 */

#ifdef __cplusplus
typedef bool HBOOL;
//typedef bool BOOL;
#else 
/*typedef enum { false, true } HBOOL; */
typedef HUINT8 HBOOL;
//typedef HUINT8 BOOL;
#define false 0
#define true  1
#endif

#define FALSE 0
#define TRUE 1


#endif /* _ASMLANGUAGE */
#endif /* HTYPE_H */
