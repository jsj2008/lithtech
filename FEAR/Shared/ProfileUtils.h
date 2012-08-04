// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileUtils.h
//
// PURPOSE : Profile utilities
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include "ltbasetypes.h"

#define PROFILE_DIR "Profiles" FILE_PATH_SEPARATOR
#define PROFILE_EXT ".gdb"
#define MAX_PROFILE_NAME	16

class ILTClient;

const char*	GetAbsoluteProfileDir( ILTClient* pLTClient, char const* pszProfileName );
const char* GetRelativeProfileDir( ILTClient* pLTClient, char const* pszProfileName );
const char*	GetAbsoluteProfileFile( ILTClient* pLTClient, char const* pszProfileName );
const char* GetRelativeProfileFile( ILTClient* pLTClient, char const* pszProfileName );

//Fix player name... may return a null string if no valid name is possible
const wchar_t* FixPlayerName(wchar_t const* pszPlayerName);

//filter characters out of a player name (GameSpy requirement)
wchar_t PlayerNameCharacterFilter(wchar_t c, uint32 nPos);


#endif // PROFILEUTILS_H

