//------------------------------------------------------------------
//
//  FILE      : StdLithDefs.h
//
//  PURPOSE   : Defines types and stuff for StdLith files.
//
//  CREATED   : September 7 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------


#ifndef __STDLITHDEFS_H__
#define __STDLITHDEFS_H__


// Includes....
#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __STRING_H__
#include <string.h>
#define __STRING_H__
#endif

#ifndef __ASSERT_H__
#include <assert.h>
#define __ASSERT_H__
#endif

// These are also in sdk/incs/ltbasetypes.h, however, since the stuff in lithshared 
// should compile with out the engine, they are duplicated here
// BEGIN LTBASETYPES.H

typedef char				int8;
typedef short int			int16;
typedef int					int32;

#ifdef __LINUX
typedef long long           int64;
#else
typedef __int64             int64;
#endif

typedef unsigned char		uint8;
typedef unsigned short int	uint16;
typedef unsigned int		uint32;

#ifdef __LINUX
typedef unsigned long long  uint64;
#else
typedef unsigned __int64    uint64;
#endif

typedef unsigned int LTBOOL;
typedef float LTFLOAT;
typedef uint32 LTRESULT;
// END LTBASETYPES.H

#ifndef ASSERT
    #define ASSERT assert
#endif

#ifndef TRUE
    #define TRUE    1
#endif

#ifndef FALSE
    #define FALSE   0
#endif

#define BAD_INDEX   ((uint32)-1)

#endif  // __STDLITH_DEFS_H__



