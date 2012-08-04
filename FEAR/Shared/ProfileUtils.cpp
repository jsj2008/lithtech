#include "Stdafx.h"
#include "ProfileUtils.h"
#include "ltbasedefs.h"
#include "NetDefs.h"
#include "iltclient.h"
#include "iltfilemgr.h"

/***********************************************************************************/

const char*	GetAbsoluteProfileDir( ILTClient* pLTClient, char const* pszProfileName ) 
{
	static std::string directory;

	char szUserDirectory[MAX_PATH];
	pLTClient->FileMgr()->GetAbsoluteUserFileName( "", szUserDirectory, MAX_PATH );

	directory = szUserDirectory;
	directory += PROFILE_DIR;
	directory += pszProfileName;
	return directory.c_str();
}

const char*	GetRelativeProfileDir( ILTClient* pLTClient, char const* pszProfileName ) 
{
	static std::string directory;

	directory = PROFILE_DIR;
	directory += pszProfileName;
	return directory.c_str();
}

const char*	GetAbsoluteProfileFile( ILTClient* pLTClient, char const* pszProfileName ) 
{
	static std::string sProfileFile;
	sProfileFile = GetAbsoluteProfileDir( pLTClient, pszProfileName );
	sProfileFile += PROFILE_EXT;
	return sProfileFile.c_str();
}

const char* GetRelativeProfileFile( ILTClient* pLTClient, char const* pszProfileName )
{
	static std::string sProfileFile;
	sProfileFile = GetRelativeProfileDir( pLTClient, pszProfileName );
	sProfileFile += PROFILE_EXT;
	return sProfileFile.c_str();
}

//Fix player name... may return a null string if no valid name is found
const wchar_t*	FixPlayerName(wchar_t const* pszPlayerName) 
{
	static wchar_t szName[MAX_PLAYER_NAME+1];

	wchar_t *pName = (wchar_t *)pszPlayerName;

	//strip leading whitespace
	while (pName && iswspace(*pName))
	{
		pName++;
	}

	if (pName)
	{
		uint32 n = 0;
		while (pName && *pName && n < (MAX_PLAYER_NAME-1))
		{
			wchar_t c = PlayerNameCharacterFilter(*pName,n);
			if (c)
			{
				szName[n] = c;
				n++;
			}
			pName++;
		}
		szName[n] = 0;

		//strip trailing whitespace
		int nLen = LTStrLen(szName) - 1;
		while (nLen > 0 && iswspace(szName[nLen]))
		{
			szName[nLen] = 0;
			nLen--;
		}

	}
	else
		szName[0] = 0;

	return szName;
}

wchar_t PlayerNameCharacterFilter(wchar_t c, uint32 nPos)
{
	// spaces and backslashes are not allowed
	if (wcschr(L" \\",c))
	{
		return 0;
	}

	// certain special characters are not allowed for the first character
	if (nPos == 0 && wcschr(L"@+:#0123456789.,;",c))
	{
		return 0;
	}
	return c;
}

