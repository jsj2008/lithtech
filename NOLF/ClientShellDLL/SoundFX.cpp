// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.cpp
//
// PURPOSE : Sound fx - Implementation
//
// CREATED : 6/20/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundFX.h"
#include "iltclient.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *(SNDCREATESTRUCT*)psfxCreateStruct;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFX::CreateObject
//
//	PURPOSE:	Create object associated with the CSoundFX
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	if (!m_cs.pSndName || !(*m_cs.pSndName)) return LTFALSE;

	// If we're non-looping we're done...

    return (m_cs.bLoop);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFX::Update
//
//	PURPOSE:	See if we should go away or not...
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundFX::Update()
{
	// Stay around as long as we have a valid sound...
	return (LTBOOL) m_hSnd;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFX::Play
//
//	PURPOSE:	Play the sound
//
// ----------------------------------------------------------------------- //

void CSoundFX::Play()
{
 	if (!m_cs.pSndName || !(*m_cs.pSndName)) return;

	// Make sure we're not playing...

	Stop();

	uint32	dwFlags = 0;

	if (m_cs.fPitchShift != 1.0)
	{
		dwFlags |= PLAYSOUND_CTRL_PITCH;
	}

	if (m_cs.bLoop)
	{
		dwFlags |= (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
	}


	if (m_cs.bLocal)
	{
		m_hSnd = g_pClientSoundMgr->PlaySoundLocal(m_cs.pSndName, m_cs.ePriority,
			dwFlags, m_cs.nVolume, m_cs.fPitchShift);
	}
	else
	{
 		m_hSnd = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, m_cs.pSndName, 
				m_cs.fRadius, m_cs.ePriority, dwFlags,
				m_cs.nVolume, m_cs.fPitchShift);
	}
}

