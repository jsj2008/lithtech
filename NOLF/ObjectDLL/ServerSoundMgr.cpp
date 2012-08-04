// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundMgr.cpp
//
// PURPOSE : ServerSoundMgr implementation - Controls sound on the server
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerSoundMgr.h"
#include "CommonUtilities.h"

// Global pointer to server sound mgr...

CServerSoundMgr*  g_pServerSoundMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CServerSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::CServerSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::~CServerSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::~CServerSoundMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CServerSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	g_pServerSoundMgr = this;

    return CGameSoundMgr::Init(pInterface, szAttributeFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CServerSoundMgr::Term()
{
    g_pServerSoundMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CacheSounds()
//
//	PURPOSE:	Cache the appropriate sounds...
//
// ----------------------------------------------------------------------- //

void CServerSoundMgr::CacheSounds(const char* pTag, const char* pAttributeBase)
{
	if (!pTag || !pAttributeBase || !g_pLTServer) return;

	// Cache all the sounds...

	CString strRet;
	int nCurSound = 0;

	char aAttName[100];
	sprintf(aAttName, "%s%d", pAttributeBase, nCurSound);

	while (m_buteMgr.Exist(pTag, aAttName))
	{
		strRet = m_buteMgr.GetString(pTag, aAttName);

		if (strRet.GetLength() > 0)
		{
            g_pLTServer->CacheFile(FT_SOUND, strRet.GetBuffer(0));
		}

		nCurSound++;
		sprintf(aAttName, "%s%d", pAttributeBase, nCurSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySoundDirect()
//
//	PURPOSE:	Allows the game to *basically* bypass the sound mgr and
//				call directly into the engine sound code.  This is mainly
//				for backwards compatibility.
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySoundDirect(PlaySoundInfo & psi)
{
	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySound()
//
//	PURPOSE:	Play the sound associated with the sound info...
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySound(PlaySoundInfo & psi)
{
	HLTSOUND hSnd = LTNULL;

	// Play the sound...

	LTRESULT hResult = g_pLTServer->PlaySound(&psi);

	if (hResult == LT_OK)
	{
		hSnd = psi.m_hSound;
	}
	else
	{
		_ASSERT(LTFALSE);
		m_pInterface->CPrint("ERROR in CServerSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		return LTNULL;
	}

	return hSnd;
}