#include "ltaconverter.h"
#include "ltalimits.h"
#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

//used for character type identification
#define CHAR_TYPE_TABLE_SIZE		128

enum ECharType {	INVALID,
					WHITESPACE,
					DIGIT,
					LETTER,
					SIGN,
					DECIMAL,
					END_OF_STRING
				};

static ECharType	g_LTACharTypeTable[CHAR_TYPE_TABLE_SIZE];
static bool			g_bTypeTableInit = false;


//--InitMyCharType--
// MUST BE CALLED BEFORE MyCharType, since this initializes the table that it uses
static void InitCharTypeTable()
{
	//fill everything to be invalid, alpha, digit, or whitespace
	for(uint32 nCurr = 0; nCurr < CHAR_TYPE_TABLE_SIZE; nCurr++)
	{
		//check for letters
		if(isalpha(nCurr))
		{
			g_LTACharTypeTable[nCurr] = LETTER;
		}
		//check for digits
		else if(isdigit(nCurr))
		{
			g_LTACharTypeTable[nCurr] = DIGIT;
		}
		//check for whitespace
		else if(isspace(nCurr))
		{
			g_LTACharTypeTable[nCurr] = WHITESPACE;
		}
		//everything else
		else
		{
			g_LTACharTypeTable[nCurr] = INVALID;
		}			
	}

	//now init all the sign characters
	g_LTACharTypeTable['-'] = SIGN;
	g_LTACharTypeTable['+'] = SIGN;

	//now init all the decimal characters
	g_LTACharTypeTable['.'] = DECIMAL;

	//now init all the end of string characters
	g_LTACharTypeTable['\0'] = END_OF_STRING;

	//not really concerned with any others for now

	//flag the table as set up
	g_bTypeTableInit = true;
}


// Given a character, it will find the type of that character, whether that be
// end of string, digit, alpha, special, or whitespace. Assumes that it is a valid
// character in the range of 0...CHAR_TYPE_TABLE_SIZE - 1 inclusive
static inline ECharType LTACharType(char ch)
{
	//should probably assert ch is in range....

	return g_LTACharTypeTable[ch];
}


// Takes a string buffer and converts it into a double precision floating point number.
// Will return 0 if the string is not valid. Does not support e notation, and instead only
// supports [-]I[.DDDDDD] format
double CLTAConverter::StrToReal(const char* pszBuffer)
{
	//make sure the table is set up
	if(g_bTypeTableInit == false)
	{
		InitCharTypeTable();
	}

	//keep track of negative sign
	int nSign = 1;

	//the type of the current char
	ECharType eCurrType = INVALID;

	//index into the buffer
	int nOff = 0;

	//skip over whitespace
	do {
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);

		//see if we hit the end of the string
		if(eCurrType == END_OF_STRING)
			return (double)0;

		//bust out if this isn't a whitespace
		if(eCurrType != WHITESPACE)
		{
			break;
		}

		//move to the next char
		nOff++;

	}while(1);
	

	//see if we hit a sign
	if(eCurrType == SIGN)
	{
		//see if it was neg
		if(pszBuffer[nOff] == '-')
		{
			nSign = -1;
		}

		//skip on to the next character
		nOff++;
		
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);
	}

	//if this isn't a digit, bail
	if(eCurrType !=DIGIT)
	{
		return (double)0;
	}

	//the integer part of the number
	int32 nIntPart = 0;

	//now parse in the integer part
	while(eCurrType == DIGIT)
	{
		//modify the number
		nIntPart *= 10;
		nIntPart += (int)(pszBuffer[nOff] - '0');

		//move onto the next character
		nOff++;
		
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);
	}

	//the decimal part of the number (as an integer)
	int32 nDecPart	= 0;
	//the divisor of the decimal part to shift it to the right the apprpriate
	//amount (so the decimal part actually equals nDecPart / nDecDivisor)
	int32 nDecDivisor = 1;

	//now see if there is a decimal part
	if(eCurrType == DECIMAL)
	{
		//move onto the next character
		nOff++;
		
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);

		while(eCurrType == DIGIT)
		{
			//modify the decimal part
			nDecPart *= 10;
			nDecPart += (int32)(pszBuffer[nOff] - '0');

			//increment the divisor
			nDecDivisor *= 10;

			//move onto the next character
			nOff++;
		
			//get the current char type
			eCurrType = LTACharType(pszBuffer[nOff]);
		}
	}

	//return the answer
	return nSign * ((double)nIntPart + (double)nDecPart / (double)nDecDivisor);
}

// Takes a string buffer and converts it into an integer number.
// Will return 0 if the string is not valid. Does not support e notation, and instead only
// supports [-]I format
int32 CLTAConverter::StrToInt(const char* pszBuffer)
{
	//make sure the table is set up
	if(g_bTypeTableInit == false)
	{
		InitCharTypeTable();
	}

	//keep track of negative sign
	int nSign = 1;

	//the type of the current char
	ECharType eCurrType = INVALID;

	//index into the buffer
	uint32 nOff = 0;

	//skip over whitespace
	do {
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);

		//see if we hit the end of the string
		if(eCurrType == END_OF_STRING)
			return 0;

		//bust out if this isn't a whitespace
		if(eCurrType != WHITESPACE)
		{
			break;
		}

		//move to the next char
		nOff++;

	}while(1);
	

	//see if we hit a sign
	if(eCurrType == SIGN)
	{
		//see if it was neg
		if(pszBuffer[nOff] == '-')
		{
			nSign = -1;
		}

		//skip on to the next character
		nOff++;
		
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);
	}

	//if this isn't a digit, bail
	if(eCurrType !=DIGIT)
	{
		return 0;
	}

	//the integer part of the number
	int32 nIntPart = 0;

	//now parse in the integer part
	while(eCurrType == DIGIT)
	{
		//modify the number
		nIntPart *= 10;
		nIntPart += (int32)(pszBuffer[nOff] - '0');

		//move onto the next character
		nOff++;
		
		//get the current char type
		eCurrType = LTACharType(pszBuffer[nOff]);
	}

	//return the answer
	return nSign * nIntPart;
}


bool CLTAConverter::StrToBool(const char* pszStr)
{
	if(stricmp(pszStr, "false") == 0)
	{
		return false;
	}
	return true;
}


//functions for converting from a type to a string
uint32 CLTAConverter::IntToStr(int32 nVal, char* pszBuffer, uint32 nBufferLen)
{
	//sanity checks
	ASSERT(pszBuffer);
	ASSERT(nBufferLen > 0);

	//buffer position
	uint32 nBufferOff = 0;

	//see if we need to print out a sign
	if(nVal < 0)
	{
		if(nBufferOff < nBufferLen - 1)
		{
			pszBuffer[nBufferOff++] = '-';
		}
		nVal = -nVal;
	}

	uint32 nMask = 1;

	//count up the number of digits
	while(nVal / nMask > 9)
	{
		nMask *= 10;
	}

	//now print out the digits
	while(nMask > 0)
	{
		if(nBufferOff >= nBufferLen - 1)
		{
			break;
		}

		pszBuffer[nBufferOff++] = '0' + nVal / nMask;
		
		nVal  %= nMask;
		nMask /= 10;
	}

	//close off the string
	pszBuffer[nBufferOff] = '\0';
	return nBufferOff;
}

uint32 CLTAConverter::RealToStr(double fVal, char* pszBuffer, uint32 nBufferLen)
{
	return sprintf(pszBuffer, "%.*f", LTA_DECIMAL_ACCURACY, fVal);
}

uint32 CLTAConverter::BoolToStr(bool bVal, char* pszBuffer, uint32 nBufferLen)
{
	if(bVal)
	{
		strncpy(pszBuffer, "true", nBufferLen);
		return 4;
	}

	strncpy(pszBuffer, "false", nBufferLen);
	return 5;
}
	