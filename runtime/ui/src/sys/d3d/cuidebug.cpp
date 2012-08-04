//-------------------------------------------------------------------
//
//   MODULE    : CUIDEBUG.CPP
//
//   PURPOSE   : implements a simple system-specific debugging printout
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


//	In which live all the CUI Debugging Macros and Func. Defs
//	common to the CUI system


// this file is empty if building a release version of the engine
#if defined __DEBUG || defined _DEBUG 

#ifndef  _WINDOWS_
#include "windows.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#define __CUIDEBUG_H__
#endif

#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif


// initialization of static data members
char CUIDebug::sm_pText[256];


//  --------------------------------------------------------------------------
void CUIDebug::DebugPrint(const char* pFormat, ...)
{
	va_list pVAList;

	// init the varg list
	va_start(pVAList, pFormat);

	// create the string... don't overflow our buffer.
	_vsnprintf(sm_pText, 255, pFormat, pVAList);

	// print the string to debug-out
	OutputDebugString(sm_pText);

	// reset the varg list
	va_end(pVAList);
}


#endif // defined __DEBUG || defined _DEBUG 