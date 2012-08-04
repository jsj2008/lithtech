// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VersionMgr.h"
#include "regmgr.h"
#include "ClientServerShared.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::CVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVersionMgr::CVersionMgr()
{
	// Determine what language version the game is using...

	m_szLanguage[0]	= '\0';
	m_bLowViolence	= false;

	m_regMgr.Init("Monolith Productions", GAME_NAME, "1.0");

	// Get the language registry key...

	if (m_regMgr.IsValid())
	{
		unsigned int nBufSize = sizeof(m_szLanguage);
		m_regMgr.Get("Language", m_szLanguage, nBufSize, "English");

/* jrg - 9/26/02 - removed redundant low-violence check  (this is handled by a flag in CRes now)
		if (m_szLanguage[0])
		{
			if (_stricmp(m_szLanguage, "German") == 0)
			{
				m_bLowViolence = true;
			}
		}
*/

		nBufSize = sizeof(m_szNetRegion);
		m_regMgr.Get("NetRegionCode", m_szNetRegion, nBufSize, "EN");
	}
	else
	{
		strcpy(m_szNetRegion, "EN");
	}

	m_nCurrentSaveVersion = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetCurrentSaveVersion()
//
//	PURPOSE:	Grab the "current" save version.  Meaning the version
//				of the save file currently being loaded.
//
// ----------------------------------------------------------------------- //

CVersionMgr::TSaveVersion CVersionMgr::GetCurrentSaveVersion() const
{
	// The only time we care about old save versions is when we are restoring a
	// previously saved game.

	if( m_nLastLGFlags == LOAD_RESTORE_GAME )
		return m_nCurrentSaveVersion;

	return CVersionMgr::kSaveVersion__CurrentBuild;
}