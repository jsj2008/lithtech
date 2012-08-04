//------------------------------------------------------------------
//
//  FILE      : BDEFS.H
//
//  PURPOSE   : Base definitions header file
//
//  CREATED   : 1st May 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __BDEFS_H__
#define __BDEFS_H__

#if defined(LITHTECH_ESD) && defined(LITHTECH_ESD_INC)
    #include "pntypes.h"
#endif

// Make sure we are using DX8 for direct input...
#define DIRECTINPUT_VERSION     0x0800

#ifndef __DSYS_H__
#include "dsys.h"
#endif

// Turn off the unused formal parameter warning because MS's compiler can't do it with a command line parameter
#pragma warning (disable:4100)
// Turn off the unreferenced local function warning
#pragma warning (disable:4505)
// Turn off too-long template names warning
#pragma warning (disable:4786)

//C++ exception handler used, but unwind semantics are not enabled. Specify -GX
#pragma warning (disable: 4530)

#ifndef __LITHTECH_H__
#include "lithtech.h"
#endif

#ifndef __STDLITH_H__
#include "stdlith.h"
#endif

#ifndef __DE_MEMORY_H__
#include "de_memory.h"
#endif

// Very useful macros
#define SQR(x)                      ((x)*(x)) 


#ifndef MAKEUINT32
    #define MAKEUINT32(x,y)             ((((uint32)(x)) << 16) | (((uint32)(y)) & 0xFFFF))    
#endif

#ifndef MAKEUINT16
    #define MAKEUINT16(x,y)             ((((uint8)(x)) << 8) | (((uint8)(y)) & 0xFF))
#endif

#ifndef LOUINT16
    #define LOUINT16(x)                 ((uint16)((x) & 0xFFFF))
#endif

#ifndef HIUINT16
    #define HIUINT16(x)                 ((uint16)(((x)>>16) & 0xFFFF))
#endif

#ifndef LOUINT8
    #define LOUINT8(x)                  ((uint8)((x) & 0xFF))
#endif

#ifndef HIUINT8
    #define HIUINT8(x)                  ((uint8)(((x) >> 8) & 0xFF))
#endif

#define RAND(min,max)               ((rand() % ((max)-(min))) + (min))

#define DIFF(x,y)                   ((x)<(y) ? ((y)-(x)) : ((x)-(y)))
#define ABS(x)                      ((x)<0 ? (-(x)) : (x))

#define MIN(x,y)                    (((x)<(y)) ? (x) : (y))
#define MAX(x,y)                    (((x)>(y)) ? (x) : (y))

#define ROUND(x)                    ((int)(x+0.5))
#define LERP(a, l, h)               ((l) + (((h)-(l))*(a)))
#define SIGN(x)                     ((x)<0 ? -1 : 1)

#define CLAMP(a,x,y)                (((a)<(x)) ? (x) : ((a)>(y)) ? (y) : (a))
#define FLOOR(x)                    ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define CEIL(x)                     ((int)(x) + ((x) > 0 && (x) != (int)(x)))

#define STEP(x, a)                  ((float)((x) >= (a)))

// Maximums...
#define MAX_BYTE                    255
#define MAX_SBYTE                   127

#define MAX_WORD                    65535
#define MAX_SWORD                   32767

#define MAX_DWORD                   4294967295
#define MAX_SDWORD                  2147483647

#define MAX_CREAL                   1E+37

// Useful delete macros.
#define DeleteP(x)      { delete x; x=NULL; }
#define DeleteIf(x)     { if (x) { delete x; x=NULL; } }

// Make sure D3D doesn't define this guy...
#define MAKE_FOURCC

// Includes
#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __STDLIB_H__
#include <stdlib.h>
#define __STDLIB_H__
#endif

#ifndef __STRING_H__
#include <string.h> 
#define __STRING_H__
#endif

#ifndef __MATH_H__
#include <math.h>
#define __MATH_H__
#endif

#ifndef __STDARG_H__
#include <stdarg.h>
#define __STDARG_H__
#endif

#ifndef __CTYPE_H__
#include <ctype.h>
#define __CTYPE_H__
#endif

#ifndef __GAMEMATH_H__
#include "gamemath.h"
#endif

#ifndef __NEXUS_H__
#include "nexus.h"
#endif

// Helpful error output routines.
#define dsi_ConsolePrint dsi_PrintToConsole
extern void dsi_PrintToConsole(const char *pMsg, ...);    // From dsys_interface.h
extern void dsi_OnReturnError(int err);

extern int32 g_DebugLevel;
extern int32 g_CV_DebugModelRez;
extern char *g_ReturnErrString;

#define DEBUG_MODEL_REZ( toPrint ) {\
	if( g_CV_DebugModelRez )\
		dsi_PrintToConsole toPrint; }\

#define FN_NAME(name) \
    static char *___bdefs__pFnName = #name;

#define ERR(code, error) \
    RETURN_ERROR_NO_TOKEN_PASTE(code, ___bdefs__pFnName, error)

#define DEBUG_PRINT(debugLevel, toPrint) { \
    if (g_DebugLevel >= debugLevel) \
        dsi_PrintToConsole toPrint; }

#define RETURN_ERROR_PARAM(debugLevel, fnName, err, param) { \
    dsi_OnReturnError(err);\
    if (g_DebugLevel >= debugLevel) \
        dsi_PrintToConsole(g_ReturnErrString, #fnName, #err, param); \
    return (err); }

#define RETURN_ERROR_NO_TOKEN_PASTE(debugLevel, fnName, err) \
{ \
    dsi_OnReturnError(err);\
    if (g_DebugLevel >= debugLevel)\
    {\
        dsi_PrintToConsole(g_ReturnErrString, fnName, #err, " "); \
    }\
    return (err); \
}

#define RETURN_ERROR(debugLevel, fnName, err) \
    RETURN_ERROR_NO_TOKEN_PASTE(debugLevel, #fnName, err)

// A lot like RETURN_ERROR, but it doesn't return the error value.
#define GENERATE_ERROR(debugLevel, fnName, err, errorString)\
{\
    dsi_OnReturnError(err);\
    if (g_DebugLevel >= debugLevel)\
    {\
        dsi_PrintToConsole(g_ReturnErrString, #fnName, #err, errorString); \
    }\
}

// Used for all the parameter checking.
#define CHECK_PARAMS(condition, fnName)\
    if (!(condition)) \
    { RETURN_ERROR(2, fnName, LT_INVALIDPARAMS); }

#define CHECK_PARAMS2(condition)\
    if (!(condition)) \
    { RETURN_ERROR_NO_TOKEN_PASTE(2, ___bdefs__pFnName, LT_INVALIDPARAMS); }


#define CHECK_RET(condition, fnName, ret) \
    if (!(condition))\
    { GENERATE_ERROR(2, fnName, LT_INVALIDPARAMS, ""); return ret; }

#define CHECK_NORET(condition, fnName) \
    if (!(condition))\
    { GENERATE_ERROR(2, fnName, LT_INVALIDPARAMS, ""); return; }


#endif  // __BDEFS_H__



