// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMgr.cpp
//
// PURPOSE : ClientSoundMgr implementation - Controls sound on the client
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientSoundMgr.h"
#include "CommonUtilities.h"
#include "VarTrack.h"

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

LTBOOL CClientSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	g_pClientSoundMgr = this;

	g_vtSoundPlayOnlyIfHeard.Init(g_pLTClient, "SoundPlayOnlyIfHeard", NULL, 1.0f);

	return CGameSoundMgr::Init(pInterface, szAttributeFile);
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

HLTSOUND CClientSoundMgr::PlayInterfaceSound(char *pName, uint32 dwFlags)
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

HLTSOUND CClientSoundMgr::PlaySoundLocal(char *pName, SoundPriority ePriority,
	uint32 dwFlags, uint8 nVolume, float fPitchShift)
{
    if (!pName) return LTNULL;
	if (!GetConsoleInt("SoundEnable",1)) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;

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

	if ((psi.m_dwFlags & PLAYSOUND_3D) && !(psi.m_dwFlags & PLAYSOUND_LOOP) && 
		g_vtSoundPlayOnlyIfHeard.GetFloat())
	{
		LTVector vListenerPos;
		LTBOOL bListenerInClient;
		LTRotation rRot;
		g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

		LTVector vPos = psi.m_vPosition - vListenerPos;
		if (vPos.Mag() > psi.m_fOuterRadius)
		{
			return LTNULL;
		}
	}

	LTRESULT hResult = g_pLTClient->PlaySound(&psi);

	if (hResult == LT_OK)
	{
		hSnd = psi.m_hSound;
	}
	else
	{
		_ASSERT(LTFALSE);
		m_pInterface->CPrint("ERROR in CClientSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		return LTNULL;
	}

	return hSnd;
}