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
#include "SoundDB.h"
#include "ClientUtilities.h"

// Global pointer to client sound mgr...

CClientSoundMgr*  g_pClientSoundMgr = NULL;

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

bool CClientSoundMgr::Init()
{
	g_pClientSoundMgr = this;

	g_vtSoundPlayOnlyIfHeard.Init(g_pLTClient, "SoundPlayOnlyIfHeard", NULL, 0.0f);

	return CGameSoundMgr::Init();
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
    g_pClientSoundMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlayInterfaceSound
//
//	PURPOSE:	Plays an unfiltered interface sound.
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlayInterfaceSound(const char *pName, uint32 dwFlags)
{
	if (!pName || !pName[0]) return NULL;
	if (!GetConsoleInt("SoundEnable",1)) return NULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority	= SOUNDPRIORITY_MISC_MEDIUM;
	psi.m_nMixChannel = PLAYSOUND_MIX_UI;

    return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlayInterfaceDBSound
//
//	PURPOSE:	Plays an unfiltered interface sound.
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlayInterfaceDBSound(const char *pRecordName, uint32 dwFlags)
{
	if (!pRecordName || !pRecordName[0]) return NULL;
	if (!GetConsoleInt("SoundEnable",1)) return NULL;
		
	HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord(pRecordName);

	if (hSoundRec)
	{
		PlaySoundInfo psi;
		PLAYSOUNDINFO_INIT(psi);

		psi.m_dwFlags = PLAYSOUND_LOCAL;
		psi.m_dwFlags |= dwFlags;

		strncpy(psi.m_szSoundName, g_pSoundDB->GetRandomSoundFileWeighted(hSoundRec), _MAX_PATH);
		psi.m_nPriority	= SOUNDPRIORITY_MISC_MEDIUM;
		psi.m_nMixChannel = PLAYSOUND_MIX_UI;

		return PlaySound(psi);
	}
	else
	{
		return NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head
//
// ----------------------------------------------------------------------- //
HLTSOUND CClientSoundMgr::PlayDBSoundLocal(HRECORD hSR, SoundPriority ePriority/* =SOUNDPRIORITY_INVALID */, uint32 dwFlags/* =0 */, uint8 nVolume/* =SMGR_DEFAULT_VOLUME */, 
										   float fPitchShift/* =1.0f */, SoundClass eSoundClass /* = DEFAULT_SOUND_CLASS  */,
										   int16 nMixChannel /* = 0 */)
{
	if( hSR  )
	{
		SoundRecord sr;
		g_pSoundDB->FillSoundRecord(hSR,sr);

		// Check for the play chance

		if( sr.m_fPlayChance < 1.0f )
		{
			if( GetRandom(0.0f,1.0f) > sr.m_fPlayChance )
				return NULL;
		}

		// Get a random sound file from the SoundRecord's play list...
		return PlaySoundLocal(g_pSoundDB->GetRandomSoundFileWeighted(hSR), 
						(ePriority != SOUNDPRIORITY_INVALID ? ePriority : sr.m_ePriority), 
						dwFlags | sr.m_nFlags, 
						(nVolume != SMGR_INVALID_VOLUME ? nVolume : sr.m_nVolume),
						(fPitchShift < 1.0f ? fPitchShift : sr.m_fPitch),
						eSoundClass,
						(nMixChannel == 0 ? sr.m_nMixChannel : nMixChannel) );
	}

	return NULL;

}


HLTSOUND CClientSoundMgr::PlaySoundLocal(const char *pName, SoundPriority ePriority,
										 uint32 dwFlags, uint8 nVolume, float fPitchShift, 
										 SoundClass eSoundClass, int16 nMixChannel )
{
	if (LTStrEmpty(pName) ) return NULL;
	if (!GetConsoleInt("SoundEnable",1)) return NULL;

#ifdef PLATFORM_WIN32
	// NOTE: This is only for win32 for now, since it's using
	// waves. In the future, we could add a platform-specific
	// sound checker.

	// Split the path up to find extension...
	
	if (!CResExtUtil::IsFileOfType(pName, RESEXT_SOUND))
	{
		DebugCPrint(0,"CGameSoundMgr::PlaySoundFromObject() - invalid filename '%s'",pName);
		return NULL;
	}
#endif // PLATFORM_WIN32

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_nSoundVolumeClass = eSoundClass;
	psi.m_nMixChannel = nMixChannel;
	psi.m_fDopplerFactor = 1.0f;

	psi.m_dwFlags = dwFlags | PLAYSOUND_LOCAL;

	// Make sure local sounds aren't 3d...
	psi.m_dwFlags &= ~PLAYSOUND_3D;
	psi.m_dwFlags &= ~PLAYSOUND_USEOCCLUSION;

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
	HLTSOUND hSnd = NULL;
	if (!GetConsoleInt("SoundEnable",1)) return NULL;

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
			return NULL;
		}
	}


	LTRESULT hResult = g_pLTClient->SoundMgr()->PlaySound(&psi, hSnd);

	if (hResult != LT_OK)
	{
		DebugCPrint(1,"ERROR in CClientSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		
		return NULL;
	}

	// [RP] The sound handle that gets passed into PlaySound(), hSnd, will *always* get set.  Since the
	// SoundTracks get recycled if we return hSnd we may be setting a handle to a SoundTrack that will
	// get removed by the engine and put back on the free list.  Any future calls to KillSound() using that
	// handle will be killing the wrong sound or *worse*, a sound that doesn't exist.  Returning the handle 
	// of the PlaySoundInfo struct will ensure we only return a valid handle if explicitly told to (ie. PLAYSOUND_GETHANDLE);

	return psi.m_hSound;
}