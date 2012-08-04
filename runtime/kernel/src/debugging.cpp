//------------------------------------------------------------------
//
//  FILE      : Debugging.cpp
//
//  PURPOSE   : Implements the helper debugging routines.
//
//  CREATED   : August 6 1996
//
//  COPYRIGHT : 
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include <stdio.h>


void DebugOut(const char *pMsg, ...) {
    
	static const uint32 knBufferSize = 2048;
	char msg[knBufferSize];
    va_list marker;


    va_start(marker, pMsg);
    LTVSNPrintF(msg, knBufferSize, pMsg, marker);
    va_end(marker);

#if defined(_WIN32) || defined(__XBOX)
    OutputDebugString(msg);
#else
	// __LINUX
	#if defined(__DEBUG)
		printf(msg);
	#endif
#endif
}





