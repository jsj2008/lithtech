// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponSoundFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 7/28/98
//
// ----------------------------------------------------------------------- //

#include "WeaponSoundFX.h"
#include "RiotClientShell.h"
#include "clientheaders.h"
#include "RiotMsgIds.h"
#include "WeaponDefs.h"

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponSoundFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponSoundFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	WSOUNDCREATESTRUCT* pWS = (WSOUNDCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vPos, pWS->vPos);
	m_nClientId = pWS->nClientId;
	m_nType		= pWS->nType;
	m_nWeaponId	= pWS->nWeaponId;
	m_hSound	= pWS->hSound;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponSoundFX::CreateObject
//
//	PURPOSE:	Create object associated with the CWeaponSoundFX
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponSoundFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	uint32 dwId;
	if (m_pClientDE->GetLocalClientID(&dwId) != LT_OK) return LTFALSE;

	// Don't play sounds for this client...

	if (int(dwId) == m_nClientId) return LTFALSE;


	char* pSound = LTNULL;
	uint8 nPriority = SOUNDPRIORITY_PLAYER_MEDIUM;
	LTFLOAT fRadius = 2000.0f;

	if (m_nType == WEAPON_SOUND_DRYFIRE)
	{
		pSound = GetWeaponDryFireSound((RiotWeaponId)m_nWeaponId);
	}
	else if (m_nType == WEAPON_SOUND_FIRE)
	{
		pSound = GetWeaponFireSound((RiotWeaponId)m_nWeaponId);
	}
	else if (m_hSound)
	{
		pSound = const_cast<char *>(m_pClientDE->GetStringData(m_hSound));
	}

	if (pSound)
	{
		PlaySoundFromPos(&m_vPos, pSound, fRadius, nPriority);
	}

	if (m_hSound)
	{
		m_pClientDE->FreeString(m_hSound);
		m_hSound = LTNULL;
	}

	return LTFALSE;  // Delete me, I'm done :)
}

