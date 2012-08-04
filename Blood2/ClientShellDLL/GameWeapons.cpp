//----------------------------------------------------------
//
// MODULE  : GAMEWEAPONS.CPP
//
// PURPOSE : Blood2 weapon classes
//
// CREATED : 9/20/97
//
//----------------------------------------------------------

// Includes....
#include <crtdbg.h>
#include <mbstring.h>
#include "GameWeapons.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "ExplosionFX.h"
#include "SoundTypes.h"
#include "WeaponPowerupFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMelee::UpdateFiringState()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CMelee::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if(!m_pClientDE)		return;

	// Randomly pick a new hit type until we go into the fire state...
	if(m_eState == WS_REST)
	{
		if(GetRandom(0,1))
			m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire");
		else
			m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire2");
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::UpdateFiringState()
//
//	PURPOSE:	Fires the Voodoo doll
//
// ----------------------------------------------------------------------- //

void CVoodooDoll::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if(!m_pClientDE)		return;

	// Randomly pick a new hit type until we go into the fire state...
	if(m_eState == WS_REST)
	{
		m_nHitFX = GetRandom(0,4);

		switch(m_nHitFX)
		{
			case	0:	// Chest (red)
				m_nHitType = DAMAGE_TYPE_NORMAL;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire");
				break;

			case	1:	// Nuts (red)
				m_nHitType = DAMAGE_TYPE_NORMAL;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire_lleg");
				break;

			case	2:	// Eyes (blue)
				m_nHitType = DAMAGE_TYPE_BLIND;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire_larm");
				break;

			case	3:	// Arm (white)
				m_nHitType = DAMAGE_TYPE_DROPWEAPON;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire_rarm");
				break;

			case	4:	// Leg (green)
				m_nHitType = DAMAGE_TYPE_SLOW;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire_rleg");
				break;

			default:	// Default (red)
				m_nHitType = DAMAGE_TYPE_NORMAL;
				m_nFireAnim = m_pClientDE->GetAnimIndex(m_hObject, "fire");
				break;
		}
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::FireMsgSpecialData()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

DBOOL CVoodooDoll::FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)
{
	byFlags |= FIREMSG_SPECIAL_DWORD;

	m_pClientDE->WriteToMessageByte(hWrite, byFlags);
	m_pClientDE->WriteToMessageDWord(hWrite, m_nHitFX);

	return	DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShotgun::UpdateFiringState()
//
//	PURPOSE:	Special update stuff for shotgun
//
// ----------------------------------------------------------------------- //

void CShotgun::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (m_nOrigFireAnim == -1)
		m_nOrigFireAnim = m_nFireAnim;

	// See if we need to switch the fire anim
	if(m_eState == WS_REST)
	{
		if(m_bSwitchBarrels)
		{
			m_bSwitchBarrels = DFALSE;
			m_bSecondBarrel = !m_bSecondBarrel;

			if(m_nFireAnim == m_nAltFireAnim)
				m_nFireAnim = m_nOrigFireAnim;
			else
				m_nFireAnim = m_nAltFireAnim;
		}
	}
	else if(m_eState == WS_FIRING)
	{
		if(!m_bSwitchBarrels)
			m_bSwitchBarrels = DTRUE;
	}

	// Make sure we don't fire two barrel if the first one has been used already...
	if (m_bSecondBarrel && bAltFiring)
	{
		bFiring = DTRUE;
		bAltFiring = DFALSE;
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrb::UpdateFiringState()
//
//	PURPOSE:	Special update stuff for orb
//
// ----------------------------------------------------------------------- //

void COrb::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	if(m_eState == WS_REST)
		m_pClientDE->SetModelLooping(m_hObject, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLifeLeech::UpdateFiringState()
//
//	PURPOSE:	Special update stuff for leech
//
// ----------------------------------------------------------------------- //

void CLifeLeech::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaCannon::UpdateFiringState()
//
//	PURPOSE:	Updates the power-up sprite position for alternate fire
//
// ----------------------------------------------------------------------- //

void CTeslaCannon::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	if(m_eState == WS_START_ALT_FIRING && bSprite)
	{
//		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
//		CSFXMgr* psfxMgr = pShell->GetSFXMgr();

		numAltFires = 0;
		bSprite = DFALSE;

/*		WEAPPOWERCREATESTRUCT	weapFX;
		weapFX.hGun				= m_hObject;
		VEC_COPY(weapFX.vPosOffset, m_vAdjFlashPos);
		VEC_SET(weapFX.vScale, 0.075f, 0.075f, 0.0f);
		weapFX.fLifeTime		= 1.5f;
		weapFX.fInitAlpha		= 1.0f;
		weapFX.bFade			= DFALSE;
		weapFX.pSpriteFile		= m_pClientDE->CreateString("Sprites\\teslaAltM.spr");

		if(psfxMgr)
			psfxMgr->CreateSFX(SFX_WEAPONPOWERUP_ID, &weapFX, DFALSE);

		m_pClientDE->FreeString(weapFX.pSpriteFile);
*/	}
	else if(m_eState != WS_START_ALT_FIRING)
	{
		bSprite = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaCannon::FireMsgSpecialData()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

DBOOL CTeslaCannon::FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)
{
	numAltFires++;
	byFlags |= FIREMSG_SPECIAL_DWORD;

	m_pClientDE->WriteToMessageByte(hWrite, byFlags);
	m_pClientDE->WriteToMessageDWord(hWrite, numAltFires);

	return	DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularity::UpdateFiringState()
//
//	PURPOSE:	Special update stuff for singularity
//
// ----------------------------------------------------------------------- //

void CSingularity::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	if(m_eState == WS_START_ALT_FIRING && bSprite)
	{
//		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
//		CSFXMgr* psfxMgr = pShell->GetSFXMgr();

		bSprite = DFALSE;

/*		WEAPPOWERCREATESTRUCT	weapFX;
		weapFX.hGun				= m_hObject;
		VEC_COPY(weapFX.vPosOffset, m_vAdjFlashPos);
		VEC_SET(weapFX.vScale, 0.375f, 0.375f, 0.0f);
		weapFX.fLifeTime		= 1.66f;
		weapFX.fInitAlpha		= 1.0f;
		weapFX.bFade			= DFALSE;
		weapFX.pSpriteFile		= m_pClientDE->CreateString("Sprites\\bla1.spr");

		if(psfxMgr)
			psfxMgr->CreateSFX(SFX_WEAPONPOWERUP_ID, &weapFX, DFALSE);

		m_pClientDE->FreeString(weapFX.pSpriteFile);
*/	}
	else if(m_eState != WS_START_ALT_FIRING)
	{
		bSprite = DTRUE;
	}
}


#ifndef _DEMO

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapProximityBomb::UpdateFiringState()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

void CWeapProximityBomb::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	//********************************************************************************
	// Override the WS_STOP_FIRING state only... and leave the rest the same
	if(m_eState == WS_STOP_FIRING)
	{
		if (!PlayAnimation(m_nStopFireAnim))
		{
			PlayAnimation(m_nDrawAnim);
			m_eState = WS_DRAW;
		}

		m_fIdleStartTime = m_pClientDE->GetTime();
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	//********************************************************************************
	// Handle some extra situations for the other states
	if(m_eState == WS_START_FIRING)
		m_fStartTime = m_pClientDE->GetTime();

	if(m_eState == WS_FIRING)
	{
		DFLOAT	fPercent;

		m_fTimeHeld = m_pClientDE->GetTime() - m_fStartTime;
		fPercent = m_fTimeHeld / BOMBS_MAX_DIST_TIME;
		if(fPercent > 1.0f)	fPercent = 1.0f;

		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
		pShell->SetPowerBarLevel(fPercent);

		m_fProjVelocity = BOMBS_MIN_VELOCITY + (BOMBS_VEL_INCREASE * fPercent);

		if(m_fTimeHeld > BOMBS_MAX_HOLD_TIME)
		{
			pShell->CSPrint("Idiot! Let go next time!");
			m_fProjVelocity = 0.0f;
			m_fTimeHeld = 0.0f;
			m_eState = WS_STOP_FIRING;
			m_pClientDE->SetModelLooping(m_hObject, DFALSE);
		}
	}
	else
		m_pClientDE->SetModelLooping(m_hObject, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapProximityBomb::FireMsgSpecialData()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

DBOOL CWeapProximityBomb::FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)
{
	byFlags |= FIREMSG_SPECIAL_FLOAT;

	m_pClientDE->WriteToMessageByte(hWrite, byFlags);
	m_pClientDE->WriteToMessageFloat(hWrite, m_fProjVelocity);

	return	DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapRemoteBomb::UpdateFiringState()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

void CWeapRemoteBomb::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	//********************************************************************************
	// Override the WS_STOP_FIRING state only... and leave the rest the same
	if(m_eState == WS_STOP_FIRING)
	{
		if (!PlayAnimation(m_nStopFireAnim))
		{
			PlayAnimation(m_nDrawAnim);
			m_eState = WS_DRAW;
		}

		m_fIdleStartTime = m_pClientDE->GetTime();
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	//********************************************************************************
	// Handle some extra situations for the other states
	if(m_eState == WS_START_FIRING)
		m_fStartTime = m_pClientDE->GetTime();

	if(m_eState == WS_FIRING)
	{
		DFLOAT	fPercent;

		m_fTimeHeld = m_pClientDE->GetTime() - m_fStartTime;
		fPercent = m_fTimeHeld / BOMBS_MAX_DIST_TIME;
		if(fPercent > 1.0f)	fPercent = 1.0f;

		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
		pShell->SetPowerBarLevel(fPercent);

		m_fProjVelocity = BOMBS_MIN_VELOCITY + (BOMBS_VEL_INCREASE * fPercent);

/*		if(m_fTimeHeld > BOMBS_MAX_HOLD_TIME)
		{
			pShell->CSPrint("Idiot! Let go next time!");
			m_fProjVelocity = 0.0f;
			m_fTimeHeld = 0.0f;
			m_eState = WS_STOP_FIRING;
			m_pClientDE->SetModelLooping(m_hObject, DFALSE);
		}*/
	}
	else
		m_pClientDE->SetModelLooping(m_hObject, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapRemoteBomb::FireMsgSpecialData()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

DBOOL CWeapRemoteBomb::FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)
{
	byFlags |= FIREMSG_SPECIAL_FLOAT;

	m_pClientDE->WriteToMessageByte(hWrite, byFlags);
	m_pClientDE->WriteToMessageFloat(hWrite, m_fProjVelocity);

	return	DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapTimeBomb::UpdateFiringState()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

void CWeapTimeBomb::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE)	return;

	//********************************************************************************
	// Override the WS_STOP_FIRING state only... and leave the rest the same
	if(m_eState == WS_STOP_FIRING)
	{
		if (!PlayAnimation(m_nStopFireAnim))
		{
			PlayAnimation(m_nDrawAnim);
			m_eState = WS_DRAW;
		}

		m_fIdleStartTime = m_pClientDE->GetTime();
	}

	CViewWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	//********************************************************************************
	// Handle some extra situations for the other states
	if(m_eState == WS_START_FIRING)
		m_fStartTime = m_pClientDE->GetTime();

	if(m_eState == WS_FIRING)
	{
		DFLOAT	fPercent;

		m_fTimeHeld = m_pClientDE->GetTime() - m_fStartTime;
		fPercent = m_fTimeHeld / BOMBS_MAX_DIST_TIME;
		if(fPercent > 1.0f)	fPercent = 1.0f;

		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
		pShell->SetPowerBarLevel(fPercent);

		m_fProjVelocity = BOMBS_MIN_VELOCITY + (BOMBS_VEL_INCREASE * fPercent);

		if(m_fTimeHeld > BOMBS_MAX_HOLD_TIME)
		{
			pShell->CSPrint("Idiot! Let go next time!");
			m_fProjVelocity = 0.0f;
			m_fTimeHeld = 0.0f;
			m_eState = WS_STOP_FIRING;
			m_pClientDE->SetModelLooping(m_hObject, DFALSE);
		}
	}
	else
		m_pClientDE->SetModelLooping(m_hObject, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapTimeBomb::FireMsgSpecialData()
//
//	PURPOSE:	Special update
//
// ----------------------------------------------------------------------- //

DBOOL CWeapTimeBomb::FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags)
{
	byFlags |= FIREMSG_SPECIAL_FLOAT;

	m_pClientDE->WriteToMessageByte(hWrite, byFlags);
	m_pClientDE->WriteToMessageFloat(hWrite, m_fProjVelocity);

	return	DTRUE;
}


#endif // _DEMO