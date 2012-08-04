//----------------------------------------------------------
//
// MODULE  : ViewWeapon.cpp
//
// PURPOSE : Handles client side weapons.
//
// CREATED : 9/22/97
//
//----------------------------------------------------------

// Includes...

#include <crtdbg.h>
#include <mbstring.h>
#include "ViewWeapon.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "SoundTypes.h"


DBYTE g_byRandomWeaponSeed;


// Defines....

#define ANIM_GUN_NONE		0
#define ANIM_GUN_FIRING		1
#define ANIM_GUN_FIRING2	2

#define LOWER_SPEED			2
#define LOWER_AMOUNT		(PI/4)

#define WEAPON_KEY_FIRE			"FIRE_KEY"
#define WEAPON_KEY_SOUND		"SOUND_KEY"
#define WEAPON_KEY_SOUNDLOOP	"SOUNDLOOP_KEY"
#define WEAPON_KEY_SOUNDSTOP	"SOUNDSTOP_KEY"
#define WEAPON_KEY_HIDE			"HIDE_KEY"
#define WEAPON_KEY_SHOW			"SHOW_KEY"

#define INVALID_ANI		((DDWORD)-1)


DRotation g_rotGun;
HLOCALOBJ g_hWeaponModel;


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::CViewWeapon()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CViewWeapon::CViewWeapon()
{
	m_nType				= WEAP_NONE;
	VEC_INIT(m_vViewModelOffset);
	VEC_INIT(m_vFlashPos);
	VEC_INIT(m_vAdjFlashPos);
	m_fViewKick			= 0;
	m_bCumulativeKick	= DFALSE;
	m_bLeftHand			= DFALSE;
	
	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fLastEjectedTime	= 0.0f;
	m_bOkToFire			= DTRUE;
	m_nFiredType		= 0;
	m_pClientDE			= NULL;
	m_hFlashSprite		= NULL;
	m_hAltFlashSprite	= NULL;
	m_fFlashDuration	= 0.0f;
	m_nFlashVisible		= 0;
	m_nIgnoreFX			= 0;
	m_hLoopSound		= DNULL;

	m_nFlags			= FLAG_ENVIRONMENTMAP;
	m_fChromeValue		= 0.5f;

	m_hObject			= DNULL;
}


// --------------------------------------------------------------------------- //
//	ROUTINE:	CViewWeapon::~CViewWeapon()
//	PURPOSE:	Deletes this weapon object.
// --------------------------------------------------------------------------- //
CViewWeapon::~CViewWeapon()
{
	Term();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Create()
//
//	PURPOSE:	Sets up the weapon info, creates the model.
//
// --------------------------------------------------------------------------- //
DBOOL CViewWeapon::Create(CClientDE* pClientDE, DBYTE byWeaponID, DBOOL bLeftHand)
{
	m_pClientDE			= pClientDE;

	m_bLeftHand			= bLeftHand;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fLastEjectedTime	= 0.0f;
	m_bOkToFire			= DTRUE;
	m_nFiredType		= 0;

	m_fLowerOffset		= 0;

	m_nType					= byWeaponID;
	m_eState				= WS_DRAW;

	WeaponData *wd			= &g_WeaponDefaults[m_nType-1];

	m_nFireType				= wd->m_nFireType;
	m_pViewModelFilename	= wd->m_szViewModelFilename;
	m_pLeftViewModelFilename = wd->m_szLeftViewModelFilename;
	m_pViewModelSkinname	= wd->m_szViewModelSkin;
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
	VEC_COPY(m_vViewModelOffset, wd->m_vViewModelOffset)
	VEC_COPY(m_vMuzzleOffset, wd->m_vMuzzleOffset)
	VEC_COPY(m_vFlashPos, wd->m_vFlash)
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

	m_szPic					= DNULL;
	m_szPicH				= DNULL;

	m_bAccuracyCheck		= DTRUE;
	
	if ((m_szFlashSprite[0] != '\0') || (m_szAltFlashSprite[0] != '\0'))
		CreateFlash(wd->m_fFlashScale);


	if (m_bLeftHand)
	{
		m_vViewModelOffset.x = -m_vViewModelOffset.x;
		m_vFlashPos.x = -m_vFlashPos.x;
		m_vMuzzleOffset.x = -m_vMuzzleOffset.x;
	}

	SetupViewModel();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::CreateFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CViewWeapon::CreateFlash(DFLOAT fScale)
{
	if (!m_pClientDE) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	
	DVector vFlashScale;
	VEC_SET(vFlashScale, fScale, fScale, fScale);

	if (m_szFlashSprite[0] != '\0')
	{
		createStruct.m_ObjectType = OT_SPRITE;
		_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_szFlashSprite);
		createStruct.m_Flags	  = 0;

		m_hFlashSprite = m_pClientDE->CreateObject(&createStruct);
		m_pClientDE->SetObjectScale(m_hFlashSprite, &vFlashScale);
	}

	if (!_mbscmp((const unsigned char*)m_szFlashSprite, (const unsigned char*)m_szAltFlashSprite))
	{
		m_hAltFlashSprite = m_hFlashSprite;
	}
	else if (m_szAltFlashSprite[0] != '\0')
	{
		createStruct.m_ObjectType = OT_SPRITE;
		_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_szAltFlashSprite);
		createStruct.m_Flags	  = 0;

		m_hAltFlashSprite = m_pClientDE->CreateObject(&createStruct);
		m_pClientDE->SetObjectScale(m_hAltFlashSprite, &vFlashScale);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Term()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CViewWeapon::Term()
{
	SendSoundMsg(DNULL, 0x02);

	if (m_hAltFlashSprite && (m_hAltFlashSprite != m_hFlashSprite))
	{
		m_pClientDE->DeleteObject(m_hAltFlashSprite);
		m_hAltFlashSprite = DNULL;
	}

	if (m_hFlashSprite)
	{
		m_pClientDE->DeleteObject(m_hFlashSprite);
		m_hFlashSprite = DNULL;
		m_hAltFlashSprite = DNULL;
	}

	if( m_szPic )
	{
		m_pClientDE->FreeString( m_szPic );
		m_szPic = DNULL;
	}
	if( m_szPicH )
	{
		m_pClientDE->FreeString( m_szPicH );
		m_szPicH = DNULL;
	}

	if (m_hObject)
	{
		m_pClientDE->DeleteObject(m_hObject);
		m_hObject = DNULL;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::AdjustAngleAndSetPos()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CViewWeapon::AdjustAngleAndSetPos(DFLOAT fPitch, DFLOAT fYaw, DVector *pos)
{
	DRotation rotGun;
	DVector vR;
	DVector vU;
	DVector vF;
	DVector myPos;

	if (!m_hObject) return;

	// Compute offset for gun &
	// Move gun to correct position
	m_pClientDE->SetupEuler(&rotGun, fPitch, fYaw, (float)0);
	m_pClientDE->SetObjectRotation(m_hObject, &rotGun);
	ROT_COPY(g_rotGun, rotGun);

	m_pClientDE->GetRotationVectors(&rotGun, &vU, &vR, &vF);

	VEC_COPY(myPos, *pos)
	VEC_MULSCALAR(vU, vU, (m_vViewModelOffset.y + m_fBobHeight));
	VEC_MULSCALAR(vR, vR, (m_vViewModelOffset.x + m_fBobWidth));
	VEC_MULSCALAR(vF, vF, m_vViewModelOffset.z);

	VEC_ADD(myPos, myPos, vU);
	VEC_ADD(myPos, myPos, vR);
	VEC_ADD(myPos, myPos, vF);

	m_pClientDE->SetObjectPos(m_hObject, &myPos);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Update()
//
//	PURPOSE:	Updates the gun's position and animation state.
//
// --------------------------------------------------------------------------- //

void CViewWeapon::Update(DFLOAT fPitch, DFLOAT fYaw, DVector *pos)
{
	DFLOAT cTime = m_pClientDE->GetTime();
	DRotation rot;
	DVector vR;
	DVector vU;
	DVector vF;
	DVector myPos;
	static DFLOAT fFireAnimLength;

	if (!m_hObject) return;

	//******************************************************************
	// Place the flash at the muzzle of the gun if that node exists
	DVector		tempPos;
	DRotation	tempRot;

	ROT_INIT(tempRot);
	VEC_INIT(tempPos);

	m_pClientDE->SetObjectRotation(m_hObject, &tempRot);
	m_pClientDE->SetObjectPos(m_hObject, &tempPos);

	if(m_pClientDE->GetModelNodeTransform(m_hObject, "muzzle", &tempPos, &tempRot))
	{
		VEC_ADD(m_vFlashPos, tempPos, m_vViewModelOffset);
		VEC_MULSCALAR(m_vFlashPos, m_vFlashPos, 10.0f);
	}
	//******************************************************************

	// Compute offset for gun & move gun to correct position
	m_fLowerOffset = 0;
	AdjustAngleAndSetPos(fPitch, fYaw, pos);

	// Now figure out the muzzle position for the flash.
	m_pClientDE->GetObjectPos(m_hObject, &myPos);
	m_pClientDE->SetupEuler(&rot, fPitch, fYaw, (float)0);
	m_pClientDE->GetRotationVectors(&rot, &vU, &vR, &vF);

	VEC_COPY(m_vAdjFlashPos, myPos)
	VEC_MULSCALAR(vU, vU, (m_vFlashPos.y + m_fBobHeight));
	VEC_MULSCALAR(vR, vR, (m_vFlashPos.x + m_fBobWidth));
	VEC_MULSCALAR(vF, vF, m_vFlashPos.z );

	VEC_ADD(m_vAdjFlashPos, m_vAdjFlashPos, vU);
	VEC_ADD(m_vAdjFlashPos, m_vAdjFlashPos, vR);
	VEC_ADD(m_vAdjFlashPos, m_vAdjFlashPos, vF);

	VEC_COPY(m_vMuzzleOffset, m_vAdjFlashPos);

	if (m_nFlashVisible)
		UpdateFlash(m_vAdjFlashPos);

	// Set the object's alpha the same as the client object
	DFLOAT fR, fG, fB, fA;
	m_pClientDE->GetObjectColor(m_pClientDE->GetClientObject(), &fR, &fG, &fB, &fA);
	m_pClientDE->SetObjectColor(m_hObject, 0, 0, 0, fA);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::UpdateBob()
//
//	PURPOSE:	Sets the weapon bob width and height values.
//
// --------------------------------------------------------------------------- //

void CViewWeapon::UpdateBob(DFLOAT fHeight, DFLOAT fWidth)
{
	m_fBobHeight = fHeight;
	m_fBobWidth  = fWidth;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::UpdateFlash()
//
//	PURPOSE:	Updates the muzzle flash sprite position and visibility.
//
// --------------------------------------------------------------------------- //

void CViewWeapon::UpdateFlash(DVector vFlashPos)
{
	if (!m_pClientDE) return;

	DFLOAT fCurTime = m_pClientDE->GetTime();

	if ((fCurTime >= m_fFlashStartTime + m_fFlashDuration) && (fCurTime >= m_fFlashStartTime + m_pClientDE->GetFrameTime() + 0.05))
	{
		m_pClientDE->SetObjectFlags(m_hFlashSprite, 0);
		m_pClientDE->SetObjectFlags(m_hAltFlashSprite, 0);
		m_nFlashVisible = 0;
	}	
	else
	{
		if (m_nFlashVisible == 1)
		{
			m_pClientDE->SetObjectFlags(m_hFlashSprite, FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT);
			m_pClientDE->SetObjectPos(m_hFlashSprite, &vFlashPos);
		}
		else if (m_nFlashVisible == 2)
		{
			m_pClientDE->SetObjectFlags(m_hAltFlashSprite, FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT);
			m_pClientDE->SetObjectPos(m_hAltFlashSprite, &vFlashPos);
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Hide()
//
//	PURPOSE:	Turn off FLAG_VISIBLE for this weapon.
//
// --------------------------------------------------------------------------- //

void CViewWeapon::Hide()
{
	if (!m_hObject) return;
	
	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
	m_pClientDE->SetObjectFlags(m_hObject, (dwFlags & ~FLAG_VISIBLE));

	if (m_nFlashVisible && m_hFlashSprite && m_hAltFlashSprite)
	{
		m_pClientDE->SetObjectFlags(m_hFlashSprite, 0);
		m_pClientDE->SetObjectFlags(m_hAltFlashSprite, 0);
		m_nFlashVisible = 0;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Show()
//
//	PURPOSE:	Turn on FLAG_VISIBLE for this weapon.
//
// --------------------------------------------------------------------------- //

void CViewWeapon::Show()
{
	if (!m_hObject) return;

	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
	m_pClientDE->SetObjectFlags(m_hObject, (dwFlags | FLAG_VISIBLE));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::SendFireMsg
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CViewWeapon::SendFireMsg(DVector* pvPos, DVector* pvFire, DBYTE byRandomSeed, DBOOL bAltFire)
{
	if (!m_pClientDE || !m_hObject) return;

	// Make sure we always ignore the fire sound...
	m_nIgnoreFX = 0;

	// Put alt fire value in top bit...
	DBYTE byFlags = 0;
	if (bAltFire)
		byFlags |= 0x01;
	if (m_bLeftHand)
		byFlags |= 0x02;

	// Send Fire message to server...

	HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(CMSG_WEAPON_FIRE);
	m_pClientDE->WriteToMessageVector(hWrite, pvPos);
	m_pClientDE->WriteToMessageVector(hWrite, pvFire);
	m_pClientDE->WriteToMessageByte(hWrite, byRandomSeed);
	m_pClientDE->WriteToMessageByte(hWrite, (DBYTE)m_nType);

	if(!FireMsgSpecialData(hWrite, byFlags))
		m_pClientDE->WriteToMessageByte(hWrite, byFlags);

	m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::IsIdle()
//
//	PURPOSE:	Terminates this weapon
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeapon::IsChanging()
{
	if((m_eState == WS_DRAW) || (m_eState == WS_HOLSTER))
		return	DTRUE;
	else
		return	DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Draw()
//
//	PURPOSE:	draws the weapon, sets filenames
//
// ----------------------------------------------------------------------- //

void CViewWeapon::Draw()
{ 
	m_eState = WS_DRAW; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::CheckAmmo()
//
//	PURPOSE:	returns amount of ammo available to this weapon
//
// ----------------------------------------------------------------------- //

DFLOAT CViewWeapon::CheckAmmo(DBOOL bAltFire)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return 0;

    // If the No Ammo is required for the Weapon then return 1
    if (GetAmmoType(bAltFire) == AMMO_NONE)
    {
        return 1.0f;
    }

	if (bAltFire)
		return (DFLOAT)pShell->GetAltAmmo();
	else
		return (DFLOAT)pShell->GetAmmo();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::PlayFireSound()
//
//	PURPOSE:	Plays the firing sound
//
// ----------------------------------------------------------------------- //

HSOUNDDE CViewWeapon::PlayFireSound(DBOOL bAltFire)
{
	HSOUNDDE hSound = DNULL;
/*  TODO
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
		SoundType sndtype = IsOwnerAPlayer() ? SOUNDTYPE_PLAYER : SOUNDTYPE_AI;

		hSound = PlaySoundFromObject(m_hOwner, sound, Radius, sndtype, SOUNDPRIORITY_HIGH, bLoop, DTRUE, DTRUE);

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
			m_pClientDE->GetSoundDuration(hSound, &fTime);
			fTime += m_pClientDE->GetTime();

			if (m_hCurFireSounds[nIndex])
				m_pClientDE->KillSound(m_hCurFireSounds[nIndex]);

			m_hCurFireSounds[nIndex] = hSound;
			m_fCurFireSoundsEndTime[nIndex] = fTime;

		}
		else
		{
			m_pClientDE->KillSound(hSound);
			hSound = DNULL;
		}
	}
*/
	return hSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::PlayEmptyWeaponSound()
//
//	PURPOSE:	Plays the firing sound
//
// ----------------------------------------------------------------------- //

HSOUNDDE CViewWeapon::PlayEmptyWeaponSound(DBOOL bAltFire)
{
	HSOUNDDE hSound = DNULL;
/*
	char *sound;
	if (bAltFire)
		sound = m_szAltEmptyWeaponSound;
	else
		sound = m_szEmptyWeaponSound;

//	if (pWeap->m_nAmmo > 0) sound = m_FireSound;

	if (_mbstrlen(sound) > 0)
	{
		DFLOAT Radius = 800.0f;
		SoundType sndtype = IsOwnerAPlayer() ? SOUNDTYPE_PLAYER : SOUNDTYPE_AI;

		hSound = PlaySoundFromObject(m_hOwner, sound, Radius, sndtype, SOUNDPRIORITY_HIGH);
	}
*/
	return hSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::FireVector()
//
//	PURPOSE:	Fires a vector for vector weapons
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeapon::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	if (!m_pClientDE)		return DFALSE;

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

	SurfaceType	eType	= SURFTYPE_UNKNOWN;

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::FireProjectile()
//
//	PURPOSE:	Fires a projectile
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeapon::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	if (!m_pClientDE)
		return DFALSE;

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DDWORD CViewWeapon::Fire()
{
	if (!m_pClientDE) return 0;

	//***** General variable declarations *****//
	DFLOAT		fReloadTime;
	DBYTE		nFiredType = 0;
	DFLOAT		cTime = m_pClientDE->GetTime();
	DFLOAT		range;
	Spread		*spread;
	DDWORD		dwShotsPerFire;
	D_WORD		nAmmoUse;
	DBOOL		bHitPlayerObj = DFALSE;


	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if( !pShell )
		return 0;

	// Can't fire while binocs active.
	if( pShell->BinocularsActive( ))
		return 0;

	// Calculate a random seed...(srand uses this value so it can't be 1, since
	// that has a special meaning for srand)
	g_byRandomWeaponSeed = GetRandom(2, 255);
	srand(g_byRandomWeaponSeed);

	// Reload time depends on current barrel or alt fire
	if(m_bLastFireAlt)		fReloadTime = m_fAltReloadTime;
		else				fReloadTime = m_fReloadTime;

	// See if any reload time modifiers are in effect
	fReloadTime *= 1;	// TODO m_pInventoryMgr->GetFireRateMultiplier();

	// Set up varaible for primary or alternate firing states
	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	if(!bAltFire)
	{
		dwShotsPerFire = m_dwShotsPerFire;
		spread = &m_Spread;
		range = m_fRange;
		m_fDamage = GetRandom(m_fMinDamage, m_fMaxDamage);
		nAmmoUse = m_nAmmoUse;
	}
	else
	{
		dwShotsPerFire = m_dwAltShotsPerFire;
		spread = &m_AltSpread;
		range = m_fAltRange;
		m_fDamage = GetRandom(m_fMinAltDamage, m_fMaxAltDamage);
		nAmmoUse = m_nAltAmmoUse;
	}

	// Only fire ammo if we have the ammo
	DFLOAT fAmmoCount = CheckAmmo(bAltFire);
	if (fAmmoCount >= nAmmoUse)
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
	}
	else
		dwShotsPerFire = 0;

	//***** If no shots are available, play empty sound and exit the function *****//
	if(dwShotsPerFire <= 0)
	{
		PlayEmptyWeaponSound(bAltFire);
		return 0;	// Don't fire if we don't have ammo
	}

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

	// Reset the shell eject flag for the next fire..
	if(m_nAmmoType == AMMO_BULLET || m_nAmmoType == AMMO_SHELL)
		m_bEjectShell = DTRUE;
	else
		m_bEjectShell = DFALSE;

	m_nFiredType = nFiredType;
	if (((m_nFiredType == 1) && m_hFlashSprite) || ((m_nFiredType == 2) && m_hAltFlashSprite))
	{
		m_nFlashVisible = (DBYTE)m_nFiredType;
		m_fFlashStartTime = m_pClientDE->GetTime();
	}

	return nFiredType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::FireSpread()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

void CViewWeapon::FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir)
{
	if (!m_pClientDE) return;

	DVector		vTmp;
	DVector		vU, vR, vF;
	m_pClientDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF); 

	// Check to see if the player is standing too close to something, and we need
	// to move the firing position back in order to hit it
	if(FiringTooClose(&vF, 60.0f, &vTmp))
		{ VEC_COPY(m_vPosition, vTmp); }

	// Send the fire message to the client..
	SendFireMsg(&m_vPosition, &vF, g_byRandomWeaponSeed, bAltFire);

	VEC_COPY(*rDir, vR);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::FiringTooClose
//
//	PURPOSE:	Trick to improve accuracy and keep the effect at the weapon end
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeapon::FiringTooClose(DVector *vFire, DFLOAT fDist, DVector *vNewPos)
{
	//***** Make sure the server and inventory systems are valid *****//
	if (!m_pClientDE || !m_hObject) return DFALSE;

	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	DFLOAT fOffset = pShell->GetEyeLevel();

	DVector	vOffset;
	VEC_SET(vOffset, 0.0f, fOffset, 0.0f);

	// Setup the vectors for the intersect
	m_pClientDE->GetObjectPos(m_pClientDE->GetClientObject(), vNewPos);
	VEC_ADD(*vNewPos, *vNewPos, vOffset);

	VEC_NORM(*vFire);
	VEC_MULSCALAR(*vFire, *vFire, fDist);

	IntersectQuery	iq;
	IntersectInfo	ii;

	// Set the intersection query values
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
//	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, *vNewPos);
	VEC_ADD(iq.m_To, iq.m_From, *vFire);
	VEC_NORM(*vFire);

	if(m_pClientDE->IntersectSegment(&iq, &ii))
		return DTRUE;

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::SetFirePosRot
//
//	PURPOSE:	Update the firing state of the gun, keep track of muzzle
//				position & rotation (used by AIs)
//
// ----------------------------------------------------------------------- //

void CViewWeapon::SetFirePosRot(DVector *firedPos, DRotation *rotP, DBOOL bAltFire)
{
	VEC_COPY(m_vPosition, *firedPos);
	ROT_COPY(m_rRotation, *rotP);

	m_eState = bAltFire ? WS_ALT_FIRING : WS_FIRING;
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::UpdateFiringState
//
//	PURPOSE:	Update the firing state of the gun, keep track of muzzle
//				position & rotation (used by player objects)
//
// ----------------------------------------------------------------------- //

void CViewWeapon::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	if (!m_pClientDE) return;

	VEC_COPY(m_vPosition, *firedPos);
	ROT_COPY(m_rRotation, *rotP);

	DFLOAT fTime = m_pClientDE->GetTime();
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
					m_fIdleDelay = GetRandom(m_fMinIdleDelay, m_fMaxIdleDelay);

					m_pClientDE->SetModelLooping(m_hObject, m_bLoopStatic);
				}
			}

		case WS_REST:
			{
				DFLOAT fReloadTime;
				if (m_bLastFireAlt)
					fReloadTime = m_fAltReloadTime;
				else
					fReloadTime = m_fReloadTime;

				fReloadTime *= 1;	// m_pInventoryMgr->GetFireRateMultiplier();

				// See if we should play an idle animation
				if((m_pClientDE->GetTime() - m_fIdleStartTime) > m_fIdleDelay)
				{
					m_pClientDE->SetModelLooping(m_hObject, DFALSE);

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
							m_pClientDE->SetModelLooping(m_hObject, m_bLoopAnim);

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
							m_pClientDE->SetModelLooping(m_hObject, m_bAltLoopAnim);

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

					m_pClientDE->SetModelLooping(m_hObject, m_bLoopStatic);
					m_fIdleStartTime = fTime;
				}
			}
			break;

		case WS_HOLSTER:
			{
				if(!PlayAnimation(m_nHolsterAnim))
				{	
					m_eState = WS_HOLSTERED;

					m_pClientDE->SetModelLooping(m_hObject, DFALSE);
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
						m_pClientDE->SetModelLooping(m_hObject, DTRUE);
					else
						m_pClientDE->SetModelLooping(m_hObject, DFALSE);

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
				DDWORD nAmmoUse;
				DFLOAT fAmmo;
				WeaponState eNextState;

				if (bAltFire)
				{
					bLoopAnim	= m_bAltLoopAnim;
					nFireAnim	= m_nAltFireAnim;
					bNowFiring	= bAltFiring;
					fReloadTime = m_fAltReloadTime;
					eNextState	= WS_STOP_ALT_FIRING;
					nAmmoUse	= m_nAltAmmoUse;
					fAmmo		= CheckAmmo(DTRUE);
				}
				else
				{
					bLoopAnim	= m_bLoopAnim;
					nFireAnim	= m_nFireAnim;
					bNowFiring	= bFiring;
					fReloadTime = m_fReloadTime;
					eNextState	= WS_STOP_FIRING;
					nAmmoUse	= m_nAmmoUse;
					fAmmo		= CheckAmmo(DFALSE);
				}

				// No fire anim, so fire right away
				if (nFireAnim == INVALID_ANI && !bLoopAnim)
				{
					if(fAmmo >= nAmmoUse)
						Fire();

					m_eState = eNextState;
				}
				// Else looping anim, so fire based on rate of fire
				else if (bLoopAnim)
				{
					if (!bNowFiring || (fAmmo < nAmmoUse))
					{
						m_eState = eNextState;
						m_pClientDE->SetModelLooping(m_hObject, DFALSE);
					}
				}
				else if (m_bSemiAuto && !bAltFire)
				{
					// Stopped firing
					if (!m_bLastFiring && bNowFiring && (fAmmo >= nAmmoUse))
					{
						m_pClientDE->ResetModelAnimation(m_hObject);
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
				/// TODO: This may need to be copied from the server? GK
//				else if (m_pInventoryMgr->GetFireRateMultiplier() < 1.0f)
//				{
//					if (!m_bLastFiring && bNowFiring && (fTime - m_fLastShotTime) > fReloadTime * m_pInventoryMgr->GetFireRateMultiplier())
//						m_pClientDE->ResetModelAnimation(m_hObject);
//				}
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

					m_pClientDE->SetModelLooping(m_hObject, m_bLoopStatic);
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
//	ROUTINE:	CViewWeapon::CancelFiringState
//
//	PURPOSE:	Cancel the current weapon state and reset
//
// ----------------------------------------------------------------------- //

void CViewWeapon::CancelFiringState()
{
	if (!m_pClientDE) return;

	m_pClientDE->ResetModelAnimation(m_hObject);
	PlayAnimation(m_nRestAnim);
	m_pClientDE->ResetModelAnimation(m_hObject);
	m_eState = WS_REST;

	SendSoundMsg(DNULL, 0x02);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::SetupViewModel
//
//	PURPOSE:	Sets the model filenames and animations.
//
// ----------------------------------------------------------------------- //

void CViewWeapon::SetupViewModel()
{
	if (!m_pClientDE) return;

	CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
	HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();

	// Choose the right or lefthanded model
	char	*pModelFilename;
	if(m_bLeftHand)		pModelFilename = m_pLeftViewModelFilename;
		else			pModelFilename = m_pViewModelFilename;

	if(pModelFilename && m_pViewModelSkinname)
	{
		char	firstChar = 0;
		char	start[] = "C_";
		char	*change = DNULL;

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		// Get the character to use for the skin

#ifdef _ADDON
		DBOOL bEnemyChar = DFALSE;
		if (pShell->GetCharacter() >= CHARACTER_M_CULTIST)
		{
			bEnemyChar = DTRUE;
		}
#endif
		switch(pShell->GetCharacter())
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

		// Copy over the filenames so we preserver the originals
		strcpy(createStruct.m_Filename, pModelFilename);
		strcpy(createStruct.m_SkinName, m_pViewModelSkinname);

		strupr(createStruct.m_Filename);	// [blg]
		strupr(createStruct.m_SkinName);

		createStruct.m_Flags = FLAG_MODELGOURAUDSHADE | FLAG_REALLYCLOSE | FLAG_VISIBLE | m_nFlags;

		// Change the weapon skin to the correct character
		change = strstr(createStruct.m_SkinName, start);
		if (change) change[0] = firstChar;

		// Change the knife model to the correct character
		if(m_nType == WEAP_MELEE)
		{
			change = strstr(createStruct.m_Filename, start);
			if (change) change[0] = firstChar;
		}

		// Change to the generic orb skin if necessary

#ifdef _ADDON
		if (m_nType == WEAP_ORB && bEnemyChar)
		{
			strcpy(createStruct.m_SkinName, "skins_ao\\weapons_ao\\generic_orb_pv_t.dtx");
		}
#endif

		// Set the remaining info
		change = DNULL;
		createStruct.m_ObjectType = OT_MODEL;
		m_pClientDE->GetObjectPos(hPlayerObj, &createStruct.m_Pos);

		// Delete the object to avoid duplicates
/*		if(m_hObject)
		{
			m_pClientDE->DeleteObject(m_hObject);
			m_hObject = DNULL;
		}
*/
		m_hObject = m_pClientDE->CreateObject(&createStruct);

		if(m_nFlags & FLAG_ENVIRONMENTMAP)
			m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fChromeValue);
		else
			m_pClientDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, 1.0f);

		m_pClientDE->SetObjectClientFlags(m_hObject, CF_NOTIFYMODELKEYS);

		// Set the new model filenames
//		m_pClientDE->SetObjectFilenames(m_hObject, model, skin);

		m_pClientDE->SetModelLooping(m_hObject, DFALSE);
		m_pClientDE->SetModelAnimation(m_hObject, -1);

		// Tell it that this weapon this is it's parent for now.

		m_nRestAnim			= m_pClientDE->GetAnimIndex(m_hObject, "static_model");
		m_nIdleAnim			= m_pClientDE->GetAnimIndex(m_hObject, "idle");
		m_nDrawAnim			= m_pClientDE->GetAnimIndex(m_hObject, "draw");
		m_nDrawDuelAnim		= m_pClientDE->GetAnimIndex(m_hObject, "dh_draw");
		m_nHolsterAnim		= m_pClientDE->GetAnimIndex(m_hObject, "holster");
		m_nHolsterDuelAnim	= m_pClientDE->GetAnimIndex(m_hObject, "dh_holster");
		m_nStartFireAnim	= m_pClientDE->GetAnimIndex(m_hObject, "start_fire");
		m_nFireAnim			= m_pClientDE->GetAnimIndex(m_hObject, "fire");
		m_nStopFireAnim		= m_pClientDE->GetAnimIndex(m_hObject, "end_fire");
		m_nStartAltFireAnim	= m_pClientDE->GetAnimIndex(m_hObject, "start_alt_fire");
		m_nAltFireAnim		= m_pClientDE->GetAnimIndex(m_hObject, "alt_fire");
		m_nStopAltFireAnim	= m_pClientDE->GetAnimIndex(m_hObject, "end_alt_fire");

		if (m_nIdleAnim == INVALID_ANI)
			m_nIdleAnim = m_nRestAnim;

		if (m_nAltFireAnim == INVALID_ANI)
		{
			m_nStartAltFireAnim = m_nStartFireAnim;
			m_nAltFireAnim = m_nFireAnim;
			m_nStopAltFireAnim = m_nStopFireAnim;
		}

		PlayAnimation(m_nDrawAnim);
		m_fIdleDelay = GetRandom(m_fMinIdleDelay, m_fMaxIdleDelay);
		m_fIdleStartTime = m_pClientDE->GetTime();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::PlayAnimation()
//
//	PURPOSE:	Set model to new animation
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeapon::PlayAnimation(DDWORD dwNewAni)
{
	if (!m_pClientDE || dwNewAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= m_pClientDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= m_pClientDE->GetModelPlaybackState(m_hObject);

	if (dwAni == dwNewAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != dwNewAni)
	{
		m_pClientDE->SetModelAnimation(m_hObject, dwNewAni);
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::OnModelKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CViewWeapon::OnModelKey(HLOCALOBJ hObj, ArgList* pArgList)
{
	char*	pKey;
	int		count = pArgList->argc;

	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();

	for(int i = 0; i < count; i++)
	{
		pKey = pArgList->argv[i];
		if(!pKey) break;

		if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_FIRE) == 0)
		{
			Fire();
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUND) == 0)
		{
			char*	pSound = pArgList->argv[++i];
			char	pDir[100];

			if(pSound)
			{
				_mbscpy((unsigned char*)pDir, (const unsigned char*)"Sounds\\Weapons\\");
				_mbscat((unsigned char*)pDir, (const unsigned char*)pSound);

				SendSoundMsg(pDir, 0x00);
//				PlaySoundFromObject(m_hObject, pDir, 1000, SOUNDPRIORITY_PLAYER_HIGH,
//					DFALSE, DFALSE, DFALSE, 100 );
			}
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUNDLOOP) == 0)
		{
			char*	pSound = pArgList->argv[++i];
			char	pDir[100];

			if(pSound && !m_hLoopSound)
			{
				_mbscpy((unsigned char*)pDir, (const unsigned char*)"Sounds\\Weapons\\");
				_mbscat((unsigned char*)pDir, (const unsigned char*)pSound);

				SendSoundMsg(pDir, 0x01);
//				m_hLoopSound = PlaySoundFromObject(m_hObject, pDir, 1000, SOUNDPRIORITY_PLAYER_HIGH, DTRUE, DTRUE);
			}
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUNDSTOP) == 0)
		{
			SendSoundMsg(DNULL, 0x02);

//			if(m_hLoopSound)
//				m_pClientDE->KillSound(m_hLoopSound);
//			m_hLoopSound = DNULL;
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_HIDE) == 0)
		{
			char*	pNode = pArgList->argv[++i];
			DBOOL	nodeStat;
			m_pClientDE->GetModelNodeHideStatus(m_hObject, pNode, &nodeStat);

			if(!nodeStat)
				m_pClientDE->SetModelNodeHideStatus(m_hObject, pNode, DTRUE);
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SHOW) == 0)
		{
			char*	pNode = pArgList->argv[++i];
			DBOOL	nodeStat;
			m_pClientDE->GetModelNodeHideStatus(m_hObject, pNode, &nodeStat);

			if(nodeStat)
				m_pClientDE->SetModelNodeHideStatus(m_hObject, pNode, DFALSE);
		}
	}
	/*
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1)
		{
			char* pSound = pArgList->argv[1];
			if (pSound)
			{
				char buf[100];
				sprintf(buf,"Sounds\\Weapons\\%s.wav", pSound);

				PlaySoundLocal(buf, SOUNDPRIORITY_PLAYER_HIGH, DFALSE, DFALSE, 100, DTRUE );

				// Send message to Server so that other client's can hear this sound...

				HSTRING hSound = m_pClientDE->CreateString(buf);

				HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(CMSG_WEAPON_SOUND);
				m_pClientDE->WriteToMessageByte(hWrite, WEAPON_SOUND_KEY);
				m_pClientDE->WriteToMessageByte(hWrite, m_nWeaponId);
				m_pClientDE->WriteToMessageVector(hWrite, &m_vFlashPos);
				m_pClientDE->WriteToMessageHString(hWrite, hSound);
				m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);

				m_pClientDE->FreeString(hSound);
			}
		}
	}
	*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeapon::SendSoundMsg
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CViewWeapon::SendSoundMsg(char *szSound, DBYTE byFlags)
{
	if (!m_pClientDE) return;

	HSTRING	sSound = m_pClientDE->CreateString(szSound);

	// Send Sound message to server...
	HMESSAGEWRITE hWrite = m_pClientDE->StartMessage(CMSG_WEAPON_SOUND);
	m_pClientDE->WriteToMessageByte(hWrite, byFlags);

	if(byFlags != 0x02)
		m_pClientDE->WriteToMessageHString(hWrite, sSound);

	m_pClientDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST|MESSAGE_GUARANTEED);

	if(sSound)	m_pClientDE->FreeString(sSound);
}
