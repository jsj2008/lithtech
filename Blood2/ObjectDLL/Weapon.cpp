//----------------------------------------------------------
//
// MODULE  : Weapon.cpp
//
// PURPOSE : General purpose weapon parent
//
// CREATED : 9/20/97
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include "InventoryMgr.h"
#include "BaseCharacter.h"
#include "generic_msg_de.h"
#include "GameProjectiles.h"
#include "PlayerObj.h"
#include "SFXMsgIds.h"
#include "ViewWeaponModel.h"
#include "ClientSparksSFX.h"
#include "ClientMarkSFX.h"
#include "Impacts.h"
//#include "ShellCasing.h"
#include "ObjectUtilities.h"
#include "VolumeBrushTypes.h"
#include "ClientSplashSFX.h"
#include "ClientExplosionSFX.h"
#include "corpse.h"
#include "ai_mgr.h"
#include "DestructableBrush.h"
#include "WeaponDefs.h"
#include "Door.h"
#include "BloodServerShell.h"

void BPrint(char*);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CWeapon()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CWeapon::CWeapon(DBYTE dbWeapType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	m_hOwner				= DNULL;
	m_hClient				= DNULL;
	m_pInventoryMgr			= DNULL;
	m_bInitialized			= DFALSE;
	m_bClientNotified		= DFALSE;
	m_nType					= dbWeapType;
	m_eState				= WS_HOLSTERED;

	m_pViewModel			= DNULL;

	WeaponData *wd			= &g_WeaponDefaults[m_nType-1];

	m_nFireType				= wd->m_nFireType;
	m_pViewModelFilename	= wd->m_szViewModelFilename;
	m_pLeftViewModelFilename = wd->m_szLeftViewModelFilename;
	m_pViewModelSkinname	= wd->m_szViewModelSkin;
	m_pHandModelFilename	= wd->m_szHandModelFilename;
	m_pHandModelSkinname	= wd->m_szHandModelSkin;
	m_nAmmoType				= wd->m_nAmmoType;
	m_nAmmoUse				= wd->m_nAmmoUse;
	m_nAltAmmoUse			= wd->m_nAltAmmoUse;
	m_fDamage				= 0;
	m_fMinDamage			= wd->m_fMinDamage;
	m_fMaxDamage			= wd->m_fMaxDamage;
	m_fMinAltDamage			= wd->m_fMinAltDamage;
	m_fMaxAltDamage			= wd->m_fMaxAltDamage;
	m_fReloadTime			= wd->m_fReloadTime;
	m_fAltReloadTime		= wd->m_fAltReloadTime;
	m_Spread				= wd->m_Spread;
	m_AltSpread				= wd->m_AltSpread;
	m_fProjVelocity			= wd->m_fProjVelocity;
	m_fAltProjVelocity		= wd->m_fAltProjVelocity;
	m_fRange				= wd->m_fRange;
	m_fAltRange				= wd->m_fAltRange;
	m_dwShotsPerFire		= wd->m_dwShotsPerFire;
	m_dwAltShotsPerFire		= wd->m_dwAltShotsPerFire;
	m_dwStrengthReq			= wd->m_dwStrengthReq;
	m_dwTwoHandStrengthReq	= wd->m_dwTwoHandStrengthReq;
	m_bDualHanded = (m_dwTwoHandStrengthReq > 0);
	m_nDamageRadius			= wd->m_nDamageRadius;
	m_nAltDamageRadius		= wd->m_nAltDamageRadius;
	m_bAltFireZoom			= wd->m_bAltFireZoom;
	m_bSemiAuto				= wd->m_bSemiAuto;

	m_szFireSound			= wd->m_szFireSound;
	m_szAltFireSound		= wd->m_szAltFireSound;
	m_szEmptyWeaponSound	= wd->m_szEmptyWeaponSound;
	m_szAltEmptyWeaponSound	= wd->m_szAltEmptyWeaponSound;
	m_szProjectileClass		= wd->m_szProjectileClass;
	m_szAltProjectileClass	= wd->m_szAltProjectileClass;
	m_szWeaponName			= wd->m_szWeaponName;
	m_nWeaponNameID			= wd->m_nWeaponNameID;
	m_szFlashSprite			= wd->m_szFlashSprite;
	m_szAltFlashSprite		= wd->m_szAltFlashSprite;
	m_fFlashDuration		= wd->m_fFlashDuration;
	m_fFlashScale			= wd->m_fFlashScale;
	VEC_COPY(m_vHandModelOffset, wd->m_vHandModelOffset)
	VEC_COPY(m_vViewModelOffset, wd->m_vViewModelOffset)
	VEC_COPY(m_vMuzzleOffset, wd->m_vMuzzleOffset)
	VEC_COPY(m_vRecoil, wd->m_vRecoil)
	VEC_COPY(m_vFlash, wd->m_vFlash)
	m_fEjectInterval		= wd->m_fEjectInterval;
	m_fViewKick				= wd->m_fViewKick;				// Amount to adjust view when firing.
	m_bCumulativeKick		= wd->m_bCumulativeKick;		// Kick is Cumulative
	m_bLoopAnim				= wd->m_bLoopAnim;
	m_bAltLoopAnim			= wd->m_bAltLoopAnim;
	m_bLoopStatic			= DFALSE;

	m_fLastShotTime			= 0;
	m_fFireTime				= 0;
	m_bLastFireAlt			= DFALSE;

	if (m_nAmmoType == AMMO_BULLET || m_nAmmoType == AMMO_SHELL)
	{
		m_bEjectShell = DTRUE;
	}
	else
	{
		m_bEjectShell = DFALSE;
	}

	m_fIdleStartTime		= 0.0f;
	m_fIdleDelay			= 0.0f;
	m_fMinIdleDelay			= 20.0f;
	m_fMaxIdleDelay			= 60.0f;

	// Create a muzzle flash
	m_hFlash				= DNULL;
	m_pViewModel			= DNULL;
	m_pHandModel			= DNULL;
	m_hHandAttachment		= DNULL;

	m_bFlashShowing			= DFALSE;

	m_nRestAnim				= INVALID_ANI;
	m_nIdleAnim				= INVALID_ANI;
	m_nDrawAnim				= INVALID_ANI;
	m_nDrawDuelAnim			= INVALID_ANI;
	m_nHolsterAnim			= INVALID_ANI;
	m_nHolsterDuelAnim		= INVALID_ANI;
	m_nStartFireAnim		= INVALID_ANI;
	m_nFireAnim				= INVALID_ANI;
	m_nStopFireAnim			= INVALID_ANI;
	m_nStartAltFireAnim		= INVALID_ANI;
	m_nAltFireAnim			= INVALID_ANI;
	m_nStopAltFireAnim		= INVALID_ANI;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;

	for (int i=0; i < MAX_FIRE_SOUNDS; i++)
	{
		m_hCurFireSounds[i] = DNULL;
		m_fCurFireSoundsEndTime[i] = 0.0f;
	}

	m_nUpdateWait = 10;

	m_nFlags				= DNULL;

	m_szPic					= DNULL;
	m_szPicH				= DNULL;

	m_bAccuracyCheck		= DTRUE;
	m_bMultiDamageBoost		= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::~CWeapon()
//
//	PURPOSE:	destructor
//
// ----------------------------------------------------------------------- //

CWeapon::~CWeapon()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Init()
//
//	PURPOSE:	Initializes the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Init(HOBJECT hOwner, CInventoryMgr* pInventoryMgr, DBOOL bLeftHand, HOBJECT hMuzzleFlash)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hOwner || !pInventoryMgr) return;

	m_hOwner = hOwner;
	m_pInventoryMgr = pInventoryMgr;
	m_bLeftHand = bLeftHand;

	if (m_bLeftHand)
	{
		m_vViewModelOffset.x = -m_vViewModelOffset.x;
		m_vFlash.x = -m_vFlash.x;
		m_vMuzzleOffset.x = -m_vMuzzleOffset.x;
	}

	m_hFlash = hMuzzleFlash;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetClient()
//
//	PURPOSE:	Sets the client for the weapon, and initializes a player view model.
//
// ----------------------------------------------------------------------- //

void CWeapon::SetClient(HCLIENT hClient, CViewWeaponModel *pViewModel)
{
	m_hClient = hClient;

	// Create the view weapon model if it's a Player object
	m_pViewModel = pViewModel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Term()
//
//	PURPOSE:	Terminates this weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Term()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (m_hHandAttachment)
	{
		pServerDE->RemoveAttachment(m_hHandAttachment);
		m_hHandAttachment = DNULL;
	}

	if (m_pHandModel)
	{
		pServerDE->RemoveObject(m_pHandModel->m_hObject);
		m_pHandModel->SetWeaponOwner( NULL );
		m_pHandModel = DNULL;
	}

	if( m_szPic )
	{
		pServerDE->FreeString( m_szPic );
		m_szPic = DNULL;
	}
	if( m_szPicH )
	{
		pServerDE->FreeString( m_szPicH );
		m_szPicH = DNULL;
	}

	KillSounds();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::IsIdle()
//
//	PURPOSE:	Terminates this weapon
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::IsChanging()
{
	if(IsOwnerAPlayer())
	{
		if((m_eState == WS_DRAW) || (m_eState == WS_HOLSTER))
			return	DTRUE;
	}
	return	DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Draw()
//
//	PURPOSE:	draws the weapon, sets filenames
//
// ----------------------------------------------------------------------- //

void CWeapon::Draw()
{ 
	m_eState = WS_DRAW; 
	
	SetupMuzzleFlash();

	if (m_hClient)
	{
		SetupViewModel();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CheckAmmo()
//
//	PURPOSE:	returns amount of ammo available to this weapon
//
// ----------------------------------------------------------------------- //

DFLOAT CWeapon::CheckAmmo(DBOOL bAltFire)
{
    // If the No Ammo is required for the Weapon then return 1
    if (GetAmmoType(bAltFire) == AMMO_NONE)
    {
        return 1.0f;
    }        

	if (m_pInventoryMgr)
		return m_pInventoryMgr->GetAmmoCount(GetAmmoType(bAltFire));
	else
		return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CheckDamageMultiplier()
//
//	PURPOSE:	returns the owner's damage multiplier
//
// ----------------------------------------------------------------------- //

DFLOAT CWeapon::GetDamageMultiplier()
{
	if (m_pInventoryMgr)
		return m_pInventoryMgr->GetDamageMultiplier();
	else
		return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ShowHandModel()
//
//	PURPOSE:	Creates and shows or destroys and hides the hand model.
//
// ----------------------------------------------------------------------- //

void CWeapon::ShowHandModel(DBOOL bShow)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (bShow)	// Create it.
	{
		if (!m_pHandModel)
		{
			CreateHandModel();
		}
	}
	else	// Remove it.
	{
		if (m_hHandAttachment)
		{
			pServerDE->RemoveAttachment(m_hHandAttachment);
			m_hHandAttachment = DNULL;
		}

		if (m_pHandModel && m_pHandModel->m_hObject)
		{
			pServerDE->RemoveObject(m_pHandModel->m_hObject);
			m_pHandModel->SetWeaponOwner( NULL );
			m_pHandModel = DNULL;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Update()
//
//	PURPOSE:	Updates this weapon's position
//
// ----------------------------------------------------------------------- //

void CWeapon::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hOwner) return;

	// This flag shows that the weapon has had an update, so it's now ok to 
	// send the view object to the client.
	if (!m_bInitialized)
	{
		m_bInitialized = DTRUE;
	}

	// Kill the sound if it's played long enough
	for(int i = 0; i < MAX_FIRE_SOUNDS; i++)
	{
		DBOOL bSoundDone = DFALSE;

		if (m_hCurFireSounds[i] && pServerDE->IsSoundDone(m_hCurFireSounds[i], &bSoundDone) == LT_OK && bSoundDone)
		{
			pServerDE->KillSound(m_hCurFireSounds[i]);
			m_hCurFireSounds[i] = DNULL;
		}	
	}

	// Update muzzle flash
	if (m_bFlashShowing)
	{
		UpdateMuzzleFlash();
	}

	// Set color to match the owner (stealth pu..)
	if (m_hOwner && m_pHandModel && m_pHandModel->m_hObject)
	{
		DVector vColor;
		DFLOAT fAlpha;

		pServerDE->GetObjectColor(m_hOwner, &vColor.x, &vColor.y, &vColor.z, &fAlpha);
		pServerDE->SetObjectColor(m_pHandModel->m_hObject, vColor.x, vColor.y, vColor.z, fAlpha);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayFireSound()
//
//	PURPOSE:	Plays the firing sound
//
// ----------------------------------------------------------------------- //

void CWeapon::PlayFireSound(DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	HSOUNDDE hSound = DNULL;

	char *sound;
	DBOOL bLoop = DFALSE;

	if (bAltFire)
	{
		sound = m_szAltFireSound;
	}
	else
	{
		sound = m_szFireSound;
	}

	if (sound && _mbstrlen(sound) > 0)
	{
		DFLOAT Radius = 2000.0f;
		DBYTE sndtype = IsOwnerAPlayer() ? SOUNDPRIORITYBASE_PLAYER : SOUNDPRIORITYBASE_AI;

		hSound = PlaySoundFromObject(m_hOwner, sound, Radius, sndtype + SOUNDPRIORITYMOD_HIGH, bLoop, DTRUE, DTRUE);

		SendSoundTrigger(m_hOwner, SOUND_GUNFIRE, m_vPosition, Radius);
	}

	int		soundToKill = 0;

	if (hSound)
	{
		// See if there is an available sound slot, if not, kill the sound with the least time remaining.

		int nIndex = -1;
		for (int i=0; i < MAX_FIRE_SOUNDS; i++)
		{
			if (!m_hCurFireSounds[i]) 
			{
				nIndex = i;
				break;	// A match!
			}
			else if (nIndex == -1 || m_fCurFireSoundsEndTime[i] < m_fCurFireSoundsEndTime[nIndex])
			{
				nIndex = i;
			}
		}

		if (nIndex != -1)	// This should ALWAYS be the case.
		{
			DFLOAT fTime;
			pServerDE->GetSoundDuration(hSound, &fTime);
			fTime += pServerDE->GetTime();

			if (m_hCurFireSounds[nIndex])
				pServerDE->KillSound(m_hCurFireSounds[nIndex]);

			m_hCurFireSounds[nIndex] = hSound;
			m_fCurFireSoundsEndTime[nIndex] = fTime;

		}
		else
		{
			pServerDE->KillSound(hSound);
			hSound = DNULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayEmptyWeaponSound()
//
//	PURPOSE:	Plays the firing sound
//
// ----------------------------------------------------------------------- //

void CWeapon::PlayEmptyWeaponSound(DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	char *sound;
	if (bAltFire)
		sound = m_szAltEmptyWeaponSound;
	else
		sound = m_szEmptyWeaponSound;

//	if (pWeap->m_nAmmo > 0) sound = m_FireSound;

	if (_mbstrlen(sound) > 0)
	{
		DFLOAT Radius = 800.0f;
		DBYTE sndtype = IsOwnerAPlayer() ? SOUNDPRIORITYBASE_PLAYER : SOUNDPRIORITYBASE_AI;

		PlaySoundFromObject(m_hOwner, sound, Radius, sndtype + SOUNDPRIORITYMOD_HIGH, DFALSE, 
			DFALSE, DFALSE, 100, DFALSE, DTRUE );
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayEmptyWeaponSound()
//
//	PURPOSE:	Kills all of the currently playing weapon sounds
//
// ----------------------------------------------------------------------- //

void CWeapon::KillSounds()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	for (int i=0; i < 4; i++)
	{
		if (m_hCurFireSounds[i])
		{
			pServerDE->KillSound(m_hCurFireSounds[i]);
			m_hCurFireSounds[i] = DNULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateWeaponFX()
//
//	PURPOSE:	Update the weapon FX variable according to weapon, surfacetype, and ammo type
//
// ----------------------------------------------------------------------- //

void CWeapon::UpdateWeaponFX(DDWORD &nFX, DDWORD &nExtras, WeaponFXExtras *ext, SurfaceType eType, DBYTE nAmmoType, DFLOAT fDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;

	ext->nAmmo = nAmmoType;
	ext->nSurface = (DBYTE)eType;
	nExtras |= WFX_EXTRA_SURFACETYPE | WFX_EXTRA_AMMOTYPE;

	// Check surface type flesh to override the sparks (blood) and blood streams
	if(eType == SURFTYPE_FLESH)
	{
		nFX |= WFX_SPARKS;

		if(!pServerDE->IntRandom(0,3) || fDamage > 25.0f)
			nFX |= WFX_BLOODSPURT;
	}

	if(m_nType == WEAP_MELEE)
		nFX |= WFX_SOUND | WFX_SPARKS;

	// Check the FX against the ammo type
	if(nAmmoType == AMMO_BULLET || nAmmoType == AMMO_SHELL || nAmmoType == AMMO_BMG)
	{
		nFX |= WFX_MARK | WFX_SOUND;

		if(m_nType != WEAP_MINIGUN)
			nFX |= WFX_MUZZLESMOKE;

		if(eType == SURFTYPE_WOOD || eType == SURFTYPE_STONE)
		{
			nFX |= WFX_SMOKE | WFX_FLASH;
			if(m_nType != WEAP_MINIGUN)
				nFX |= WFX_FRAGMENTS;
		}

		if(eType == SURFTYPE_METAL)
			nFX |= WFX_SPARKS;

		if(eType == SURFTYPE_LIQUID)
			nFX |= WFX_SPLASH;
	}

	// Add in the extras flags for spark messages
	if(nFX & WFX_SPARKS)
	{
		nExtras |= WFX_EXTRA_DAMAGE;
		ext->fDamage = m_fDamage;
	}

	// Turn on or off the effect that emit from the weapon itself
	if(m_bEjectShell)
	{
		// Should the ejected shell go the other direction?
		if(m_bLeftHand)
			nFX |= WFX_LEFTHANDED;

		nFX |= WFX_EJECTSHELL;
		m_bEjectShell = DFALSE;
	}
	else
	{
		nFX &= ~WFX_MUZZLESMOKE;
		nFX &= ~WFX_MUZZLELIGHT;
		nFX &= ~WFX_EJECTSHELL;
		nFX &= ~WFX_TRACER;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::FireVector()
//
//	PURPOSE:	Fires a vector for vector weapons
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vFrom, vTo, vDist;
	DDWORD			nFX = 0;
	DDWORD			nExtras = 0;
	WeaponFXExtras	ext;
	int				nNode = 999;

	if(bTracer) nFX |= WFX_TRACER;

	// Setup the origin and destination points
	VEC_COPY(vFrom, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(vTo, m_vPosition, vDist);

	// Set the intersection query values
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	HCLASS	hVolume		= g_pServerDE->GetClass("VolumeBrush");
	HCLASS	hObjClass   = DNULL;
	SurfaceType	eType	= SURFTYPE_UNKNOWN;

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);

	while(1)
	{
		if(pServerDE->IntersectSegment(&iq, &ii))
		{
			if(m_hOwner != ii.m_hObject)	// don't hit myself.
			{
				hObjClass   = g_pServerDE->GetObjectClass(ii.m_hObject);
				eType		= GetSurfaceType(ii.m_hObject, ii.m_hPoly);

				nFX = nExtras = 0;
				UpdateWeaponFX(nFX, nExtras, &ext, eType, m_nAmmoType, m_fDamage);

				// Test to see if we hit water... and set the FX accordingly
				if(pServerDE->IsKindOf(hObjClass, hVolume))
				{
					VolumeBrush* liquid = (VolumeBrush*)g_pServerDE->HandleToObject(ii.m_hObject);

					if(liquid)
					{
						ContainerCode code = liquid->GetCode();

						if(code == CC_WATER || code == CC_BLOOD || code == CC_ACID)
						{
							liquid->GetSurfaceColor(&(ext.vColor1), &(ext.vColor2));
							nExtras |= WFX_EXTRA_DAMAGE | WFX_EXTRA_COLOR1 | WFX_EXTRA_COLOR2;

							AddImpact(&(iq.m_From), &(ii.m_Point), vFire, &(ii.m_Plane.m_Normal), nFX, nExtras, &ext);
						}
					}
				}
				// Check to see if we hit a world object that can be shot through
				else if(ii.m_hObject == pServerDE->GetWorldObject() && eType != SURFTYPE_FIRETHROUGH)
				{
					AddImpact(&(iq.m_From), &(ii.m_Point), vFire, &(ii.m_Plane.m_Normal), nFX, nExtras, &ext);
					SendSoundTrigger(m_hOwner, SOUND_GUNIMPACT, ii.m_Point, 200.0f);

					if(eType != SURFTYPE_GLASS || m_nType == WEAP_MELEE) // Continue the vector for glass
						return DTRUE;
				}
				// See if we should place a mark on a world object that was hit
				else if(IsBrushObject(hObjClass) && nFX & WFX_MARK)	// CDestructableBrush object
				{
					CDestructableBrush *pBrush = (CDestructableBrush*)pServerDE->HandleToObject(ii.m_hObject);
					if(!pBrush->IsFireThrough())
					{
						eType = pBrush->GetSurfaceType();
	
						if(pBrush->IsMarkable())
							AddWorldModelMark(ii.m_Point, ii.m_Plane.m_Normal, ii.m_hObject, eType);

						AddImpact(&(iq.m_From), &(ii.m_Point), vFire, &(ii.m_Plane.m_Normal), nFX, nExtras, &ext);
						SendSoundTrigger(m_hOwner, SOUND_GUNIMPACT, ii.m_Point, 200.0f);
						SendDamageMsg(ii.m_hObject, ii.m_Point, vFire, m_fDamage);

						// Continue the vector for glass
						if(eType != SURFTYPE_GLASS)	return DTRUE;
					}
				}
				// Did we hit a door?
				else if(IsDoor(hObjClass))
				{
					Door *pDoor = (Door*)pServerDE->HandleToObject(ii.m_hObject);

					if (!pDoor->IsFireThrough())
					{
						AddImpact(&(iq.m_From), &(ii.m_Point), vFire, &(ii.m_Plane.m_Normal), nFX, nExtras, &ext);
						SendSoundTrigger(m_hOwner, SOUND_GUNIMPACT, ii.m_Point, 200.0f);
						SendDamageMsg(ii.m_hObject, ii.m_Point, vFire, m_fDamage);
						return DTRUE;
					}
				}
				// General case
				else
				{
					AddImpact(&(iq.m_From), &(ii.m_Point), vFire, &(ii.m_Plane.m_Normal), nFX, nExtras, &ext);
					SendDamageMsg(ii.m_hObject, ii.m_Point, vFire, m_fDamage);

					if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(ii.m_hObject), pServerDE->GetClass("CBaseCharacter")))
						SendSoundTrigger(m_hOwner, SOUND_GUNIMPACT, ii.m_Point, 200.0f);
					return DTRUE;
				}
			}

			// Add a bit to the vector and carry the shot through
			VEC_ADD(iq.m_From, ii.m_Point, *vFire);
		}
		else
			return DFALSE;		//did not hit an object in max range
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::FireProjectile()
//
//	PURPOSE:	Fires a projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CWeapon::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (!pServerDE)
		return DFALSE;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ROT_COPY(ocStruct.m_Rotation, m_rRotation);
	VEC_COPY(ocStruct.m_Pos, m_vPosition);

	if(m_hClient)
		ocStruct.m_UserData = pServerDE->GetClientID(m_hClient);
	else
		ocStruct.m_UserData = 0;

	CProjectile	*pProject = NULL;
	HCLASS		hClass = NULL;
	DFLOAT		fVelocity;
	int			nRadius;

	if (!bAltFire && m_szProjectileClass)
	{
		hClass = pServerDE->GetClass(m_szProjectileClass);
		fVelocity = m_fProjVelocity;
		nRadius = m_nDamageRadius;

//		pServerDE->CPrint("Primary projectile fired! Velocity = %f", fVelocity);
	}
	else if (m_szAltProjectileClass)
	{
		hClass = pServerDE->GetClass(m_szAltProjectileClass);
		fVelocity = m_fAltProjVelocity;
		nRadius = m_nAltDamageRadius;

//		pServerDE->CPrint("Alt projectile fired! Velocity = %f", fVelocity);
	}

	if (hClass)
	{
		if (pProject = (CProjectile*)pServerDE->CreateObject(hClass, &ocStruct))
		{
			if(g_pBloodServerShell->IsMultiplayerGame() && m_bMultiDamageBoost)
				pProject->Setup(vFire, m_nType, m_fDamage * GetDamageMultiplier() * 2.0f, fVelocity, nRadius, m_hOwner);
			else
				pProject->Setup(vFire, m_nType, m_fDamage * GetDamageMultiplier(), fVelocity, nRadius, m_hOwner);
		}
	}
	else  // No class! there isn't really a projectile, so fire a vector.
	{
		FireVector(vFire, dist);
	}

	return pProject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DDWORD CWeapon::Fire()
{
	//***** Make sure the server and inventory systems are valid *****//
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pInventoryMgr)
		return 0;

	//***** General variable declarations *****//
	DFLOAT		fReloadTime;
	DBYTE		nFiredType = 0;
	DFLOAT		cTime = pServerDE->GetTime();
	DFLOAT		range;
	Spread		*spread;
	DDWORD		dwShotsPerFire;
	D_WORD		nAmmoUse;
	DBOOL		bHitPlayerObj = DFALSE;


	// Reload time depends on current barrel or alt fire
	if(m_bLastFireAlt)		fReloadTime = m_fAltReloadTime;
		else				fReloadTime = m_fReloadTime;

	// See if any reload time modifiers are in effect
	fReloadTime *= m_pInventoryMgr->GetFireRateMultiplier();

	// Set up varaible for primary or alternate firing states
	DBOOL bAltFire = (m_eState == WS_ALT_FIRING) || (m_eState == WS_START_ALT_FIRING) || (m_eState == WS_STOP_ALT_FIRING);

	if(!bAltFire)
	{
		dwShotsPerFire = m_dwShotsPerFire;
		spread = &m_Spread;
		range = m_fRange;
		m_fDamage = pServerDE->Random(m_fMinDamage, m_fMaxDamage);
		nAmmoUse = m_nAmmoUse;
	}
	else
	{
		dwShotsPerFire = m_dwAltShotsPerFire;
		spread = &m_AltSpread;
		range = m_fAltRange;
		m_fDamage = pServerDE->Random(m_fMinAltDamage, m_fMaxAltDamage);
		nAmmoUse = m_nAltAmmoUse;
	}

	// Only fire ammo if we have the ammo
	DFLOAT fAmmoCount = CheckAmmo(bAltFire);
	if (fAmmoCount)
	{
		switch(GetAmmoType(bAltFire))
		{
			// Shells only use 1 shell per fire, no matter how many vectors
			case AMMO_SHELL:
				fAmmoCount -= (DFLOAT)nAmmoUse;
				break;
			// default is 1 bullet per vector
			default:
				if ((DFLOAT)(dwShotsPerFire * nAmmoUse) > fAmmoCount)
					dwShotsPerFire = (int)fAmmoCount / nAmmoUse;
				fAmmoCount -= (DFLOAT)(dwShotsPerFire * nAmmoUse);
				break;
		}
		m_pInventoryMgr->SetAmmoCount(GetAmmoType(bAltFire), fAmmoCount);
	}
	else
		dwShotsPerFire = 0;

	//***** If no shots are available, play empty sound and exit the function *****//
	if(dwShotsPerFire <= 0)
	{
		PlayEmptyWeaponSound(bAltFire);
		if (!IsOwnerAPlayer())		m_eState = WS_REST;		//SCHLEGZ
		return 0;	// Don't fire if we don't have ammo
	}

	//***** For non-melee weapons... display a muzzle flash *****//
	if(m_nFireType != TYPE_MELEE)
		ShowMuzzleFlash(&m_vPosition );

	PlayFireSound(bAltFire);

	DVector		shellDir;
	FireSpread(spread, dwShotsPerFire, range, bAltFire, &shellDir);

	m_fLastShotTime = cTime;

	if(m_eState == WS_ALT_FIRING)
	{
		m_bLastFireAlt = DTRUE;
		nFiredType = 2;
	}
	else
	{
		m_bLastFireAlt = DFALSE;
		nFiredType = 1;
	}

	if (!IsOwnerAPlayer())		m_eState = WS_REST;		//SCHLEGZ

	// Reset the shell eject flag for the next fire..
	if(m_nAmmoType == AMMO_BULLET || m_nAmmoType == AMMO_SHELL)
		m_bEjectShell = DTRUE;
	else
		m_bEjectShell = DFALSE;

	return nFiredType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::FireSpread()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir)
{
	//***** Make sure the server and inventory systems are valid *****//
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pInventoryMgr)		return;

	//***** Variables to calculate the spread of the weapon fire *****//
	DFLOAT		hOffset, vOffset;
	DVector		vTmp, vFire;
	DVector		vU, vR, vF;
	DBOOL		bTracer = DFALSE;
	pServerDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF); 

	// Check to see if the player is standing too close to something, and we need
	// to move the firing position back in order to hit it
//	if(IsPlayer(m_hOwner) && FiringTooClose(&vF, 60.0f, &vTmp))
//		{ VEC_COPY(m_vPosition, vTmp); }

	for(int i = (shots - 1); i >= 0; i--)
	{
		VEC_COPY(vFire, vF)

		if(spread->h)
		{
			hOffset = pServerDE->Random(-spread->h, spread->h) / 2000.0f;
			VEC_MULSCALAR(vTmp, vR, hOffset)
			VEC_ADD(vFire, vFire, vTmp)
		}

		if(spread->v)
		{
			vOffset = pServerDE->Random(-spread->v, spread->v) / 2000.0f;
			VEC_MULSCALAR(vTmp, vU, vOffset)
			VEC_ADD(vFire, vFire, vTmp)
		}

		VEC_NORM(vFire)

		if(m_bAccuracyCheck && IsPlayer(m_hOwner))
			AlignFireVector(&vFire, range);

		// See if we should fire a tracer for bullets
		if(GetAmmoType(bAltFire) == AMMO_BULLET)
			bTracer = DTRUE;

		// Reset the splash
		m_bSplash = DTRUE;

		// Fire the appropriate thing
		if((bAltFire && !m_szAltProjectileClass) || (!bAltFire && !m_szProjectileClass))
			FireVector(&vFire, range, bTracer);
		else
			FireProjectile(&vFire, range, bAltFire);
	}

	VEC_COPY(*rDir, vR);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::FiringTooClose
//
//	PURPOSE:	Trick to improve accuracy and keep the effect at the weapon end
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::FiringTooClose(DVector *vFire, DFLOAT fDist, DVector *vNewPos)
{
	//***** Make sure the server and inventory systems are valid *****//
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pViewModel) return DFALSE;

   	CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	DFLOAT fOffset = 0.0f;

	if(pObj)	fOffset = pObj->GetEyeLevel();

	DVector	vOffset;
	VEC_SET(vOffset, 0.0f, fOffset, 0.0f);

	// Setup the vectors for the intersect
	pServerDE->GetObjectPos(m_pViewModel->m_hObject, vNewPos);
	VEC_ADD(*vNewPos, *vNewPos, vOffset);

	VEC_NORM(*vFire);
	VEC_MULSCALAR(*vFire, *vFire, fDist);

	IntersectQuery	iq;
	IntersectInfo	ii;

	// Set the intersection query values
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, *vNewPos);
	VEC_ADD(iq.m_To, iq.m_From, *vFire);
	VEC_NORM(*vFire);

	if(pServerDE->IntersectSegment(&iq, &ii))
		return DTRUE;

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AlignFireVector
//
//	PURPOSE:	Trick to improve accuracy and keep the effect at the weapon end
//
// ----------------------------------------------------------------------- //

void CWeapon::AlignFireVector(DVector *vFire, DFLOAT fDist)
{
	//***** Make sure the server and inventory systems are valid *****//
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hOwner) return;

   	CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	DFLOAT fOffset = 0.0f;

	if(pObj)	fOffset = pObj->GetEyeLevel();

	DVector	vPos, vOffset;
	VEC_SET(vOffset, 0.0f, fOffset, 0.0f);

	// Setup the vectors for the intersect
	pServerDE->GetObjectPos(m_hOwner, &vPos);
	VEC_ADD(vPos, vPos, vOffset);

	VEC_NORM(*vFire);
	VEC_MULSCALAR(*vFire, *vFire, fDist);

	IntersectQuery	iq;
	IntersectInfo	ii;

	// Set the intersection query values
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, vPos);
	VEC_ADD(iq.m_To, iq.m_From, *vFire);

	if(pServerDE->IntersectSegment(&iq, &ii))
	{
		VEC_SUB(*vFire, ii.m_Point, m_vPosition);
		VEC_NORM(*vFire);
	}
	else
	{
		VEC_SUB(*vFire, iq.m_To, m_vPosition);
		VEC_NORM(*vFire);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetFirePosRot
//
//	PURPOSE:	Update the firing state of the gun, keep track of muzzle
//				position & rotation (used by AIs)
//
// ----------------------------------------------------------------------- //

void CWeapon::SetFirePosRot(DVector *firedPos, DRotation *rotP, DBOOL bAltFire)
{
	VEC_COPY(m_vPosition, *firedPos);
	ROT_COPY(m_rRotation, *rotP);

	m_eState = bAltFire ? WS_ALT_FIRING : WS_FIRING;
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetHandModelPos
//
//	PURPOSE:	Returns the position of the hand gun model, if any.
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::GetHandModelPos(DVector *pvPos, DRotation *prRot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pHandModel || !m_pHandModel->m_hObject || !pvPos || !prRot) return DFALSE;


	pServerDE->GetObjectPos(m_pHandModel->m_hObject, pvPos);
	pServerDE->GetObjectRotation(m_pHandModel->m_hObject, prRot);
	return DTRUE;
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateFiringState
//
//	PURPOSE:	Update the firing state of the gun, keep track of muzzle
//				position & rotation (used by player objects)
//
// ----------------------------------------------------------------------- //

void CWeapon::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pViewModel) return;

	VEC_COPY(m_vPosition, *firedPos);
	ROT_COPY(m_rRotation, *rotP);

	DFLOAT fTime = pServerDE->GetTime();
	DBOOL bAltFire = DFALSE;

	switch (m_eState)
	{
		case WS_IDLE:
			{
				if(!PlayAnimation(m_nIdleAnim))
				{	
					PlayAnimation(m_nRestAnim);
					m_eState = WS_REST;
					m_fIdleStartTime = fTime;
					m_fIdleDelay = pServerDE->Random(m_fMinIdleDelay, m_fMaxIdleDelay);

					pServerDE->SetModelLooping(m_pViewModel->m_hObject, m_bLoopStatic);
				}
			}

		case WS_REST:
			{
				DFLOAT fReloadTime;
				if (m_bLastFireAlt)
					fReloadTime = m_fAltReloadTime;
				else
					fReloadTime = m_fReloadTime;

				fReloadTime *= m_pInventoryMgr->GetFireRateMultiplier();

				// See if we should play an idle animation
				if((pServerDE->GetTime() - m_fIdleStartTime) > m_fIdleDelay)
				{
					pServerDE->SetModelLooping(m_pViewModel->m_hObject, DFALSE);

					PlayAnimation(m_nIdleAnim);
					m_eState = WS_IDLE;
				}

				// Make sure enough time has gone by to allow firing
				if(((fTime - m_fLastShotTime) > fReloadTime))
				{
					if (bFiring && (CheckAmmo(DFALSE) >= m_nAmmoUse))
					{
						m_fLastShotTime = fTime;
	
						m_eState = WS_START_FIRING;
						if (!PlayAnimation(m_nStartFireAnim))
						{
							pServerDE->SetModelLooping(m_pViewModel->m_hObject, m_bLoopAnim);

							PlayAnimation(m_nFireAnim);
							m_eState = WS_FIRING;
						}
					}
					else if (bAltFiring && !IsAltFireZoom() && (CheckAmmo(DTRUE) >= m_nAltAmmoUse))
					{
						m_fLastShotTime = fTime;

						m_eState = WS_START_ALT_FIRING;
						if (!PlayAnimation(m_nStartAltFireAnim))
						{
							pServerDE->SetModelLooping(m_pViewModel->m_hObject, m_bAltLoopAnim);

							PlayAnimation(m_nAltFireAnim);
							m_eState = WS_ALT_FIRING;
						}
					}
				}
			}
			break;

		case WS_DRAW:
			{
				if(!PlayAnimation(m_nDrawAnim))
				{	
					PlayAnimation(m_nRestAnim);
					m_eState = WS_REST;

					pServerDE->SetModelLooping(m_pViewModel->m_hObject, m_bLoopStatic);
					m_fIdleStartTime = fTime;
				}
			}
			break;

		case WS_HOLSTER:
			{
				if(!PlayAnimation(m_nHolsterAnim))
				{	
					m_eState = WS_HOLSTERED;

					pServerDE->SetModelLooping(m_pViewModel->m_hObject, DFALSE);
				}
			}
			break;

		case WS_START_ALT_FIRING:
		case WS_START_FIRING:
			{
				bAltFire = (m_eState == WS_START_ALT_FIRING);
				if (!PlayAnimation(bAltFire ? m_nStartAltFireAnim : m_nStartFireAnim))
				{
					if (bAltFire ? m_bAltLoopAnim : m_bLoopAnim)
						pServerDE->SetModelLooping(m_pViewModel->m_hObject, DTRUE);
					else
						pServerDE->SetModelLooping(m_pViewModel->m_hObject, DFALSE);

					PlayAnimation(bAltFire ? m_nAltFireAnim : m_nFireAnim);
					m_eState = bAltFire ? WS_ALT_FIRING : WS_FIRING;
				}
				m_fIdleStartTime = fTime;
			}
			break;

		case WS_ALT_FIRING:
		case WS_FIRING:
			{
				bAltFire = (m_eState == WS_ALT_FIRING);
				DBOOL bLoopAnim;
				DDWORD nFireAnim;
				DBOOL  bNowFiring;
				DFLOAT fReloadTime;
				WeaponState eNextState;

				if (bAltFire)
				{
					bLoopAnim	= m_bAltLoopAnim;
					nFireAnim	= m_nAltFireAnim;
					bNowFiring	= bAltFiring;
					fReloadTime = m_fAltReloadTime;
					eNextState	= WS_STOP_ALT_FIRING;
				}
				else
				{
					bLoopAnim	= m_bLoopAnim;
					nFireAnim	= m_nFireAnim;
					bNowFiring	= bFiring;
					fReloadTime = m_fReloadTime;
					eNextState	= WS_STOP_FIRING;
				}

				// No fire anim, so fire right away
				if (nFireAnim == INVALID_ANI && !bLoopAnim)
				{
					OnFireKey();
					m_eState = eNextState;
				}
				// Else looping anim, so fire based on rate of fire
				else if (bLoopAnim)
				{
					if (!bNowFiring || CheckAmmo(bAltFire) == 0.0f)
					{
						m_eState = eNextState;
						pServerDE->SetModelLooping(m_pViewModel->m_hObject, DFALSE);
					}
				}
				else if (m_bSemiAuto)
				{
					// Stopped firing
					if (!m_bLastFiring && bNowFiring)
					{
						pServerDE->ResetModelAnimation(m_pViewModel->m_hObject);
					}
					else if (!PlayAnimation(bAltFire ? m_nAltFireAnim : m_nFireAnim))
					{
						m_eState = eNextState;
					}
				}
				// Else just wait for the animation to stop
				else if (!PlayAnimation(bAltFire ? m_nAltFireAnim : m_nFireAnim))
				{
					m_eState = eNextState;
				}
				// check if there is a fire rate multiplier
				else if (m_pInventoryMgr->GetFireRateMultiplier() < 1.0f)
				{
					if (!m_bLastFiring && bNowFiring && (fTime - m_fLastShotTime) > fReloadTime * m_pInventoryMgr->GetFireRateMultiplier())
						pServerDE->ResetModelAnimation(m_pViewModel->m_hObject);
				}
				m_fIdleStartTime = fTime;
			}
			break;

		case WS_STOP_ALT_FIRING:
		case WS_STOP_FIRING:
			{
				bAltFire = (m_eState == WS_STOP_ALT_FIRING);
				if (!PlayAnimation(bAltFire ? m_nStopAltFireAnim : m_nStopFireAnim))
				{
					PlayAnimation(m_nRestAnim);
					m_eState = WS_REST;

					pServerDE->SetModelLooping(m_pViewModel->m_hObject, m_bLoopStatic);
				}
				m_fIdleStartTime = fTime;
			}
			break;

		default : break;
	}
	m_bLastFiring = bFiring | bAltFiring;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateHandModel
//
//	PURPOSE:	Create the hand-model associated with the weapon.
//
// ----------------------------------------------------------------------- //

void CWeapon::CreateHandModel()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pHandModelFilename || !m_pHandModelSkinname) return;

	HCLASS hOwnerClass = pServerDE->GetObjectClass(m_hOwner);

	DVector vPos;
	DRotation rRot;
	char* szNode = m_bLeftHand ? "l_gun" : "r_gun";

	pServerDE->GetObjectPos(m_hOwner, &vPos);

	// Create the hand view model
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL; 
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE;
	VEC_COPY(ocStruct.m_Pos, vPos);

	if(m_hClient)
	{
		ocStruct.m_UserData = pServerDE->GetClientID(m_hClient);
		if(m_bLeftHand)
			ocStruct.m_NextUpdate = 0.02f;
	}
	else
		ocStruct.m_UserData = 0;

	_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)m_pHandModelFilename);
	_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)m_pHandModelSkinname);

	HCLASS hClass = pServerDE->GetClass("CHandWeaponModel");
	m_pHandModel = (CHandWeaponModel*)pServerDE->CreateObject(hClass, &ocStruct);

	if (m_pHandModel && m_pHandModel->m_hObject)
	{
		// Set to the animation with the proper origin offset
		DDWORD dwAnim = pServerDE->GetAnimIndex(m_pHandModel->m_hObject, "handheld");
		if (dwAnim != INVALID_ANI)
			pServerDE->SetModelAnimation(m_pHandModel->m_hObject, dwAnim);

		m_pHandModel->SetType(m_nType);
		m_pHandModel->SetPlayerOwned( IsPlayer( m_hOwner ));
		m_pHandModel->SetWeaponOwner( this );

		if (m_hClient)
			m_pHandModel->SetClient(m_hClient, m_bLeftHand);

		DVector vOffset;
		DRotation rRotOffset;
		VEC_INIT(vOffset);
		ROT_INIT(rRotOffset);

		// Create an attachment and show the model if there is a node
		if (pServerDE->GetModelNodeTransform(m_hOwner, szNode, &vPos, &rRot))
		{
			pServerDE->CreateAttachment(m_hOwner, m_pHandModel->m_hObject, szNode, &vOffset, &rRotOffset, &m_hHandAttachment);
			m_pHandModel->SetVisible(DTRUE);
		}
		else
		{	// hide it
			pServerDE->CreateAttachment(m_hOwner, m_pHandModel->m_hObject, DNULL, &vOffset, &rRotOffset, &m_hHandAttachment);
			m_pHandModel->SetVisible(DFALSE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::DropHandModel
//
//	PURPOSE:	Drops the hand model
//
// ----------------------------------------------------------------------- //

void CWeapon::DropHandModel()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pHandModel || !m_pHandModel->m_hObject) return;

	// Reset the color in case owner had stealth or something.
	pServerDE->SetObjectColor(m_pHandModel->m_hObject, 0.0f, 0.0f, 0.0f, 1.0f);

	if (m_hHandAttachment)
	{
		pServerDE->RemoveAttachment(m_hHandAttachment);
		m_hHandAttachment = DNULL;
	}

	DVector vPos;
	DRotation rRot;

	pServerDE->GetObjectPos(m_hOwner, &vPos);
	pServerDE->GetObjectRotation(m_hOwner, &rRot);

	pServerDE->SetObjectPos(m_pHandModel->m_hObject, &vPos);
	pServerDE->SetObjectRotation(m_pHandModel->m_hObject, &rRot);

	m_pHandModel->Drop();
	m_pHandModel->SetWeaponOwner( NULL );
	m_pHandModel = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetupViewModel
//
//	PURPOSE:	Sets the model filenames and animations.
//
// ----------------------------------------------------------------------- //

void CWeapon::SetupViewModel()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pViewModel) return;

	HCLASS hOwnerClass = pServerDE->GetObjectClass(m_hOwner);

	// Choose the right or lefthanded model
	char	*pModelFilename;
	if(m_bLeftHand)		pModelFilename = m_pLeftViewModelFilename;
		else			pModelFilename = m_pViewModelFilename;

	if(pModelFilename && m_pViewModelSkinname)
	{
		char	model[128];
		char	skin[128];
		char	firstChar;
		char	start[] = "C_";
		char	*change = DNULL;

		// Get the character to use for the skin

#ifdef _ADDON
		DBOOL bEnemyChar = DFALSE;
#endif

   		CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
		if(pObj)
		{
			switch(pObj->GetCharacter())
			{
				case	CHARACTER_CALEB:		firstChar = 'C'; break;
				case	CHARACTER_OPHELIA:		firstChar = 'O'; break;
				case	CHARACTER_ISHMAEL:		firstChar = 'I'; break;
				case	CHARACTER_GABREILLA:	firstChar = 'G'; break;

#ifdef _ADDON
				case	CHARACTER_M_CULTIST:	firstChar = 'C'; break;
				case	CHARACTER_F_CULTIST:	firstChar = 'O'; break;
				case	CHARACTER_SOULDRUDGE:	firstChar = 'C'; break;
				case	CHARACTER_PROPHET:		firstChar = 'I'; break;
#endif

			}

#ifdef _ADDON
			if (pObj->GetCharacter() >= CHARACTER_M_CULTIST)
			{
				bEnemyChar = DTRUE;
			}
#endif
		}

		// Copy over the filenames so we preserver the originals
		_mbscpy((unsigned char*)model, (const unsigned char*)pModelFilename);
		_mbscpy((unsigned char*)skin, (const unsigned char*)m_pViewModelSkinname);

		// Change the weapon skin to the correct character
		change = strstr(skin, start);
		change[0] = firstChar;

		// Change the knife model to the correct character
		if(m_nType == WEAP_MELEE)
		{
			change = strstr(model, start);
			change[0] = firstChar;
		}

		// Check for generic orb

#ifdef _ADDON
		if (m_nType == WEAP_ORB && bEnemyChar)
		{

		}
#endif

		// Continue
		change = DNULL;

		if(m_nFlags & FLAG_ENVIRONMENTMAP)
		{
			pServerDE->SetObjectColor(m_pViewModel->m_hObject, 1.0f, 1.0f, 1.0f, m_fChromeValue);
			DDWORD f = pServerDE->GetObjectFlags(m_pViewModel->m_hObject);
			pServerDE->SetObjectFlags(m_pViewModel->m_hObject, f | FLAG_ENVIRONMENTMAP | FLAG_VISIBLE);
		}
		else
		{
			pServerDE->SetObjectColor(m_pViewModel->m_hObject, 0.0f, 0.0f, 0.0f, 1.0f);
			DDWORD f = pServerDE->GetObjectFlags(m_pViewModel->m_hObject);
			f |= FLAG_VISIBLE;
			pServerDE->SetObjectFlags(m_pViewModel->m_hObject, f & ~FLAG_ENVIRONMENTMAP);
		}

		// Set the new model filenames
		pServerDE->SetObjectFilenames(m_pViewModel->m_hObject, model, skin);

		pServerDE->SetModelLooping(m_pViewModel->m_hObject, DFALSE);
		pServerDE->SetModelAnimation(m_pViewModel->m_hObject, -1);

		// Tell it that this weapon this is it's parent for now.
		m_pViewModel->Init(this);
		m_pViewModel->UseModelKeys(DTRUE);

		m_nRestAnim			= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "static_model");
		m_nIdleAnim			= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "idle");
		m_nDrawAnim			= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "draw");
		m_nDrawDuelAnim		= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "dh_draw");
		m_nHolsterAnim		= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "holster");
		m_nHolsterDuelAnim	= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "dh_holster");
		m_nStartFireAnim	= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "start_fire");
		m_nFireAnim			= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "fire");
		m_nStopFireAnim		= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "end_fire");
		m_nStartAltFireAnim	= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "start_alt_fire");
		m_nAltFireAnim		= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "alt_fire");
		m_nStopAltFireAnim	= pServerDE->GetAnimIndex(m_pViewModel->m_hObject, "end_alt_fire");

		if (m_nIdleAnim == INVALID_ANI)
			m_nIdleAnim = m_nRestAnim;

		if (m_nAltFireAnim == INVALID_ANI)
		{
			m_nStartAltFireAnim = m_nStartFireAnim;
			m_nAltFireAnim = m_nFireAnim;
			m_nStopAltFireAnim = m_nStopFireAnim;
		}

		PlayAnimation(m_nDrawAnim);
		m_fIdleDelay = pServerDE->Random(m_fMinIdleDelay, m_fMaxIdleDelay);
		m_fIdleStartTime = pServerDE->GetTime();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateMuzzleFlash()
//
//	PURPOSE:	Creates a muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeapon::SetupMuzzleFlash()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	WeaponData *wd = &g_WeaponDefaults[m_nType-1];

	if (m_hFlash && wd)
	{
		pServerDE->SetLightRadius(m_hFlash, wd->m_fFlashRadius);
		pServerDE->SetLightColor(m_hFlash, VEC_EXPAND(wd->m_vFlashColor));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ShowMuzzleFlash()
//
//	PURPOSE:	Makes the muzzle flash visible
//
// ----------------------------------------------------------------------- //

void CWeapon::ShowMuzzleFlash(DVector *vPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	WeaponData *wd = &g_WeaponDefaults[m_nType-1];
	DFLOAT fRadius = wd->m_fFlashRadius;

	if(m_hFlash && fRadius)
	{
		if(!m_bFlashShowing)
		{
			// Set visible
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hFlash);
			dwFlags |= FLAG_VISIBLE;
			pServerDE->SetObjectFlags(m_hFlash, dwFlags);

			// Move to current position
			pServerDE->TeleportObject(m_hFlash, vPos);
			m_fFlashStartTime = pServerDE->GetTime();
			m_bFlashShowing = DTRUE;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateMuzzleFlash()
//
//	PURPOSE:	Checks the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeapon::UpdateMuzzleFlash()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (m_hFlash && m_bFlashShowing)
	{
		if (pServerDE->GetTime() > m_fFlashStartTime + m_fFlashDuration)
		{
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hFlash);
			dwFlags &= ~FLAG_VISIBLE;
			pServerDE->SetObjectFlags(m_hFlash, dwFlags);
			m_bFlashShowing = DFALSE;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SendDamageMsg()
//
//	PURPOSE:    Weapon Damage Message
//
// ----------------------------------------------------------------------- //
DBOOL CWeapon::SendDamageMsg(HOBJECT firedTo, DVector vPoint, DVector *vFire, DFLOAT fDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (!pServerDE)
		return DFALSE;

	BaseClass *ffObj = pServerDE->HandleToObject(m_hOwner);

	// Send a damage message to the object
	if(g_pBloodServerShell->IsMultiplayerGame() && m_bMultiDamageBoost)
		DamageObject(m_hOwner, ffObj, firedTo, fDamage * GetDamageMultiplier() * 2.0f, *vFire, vPoint, m_nDamageType);
	else
		DamageObject(m_hOwner, ffObj, firedTo, fDamage * GetDamageMultiplier(), *vFire, vPoint, m_nDamageType);

    return DTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::OnFireKey()
//
//	PURPOSE:    The view model or the weapon model has fired..
//
// ----------------------------------------------------------------------- //

void CWeapon::OnFireKey()
{
	DBYTE nFiredType = (DBYTE)Fire();
	SendClientFiring(nFiredType);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SendClientInfo()
//
//	PURPOSE:    Sends info that the client needs to show weapon.
//
// ----------------------------------------------------------------------- //

void CWeapon::SendClientInfo(DBYTE slot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !IsOwnerAPlayer())
		return;

	CPlayerObj *pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	if (!pPlayer->IsActivated())
		return;

	HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_NEWWEAPON);

	pServerDE->WriteToMessageByte(hMsg, slot);
	pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_nType);
	pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_bLeftHand);
/*	pServerDE->WriteToMessageObject(hMsg, m_pViewModel->m_hObject);

	if(m_nWeaponNameID)
	{
		HSTRING hstr = pServerDE->FormatString(m_nWeaponNameID);
		pServerDE->WriteToMessageString(hMsg, pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);
	}
	else
		pServerDE->WriteToMessageString(hMsg, m_szWeaponName);

	pServerDE->WriteToMessageString(hMsg, m_szFlashSprite);
	pServerDE->WriteToMessageString(hMsg, m_szAltFlashSprite);
	pServerDE->WriteToMessageFloat(hMsg, m_fFlashDuration);
	pServerDE->WriteToMessageFloat(hMsg, m_fFlashScale);
	pServerDE->WriteToMessageVector(hMsg, &m_vViewModelOffset);
	pServerDE->WriteToMessageVector(hMsg, &m_vMuzzleOffset);
	pServerDE->WriteToMessageVector(hMsg, &m_vFlash);
	pServerDE->WriteToMessageFloat(hMsg, m_fEjectInterval);
	pServerDE->WriteToMessageFloat(hMsg, m_fViewKick);
	pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_bCumulativeKick);

	pServerDE->WriteToMessageHString(hMsg, m_szPic);
	pServerDE->WriteToMessageHString(hMsg, m_szPicH);
*/
	pServerDE->EndMessage(hMsg);

	m_bClientNotified = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SendClientFiring()
//
//	PURPOSE:    Sends info that the client needs to show a weapon firing.
//
// ----------------------------------------------------------------------- //
void CWeapon::SendClientFiring(DBYTE type)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !IsOwnerAPlayer())
		return;

	HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_FIREDWEAPON);

	pServerDE->WriteToMessageByte(hMsg, type);

	pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::PlayAnimation()
//
//	PURPOSE:	Set model to new animation
//
// ----------------------------------------------------------------------- //

DBOOL CWeapon::PlayAnimation(DDWORD dwNewAni)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pViewModel || dwNewAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_pViewModel->m_hObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_pViewModel->m_hObject);

	if (dwAni == dwNewAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != dwNewAni)
	{
		pServerDE->SetModelAnimation(m_pViewModel->m_hObject, dwNewAni);
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddImpact
//
//	PURPOSE:	Add an impact sprite
//
// ----------------------------------------------------------------------- //

void CWeapon::AddImpact(DVector *pvSource, DVector *pvDest, DVector *pvForward, DVector *pvNormal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector	vAdjDest;

//	ObjectCreateStruct theStruct;
//	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_MULSCALAR(vAdjDest, *pvNormal, 1.25f);
	VEC_ADD(vAdjDest, vAdjDest, *pvDest);
//	VEC_ADD(theStruct.m_Pos, *pvDest, temp);
//	ROT_COPY(theStruct.m_Rotation, m_rRotation);

//	HCLASS hClass = pServerDE->GetClass("CClientWeaponSFX");

//	CClientWeaponSFX *pWeaponFX = DNULL;

//	if(hClass)
//		pWeaponFX = (CClientWeaponSFX*)pServerDE->CreateObject(hClass, &theStruct);

//	if(pWeaponFX)
	SendWeaponSFXMessage(pvSource, &vAdjDest, pvForward, pvNormal, nFX, nExtras, ext);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddSplash
//
//	PURPOSE:	Make a water splash
//
// ----------------------------------------------------------------------- //

void CWeapon::AddSplash(DVector vPos, DVector vNormal, HOBJECT hObject, DVector vColor1, DVector vColor2)
{
	ServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);
	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientSplashSFX");
	CClientSplashSFX *pSplash = DNULL;

	if(hClass)
		pSplash = (CClientSplashSFX *)pServerDE->CreateObject(hClass, &ocStruct);

	if(pSplash)
	{
		DBOOL		bRipple;
		DFLOAT		fRadius, fPosRadius, fHeight, fDensity, fSpread, fSprayTime, fDuration, fGravity;
		DVector		vRipScale;
		char		*pParticle;

		fRadius			= 100.0f;
		fPosRadius		= 1.0f;
		fHeight			= 175.0f;
		fDensity		= 4.0f;
		fSpread			= 10.0f;
		VEC_SET(vRipScale, 0.1f, 0.1f, 0.0f);
		bRipple			= DTRUE;
		fSprayTime		= 0.3f;
		fDuration		= 0.75f;
		fGravity		= -500.0f;
		pParticle		= "spritetextures\\drop32_1.dtx";

		pSplash->Setup(&vPos, &vNormal, fRadius, fPosRadius, fHeight, fDensity, fSpread, &vColor1,
					   &vColor2, &vRipScale, bRipple, fSprayTime, fDuration, fGravity, pParticle);
	}
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddSparks
//
//	PURPOSE:	Add impact sparks
//
// ----------------------------------------------------------------------- //

void CWeapon::AddSparks(DVector vPos, DVector vNormal, DFLOAT fDamage, HOBJECT hObject, SurfaceType eType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);
	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientSparksSFX");
	CClientSparksSFX *pSpark = DNULL;

	if(hClass)
		pSpark = (CClientSparksSFX *)pServerDE->CreateObject(hClass, &ocStruct);

	if(pSpark)
	{
		DVector	vColor1, vColor2;
		char	*pTextureFile;

		switch(eType)
		{
			case SURFTYPE_FLESH:
			{
				DVector		temp;
				VEC_INIT(temp);
				temp.y = 50.0f;

				VEC_ADD(temp, temp, vNormal);
				
				VEC_SET(vColor1, 200.0f, 0.0f, 0.0f);
				VEC_SET(vColor2, 255.0f, 0.0f, 0.0f);

				switch(pServerDE->IntRandom(1,NRES(4)))
				{
					case 1:		pTextureFile = "spritetextures\\particles\\blooddrop_1.dtx";		break;
					case 2:		pTextureFile = "spritetextures\\particles\\blooddrop_2.dtx";		break;
					case 3:		pTextureFile = "spritetextures\\particles\\blooddrop_3.dtx";		break;
					case 4:		pTextureFile = "spritetextures\\particles\\blooddrop_4.dtx";		break;
				}

				pSpark->Setup(&temp, &vColor1, &vColor2, pTextureFile, (DBYTE)(fDamage * 3), 2.0f, 5.0f, 200.0f, -150.0f);
				return;
			}
			case SURFTYPE_METAL:
			{
				VEC_SET(vColor1, 200.0f, 200.0f, 200.0f);
				VEC_SET(vColor2, 255.0f, 255.0f, 0.0f);
				VEC_MULSCALAR(vNormal, vNormal, 75.0f);
				pTextureFile = "spritetextures\\particles\\particle2.dtx";
				pSpark->Setup(&vNormal, &vColor1, &vColor2, pTextureFile, (DBYTE)pServerDE->IntRandom(5, 30), 0.15f, 0.5f, 200.0f, -200.0f);
				return;
			}
			case SURFTYPE_STONE:
			case SURFTYPE_WOOD:
				break;
			default:
			{
				VEC_SET(vColor1, 200.0f, 200.0, 200.0f);
				VEC_SET(vColor2, 200.0f, 200.0f, 0.0f);
				VEC_MULSCALAR(vNormal, vNormal, 100.0f);
				pTextureFile = "spritetextures\\particles\\particle2.dtx";
				pSpark->Setup(&vNormal, &vColor1, &vColor2, pTextureFile, (DBYTE)fDamage, 0.15f, 0.5f, 200.0f, -200.0f);
				break;
			}
		}
	}
	return;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddWorldModelMark
//
//	PURPOSE:	Adds bullet mark to the wall.This is ONLY used for WorldModels,
//				normal wall marks use a ClientWeaponFX WFX_MARK flag.
//
// ----------------------------------------------------------------------- //

void CWeapon::AddWorldModelMark(DVector vPos, DVector vNormal, HOBJECT hObject, SurfaceType eType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	DVector vTmp;
	VEC_MULSCALAR(vTmp, vNormal, 0.5f);
	VEC_COPY(ocStruct.m_Pos, vPos);
	VEC_ADD(ocStruct.m_Pos, ocStruct.m_Pos, vTmp);

	HCLASS hClass = pServerDE->GetClass("CClientMarkSFX");

	if (hClass)
	{
		CClientMarkSFX *pMark = DNULL;

		pMark = (CClientMarkSFX *)pServerDE->CreateObject(hClass, &ocStruct);

		DBOOL bAltFire = (m_eState == WS_ALT_FIRING);
	
		if (pMark)
		{
			pMark->Setup( &vPos, &vNormal, m_nType, GetAmmoType(bAltFire), eType, hObject );
		}
	}
}

