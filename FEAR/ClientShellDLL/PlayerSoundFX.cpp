// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerSoundFX.cpp
//
// PURPOSE : Player sound special FX - Implementation
//
// CREATED : 7/28/98 (was WeaponSoundFX)
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerSoundFX.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSoundFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

bool CPlayerSoundFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	PLAYERSOUNDCREATESTRUCT* pPSCS = (PLAYERSOUNDCREATESTRUCT*)psfxCreateStruct;

	m_vPos		= pPSCS->vPos;
	m_nClientId = pPSCS->nClientId;
	m_nType		= pPSCS->nType;
	m_hWeapon	= pPSCS->hWeapon;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSoundFX::CreateObject
//
//	PURPOSE:	Create object associated with the CPlayerSoundFX
//
// ----------------------------------------------------------------------- //

bool CPlayerSoundFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return false;

	uint32 dwId;
	if (m_pClientDE->GetLocalClientID(&dwId) != LT_OK) return false;

	// Don't play sounds for this client...

	if (int(dwId) == m_nClientId) return false;

	PlayerSoundId eSndType = (PlayerSoundId)m_nType;

	if (::IsWeaponSound(eSndType))
	{
		if( !m_hWeapon )
			return false;

		PlayWeaponSound( m_hWeapon, !USE_AI_DATA, m_vPos, (PlayerSoundId)m_nType );
	}
	else
	{
		switch (eSndType)
		{
			case PSI_JUMP :
			{
				HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("Jump3D");
				if (hSoundRec)
				{
					g_pClientSoundMgr->PlayDBSoundFromPos(m_vPos, hSoundRec, 1000.0f,
						SOUNDPRIORITY_MISC_HIGH,  PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
						DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);
				}
			}
			break;

			case PSI_LAND :
			{
				HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("Landing3D");
				if (hSoundRec)
				{
					g_pClientSoundMgr->PlayDBSoundFromPos(m_vPos, hSoundRec, 1000.0f,
						SOUNDPRIORITY_MISC_HIGH,  PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
						DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);
				}
			}
			break;

			default : break;
		}
	}


	return false;  // Delete me, I'm done :)
}