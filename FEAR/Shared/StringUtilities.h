// ----------------------------------------------------------------------- //
//
// MODULE  : StringUtilities.h
//
// PURPOSE : Contains string utility functions
//
// CREATED : 09/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STRINGUTILITIES_H__
#define __STRINGUTILITIES_H__

#include <stdarg.h>
#include "StringEditMgr.h"

#define INVALID_STRINGEDIT_NOTINIT_W	L"[ERROR: StringEditRuntime Not Initialized]"
#define INVALID_STRINGEDIT_NOTINIT_A	"[ERROR: StringEditRuntime Not Initialized]"

// formats a string using a printf argument list
inline int32 FormatString( const char* pszStringID, wchar_t* wszBuffer, uint32 nBufferSize, ... )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return -1;

	va_list marker;
	va_start( marker, nBufferSize );
	int32 iReturn = g_pLTIStringEdit->FormatMessage( g_pLTDBStringEdit, pszStringID, wszBuffer, nBufferSize, marker );
	va_end( marker );

	return iReturn;
}

// formats a string using a printf argument list given an index from IStringEditMgr::GetIndex
inline int32 FormatString( uint32 nIndex, wchar_t* wszBuffer, uint32 nBufferSize, ... )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return -1;

	va_list marker;
	va_start( marker, nBufferSize );
	int32 iReturn = g_pLTIStringEdit->FormatMessage( g_pLTDBStringEdit, nIndex, wszBuffer, nBufferSize, marker );
	va_end( marker );

	return iReturn;
}

// loads a string from the string keeper
inline const wchar_t* LoadString( const char* pszStringID )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return INVALID_STRINGEDIT_NOTINIT_W;

	if (!pszStringID || !pszStringID[0]) 
	{
		return L"";
	}

	return g_pLTIStringEdit->GetString( g_pLTDBStringEdit, pszStringID );
}

// loads a string from the string keeper given an index from IStringEditMgr::GetIndex
inline const wchar_t* LoadString( uint32 nIndex )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return INVALID_STRINGEDIT_NOTINIT_W;

	return g_pLTIStringEdit->GetString( g_pLTDBStringEdit, nIndex );
}

// returns the StringID from an index
inline const char* StringIDFromIndex( uint32 nIndex )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return INVALID_STRINGEDIT_NOTINIT_A;

	return g_pLTIStringEdit->GetStringID( g_pLTDBStringEdit, nIndex );
}

inline uint32 IndexFromStringID( const char* pszStringID )
{
	LTASSERT( g_pLTIStringEdit, "StringEditRuntime interface not initialized!" );
	LTASSERT( g_pLTDBStringEdit, "StringEditRuntime database not opened!" );
	if( (g_pLTIStringEdit == NULL) || (g_pLTDBStringEdit == NULL) )
		return INVALID_STRINGEDIT_INDEX;

	if (!pszStringID || !pszStringID[0])
		return INVALID_STRINGEDIT_INDEX;

	return g_pLTIStringEdit->GetIndex( g_pLTDBStringEdit, pszStringID );
}

#endif  // __STRINGUTILITIES_H__
