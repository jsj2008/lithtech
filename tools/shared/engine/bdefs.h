//------------------------------------------------------------------
//
//	FILE	  : BDEFS.H
//
//	PURPOSE	  : Base definitions header file
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __BDEFS_H__
#define __BDEFS_H__

	// turn off the stl debug symbol name being too large warning
	#pragma warning (disable:4786)

	#ifdef DIRECTENGINE_COMPILE
		#include "dsys.h"

		// Turn off the unused formal parameter warning because MS's compiler can't do it with a command line parameter
		#pragma warning (disable:4100)
		// Turn off the unreferenced local function warning
		#pragma warning (disable:4505)
	#endif

	// Special includes!
	#ifdef BDEFS_WINDOWS_H
		#include <windows.h>
	#endif

	// If dsys.h has been included, then it already included the windows headers.
	#ifndef DSYS_INCLUDED
		#ifdef BDEFS_MFC
			#include "stdafx.h"
		#endif

		#ifdef DIRECTEDITOR_BUILD
			#include <mmsystem.h>
		#endif
	#endif
 
	#ifdef BDEFS_STL
		#ifdef BDEFS_MFC
			#define _INC_WINDOWS
		#endif

		#include "list.h"
		#include "vector.h"
		#include "iterator.h"
		#include "stack.h"
	#endif

	#include "lithtech.h"

	#include "stdlith.h"
	#include "de_memory.h"


	// Define base number type
	typedef float		CReal;
	typedef double		CDReal;


	// Very useful macros
	#define SQR(x)						( (x)*(x) )	

	
	#ifndef MAKEUINT32
		#define MAKEUINT32(x,y)				( (((uint32)(x)) << 16) | (((uint32)(y)) & 0xFFFF) )	
	#endif

	#ifndef MAKEUINT16
		#define MAKEUINT16(x,y)				( (((uint8)(x)) << 8) | (((uint8)(y)) & 0xFF) )
	#endif
	
	#ifndef	LOUINT16
		#define LOUINT16(x)					( (uint16)((x) & 0xFFFF) )
	#endif

	#ifndef HIUINT16
		#define HIUINT16(x)					( (uint16)(((x)>>16) & 0xFFFF) )
	#endif

	#ifndef LOUINT8
		#define	LOUINT8(x)					( (uint8)((x) & 0xFF) )
	#endif

	#ifndef HIUINT8
		#define HIUINT8(x)					( (uint8)(((x) >> 8) & 0xFF) )
	#endif

	#define RAND(min,max)				( (rand() % ((max)-(min))) + (min) )

	#define	DIFF(x,y)					( (x)<(y) ? ((y)-(x)) : ((x)-(y)) )
	#define	ABS(x)						( (x)<0 ? (-(x)) : (x) )

	#define MIN(x,y)					( ((x)<(y)) ? (x) : (y) )
	#define MAX(x,y)					( ((x)>(y)) ? (x) : (y) )

	#define ROUND(x)					( (int)(x+0.5) )
	#define LERP(a, l, h)				( (l) + (((h)-(l))*(a)) )
	#define SIGN(x)						( (x)<0 ? -1 : 1 )

	#define CLAMP(a,x,y)				( ((a)<(x)) ? (x) : ((a)>(y)) ? (y) : (a) )
	#define FLOOR(x)					( (int)(x) - ((x) < 0 && (x) != (int)(x)) )
	#define CEIL(x)						( (int)(x) + ((x) > 0 && (x) != (int)(x)) )

	#define STEP(x, a)					((CReal)((x) >= (a)))

	// Maximums...
	#define	MAX_BYTE					255
	#define	MAX_SBYTE					127

	#define	MAX_WORD					65535
	#define	MAX_SWORD					32767

	#define	MAX_DWORD					4294967295
	#define	MAX_SDWORD					2147483647

	#define	MAX_CREAL					1E+37

	// Useful delete macros.
	#define DeleteP(x)		{ delete x; x=NULL; }
	#define DeleteIf(x)		{ if(x) { delete x; x=NULL; } }


	// Includes
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>	
	#include <math.h>
	#include <stdarg.h>
	#include <ctype.h>
	
	#include "ltcompat.h"
	
	// Helpful error output routines.
	#define dsi_ConsolePrint dsi_PrintToConsole
	extern void dsi_PrintToConsole(char *pMsg, ...);	// From dsys_interface.h
	extern void dsi_OnReturnError(int err);

	extern int32 g_DebugLevel;
	extern char *g_ReturnErrString;

	#define FN_NAME(name) \
		static char *___bdefs__pFnName = #name;

	#define ERR(code, error) \
		RETURN_ERROR_NO_TOKEN_PASTE(code, ___bdefs__pFnName, error)

	#define DEBUG_PRINT(debugLevel, toPrint) { \
		if(g_DebugLevel >= debugLevel) \
			dsi_PrintToConsole toPrint; }
	
	#define RETURN_ERROR_PARAM(debugLevel, fnName, err, param) { \
		dsi_OnReturnError(err);\
		if(g_DebugLevel >= debugLevel) \
			dsi_PrintToConsole(g_ReturnErrString, #fnName, #err, param); \
		return (err); }

	#define RETURN_ERROR_NO_TOKEN_PASTE(debugLevel, fnName, err) \
	{ \
		dsi_OnReturnError(err);\
		if(g_DebugLevel >= debugLevel)\
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
		if(g_DebugLevel >= debugLevel)\
		{\
			dsi_PrintToConsole(g_ReturnErrString, #fnName, #err, errorString); \
		}\
	}

	// Used for all the parameter checking.
	#define CHECK_PARAMS(condition, fnName)\
		if(!(condition)) \
		{ RETURN_ERROR(2, fnName, LT_INVALIDPARAMS); }

	#define CHECK_PARAMS2(condition)\
		if(!(condition)) \
		{ RETURN_ERROR_NO_TOKEN_PASTE(2, ___bdefs__pFnName, LT_INVALIDPARAMS); }


	#define CHECK_RET(condition, fnName, ret) \
		if(!(condition))\
		{ GENERATE_ERROR(2, fnName, LT_INVALIDPARAMS, ""); return ret; }

	#define CHECK_NORET(condition, fnName) \
		if(!(condition))\
		{ GENERATE_ERROR(2, fnName, LT_INVALIDPARAMS, ""); return; }



	// More special includes!
	#ifdef DIRECTEDITOR_BUILD
		#include "objbase.h"
	#endif

	#ifdef PREPROCESSOR_BUILD
		#include "preprocessorbase.h"
	#endif

#endif  // __BDEFS_H__



