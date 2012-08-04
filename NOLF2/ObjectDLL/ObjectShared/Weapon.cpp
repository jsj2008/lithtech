// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.cpp
//
// PURPOSE : Weapon class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "weapon.h"
#include "ServerUtilities.h"
#include "iltserver.h"
#include "ltengineobjects.h"
#include "MsgIds.h"
#include "HHWeaponModel.h"
#include "Character.h"
#include "WeaponFXTypes.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "Weapons.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "AIUtils.h"
#include "Projectile.h"
#include "FXButeMgr.h"

extern LTBOOL g_bInfiniteAmmo;
extern CGameServerShell* g_pGameServerShell;
extern CAIStimulusMgr* g_pAIStimulusMgr;

#define INFINITE_AMMO_AMOUNT 1000

uint16 g_wIgnoreFX = 0;
uint8 g_nRandomWeaponSeed = 255;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CWeapon
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeapon::CWeapon()
{
	m_eState			= W_IDLE;
    m_bHave             = LTFALSE;
	m_nAmmoInClip		= 1;
	m_fDamageFactor		= 1.0f;
	m_fRangeAdjust		= 1.0f;

	m_fLifeTime			= -1.0f;

	m_nWeaponId			= 0;

    m_pParent           = LTNULL;

	m_nCurTracer		= 0;

	m_nLastTimestamp	= 0;
	m_bFirstSelection	= LTTRUE;

	m_nHandAni			= INVALID_ANI;
	m_nPreFireAni		= INVALID_ANI;
	m_nFireAni			= INVALID_ANI;
	m_nPostFireAni		= INVALID_ANI;

	m_fLastFireTime		= 0.0f;

	m_pWeaponData		= NULL;
	m_pAmmoData			= NULL;

	m_bHidden			= false;

	Drop();
}
	
void CWeapon::Drop()
{
    m_bHave = LTFALSE;
	if (m_pWeaponData)
		m_nAmmoId = m_pWeaponData->nDefaultAmmoId;
	else
		m_nAmmoId = 0;
	for( int i = 0; i < W_MAX_MODS; ++i )
	{
		m_Mods[i] = WMGR_INVALID_ID;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::~CWeapon
//
//	PURPOSE:	Deallocate memory
//
// ----------------------------------------------------------------------- //

CWeapon::~CWeapon()
{
	KillLoopSound();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Init
//
//	PURPOSE:	Init object
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapon::Init(CWeapons* pParent, HOBJECT hObj, int nWeaponId, int nAmmoId)
{
    if (!pParent || !hObj) return LTFALSE;

	m_pParent	= pParent;
	m_hObject   = hObj;
	m_nWeaponId	= nWeaponId;
	m_nAmmoId	= nAmmoId;

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
    if (!m_pWeaponData) return LTFALSE;

	m_pAmmoData = g_pWeaponMgr->GetAmmo(m_nAmmoId);
    if (!m_pAmmoData) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CWeapon::ReloadClip(LTBOOL bPlayReload, int nNewAmmo /*=-1*/, uint8 nAmmoId /*=WMGR_INVALID_ID*/ )
{
	if (!m_pParent) return;

	// See if we want to reload with a different ammo type...
	
	if( nAmmoId	!= WMGR_INVALID_ID && nAmmoId != m_nAmmoId )
	{
		SetAmmoId( nAmmoId );
		m_nAmmoInClip = 0;
	}

	int nAmmo = nNewAmmo >= 0 ? nNewAmmo : m_pParent->GetAmmoCount(m_nAmmoId);
	int nShotsPerClip = m_pWeaponData->nShotsPerClip;

	// Clip is full...
	LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pWeaponData->bInfiniteAmmo);
	if (m_nAmmoInClip == nShotsPerClip && !bInfiniteAmmo )
	{
		return;
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
	}


	LTFLOAT	fRadiusMult = 1.0f;
	if (IsPlayer(m_hObject))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
        fRadiusMult = pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_STEALTH,StealthModifiers::eRadius);
	}
	
	// Add a sound stimulus for the reloading sound.

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hModelObject, &vPos);
	g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyWeaponReloadSound, m_hObject, m_hModelObject, vPos, fRadiusMult, 1.f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateWeapon
//
//  PURPOSE:    Update the weapon (fire if bFire == LTTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::UpdateWeapon(WeaponFireInfo & fireInfo, LTBOOL bFire)
{
	WeaponState eRet = W_IDLE;

	if (bFire)
	{
		fireInfo.nSeed = GetRandom(2,255);
		// Put in a timestamp since the AI's use this and don't send us a timestamp
		m_fLastFireTime = g_pLTServer->GetTime();
		fireInfo.nFireTimestamp = (uint32)(m_fLastFireTime * 1000.0f);
		eRet = Fire(fireInfo);
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::Fire(WeaponFireInfo & info)
{
	if (!info.hFiredFrom || !g_pWeaponMgr || !m_pParent) return W_IDLE;

	// Make sure we always have ammo if we should...

	LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pWeaponData->bInfiniteAmmo);
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : m_pParent->GetAmmoCount(m_nAmmoId);
	int nShotsPerClip = m_pWeaponData->nShotsPerClip;

	AmmoType eAmmoType = m_pAmmoData->eType;


	// Restrict the rate of fire
	if ((info.nFireTimestamp - m_nLastTimestamp) < m_pWeaponData->m_nFireDelay)
	{
		// Hey!  That was too fast!
		// Don't actually fire but decrement the ammo count.
		
		if( nAmmo > 0 )
		{
			if (m_nAmmoInClip > 0)
			{
				if (nShotsPerClip > 0 && !bInfiniteAmmo)
				{
					m_nAmmoInClip--;
				}

				if (!bInfiniteAmmo)
				{
					m_pParent->DecrementAmmo(m_nAmmoId);
				}
			}

			// Check to see if we need to reload...

			if (nShotsPerClip > 0)
			{
				if (m_nAmmoInClip <= 0)
				{
					// Only automatically reload if we're the player... and don't play the animation.

					if ( IsPlayer(info.hFiredFrom) )
					{
						ReloadClip(LTFALSE, nAmmo);
					}
				}
			}
		}

		return W_IDLE;
	}

	// Remember when this shot went off..
	m_nLastTimestamp = info.nFireTimestamp;

	WeaponState eRet = W_IDLE;

	g_nRandomWeaponSeed = info.nSeed;


	if (nAmmo > 0)
	{
		// Ignore the exit surface, silenced, and alt-fire fx...

		g_wIgnoreFX = WFX_EXITSURFACE | WFX_SILENCED | WFX_ALTFIRESND;

		// See if the weapon is silenced...

		if (GetSilencer())
		{
			g_wIgnoreFX &= ~WFX_SILENCED;	// Do silenced
			g_wIgnoreFX |= WFX_FIRESOUND;	// Don't do fire
		}
		else if (info.bAltFire)
		{
			g_wIgnoreFX &= ~WFX_ALTFIRESND;  // Do alt-fire
			g_wIgnoreFX |= WFX_FIRESOUND;	 // Don't do fire
		}

		// If the player fired this and it is the appropriate weapon type,
		// don't worry about playing the fire sound (the player already played it)...

		if (IsPlayer(info.hFiredFrom))
		{
			if (eAmmoType == PROJECTILE)
			{
				g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND;
			}
		}

		// See if we should do tracers or not...

		if (m_pAmmoData->pTracerFX)
		{
			m_nCurTracer = ( m_nCurTracer + 1 ) %
			                   m_pAmmoData->pTracerFX->nFrequency;
			if (m_nCurTracer != 0)
			{
				g_wIgnoreFX |= WFX_TRACER;
			}
		}
		else
		{
			g_wIgnoreFX |= WFX_TRACER;
		}

		if (m_nAmmoInClip > 0)
		{
			LTRotation rRot;
			LTVector vU, vR, vF;

			g_pLTServer->GetObjectRotation(info.hFiredFrom, &rRot);
			vU = rRot.Up();
			vR = rRot.Right();
			vF = rRot.Forward();

			// Create a projectile for every vector...

			int nVectorsPerShot = m_pWeaponData->nVectorsPerRound;

			//uint8 nNewRandomSeed = GetRandom(2, 255);

			WeaponPath wp;
			LTVector vOriginalPath = info.vPath;

			wp.nWeaponId = m_nWeaponId;
			wp.vU		 = vU;
			wp.vR		 = vR;
			wp.fPerturbR = info.fPerturbR;
			wp.fPerturbU = info.fPerturbU;

			for (int i=0; i < nVectorsPerShot; i++)
			{
				srand(g_nRandomWeaponSeed);
				g_nRandomWeaponSeed = GetRandom(2, 255);

				wp.vPath = vOriginalPath;
				g_pWeaponMgr->CalculateWeaponPath(wp);
				info.vPath = wp.vPath;

				// Shoot the weapon...

				if (eAmmoType == PROJECTILE)
				{
					if (!CreateProjectile(rRot, info))
					{
						return W_IDLE;
					}
				}
				else if (eAmmoType == VECTOR)
				{
					// Don't actually create an object, just use our
					// parent's projectile object to do vector calculations...

					CProjectile* pProj = m_pParent->GetVecProjectile();
					if (pProj)
					{
						pProj->Setup(this, info);
					}
				}

				// If we are shooting multiple vectors ignore some special
				// fx after the first vector...

				g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SHELL |
					WFX_SILENCED | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
			}

			//srand(nNewRandomSeed);

			if (nShotsPerClip > 0 && !bInfiniteAmmo)
			{
				m_nAmmoInClip--;
			}

			if (!bInfiniteAmmo)
			{
				m_pParent->DecrementAmmo(m_nAmmoId);
			}
		}

		eRet = W_FIRED;

		// Check to see if we need to reload...

		if (nShotsPerClip > 0)
		{
			if (m_nAmmoInClip <= 0)
			{
				// Only automatically reload if we're the player... and don't play the animation.

				if ( IsPlayer(info.hFiredFrom) )
				{
					ReloadClip(LTFALSE, nAmmo);
				}
			}
		}
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateProjectile
//
//	PURPOSE:	Create the approprite projectile to fire.
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapon::CreateProjectile(LTRotation & rRot, WeaponFireInfo & info)
{
    if (!m_pAmmoData->pProjectileFX || !(m_pAmmoData->pProjectileFX->szClass[0])) return LTFALSE;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	// set the starting rotation
    theStruct.m_Rotation = rRot;

	// move the start position along the line of fire a little
	if ( MATH_EPSILON < m_pAmmoData->pProjectileFX->fFireOffset )
	{
		// determine the new point
		LTVector vNewFirePos;
		vNewFirePos = info.vPath;
		vNewFirePos *= m_pAmmoData->pProjectileFX->fFireOffset;
		vNewFirePos += info.vFirePos;

		// see if there is any geometry in the way
		IntersectInfo iInfo;
		IntersectQuery qInfo;
		qInfo.m_From = info.vFirePos;
		qInfo.m_To = vNewFirePos;
		qInfo.m_Direction = info.vPath;
		qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		qInfo.m_FilterFn  = SpecificObjectFilterFn;
		qInfo.m_PolyFilterFn = LTNULL;
		qInfo.m_pUserData = info.hFiredFrom;

		if ( LTTRUE == g_pLTServer->IntersectSegment( &qInfo, &iInfo ) )
		{
			// hit something, just put the fire position at the original
			theStruct.m_Pos = info.vFirePos;
		}
		else
		{
			// hit nothing, use the new fire position
			theStruct.m_Pos = vNewFirePos;
		}
	}
	else
	{
		theStruct.m_Pos = info.vFirePos;
	}

    HCLASS hClass = g_pLTServer->GetClass(m_pAmmoData->pProjectileFX->szClass);

	UBER_ASSERT1( hClass, "Unable to retreive class: %s", m_pAmmoData->pProjectileFX->szClass );

	if (hClass)
	{
        CProjectile* pProj = (CProjectile*)g_pLTServer->CreateObject(hClass, &theStruct);
		if (pProj)
		{
			if( !pProj->Setup(this, info) )
			{
				g_pLTServer->RemoveObject( pProj->m_hObject );
				return LTFALSE;
			}

			// Create a stimulus for AIs to notice projectile.
			
			pProj->CreateDangerStimulus( info.hFiredFrom );

            return LTTRUE;
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetModelObject()
//
//	PURPOSE:	Get the object associated with the weapon model
//
// ----------------------------------------------------------------------- //

HOBJECT CWeapon::GetModelObject()
{
	return m_hModelObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetModelObject()
//
//	PURPOSE:	Set the object associated with the weapon model
//
// ----------------------------------------------------------------------- //

void CWeapon::SetModelObject(HOBJECT hObj)
{
	m_hModelObject = hObj;

	if (m_hModelObject)
	{
		// Init the model animations

		g_pModelLT->GetAnimIndex( m_hModelObject, "Hand",		m_nHandAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "PreFire",	m_nPreFireAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "Fire",		m_nFireAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "PostFire",	m_nPostFireAni );

        g_pModelLT->SetLooping( m_hModelObject, MAIN_TRACKER, false );
		
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Select()
{
	// We must have ammo in the clip the very first time the weapon is selected...

	if( m_bFirstSelection )
	{
		m_bFirstSelection = LTFALSE;

		// Set the initial ammo count...

		int nAmmo = m_pParent->GetAmmoCount( m_nAmmoId );
		int nShotsPerClip = m_pWeaponData->nShotsPerClip;

		if (nAmmo > 0 && nShotsPerClip > 0)
		{
			m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Deselect()
//
//	PURPOSE:	Deselect the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Deselect()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

	SAVE_HOBJECT( m_hObject );
    SAVE_HOBJECT( m_hModelObject );

    SAVE_FLOAT( m_fLifeTime );
    SAVE_INT( m_nCurTracer );
    SAVE_BOOL( m_bHave );
    SAVE_INT( m_nAmmoInClip );
    SAVE_BYTE( m_eState );
    SAVE_FLOAT( m_fDamageFactor );
    SAVE_FLOAT( m_fRangeAdjust );
    SAVE_BYTE( m_nWeaponId );
    SAVE_BYTE( m_nAmmoId );
	SAVE_BOOL( m_bFirstSelection );
	SAVE_DWORD( m_nHandAni );
	SAVE_DWORD( m_nPreFireAni );
	SAVE_DWORD( m_nFireAni );
	SAVE_DWORD( m_nPostFireAni );

	SAVE_TIME( m_fLastFireTime );
	SAVE_DWORD( m_nLastTimestamp );

	for (int i=0; i < W_MAX_MODS; i++)
	{
        SAVE_DWORD( m_Mods[i] );
	}

	SAVE_bool( m_bHidden );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

    LOAD_HOBJECT( m_hObject );
    LOAD_HOBJECT( m_hModelObject );

    LOAD_FLOAT( m_fLifeTime );
    LOAD_INT( m_nCurTracer );
    LOAD_BOOL( m_bHave );
    LOAD_INT( m_nAmmoInClip );
    LOAD_BYTE_CAST( m_eState, WeaponState );
    LOAD_FLOAT( m_fDamageFactor );
    LOAD_FLOAT( m_fRangeAdjust );
    LOAD_BYTE( m_nWeaponId );
    LOAD_BYTE( m_nAmmoId );
	LOAD_BOOL( m_bFirstSelection );
	LOAD_DWORD_CAST( m_nHandAni, HMODELANIM );
	LOAD_DWORD_CAST( m_nPreFireAni, HMODELANIM );
	LOAD_DWORD_CAST( m_nFireAni, HMODELANIM );
	LOAD_DWORD_CAST( m_nPostFireAni, HMODELANIM );
	
	LOAD_TIME( m_fLastFireTime );
	LOAD_DWORD( m_nLastTimestamp );

	for (int i=0; i < W_MAX_MODS; i++)
	{
        LOAD_DWORD( m_Mods[i] );
	}

	LOAD_bool( m_bHidden );

	m_pWeaponData	= g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData		= g_pWeaponMgr->GetAmmo(m_nAmmoId);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetInstDamage()
//
//	PURPOSE:	Get the instant damage done by this weapon (value can vary)
//
// ----------------------------------------------------------------------- //

LTFLOAT CWeapon::GetInstDamage() const
{
	if (!m_pAmmoData) return 0.0f;

    LTFLOAT fDamage = (LTFLOAT) m_pAmmoData->nInstDamage;

	fDamage *= GetRandom(0.8f, 1.2f) * m_fDamageFactor;
	
	return fDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetProgDamage()
//
//	PURPOSE:	Get the progressive damage done by this weapon
//
// ----------------------------------------------------------------------- //

LTFLOAT CWeapon::GetProgDamage() const
{
	if (!m_pAmmoData) return 0.0f;

    LTFLOAT fDamage = m_pAmmoData->fProgDamage;

	fDamage *= m_fDamageFactor;

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddMod
//
//	PURPOSE:	Add the specified mod to the weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapon::AddMod( int nMod )
{
	MOD const *pM = g_pWeaponMgr->GetMod( nMod );
    if (!pM) return LTFALSE;

    LTBOOL bRet = LTFALSE;

	for (int i=0; i < W_MAX_MODS; i++)
	{
		if (m_Mods[i] == WMGR_INVALID_ID)
		{
			m_Mods[i] = nMod;

            return LTTRUE;
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::PlayAnimation
//
//  PURPOSE:	Plays the given animation of the model.
//
// ----------------------------------------------------------------------- //

bool CWeapon::PlayAnimation( HMODELANIM hAni, bool bForce, bool bLoop, bool bReset )
{
	if( hAni == INVALID_ANI	|| !m_hModelObject )
		return false;

	// See if we should brute force the animation or wait until the current ani is done...

	if( !bForce )
	{
		uint32 dwFlags;
		g_pModelLT->GetPlaybackState( m_hModelObject, MAIN_TRACKER, dwFlags );
		
		if( !(dwFlags & MS_PLAYDONE) )
			return false;
	}

	g_pModelLT->SetCurAnim( m_hModelObject, MAIN_TRACKER, hAni );
	g_pModelLT->SetLooping( m_hModelObject, MAIN_TRACKER, bLoop );
	
	if( bReset )
		g_pModelLT->ResetAnim( m_hModelObject, MAIN_TRACKER );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::KillLoopSound
//
//  PURPOSE:	Call to the hand held weapon to kill any looping sound it may have.
//
// ----------------------------------------------------------------------- //

void CWeapon::KillLoopSound()
{
	if( !m_hModelObject )
		return;

	CHHWeaponModel *pHHWeapon = dynamic_cast< CHHWeaponModel* >( g_pLTServer->HandleToObject( m_hModelObject ));
	if( pHHWeapon )
	{
		pHHWeapon->KillLoopSound();
	}
}

LTFLOAT CWeapon::GetLifeTime() const
{
	if (!g_pWeaponMgr) return 0.0f;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo || !pAmmo->pProjectileFX) return 0.0f;

	return m_fLifeTime < 0.0f ? pAmmo->pProjectileFX->fLifeTime : m_fLifeTime;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::HideWeapon
//
//  PURPOSE:	Hide or show the weapon...
//
// ----------------------------------------------------------------------- //

void CWeapon::HideWeapon( bool bHidden )
{
	m_bHidden = bHidden;
	g_pCommonLT->SetObjectFlags( m_hModelObject, OFT_Flags, (m_bHidden ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE), FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
}



