// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.cpp
//
// PURPOSE : Weapon class - implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "weapon.h"
#include "RiotObjectUtilities.h"
#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "RiotMsgIds.h"
#include "PVWeaponModel.h"
#include "BaseCharacter.h"
#include "WeaponFXTypes.h"
#include "RiotServerShell.h"
#include "PlayerObj.h"

extern DBOOL g_bInfiniteAmmo;
extern CRiotServerShell* g_pRiotServerShellDE;

DBYTE g_nIgnoreFX = 0;
DBYTE g_nRandomWeaponSeed = 255;

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
	m_bHave				= DFALSE;
	m_eDamageType		= DT_PUNCTURE;
	m_bInfiniteAmmo		= DFALSE;
	m_nAmmo				= 0;
	m_bCanLockOnTarget	= DFALSE;
	m_nAmmoInClip		= 1;
	m_fZoomDamageMult	= 1.0f;
	m_fDamageFactor		= 1.0f;
	m_fRangeAdjust		= 1.0f;

	m_fLifeTime			= -1.0f;  // Uses GetWeaponLifeTime() if < 0

	VEC_INIT(m_vLastFirePath);

	m_nId					= (RiotWeaponId)0;

	m_hModelObject			= DNULL;

	m_pHandWeaponName		= "Models\\Powerups\\AssaultRifle.abc";
	m_pHandWeaponSkin		= "Skins\\Powerups\\AssaultRifle_a.dtx";

	m_fLastIdleTime			= 0.0f;
	m_fTimeBetweenIdles		= WEAPON_MIN_IDLE_TIME;
	
	m_fMinFireDuration		= 0.5f;
	m_fMaxFireDuration		= 2.0f;
	m_fMinFireRest			= 1.0f;
	m_fMaxFireRest			= 5.0f;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nIdleAni1				= INVALID_ANI;
	m_nIdleAni2				= INVALID_ANI;
	m_nFireAni				= INVALID_ANI;
	m_nFireAni2				= INVALID_ANI;
	m_nFireZoomAni			= INVALID_ANI;
	m_nLastFireAni			= INVALID_ANI;
	m_nStartFireAni			= INVALID_ANI;
	m_nStopFireAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_bFire					= DFALSE;
	m_bIsZoomed				= DFALSE;
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

DBOOL CWeapon::Init(HOBJECT hObj, ModelSize eSize)
{ 
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObj) return DFALSE;

	m_hObject	 = hObj; 
	m_eModelSize = eSize;

	CacheFiles( );

	return DTRUE; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddAmmo
//
//	PURPOSE:	Update weapon ammo
//
// ----------------------------------------------------------------------- //

void CWeapon::AddAmmo(int nOffset)
{
	m_nAmmo += nOffset;

	if (m_nAmmo > GetMaxAmmo())
	{
		m_nAmmo = GetMaxAmmo();
	}

	if (m_nAmmo < 0)
	{
		m_nAmmo = 0;
	}

	int nShotsPerClip = GetShotsPerClip(m_nId);

	m_nAmmoInClip = m_nAmmo < nShotsPerClip ? m_nAmmo : 
				    (nShotsPerClip > 0 ? nShotsPerClip : m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetAmmo
//
//	PURPOSE:	Update weapon ammo
//
// ----------------------------------------------------------------------- //

void CWeapon::SetAmmo(int nAmount)
{
	m_nAmmo = nAmount;

	if (m_nAmmo > GetMaxAmmo())
	{
		m_nAmmo = GetMaxAmmo();
	}

	if (m_nAmmo < 0)
	{
		m_nAmmo = 0;
	}

	int nShotsPerClip = GetShotsPerClip(m_nId);

	m_nAmmoInClip = m_nAmmo < nShotsPerClip ? m_nAmmo : 
					(nShotsPerClip > 0 ? nShotsPerClip : m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateFiring
//
//	PURPOSE:	Update the firing state of the gun...
//
// ----------------------------------------------------------------------- //

void CWeapon::UpdateFiring()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	switch (m_eState)
	{
		case W_IDLE :
		case W_END_FIRING :
		case W_BEGIN_FIRING :
		{
			m_eState = W_BEGIN_FIRING;

			if (!PlayStartFireAnimation())
			{
				PlayFireAnimation();
				m_eState = W_FIRING;
			}
		}
		break;

		case W_FIRING_NOAMMO :
		case W_FIRING :
		{
			PlayFireAnimation();
		}
		break;

		case W_RELOADING :
		{
			if (!PlayReloadAnimation())
			{
				PlayFireAnimation();
				m_eState = W_FIRING;
			}
		}
		break;

		case W_SELECT:
		{
			if (!PlaySelectAnimation())
			{
				PlayStartFireAnimation();
				m_eState = W_BEGIN_FIRING;
			}
		}
		break;

		case W_DESELECT:
		{
			if (!PlayDeselectAnimation())
			{
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		default : break;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing state of the gun
//
// ----------------------------------------------------------------------- //

void CWeapon::UpdateNonFiring()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	switch (m_eState)
	{
		case W_IDLE :
		{
			PlayIdleAnimation();
		}
		break;

		case W_FIRING :
		case W_FIRING_NOAMMO :
		case W_END_FIRING :
		case W_BEGIN_FIRING :
		{
			m_eState = W_END_FIRING;

			if (!PlayStopFireAnimation())
			{
				// Don't idle right away after firing...
				m_fLastIdleTime = pServerDE->GetTime();

				PlayIdleAnimation();			
				m_eState = W_IDLE;
			}
		}
		break;

		case W_RELOADING :
		{
			if (!PlayReloadAnimation())
			{
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		case W_SELECT:
		{
			if (!PlaySelectAnimation())
			{
				PlayIdleAnimation();			
				m_eState = W_IDLE;
			}
		}
		break;

		case W_DESELECT:
		{
			if (!PlayDeselectAnimation())
			{
				PlayIdleAnimation();
				m_eState = W_IDLE;
			}
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateWeapon
//
//	PURPOSE:	Update the weapon (fire if bFire == DTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::UpdateWeapon(HOBJECT hFiredFrom, DVector vPath, 
								  DVector vFirePos, DBOOL bFire, DBOOL bZoomed)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return W_IDLE;

	WeaponState eRet = W_IDLE;

	// Can't idle when zoomed...

	m_bIsZoomed = bZoomed;

	
	// Determine what we should be doing...

	if (bFire) UpdateFiring();
	else UpdateNonFiring();


	if (m_bFire) 
	{
		eRet = Fire(hFiredFrom, vPath, vFirePos, GetRandom(2,255));
	}

#ifdef DEBUGING_WEAPON
	pServerDE->CPrint("Weapon State = %d", m_eState);
#endif

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::Fire(HOBJECT hFiredFrom, DVector vPath, 
						  DVector vFirePos, DBYTE nRandomSeed)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hFiredFrom) return W_IDLE;

	WeaponState eRet = W_IDLE;

	g_nRandomWeaponSeed = nRandomSeed;


	// Make sure we always have ammo if we should...

	m_nAmmo = (m_bInfiniteAmmo || g_bInfiniteAmmo) ? 1000 : m_nAmmo;


	if (m_nAmmo > 0 || m_bInfiniteAmmo)
	{
		// Don't ignore anything...yet.

		g_nIgnoreFX = 0;

		// If a player fired the shot, see if the player has the 
		// silencer powerup...if so, don't play fire sound

		if (IsPlayer(hFiredFrom))
		{
			CBaseCharacter* pPlayer = (CBaseCharacter*)pServerDE->HandleToObject(hFiredFrom);
			if (pPlayer && pPlayer->HaveTimedPowerup (PIT_ULTRA_SILENCER))
			{
				g_nIgnoreFX = WFX_FIRESOUND; 
			}

			// If the player fired this and it is the appropriate weapon type,
			// don't worry about playing the fire sound (the player already played it)...

			ProjectileType nType = GetWeaponType(m_nId);
			if (nType == PROJECTILE || nType == MELEE) 
			{
				g_nIgnoreFX = WFX_FIRESOUND;
			}
		}


		int nShotsPerClip = GetShotsPerClip(m_nId);

		if (m_nAmmoInClip > 0)
		{
			DRotation rRot;
			DVector vU, vR, vF;

			pServerDE->GetObjectRotation(hFiredFrom, &rRot);
			pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			// Create a projectile for every vector...

			int nVectorsPerShot = GetVectorsPerShot(m_nId);

			for (int i=0; i < nVectorsPerShot; i++)
			{
				VEC_COPY(m_vLastFirePath, vPath); 
				
				DVector vPos;
				VEC_COPY(vPos, vFirePos); 

				srand(g_nRandomWeaponSeed);
				g_nRandomWeaponSeed = GetRandom(2, 255);

				CalculateWeaponPathAndFirePos(m_nId, m_vLastFirePath, vPos, vU, vR);


				//pServerDE->CPrint("Server Fire Path (%d): %.2f, %.2f, %.2f", g_nRandomWeaponSeed,
				//				   m_vLastFirePath.x, m_vLastFirePath.y, m_vLastFirePath.z);

				// Create the projectile...

				ObjectCreateStruct theStruct;
				INIT_OBJECTCREATESTRUCT(theStruct);

				ROT_COPY(theStruct.m_Rotation, rRot);
				VEC_COPY(theStruct.m_Pos, vPos);

				CProjectile* pBullet = CreateProjectile(theStruct);
				if (!pBullet) return W_IDLE;

				pBullet->Setup(this, hFiredFrom);

				// If we are shooting multiple vectors ignore some special
				// fx after the first vector...

				g_nIgnoreFX |= WFX_FIRESOUND | WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
			}

			if (nShotsPerClip > 0) 
			{
				m_nAmmoInClip--;
			}

			m_nAmmo--;
		}


		eRet = W_FIRED;


		// See if we're playing the 2nd fire animation...

		if (m_hModelObject)
		{
			DDWORD dwAni = pServerDE->GetModelAnimation(m_hModelObject);
			if (dwAni == m_nFireAni2 && dwAni != INVALID_ANI)
			{
				eRet = W_FIRED2;
			}
		}

		
		// Check to see if we need to reload...

		if (nShotsPerClip > 0)
		{
			if (m_nAmmoInClip <= 0 && m_nAmmo > 0) 
			{
				m_eState = W_RELOADING;
				m_nAmmoInClip = m_nAmmo < nShotsPerClip ? m_nAmmo : nShotsPerClip;
			}
		}
	} 
	else  // NO AMMO
	{
		m_eState = W_FIRING_NOAMMO;
	}

	m_bFire = DFALSE;
	
	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateProjectile
//
//	PURPOSE:	Create the approprite projectile to fire.
//
// ----------------------------------------------------------------------- //

CProjectile* CWeapon::CreateProjectile(ObjectCreateStruct & theStruct)
{
	LPBASECLASS pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CProjectile");

		if (hClass)
		{
			pRet = pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return (CProjectile*)pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetHandModelFlashOffset
//
//	PURPOSE:	Get the flash offset from the hand-held model origin
//
// ----------------------------------------------------------------------- //

DVector	CWeapon::GetHandModelFlashOffset()
{
	DVector vOffset;
	VEC_INIT(vOffset);

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return vOffset;

	if (m_hModelObject)
	{
		CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hModelObject);
		if (pModel)
		{
			VEC_COPY(vOffset, pModel->GetFlashOffset());
		}
	}

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlaySelectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlaySelectAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_nSelectAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if (dwAni == m_nSelectAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nSelectAni)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nSelectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayDeselectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayDeselectAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_nDeselectAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if (dwAni == m_nDeselectAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nDeselectAni)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nDeselectAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayStartFireAnimation()
//
//	PURPOSE:	Set model to starting firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayStartFireAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_nStartFireAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if (dwAni == m_nStartFireAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;  
	}
	else if (dwAni != m_nStartFireAni)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nStartFireAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayStopFireAnimation()
//
//	PURPOSE:	Set model to the stop firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayStopFireAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_nStopFireAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if (dwAni == m_nStopFireAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != m_nStopFireAni)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nStopFireAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayFireAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if ((dwAni != m_nFireAni && dwAni != m_nFireAni2 && dwAni != m_nFireZoomAni) || (dwState & MS_PLAYDONE))
	{
		if (m_bIsZoomed)
		{
			m_nLastFireAni = m_nFireZoomAni;
		}
		else  // Normal firing
		{
			if (m_nFireAni2 != INVALID_ANI)
			{
				m_nLastFireAni = (m_nLastFireAni == m_nFireAni ? m_nFireAni2 : m_nFireAni);
			}
			else
			{
				m_nLastFireAni = m_nFireAni;
			}
		}

		pServerDE->SetModelAnimation(m_hModelObject, m_nLastFireAni);
		pServerDE->ResetModelAnimation(m_hModelObject);  // Start from beginning
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayReloadAnimation()
//
//	PURPOSE:	Set model to reloading animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayReloadAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_nReloadAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hModelObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hModelObject);

	if (dwAni == m_nReloadAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != m_nReloadAni && (dwState & MS_PLAYDONE))
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nReloadAni);
	}

	return DTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayIdleAnimation()
//
//	PURPOSE:	Set model to Idle animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayIdleAnimation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject || m_bIsZoomed) return DFALSE;

	// Make sure idle animation is done if one is currently playing...

	DDWORD dwAni = pServerDE->GetModelAnimation(m_hModelObject);
	if (dwAni == m_nIdleAni1 || dwAni == m_nIdleAni2)
	{ 
		if (!(pServerDE->GetModelPlaybackState(m_hModelObject) & MS_PLAYDONE))
		{
			return DTRUE;
		}
	}


	DFLOAT fTime = pServerDE->GetTime();

	// Play idle if it is time...

	if ((fTime > m_fLastIdleTime + m_fTimeBetweenIdles))
	{
		m_fLastIdleTime		= fTime;
		m_fTimeBetweenIdles	= GetRandom(WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME);

		DDWORD nAni = m_nIdleAni1;
		if (m_nIdleAni2 != INVALID_ANI)
		{
			nAni = (GetRandom(0,1)==0 ? m_nIdleAni1 : m_nIdleAni2);
		}

		if (nAni == INVALID_ANI) nAni = 0;

		pServerDE->SetModelAnimation(m_hModelObject, nAni);

		return DTRUE;
	}

	return DFALSE;
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
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (!pServerDE) return;

		pServerDE->SetModelLooping(m_hModelObject, DFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::InitAnimations
//
//	PURPOSE:	Set the animations
//
// ----------------------------------------------------------------------- //

void CWeapon::InitAnimations()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hModelObject) return;

	m_nSelectAni	= pServerDE->GetAnimIndex(m_hModelObject, "Select");
	m_nDeselectAni	= pServerDE->GetAnimIndex(m_hModelObject, "Deselect");
	m_nStartFireAni	= pServerDE->GetAnimIndex(m_hModelObject, "Start_fire");
	m_nFireAni		= pServerDE->GetAnimIndex(m_hModelObject, "Fire");
	m_nFireAni2		= pServerDE->GetAnimIndex(m_hModelObject, "Fire2");
	m_nFireZoomAni	= pServerDE->GetAnimIndex(m_hModelObject, "Fire_zoom");
	m_nStopFireAni	= pServerDE->GetAnimIndex(m_hModelObject, "End_fire");
	m_nIdleAni1		= pServerDE->GetAnimIndex(m_hModelObject, "Idle_1");
	m_nIdleAni2		= pServerDE->GetAnimIndex(m_hModelObject, "Idle_2");
	m_nReloadAni	= pServerDE->GetAnimIndex(m_hModelObject, "Reload");
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = W_SELECT;

	if (m_hModelObject && m_nSelectAni != INVALID_ANI)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nSelectAni);
		pServerDE->ResetModelAnimation(m_hModelObject);
	}

	m_fLastIdleTime	= pServerDE->GetTime();
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bIsZoomed = DFALSE;
	m_eState	= W_DESELECT;

	if (m_hModelObject && m_nDeselectAni != INVALID_ANI)
	{
		pServerDE->SetModelAnimation(m_hModelObject, m_nDeselectAni);
		pServerDE->ResetModelAnimation(m_hModelObject);
	}

	m_fLastIdleTime	= pServerDE->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::HandleStateChange()
//
//	PURPOSE:	Handle the weapon state changing
//
// ----------------------------------------------------------------------- //

void CWeapon::HandleStateChange(HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = (WeaponState) pServerDE->ReadFromMessageByte(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	DVector vOldLastModelPos, vOldLastFirePos;
	vOldLastModelPos.Init();
	vOldLastFirePos.Init();

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hModelObject);
	pServerDE->WriteToMessageDWord(hWrite, 0);
	pServerDE->WriteToMessageDWord(hWrite, m_nSelectAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nDeselectAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nStartFireAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nFireAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nFireAni2);
	pServerDE->WriteToMessageDWord(hWrite, m_nFireZoomAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nLastFireAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nStopFireAni);
	pServerDE->WriteToMessageDWord(hWrite, m_nIdleAni1);
	pServerDE->WriteToMessageDWord(hWrite, m_nIdleAni2);
	pServerDE->WriteToMessageDWord(hWrite, m_nReloadAni);
	pServerDE->WriteToMessageByte(hWrite, m_eModelSize);
	pServerDE->WriteToMessageByte(hWrite, 0);
	pServerDE->WriteToMessageFloat(hWrite, m_fMinFireDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxFireDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fMinFireRest);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxFireRest);
	pServerDE->WriteToMessageByte(hWrite, m_bInfiniteAmmo);
	pServerDE->WriteToMessageByte(hWrite, m_bHave);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nAmmo);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nAmmoInClip);
	pServerDE->WriteToMessageByte(hWrite, m_bIsZoomed);
	pServerDE->WriteToMessageByte(hWrite, m_bFire);
	pServerDE->WriteToMessageByte(hWrite, m_eState);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastIdleTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fTimeBetweenIdles);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageFactor);
	pServerDE->WriteToMessageFloat(hWrite, m_fRangeAdjust);
	pServerDE->WriteToMessageByte(hWrite, m_bCanLockOnTarget);
	pServerDE->WriteToMessageVector(hWrite, &vOldLastFirePos);
	pServerDE->WriteToMessageVector(hWrite, &vOldLastModelPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastFirePath);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	DVector vOldLastModelPos, vOldLastFirePos;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hModelObject);

	DDWORD nRestAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nSelectAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nDeselectAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nStartFireAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nFireAni			= pServerDE->ReadFromMessageDWord(hRead);
	m_nFireAni2			= pServerDE->ReadFromMessageDWord(hRead);
	m_nFireZoomAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nLastFireAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nStopFireAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_nIdleAni1			= pServerDE->ReadFromMessageDWord(hRead);
	m_nIdleAni2			= pServerDE->ReadFromMessageDWord(hRead);
	m_nReloadAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_eModelSize		= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	DBOOL bCanIdle		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fMinFireDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxFireDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_fMinFireRest		= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxFireRest		= pServerDE->ReadFromMessageFloat(hRead);
	m_bInfiniteAmmo		= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_bHave				= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_nAmmo				= (int)pServerDE->ReadFromMessageFloat(hRead);
	m_nAmmoInClip		= (int)pServerDE->ReadFromMessageFloat(hRead);
	m_bIsZoomed			= pServerDE->ReadFromMessageByte(hRead);
	m_bFire				= pServerDE->ReadFromMessageByte(hRead);
	m_eState			= (WeaponState) pServerDE->ReadFromMessageByte(hRead);
	m_fLastIdleTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fTimeBetweenIdles = pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageFactor		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRangeAdjust		= pServerDE->ReadFromMessageFloat(hRead);
	m_bCanLockOnTarget	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &vOldLastFirePos);
	pServerDE->ReadFromMessageVector(hRead, &vOldLastModelPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastFirePath);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetDamage()
//
//	PURPOSE:	Get the damage done by this weapon (value can vary)
//
// ----------------------------------------------------------------------- //

DFLOAT CWeapon::GetDamage() const
{
	DFLOAT fDamage = GetWeaponDamage(m_nId) * GetRandom(0.8f, 1.2f) * m_fDamageFactor;

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return fDamage;

	// If we're an AI damage is based on the current difficutly setting...

	if (IsAI(m_hObject) && g_pRiotServerShellDE)
	{
		DFLOAT fDifficultyFactor = 1.0f;

		switch (g_pRiotServerShellDE->GetDifficulty())
		{
			case GD_EASY:
				fDifficultyFactor = 0.5f;
			break;

			case GD_NORMAL:
				fDifficultyFactor = 0.75f;
			break;

			case GD_VERYHARD:
				fDifficultyFactor = 1.5f;
			break;

			case GD_HARD:
			default : 
			break;
		}

		fDamage *= fDifficultyFactor;
	}

	return (m_bIsZoomed ? fDamage * m_fZoomDamageMult : fDamage);
}

char *g_szPulzeRifleSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "projectile.wav", "reload.wav", "reload2.wav", "select.wav", 0 };
char *g_szSpiderSounds[] = 
{ "empty.wav", "fire.WAV", "GrenadeTick.wav", "impact.wav", "select.wav", "thud.wav", "timer.wav", 0 };
char *g_szBullGutSounds[] = 
{ "empty.wav", "fire.WAV", "impact.wav", "projectile.wav", "select.wav", 0 };
char *g_szSniperRifleSounds[] = 
{ "empty.wav", "fire.WAV", "select.wav", "zoomin.wav", "zoomout.wav", 0 };
char *g_szJuggernautSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "reload1.WAV", "reload2.wav", "select.wav", 0 };
char *g_szShredderSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "loop.wav", "reload.WAV", "select.wav", 0 };
char *g_szRedRiotSounds[] = 
{ "charge.wav", "deselect.wav", "empty.wav", "fire.wav", "idle.wav", "impact.wav", "projectile.wav", "select.wav", "spin.WAV", 0 };
char *g_szEnergyBatonSounds[] = 
{ "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szEnergyBladeSounds[] = 
{ "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szKatanaSounds[] = 
{ "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szMonoKnifeSounds[] = 
{ "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szTantoSounds[] = 
{ "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szColt45Sounds[] = 
{ "empty.wav", "fire.wav", "fire2.wav", "reload.wav", "select1.wav", "select2.wav", 0 };
char *g_szShotGunSounds[] = 
{ "fire.wav", "select.wav", 0 };
char *g_szAssaultRifleSounds[] = 
{ "empty.wav", "fire.wav", "reload.wav", "select.wav", "zoomin.wav", "zoomout.wav", 0 };
char *g_szMac10Sounds[] = 
{ "empty.wav", "fire.wav", "select.wav", 0 };
char *g_szEnergyGrenadeSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "projectile.wav", "select.wav", 0 };
char *g_szKatoGrenadeSounds[] = 
{ "bounce1.wav", "bounce2.wav", "empty.wav", "fire.wav", "impact.wav", "projectile.wav", "select.wav", 0 };
char *g_szTowSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "projectile.wav", "select.wav", "select2.wav", 0 };
char *g_szLaserCannonSounds[] = 
{ "empty.wav", "fire.wav", "impact.wav", "select.wav", 0 };
char *g_szSqueakyToySounds[] = 
{ "fire.wav", "idle.WAV", "idle1.WAV", "idle2.WAV", "idle3.WAV", "idle4.WAV", "select.wav", 0 };


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CacheFiles
//
//	PURPOSE:	Cache files used by weapon.
//
// ----------------------------------------------------------------------- //

void CWeapon::CacheFiles( )
{
	char szFile[_MAX_PATH];
	char **pSoundArray;
	int i;

	// Too late to cache files?
	if( !( g_pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	g_pServerDE->CacheFile( FT_MODEL, GetPVModelName(m_nId) );
	g_pServerDE->CacheFile( FT_MODEL, m_pHandWeaponName );
	g_pServerDE->CacheFile( FT_TEXTURE, GetPVModelSkin(m_nId) );
	g_pServerDE->CacheFile( FT_TEXTURE, m_pHandWeaponSkin );

	// Cache sounds...
	switch( m_nId )
	{
		case GUN_PULSERIFLE_ID :
			pSoundArray = g_szPulzeRifleSounds;
		break;

		case GUN_SPIDER_ID :
			pSoundArray = g_szSpiderSounds;
		break;

		case GUN_BULLGUT_ID :
			pSoundArray = g_szBullGutSounds;
		break;

		case GUN_SNIPERRIFLE_ID :
			pSoundArray = g_szSniperRifleSounds;
		break;

		case GUN_JUGGERNAUT_ID :
			pSoundArray = g_szJuggernautSounds;
		break;

		case GUN_SHREDDER_ID :
			pSoundArray = g_szShredderSounds;
		break;

		case GUN_REDRIOT_ID :
			pSoundArray = g_szRedRiotSounds;
		break;

		case GUN_ENERGYBATON_ID :
			pSoundArray = g_szEnergyBatonSounds;
		break;

		case GUN_ENERGYBLADE_ID :
			pSoundArray = g_szEnergyBladeSounds;
		break;

		case GUN_KATANA_ID :
			pSoundArray = g_szKatanaSounds;
		break;

		case GUN_MONOKNIFE_ID :
			pSoundArray = g_szMonoKnifeSounds;
		break;

		case GUN_COLT45_ID :
			pSoundArray = g_szColt45Sounds;
		break;
		
		case GUN_SHOTGUN_ID	:
			pSoundArray = g_szShotGunSounds;
		break;
		
		case GUN_ASSAULTRIFLE_ID :
			pSoundArray = g_szAssaultRifleSounds;
		break;
		
		case GUN_MAC10_ID :
			pSoundArray = g_szMac10Sounds;
		break;
	
		case GUN_TANTO_ID :
			pSoundArray = g_szTantoSounds;
		break;

		case GUN_ENERGYGRENADE_ID :
			pSoundArray = g_szEnergyGrenadeSounds;
		break;

		case GUN_KATOGRENADE_ID :
			pSoundArray = g_szKatoGrenadeSounds;
		break;

		case GUN_TOW_ID	:
			pSoundArray = g_szTowSounds;
		break;

		case GUN_LASERCANNON_ID :
			pSoundArray = g_szLaserCannonSounds;
		break;

		case GUN_SQUEAKYTOY_ID :
			pSoundArray = g_szSqueakyToySounds;
		break;

		default : 
			return;
			break;
	}

	if( GetWeaponSoundDir( m_nId ))
	{
		i = 0;
		while( pSoundArray[i] )
		{
			sprintf( szFile, "%s%s", s_FileBuffer, pSoundArray[i] );
			g_pServerDE->CacheFile( FT_SOUND, szFile );
			i++;
		}
	}
}
