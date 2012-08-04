//-------------------------------------------------------------------
// LTAConverter.h
//
// Provides a standard library for converting to and from strings
// to different value types. This allows for a standard formatting
// of numbers and strings
//
// Created: 1/21/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTACONVERTER_H__
#define __LTACONVERTER_H__

#include "ltbasedefs.h"

class CLTAConverter
{
public:

	//functions for converting from a string to a different format
	static int32	StrToInt(const char* pszStr);
	static double	StrToReal(const char* pszStr);
	static bool		StrToBool(const char* pszStr);

	//functions for converting from a type to a string. Each returns the length
	//of the string copied into the buffer
	static uint32	IntToStr(int32 nVal, char* pszBuffer, uint32 nBufferLen);
	static uint32	RealToStr(double fVal, char* pszBuffer, uint32 nBufferLen);
	static uint32	BoolToStr(bool bVal, char* pszBuffer, uint32 nBufferLen);	

private:

	//don't allow instantiation
	CLTAConverter()	{}

};

#endif


