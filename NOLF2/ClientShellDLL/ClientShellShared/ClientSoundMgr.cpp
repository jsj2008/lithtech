// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMgr.cpp
//
// PURPOSE : ClientSoundMgr implementation - Controls sound on the client
//
// CREATED : 7/10/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientSoundMgr.h"
#include "CommonUtilities.h"
#include "VarTrack.h"
#include "SoundFilterMgr.h"
#include "SoundButeMgr.h"
#include "ClientUtilities.h"

// Global pointer to client sound mgr...

CClientSoundMgr*  g_pClientSoundMgr = LTNULL;

SOUNDFILTER* g_pUnfilteredFilter = LTNULL;

VarTrack	g_vtSoundPlayOnlyIfHeard;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CServerSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientSoundMgr::CClientSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::~CServerSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientSoundMgr::~CClientSoundMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CClientSoundMgr::Init(const char* szAttributeFile)
{
	g_pClientSoundMgr = this;

	g_vtSoundPlayOnlyIfHeard.Init(g_pLTClient, "SoundPlayOnlyIfHeard", NULL, 0.0f);

	return CGameSoundMgr::Init(szAttributeFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CClientSoundMgr::Term()
{
    g_pClientSoundMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays an unfiltered interface sound.
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlayInterfaceSound(const char *pName, uint32 dwFlags)
{
	if (!pName) return LTNULL;
	if (!GetConsoleInt("SoundEnable",1)) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority	= SOUNDPRIORITY_MISC_MEDIUM;

	// Play interface sounds unfiltered...

	if (!g_pUnfilteredFilter)
	{
		g_pUnfilteredFilter = g_pSoundFilterMgr->GetFilter("UnFiltered");
	}

	psi.m_UserData = g_pUnfilteredFilter->nId;
    
    return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySoundLocal(const char *pName, SoundPriority ePriority,
	uint32 dwFlags, uint8 nVolume, float fPitchShift, uint8 nSoundClass )
{
    if (!pName) return LTNULL;
	if (!GetConsoleInt("SoundEnable",1)) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	// See if the passed in name is really a sound bute...

	int nSoundBute = g_pSoundButeMgr->GetSoundSetFromName( pName );
	if( nSoundBute != INVALID_SOUND_BUTE )
	{
		// Ok, it's a soundBute, Get it...

		SoundBute sb = g_pSoundButeMgr->GetSoundBute( nSoundBute );

		// Check for the play chance
		
		if( sb.m_fPlayChance < 1.0f )
		{
			if( GetRandom(0.0f,1.0f) > sb.m_fPlayChance )
				return LTNULL;
		}

		// Get a random sound file from the SoundButes' play list...

		strncpy( psi.m_szSoundName, g_pSoundButeMgr->GetRandomSoundFileWeighted( nSoundBute ), _MAX_PATH );
		
		// Should we use passed in values or SoundBute values...

		psi.m_nPriority		= (ePriority == SOUNDPRIORITY_MISC_LOW ? SOUNDPRIORITY_MISC_LOW : sb.m_ePriority);
		psi.m_nVolume		= (nVolume == SMGR_DEFAULT_VOLUME ? SMGR_DEFAULT_VOLUME : sb.m_nVolume);
		psi.m_fPitchShift	= (fPitchShift >= 1.0f ? 1.0f : sb.m_fPitch);
		dwFlags				= sb.m_nFlags;
	}
	else
	{
		// Just a normal sound file, play it with the passed in values...

		strncpy(psi.m_szSoundName, pName, _MAX_PATH);
		psi.m_nPriority		= ePriority;
		psi.m_nVolume		= nVolume;
		psi.m_fPitchShift	= fPitchShift;
		psi.m_nSoundVolumeClass = nSoundClass;
	}

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	// Make sure local sounds aren't 3d...
	psi.m_dwFlags &= ~PLAYSOUND_3D;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

    return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySound()
//
//	PURPOSE:	Play the sound associated with the sound info
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySound(PlaySoundInfo & psi)
{
	HLTSOUND hSnd = LTNULL;
	if (!GetConsoleInt("SoundEnable",1)) return LTNULL;

	// Play the sound...

	// Optimization, if we can't hear the sound and it isn't looping
	// don't play it!

	if ((psi.m_dwFlags & PLAYSOUND_3D) && 
		!(psi.m_dwFlags & PLAYSOUND_LOOP) && 
		 g_vtSoundPlayOnlyIfHeard.GetFloat())
	{
		LTVector vListenerPos;
		bool bListenerInClient;
		LTRotation rRot;
		g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

		LTVector vPos = psi.m_vPosition - vListenerPos;
		if (vPos.Mag() > psi.m_fOuterRadius)
		{
			return LTNULL;
		}
	}


	LTRESULT hResult = g_pLTClient->SoundMgr()->PlaySound(&psi, hSnd);

	if (hResult != LT_OK)
	{
		g_pLTClient->CPrint("ERROR in CClientSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		
		return LTNULL;
	}
	
	// [RP] The sound handle that gets passed into PlaySound(), hSnd, will *always* get set.  Since the
	// SoundTracks get recycled if we return hSnd we may be setting a handle to a SoundTrack that will
	// get removed by the engine and put back on the free list.  Any future calls to KillSound() using that
	// handle will be killing the wrong sound or *worse*, a sound that doesn't exist.  Returning the handle 
	// of the PlaySoundInfo struct will ensure we only return a valid handle if explicitly told to (ie. PLAYSOUND_GETHANDLE);

	return psi.m_hSound;
}