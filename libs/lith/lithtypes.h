/****************************************************************************
;
;   MODULE:     LITHTYPES (.H)
;
;   PURPOSE:    basic data types
;
;   HISTORY:    04/19/98 [blb] 
;
;   NOTICE:     Copyright (c) 1998, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __LITHTYPES_H__
#define __LITHTYPES_H__

// 8 bit
#ifndef BYTE
typedef unsigned char       BYTE;
#endif
#ifndef UINT8
typedef unsigned char       UINT8;
#endif
#ifndef INT8
#ifdef LITHTECH_ESD_INC // [mds] Updated to sync with real network's sdk
typedef char                INT8;
#else
typedef signed char         INT8;
#endif // LITHTECH_ESD_INC
#endif

// 16 bit
#ifndef WORD
typedef unsigned short int  WORD;
#endif
#ifndef UINT16
typedef unsigned short int  UINT16;
#endif

#ifndef INT16
typedef signed short int    INT16;
#endif

// 32 bit
#ifndef DWORD
typedef unsigned long int   DWORD;
#endif

#ifndef UINT32
#ifdef LITHTECH_ESD_INC // [mds] Updated to sync with real network's sdk
typedef unsigned long int   UINT32;
#else
typedef unsigned int        UINT32;
#endif // LITHTECH_ESD_INC
#endif
#ifndef INT32
#ifdef LITHTECH_ESD_INC // [mds] Updated to sync with real network's sdk
typedef long int            INT32;
#else
typedef int                 INT32;
#endif // LITHTECH_ESD_INC
#endif

// boolean
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif

// NULL
#ifndef NULL
#define NULL    0
#endif

// ASSERT
#ifndef ASSERT
#ifdef _DEBUG
#include "assert.h"
#define ASSERT(exp)     assert(exp)
#else
#define ASSERT(exp)     /* */
#endif
#endif

#endif



