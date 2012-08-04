// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundMgr.cpp
//
// PURPOSE : ServerSoundMgr implementation - Controls sound on the server
//
// CREATED : 7/10/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerSoundMgr.h"
#include "CommonUtilities.h"

// Global pointer to server sound mgr...

CServerSoundMgr*  g_pServerSoundMgr = NULL;

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

bool CServerSoundMgr::Init()
{
	g_pServerSoundMgr = this;

    return CGameSoundMgr::Init();
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
    g_pServerSoundMgr = NULL;
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
	// Play the sound...
	if (LTStrEmpty(psi.m_szSoundName))
		return NULL;

	HLTSOUND hSnd = NULL;
	LTRESULT hResult = g_pLTServer->SoundMgr()->PlaySound(&psi, hSnd);

	if (hResult != LT_OK)
	{
		// [RP] We are missing so many resources that this is just annoying.  When we start getting 
		// more sounds, put the assert back so we know whats missing.
		//_ASSERT(false);
		g_pLTServer->CPrint("ERROR in CServerSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		return NULL;
	}

	// [RP] The sound handle that gets passed into PlaySound(), hSnd, will *always* get set.  Since the
	// SoundTracks get recycled if we return hSnd we may be setting a handle to a SoundTrack that will
	// get removed by the engine and put back on the free list.  Any future calls to KillSound() using that
	// handle will be killing the wrong sound or *worse*, a sound that doesn't exist.  Returning the handle 
	// of the PlaySoundInfo struct will ensure we only return a valid handle if explicitly told to (ie. PLAYSOUND_GETHANDLE);

	return psi.m_hSound;
}


