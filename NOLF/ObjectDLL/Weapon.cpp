// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.cpp
//
// PURPOSE : Weapon class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
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

extern LTBOOL g_bInfiniteAmmo;
extern CGameServerShell* g_pGameServerShell;

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
	m_nAmmoId			= 0;

    m_pParent           = LTNULL;
    m_hModelObject      = LTNULL;

	m_nCurTracer		= 0;

	m_nLastTimestamp	= 0;
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

void CWeapon::ReloadClip(LTBOOL bPlayReload, int nNewAmmo)
{
	if (!m_pParent) return;

	int nAmmo = nNewAmmo >= 0 ? nNewAmmo : m_pParent->GetAmmoCount(m_nAmmoId);
	int nShotsPerClip = m_pWeaponData->nShotsPerClip;

	// Clip is full...

	if (m_nAmmoInClip == nShotsPerClip)
	{
		return;
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateWeapon
//
//  PURPOSE:    Update the weapon (fire if bFire == LTTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::UpdateWeapon(WFireInfo & fireInfo, LTBOOL bFire)
{
	WeaponState eRet = W_IDLE;

	if (bFire)
	{
		fireInfo.nSeed = GetRandom(2,255);
		// Put in a timestamp since the AI's use this and don't send us a timestamp
		fireInfo.nFireTimestamp = (uint32)(g_pLTServer->GetTime() * 1000.0f);
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

WeaponState CWeapon::Fire(WFireInfo & info)
{
	if (!info.hFiredFrom || !g_pWeaponMgr || !m_pParent) return W_IDLE;

	// Restrict the rate of fire
	if ((info.nFireTimestamp - m_nLastTimestamp) < m_pWeaponData->m_nFireDelay)
	{
		// Hey!  That was too fast!
		return W_IDLE;
	}
	// Remember when this shot went off..
	m_nLastTimestamp = info.nFireTimestamp;

	WeaponState eRet = W_IDLE;

	g_nRandomWeaponSeed = info.nSeed;


	// Make sure we always have ammo if we should...

    LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pWeaponData->bInfiniteAmmo);
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : m_pParent->GetAmmoCount(m_nAmmoId);

	AmmoType eAmmoType = m_pAmmoData->eType;

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
			++m_nCurTracer;
			if ((m_nCurTracer % m_pAmmoData->pTracerFX->nFrequency) != 0)
			{
				g_wIgnoreFX |= WFX_TRACER;
			}
		}
		else
		{
			g_wIgnoreFX |= WFX_TRACER;
		}

		int nShotsPerClip = m_pWeaponData->nShotsPerClip;

		if (m_nAmmoInClip > 0)
		{
            LTRotation rRot;
            LTVector vU, vR, vF;

            g_pLTServer->GetObjectRotation(info.hFiredFrom, &rRot);
            g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

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

LTBOOL CWeapon::CreateProjectile(LTRotation & rRot, WFireInfo & info)
{
    if (!m_pAmmoData->pProjectileFX || !(m_pAmmoData->pProjectileFX->szClass[0])) return LTFALSE;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation = rRot;
	theStruct.m_Pos = info.vFirePos;

    HCLASS hClass = g_pLTServer->GetClass(m_pAmmoData->pProjectileFX->szClass);

	if (hClass)
	{
        CProjectile* pProj = (CProjectile*)g_pLTServer->CreateObject(hClass, &theStruct);
		if (pProj)
		{
			pProj->Setup(this, info);
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
        g_pLTServer->SetModelLooping(m_hModelObject, LTFALSE);
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
    ReloadClip(LTFALSE);
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

void CWeapon::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hObject);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hModelObject);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fLifeTime);
    g_pLTServer->WriteToMessageFloat(hWrite, float(m_nCurTracer));
    g_pLTServer->WriteToMessageByte(hWrite, m_bHave);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)m_nAmmoInClip);
    g_pLTServer->WriteToMessageByte(hWrite, m_eState);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamageFactor);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRangeAdjust);
    g_pLTServer->WriteToMessageByte(hWrite, m_nWeaponId);
    g_pLTServer->WriteToMessageByte(hWrite, m_nAmmoId);

	for (int i=0; i < W_MAX_MODS; i++)
	{
        m_Mods[i].WriteToMessage(g_pLTServer, hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hModelObject);

    m_fLifeTime         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_nCurTracer        = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    m_bHave             = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_nAmmoInClip       = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    m_eState            = (WeaponState) g_pLTServer->ReadFromMessageByte(hRead);
    m_fDamageFactor     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fRangeAdjust      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_nWeaponId         = g_pLTServer->ReadFromMessageByte(hRead);
    m_nAmmoId           = g_pLTServer->ReadFromMessageByte(hRead);

	for (int i=0; i < W_MAX_MODS; i++)
	{
        m_Mods[i].ReadFromMessage(g_pLTServer, hRead);
	}

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
	fDamage *= AIDifficultyAdjust();

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
	fDamage *= AIDifficultyAdjust();

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AIDifficultyAdjust()
//
//	PURPOSE:	Adjust AI damage based on difficulty
//
// ----------------------------------------------------------------------- //

LTFLOAT CWeapon::AIDifficultyAdjust() const
{
	if (!IsAI(m_hObject)) return 1.0f;

    LTFLOAT fDifficultyFactor = 1.0f;

	// If we're an AI damage is based on the current difficutly setting...

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_EASY:
			fDifficultyFactor = 0.25f;
		break;

		case GD_NORMAL:
			fDifficultyFactor = 0.50f;
		break;

		case GD_VERYHARD:
			fDifficultyFactor = 1.5f;
		break;

		case GD_HARD:
		default :
		break;
	}

	return fDifficultyFactor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CacheFiles
//
//	PURPOSE:	Cache files used by weapon.
//
// ----------------------------------------------------------------------- //

void CWeapon::CacheFiles()
{
	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (pWeaponData)
	{
		pWeaponData->Cache(g_pWeaponMgr);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddMod
//
//	PURPOSE:	Add the specified mod to the weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapon::AddMod(CModData* pMod)
{
    if (!pMod) return LTFALSE;

	MOD* pM = g_pWeaponMgr->GetMod(pMod->m_nID);
    if (!pM) return LTFALSE;

    LTBOOL bRet = LTFALSE;

	for (int i=0; i < W_MAX_MODS; i++)
	{
		if (m_Mods[i].m_nID == WMGR_INVALID_ID)
		{
			m_Mods[i] = *pMod;

            return LTTRUE;
		}
	}

    return LTFALSE;
}