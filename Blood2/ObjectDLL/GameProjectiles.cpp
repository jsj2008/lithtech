// ----------------------------------------------------------------------- //
//
// MODULE  : GameProjectile.cpp
//
// PURPOSE : Game Projectile classs - implementation
//
// CREATED : 10/3/97
//
// ----------------------------------------------------------------------- //

#include "GameProjectiles.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "ObjectUtilities.h"
#include "ClientSparksSFX.h"
#include "VolumeBrushTypes.h"
#include "Gib.h"
#include "ClientServerShared.h"
#include "ClientExplosionSFX.h"
#include "SFXMsgIds.h"
#include "destructable.h"
#include "PlayerObj.h"

#include "zealotai.h"

#include "stdio.h"
void BPrint(char*);

BEGIN_CLASS(CGrenade)
END_CLASS_DEFAULT_FLAGS(CGrenade, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CGrenade()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenade::CGrenade() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fLifeTime				= 3.0f;

	m_pProjectileFilename	= "Models\\Ammo\\Grenade.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\Grenade.dtx";

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;

	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 0.0f);
	VEC_SET(m_vShockwaveScaleMax, 2.0f, 2.0f, 0.0f);

	m_dwFlags = m_dwFlags | FLAG_GRAVITY;

	// Set up angular velocities
	m_fPitchVel = m_fYawVel = 0.0f;
	m_fPitch	= m_fYaw = 0.0f;
	m_nBounceCount = 5;

	m_hSound		= 0;

	m_dwTrailFXID	= OBJFX_SMOKETRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CGrenade::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\projectile.wav", 600.0f, 
			SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	// Save position for bounce calculations
	pServerDE->GetObjectPos(m_hObject, &m_LastPos);
	DBOOL bRet = CProjectile::Update(pMovement);

	DVector vVel;
	DRotation rRot;

	pServerDE->GetVelocity( m_hObject ,&vVel );
	pServerDE->GetObjectRotation( m_hObject, &rRot );

	// If velocity slows enough, just stop bouncing and just wait to expire.
	if (VEC_MAG(vVel) < 5.0)
	{
		m_fPitchVel = 0;
		m_fYawVel = 0;

		// Stop the spinning
		pServerDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	// We've expired, explode now if we're a grenade.
	if (!bRet && m_dwFlags & FLAG_GRAVITY || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		Explode();
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CGrenade::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if ((m_nBounceCount == 5) && pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
		if(pObj->IsDead())	return;

		CProjectile::HandleTouch(hObj);	// Explode on impact type.
	}
	else  // Gravity on, so it's a regular grenade.. bounce it off of stuff
	{
		if (hObj == m_hFiredFrom) return; // Let it get out of our bounding box...

		// return if it hit a non solid object
		if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
			return;

		if (m_nBounceCount <= 0) return;

 		// Cast a ray from our last known position to see what we hit
		DVector vVel, vPos;

		pServerDE->GetVelocity(m_hObject, &vVel);
		pServerDE->GetObjectPos( m_hObject, &vPos );

		CollisionInfo colInfo;
		pServerDE->GetLastCollision( &colInfo );

		// Compute new velocity reflected off of the surface.
		DVector vNormal;
		if( colInfo.m_hPoly )
		{
			VEC_COPY(vNormal, colInfo.m_Plane.m_Normal);

			DFLOAT r = ( VEC_DOT(vVel, vNormal) * 0.3f );
			VEC_MULSCALAR(vNormal, vNormal, r);

			// Play a bounce sound...
			PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\altbounce.wav", 750, SOUNDPRIORITY_MISC_MEDIUM);

			VEC_SUB(vVel, vVel, vNormal);

			// Adjust the bouncing..
			m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
			m_fYawVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

			VEC_MULSCALAR(vVel, vVel, 0.5f);	// Lose some energy in the bounce.
			pServerDE->SetVelocity(m_hObject, &vVel);

		}
		else
		{
			VEC_INIT( vVel );
			pServerDE->SetVelocity( m_hObject, &vVel );

			// Play a bounce sound...
			PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\altbounce.wav", 750, SOUNDPRIORITY_MISC_MEDIUM);

		}
		m_nBounceCount--;
	}
}

#ifdef _ADD_ON

BEGIN_CLASS(CGasGrenade)
END_CLASS_DEFAULT_FLAGS(CGasGrenade, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGasGrenade::CGasGrenade()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGasGrenade::CGasGrenade() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fLifeTime				= 3.0f;

	m_pProjectileFilename	= "Models\\Ammo\\Grenade.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\Grenade.dtx";

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;

	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 0.0f);
	VEC_SET(m_vShockwaveScaleMax, 2.0f, 2.0f, 0.0f);

	m_dwFlags = m_dwFlags | FLAG_GRAVITY;

	// Set up angular velocities
	m_fPitchVel = m_fYawVel = 0.0f;
	m_fPitch	= m_fYaw = 0.0f;
	m_nBounceCount	= 5;

	m_bSmoking			= DFALSE;
	m_fSmokeTime		= 0.0f;
	m_hSound			= 0;

	m_dwTrailFXID		= OBJFX_GREEN_SMOKE;
	m_fLastWonkyTime	= 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGasGrenade::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CGasGrenade::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Play a sound if we don't have one playing already
	if(!m_hSound && !m_bSmoking)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\projectile.wav", 600.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	// See if we should blow up yet...
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	// Damage anyone who gets close to the smoke and give them wonky vision
	// **** NOTE: THIS MUST BE BEFORE THE NEXT TWO CHECKS ****
	if(m_bSmoking)
	{
		DFLOAT fTime = pServerDE->GetTime();

		if(fTime > (m_fSmokeTime + GAS_GRENADE_SMOKE_TIME))
			return DFALSE;

		if(fTime > (m_fLastWonkyTime + GAS_GRENADE_DAMAGE_DELAY))
		{
			m_fLastWonkyTime = fTime;
			DamageAndWonky(WONKY_VISION_DEFAULT_TIME);
		}

		return DTRUE;
	}

	if((m_fLifeTime > 0) && (pServerDE->GetTime() > (m_fStartTime + m_fLifeTime)))
		m_bExplode = DTRUE;

	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		m_bSmoking = DTRUE;
		m_fSmokeTime = pServerDE->GetTime();
		Explode();

		// Turn off the gravity for the object and make it invisible
		DDWORD	dwFlags = pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~(FLAG_GRAVITY | FLAG_VISIBLE);
		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		// Stop the object in its current position
		DVector vVel;
		VEC_INIT(vVel);
		pServerDE->SetVelocity( m_hObject, &vVel );
	}

	// Save position for bounce calculations
	pServerDE->GetObjectPos(m_hObject, &m_LastPos);
	DBOOL bRet = CProjectile::Update(pMovement);

	DVector vVel;
	DRotation rRot;

	pServerDE->GetVelocity( m_hObject ,&vVel );
	pServerDE->GetObjectRotation( m_hObject, &rRot );

	// If velocity slows enough, just stop bouncing and just wait to expire.
	if (VEC_MAG(vVel) < 5.0)
	{
		m_fPitchVel = 0;
		m_fYawVel = 0;

		// Stop the spinning
		pServerDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGasGrenade::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CGasGrenade::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (hObj == m_hFiredFrom) return; // Let it get out of our bounding box...

	// return if it hit a non solid object
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// See if we hit the sky...
	HCLASS hClassObj	= pServerDE->GetObjectClass(hObj);
	HCLASS hClassWorld  = pServerDE->GetObjectClass(pServerDE->GetWorldObject());
	if (pServerDE->IsKindOf(hClassObj, hClassWorld))
	{
		CollisionInfo info;
		pServerDE->GetLastCollision(&info);

		SurfaceType eType = GetSurfaceType(info.m_hObject, info.m_hPoly);

		if (eType == SURFTYPE_SKY)
		{
			pServerDE->RemoveObject(m_hObject);
			return;
		}
	}

	// Handle the special case of directly hitting a base character in mid air
	if ((m_nBounceCount == 5) && pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
		if(pObj->IsDead())	return;

		m_bExplode = DTRUE;
		m_hHitObject = hObj;
	}
	else  // Gravity on, so it's a regular grenade.. bounce it off of stuff
	{
		if (m_nBounceCount <= 0) return;

 		// Cast a ray from our last known position to see what we hit
		DVector vVel, vPos;

		pServerDE->GetVelocity(m_hObject, &vVel);
		pServerDE->GetObjectPos( m_hObject, &vPos );

		CollisionInfo colInfo;
		pServerDE->GetLastCollision( &colInfo );

		// Compute new velocity reflected off of the surface.
		DVector vNormal;
		if( colInfo.m_hPoly )
		{
			VEC_COPY(vNormal, colInfo.m_Plane.m_Normal);

			DFLOAT r = ( VEC_DOT(vVel, vNormal) * 0.3f );
			VEC_MULSCALAR(vNormal, vNormal, r);

			// Play a bounce sound...
			PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\altbounce.wav", 750, SOUNDPRIORITY_MISC_MEDIUM);

			VEC_SUB(vVel, vVel, vNormal);

			// Adjust the bouncing..
			m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
			m_fYawVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

			VEC_MULSCALAR(vVel, vVel, 0.5f);	// Lose some energy in the bounce.
			pServerDE->SetVelocity(m_hObject, &vVel);

		}
		else
		{
			VEC_INIT( vVel );
			pServerDE->SetVelocity( m_hObject, &vVel );

			// Play a bounce sound...
			PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\assault\\altbounce.wav", 750, SOUNDPRIORITY_MISC_MEDIUM);

		}

		m_nBounceCount--;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGasGrenade::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CGasGrenade::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_GAS_GRENADE;
	DVector		vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vUp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGasGrenade::DamageAndWonky()
//
//	PURPOSE:	Damage people in the gas and give them wonky vision
//
// ----------------------------------------------------------------------- //

void CGasGrenade::DamageAndWonky(DFLOAT fWonkyTime)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return;

	HOBJECT		hObj;
	HCLASS		hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS		hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS		hObjClass   = DNULL;

	// Get a forward vector and position of the player who shot the voodoo doll
	DVector		vGrenadePos;
	pServerDE->GetObjectPos(m_hObject, &vGrenadePos);

	// Go through the list of destructable objects and look for people within your FOV
	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;
		hObjClass = pServerDE->GetObjectClass(hObj);

		if(pServerDE->IsKindOf(hObjClass, hCharacter))
		{
			DVector		vObjPos, vDir;
			DFLOAT		fDist, fNewDamage;

			// Test the angle between this object and us
			pServerDE->GetObjectPos(hObj, &vObjPos);
			VEC_SUB(vDir, vObjPos, vGrenadePos);
			fDist = VEC_MAG(vDir);

			// If the target is too far away... skip it
			if(fDist > GAS_GRENADE_DAMAGE_DIST)
				{ pLink = pLink->m_pNext; continue; }

			fNewDamage = GAS_GRENADE_MAX_DAMAGE - (GAS_GRENADE_MAX_DAMAGE * (fDist / GAS_GRENADE_DAMAGE_DIST));

			DamageObject(m_hFiredFrom, this, hObj, fNewDamage, vDir, vObjPos, DAMAGE_TYPE_NORMAL);

			if(pServerDE->IsKindOf(hObjClass, hPlayer))
			{
		    	CPlayerObj *pHit = (CPlayerObj*)pServerDE->HandleToObject(hObj);

				HMESSAGEWRITE hMsg = pServerDE->StartMessage(pHit->GetClient(), SMSG_WONKYVISION);
				pServerDE->WriteToMessageFloat(hMsg, fWonkyTime);
				pServerDE->WriteToMessageByte(hMsg, 0);	// bNoMove = DFALSE
				pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
			}
		}

		pLink = pLink->m_pNext;
	}
}

#endif

#ifndef _DEMO

BEGIN_CLASS(CTimeBomb)
END_CLASS_DEFAULT_FLAGS(CTimeBomb, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTimeBomb::CTimeBomb()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTimeBomb::CTimeBomb() : CProjectile()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;

	m_pProjectileFilename	= "Models\\Ammo\\c4.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\c4_time.dtx";

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;

	m_dwFlags				= m_dwFlags | FLAG_GRAVITY;
	m_bArmed				= DFALSE;
	m_bExplode				= DFALSE;
	m_fStartTime			= 0.0f;
	m_fTimeDelay			= 5.0f;

	// Set up angular velocities
	m_fPitch				= 0.0f;
	m_fYaw					= 0.0f;
	m_fPitchVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
	m_fYawVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

	m_damage.SetDeathDelay(0.25f);		// .25 second delay if killed by explosion
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTimeBomb::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CTimeBomb::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->GetObjectPos(m_hObject, &m_LastPos);

	if(m_bExplode || m_damage.IsDead())
	{
		Explode();
		return DFALSE;
	}

	if(m_bArmed)
	{
		DVector		vVel;
		VEC_INIT(vVel);
		pServerDE->SetVelocity(m_hObject, &vVel);

		// Count down to explode time
		if (fTime > m_fStartTime + m_fTimeDelay)
			m_bExplode = DTRUE;
	}
	else
	{
		DRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);

		// Rotate the object as it flys through the air
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTimeBomb::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CTimeBomb::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_bArmed)
		return;

	if(hObj == m_hFiredFrom)
		return;

	if(hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID))
		return;

	if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
		if(pObj->IsDead())	return;

		CProjectile::HandleTouch(hObj);	// Explode on impact type.
	}
	else
	{
		CollisionInfo colInfo;
		pServerDE->GetLastCollision( &colInfo );

		DVector		vU, vR, vF;
		DRotation	rRot;

		VEC_SET(vU, 0.0f, 1.0f, 0.0f);

		pServerDE->AlignRotation(&rRot, &colInfo.m_Plane.m_Normal, &vU);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		pServerDE->RotateAroundAxis(&rRot, &vR, MATH_PI * 0.5f);
		pServerDE->SetObjectRotation(m_hObject, &rRot);

		DDWORD	dwFlags = pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		m_bArmed = DTRUE;
		m_fStartTime = pServerDE->GetTime();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTimeBomb::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CTimeBomb::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_DEFAULT_SMALL;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}




BEGIN_CLASS(CProximityBomb)
END_CLASS_DEFAULT_FLAGS(CProximityBomb, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProximityBomb::CProximityBomb()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProximityBomb::CProximityBomb() : CProjectile()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;

	m_pProjectileFilename	= "Models\\Ammo\\c4.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\c4_prox.dtx";

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;

	m_dwFlags				= m_dwFlags | FLAG_GRAVITY;
	m_bArmed				= DFALSE;
	m_bExplode				= DFALSE;
	m_fStartTime			= 0.0f;

	m_fDetectRadius			= 225.0f;
	m_fGetAwayTime			= 0.75f;
	m_fExplodeDelay			= 0.5f;
	m_bHitCharacter			= DFALSE;
	m_bPlayArmSound			= DTRUE;
	m_bPlayDetectSound		= DTRUE;

	// Set up angular velocities
	m_fPitch				= 0.0f;
	m_fYaw					= 0.0f;
	m_fPitchVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
	m_fYawVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

	m_damage.SetDeathDelay(0.25f);		// .25 second delay if killed by explosion
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProximityBomb::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CProximityBomb::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->GetObjectPos(m_hObject, &m_LastPos);

	if(m_bExplode || m_damage.IsDead())
	{
		if(!m_bHitCharacter && (pServerDE->GetTime() < m_fStartTime + m_fExplodeDelay) && !m_damage.IsDead())
		{
			if(m_bPlayDetectSound)
			{
				PlaySoundFromPos(&m_LastPos, "Sounds\\Weapons\\proximities\\detect.wav", 500.0f, SOUNDPRIORITY_MISC_MEDIUM);
				m_bPlayDetectSound = DFALSE;
			}

			return DTRUE;
		}
		else
		{
			Explode();
			return DFALSE;
		}
	}

	if(m_bArmed)
	{
		DVector		vVel;
		VEC_INIT(vVel);
		pServerDE->SetVelocity(m_hObject, &vVel);

		if(m_hFiredFrom)
		{
			HCLASS hClass = pServerDE->GetObjectClass(m_hFiredFrom);
			if(pServerDE->IsKindOf(hClass, pServerDE->GetClass("AI_Mgr")))
				m_fGetAwayTime = 2.0f;
		}

		// Wait a little while before going into detect mode
		if(pServerDE->GetTime() < m_fStartTime + m_fGetAwayTime)
			return DTRUE;
		else if(m_bPlayArmSound)
		{
			PlaySoundFromPos(&m_LastPos, "Sounds\\Weapons\\proximities\\arm.wav", 500.0f, SOUNDPRIORITY_MISC_MEDIUM);
			m_bPlayArmSound = DFALSE;
		}

		// Check to see if there's anyone in our radius of detection, then blow up

		// Get the head of the list of destructable objects
		DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
		if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DTRUE;

		// Work with the squares of the distances so as to not have to get a square root.
		HOBJECT		hObj;
		DVector		vProxPos;
		DFLOAT		fRadiusSqr		= m_fDetectRadius * m_fDetectRadius;

		pServerDE->GetObjectPos(m_hObject, &vProxPos);

		while(pLink != &CDestructable::m_DestructableHead)
		{
			hObj = ((CDestructable*)pLink->m_pData)->GetObject();
			if(!hObj)	break;

			if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
			{
				CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);

				if(!pObj->IsDead())
				{
					DVector		vDir, vPos;
					DFLOAT		fDistSqr;

					pServerDE->GetObjectPos(hObj, &vPos);
					VEC_SUB(vDir, vPos, vProxPos);

					fDistSqr = VEC_MAGSQR(vDir);
					if(fDistSqr <= fRadiusSqr)
					{
						m_bExplode = DTRUE;
						m_fStartTime = pServerDE->GetTime();
						break;
					}
				}
			}

			pLink = pLink->m_pNext;
		}
	}
	else
	{
		DRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);

		// Rotate the object as it flys through the air
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProximityBomb::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProximityBomb::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return;

	if(m_bArmed)
		return;

	if(hObj == m_hFiredFrom)
		return;

	if(hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID))
		return;

	if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
		if(pObj->IsDead())	return;

		CProjectile::HandleTouch(hObj);	// Explode on impact type.
		m_bHitCharacter = DTRUE;
	}
	else
	{
		CollisionInfo colInfo;
		pServerDE->GetLastCollision( &colInfo );

		DVector		vU, vR, vF;
		DRotation	rRot;

		VEC_SET(vU, 0.0f, 1.0f, 0.0f);

		pServerDE->AlignRotation(&rRot, &colInfo.m_Plane.m_Normal, &vU);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		pServerDE->RotateAroundAxis(&rRot, &vR, MATH_PI * 0.5f);
		pServerDE->SetObjectRotation(m_hObject, &rRot);

		DDWORD	dwFlags = pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		m_bArmed = DTRUE;
		m_fStartTime = pServerDE->GetTime();
		PlaySoundFromPos(&m_LastPos, "Sounds\\Weapons\\proximities\\stick.wav", 500.0f, SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProximityBomb::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProximityBomb::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_DEFAULT_MEDIUM;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\proximities\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}




BEGIN_CLASS(CRemoteBomb)
END_CLASS_DEFAULT_FLAGS(CRemoteBomb, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteBomb::CRemoteBomb()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRemoteBomb::CRemoteBomb() : CProjectile()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;

	m_pProjectileFilename	= "Models\\Ammo\\c4.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\c4_remote.dtx";

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;

	m_dwFlags				= m_dwFlags | FLAG_GRAVITY;
	m_bArmed				= DFALSE;
	m_bExplode				= DFALSE;
	m_fStartTime			= 0.0f;

	m_bFalling				= DFALSE;

	// Set up angular velocities
	m_fPitch				= 0.0f;
	m_fYaw					= 0.0f;
	m_fPitchVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
	m_fYawVel				= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

	m_damage.SetDeathDelay(0.25f);		// .25 second delay if killed by explosion
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteBomb::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CRemoteBomb::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->GetObjectPos(m_hObject, &m_LastPos);

	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hFiredFrom && IsPlayer(m_hFiredFrom)) 
		{
			CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);
			pObj->RemoveRemoteBomb(this);
		}

		Explode();
		return DFALSE;
	}

	if(m_bArmed)
	{
		DVector		vVel;
		VEC_INIT(vVel);
		pServerDE->SetVelocity(m_hObject, &vVel);
	}
	else
	{
		DRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);

		// Rotate the object as it flys through the air
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteBomb::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CRemoteBomb::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_bArmed)
		return;

	if(hObj == m_hFiredFrom)
		return;

	if(hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID))
		return;

	if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		if(m_bFalling)	return;

		DVector vVel;
		VEC_SET(vVel, 0.0f, 0.0f, 0.0f);
		pServerDE->SetVelocity(m_hObject, &vVel);

		m_bFalling = DTRUE;

//		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
//		if(pObj->IsDead())	return;

//		CProjectile::HandleTouch(hObj);	// Explode on impact type.
	}
	else
	{
		CollisionInfo colInfo;
		pServerDE->GetLastCollision( &colInfo );

		DVector		vU, vR, vF;
		DRotation	rRot;

		VEC_SET(vU, 0.0f, 1.0f, 0.0f);

		pServerDE->AlignRotation(&rRot, &colInfo.m_Plane.m_Normal, &vU);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		pServerDE->RotateAroundAxis(&rRot, &vR, MATH_PI * 0.5f);
		pServerDE->SetObjectRotation(m_hObject, &rRot);

		DDWORD	dwFlags = pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		m_bArmed = DTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteBomb::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CRemoteBomb::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_DEFAULT_SMALL;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}

#endif _DEMO


BEGIN_CLASS(CHowitzerShell)
END_CLASS_DEFAULT_FLAGS(CHowitzerShell, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerShell::CHowitzerShell()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHowitzerShell::CHowitzerShell() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fShockwaveDuration	= 0.75f;
	m_fLifeTime				= 5.0f;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE;

	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 0.0f);
	VEC_SET(m_vShockwaveScaleMax, 2.0f, 2.0f, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerShell::Update()
//
//	PURPOSE:	Cast a ray to see if it hits anything, then 
//
// ----------------------------------------------------------------------- //

DBOOL CHowitzerShell::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	IntersectQuery iq;
	IntersectInfo  ii;
	DRotation rRot;
	DVector vFrom, vTo, vDist, vFire;

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	// Get the forward vector
	pServerDE->GetRotationVectors(&rRot, &vTo, &vTo, &vFire);

    pServerDE->GetObjectPos(m_hObject, &vFrom);
	VEC_COPY(vTo, vFrom);
	VEC_INIT(vDist);

// Add distance to rotation    
	VEC_MULSCALAR(vDist, vFire, 3000.0f)

// Get distination point    
	VEC_ADD(vTo, vTo, vDist);

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);
    
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	DBOOL bHit;
	
	// Greg 10/5 - Don't shoot the sap who fired this.
	if (bHit = pServerDE->IntersectSegment(&iq, &ii))
	{
		if (m_hFiredFrom == ii.m_hObject)
		{
			VEC_COPY(iq.m_From, ii.m_Point);
			bHit = pServerDE->IntersectSegment(&iq, &ii);		
		}
	}

	if (bHit)
	{
		pServerDE->TeleportObject(m_hObject, &ii.m_Point);
		AddImpact(ii.m_Point, ii.m_Plane.m_Normal, ii.m_hObject);

		if(m_nRadius) 
			DamageObjectsWithinRadius();

		// If we directly hit someone, do some more damage...
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hFiredFrom), pServerDE->GetClass("CBaseCharacter")))
			DamageObject(m_hFiredFrom, this, ii.m_hObject, m_fDamage, vDist, ii.m_Point, m_nDamageType);
	}
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerShell::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CHowitzerShell::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_HOWITZER_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CHowitzerAltShell)
END_CLASS_DEFAULT_FLAGS(CHowitzerAltShell, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltShell::CHowitzerAltShell()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHowitzerAltShell::CHowitzerAltShell() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fShockwaveDuration	= 0.75f;
	m_fLifeTime				= 5.0f;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE;

	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 0.0f);
	VEC_SET(m_vShockwaveScaleMax, 2.0f, 2.0f, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltShell::Update()
//
//	PURPOSE:	Cast a ray to see if it hits anything, then 
//
// ----------------------------------------------------------------------- //

DBOOL CHowitzerAltShell::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	IntersectQuery iq;
	IntersectInfo  ii;
	DRotation rRot;
	DVector vFrom, vTo, vDist, vFire, vScale;

	VEC_INIT(vScale);
	pServerDE->ScaleObject(m_hObject, &vScale);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	// Get the forward vector
	pServerDE->GetRotationVectors(&rRot, &vTo, &vTo, &vFire);

    pServerDE->GetObjectPos(m_hObject, &vFrom);
	VEC_COPY(vTo, vFrom);
	VEC_INIT(vDist);

// Add distance to rotation    
	VEC_MULSCALAR(vDist, vFire, 3000.0f)

// Get distination point    
	VEC_ADD(vTo, vTo, vDist);

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);
    
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	DBOOL bHit;
	
	if(bHit = pServerDE->IntersectSegment(&iq, &ii))
	{
		if (m_hFiredFrom == ii.m_hObject)
		{
			VEC_COPY(iq.m_From, ii.m_Point);
			bHit = pServerDE->IntersectSegment(&iq, &ii);		
		}
	}

	if(bHit)
	{
		pServerDE->TeleportObject(m_hObject, &ii.m_Point);
		AddImpact(ii.m_Point, ii.m_Plane.m_Normal, ii.m_hObject);

		if(m_nRadius) 
			DamageObjectsWithinRadius();

		// If we directly hit someone, do some more damage...
//		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hFiredFrom), pServerDE->GetClass("CBaseCharacter")))
//			DamageObject(m_hFiredFrom, this, ii.m_hObject, m_fDamage, vDist, ii.m_Point, m_nDamageType);
	}
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltShell::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CHowitzerAltShell::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_HOWITZER_ALT;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	//***** Make the projectiles the explode from the impact *****//
	CProjectile	*pProjectile = NULL;
	DVector		vFire, vUp, vU, vR, vF;
	DRotation	rRot;
	DFLOAT		fVelocity = 30.0f, fDamage = m_fDamage / 2;
	int			nRadius = m_nRadius / 2;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);
	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CHowitzerAltFrag");
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	pServerDE->AlignRotation(&rRot, &vNormal, &vUp);
	pServerDE->RotateAroundAxis(&rRot, &vNormal, pServerDE->Random(-MATH_PI, MATH_PI));

	for(int i = 0; i < pServerDE->IntRandom(5, 10); i++)
	{
		pServerDE->RotateAroundAxis(&rRot, &vNormal, MATH_PI / 2.0f);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vFire, vNormal, fVelocity);
		VEC_MULSCALAR(vR, vR, fVelocity * pServerDE->Random(0.25f,4.0f));
		VEC_ADD(vFire, vFire, vR);

		VEC_NORM(vFire);
		VEC_MULSCALAR(vFire, vFire, fVelocity);

		if (hClass)
			pProjectile = (CProjectile*)pServerDE->CreateObject(hClass, &ocStruct);

		if(pProjectile)
			pProjectile->Setup(&vFire, m_nWeaponType, fDamage / 2.0f, fVelocity, m_nRadius / 2, m_hFiredFrom);
	}
}



BEGIN_CLASS(CHowitzerAltFrag)
END_CLASS_DEFAULT_FLAGS(CHowitzerAltFrag, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltFrag::CHowitzerAltFrag()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHowitzerAltFrag::CHowitzerAltFrag() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Sprites\\howitzerball.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_damage.SetGodMode(DTRUE);

	AddLight(100.0f, 1.0f, 0.75f, 0.0f);
	VEC_SET(m_vInitScale, 0.15f, 0.15f, 0.0f);
	VEC_SET(tempNorm, 0.0f, 1.0f, 0.0f);

	m_fTrailScale		= 1.75f;
	m_dwTrailScaleFlags	= OBJFX_SCALERADIUS;
	m_dwTrailFXID		= OBJFX_SMOKETRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltFrag::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CHowitzerAltFrag::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vVel, vGravity;
	DFLOAT		fTime = pServerDE->GetTime();

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	if(m_bExplode || m_damage.IsDead())
	{
		Explode();
 		return DFALSE;
	}

	pServerDE->GetVelocity(m_hObject, &vVel);
	pServerDE->GetGlobalForce(&vGravity);
	VEC_MULSCALAR(vGravity, vGravity, 0.02f);
	VEC_ADD(vVel, vVel, vGravity);
	pServerDE->SetVelocity(m_hObject, &vVel);

	if(m_hLight)
	{
		pServerDE->GetObjectPos(m_hObject, &vPos);
		pServerDE->SetObjectPos(m_hLight, &vPos);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltFrag::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CHowitzerAltFrag::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// return if it hit a non solid object
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// return if it hit another of this same class
	if (pServerDE->GetObjectClass(hObj) == pServerDE->GetObjectClass(m_hObject))
		return;

	m_bExplode = DTRUE;
	m_hHitObject = hObj;

 	// Cast a ray from our last known position to see what we hit
	IntersectQuery iq;
	IntersectInfo  ii;
	DVector vVel, vLift;

	pServerDE->GetVelocity(m_hObject, &vVel);
	VEC_COPY(vLift, vVel);
	pServerDE->GetObjectPos(m_hObject, &m_LastPos);

	VEC_COPY(iq.m_From, m_LastPos);			// Get start point at the last known position.
	VEC_MULSCALAR(iq.m_To, vVel, 1.1f);
	VEC_ADD(iq.m_To, iq.m_To, iq.m_From);	// Get destination point slightly past where we should be
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if(pServerDE->IntersectSegment(&iq, &ii))
		VEC_COPY(tempNorm, ii.m_Plane.m_Normal);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHowitzerAltFrag::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CHowitzerAltFrag::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_HOWITZER_MINI;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &tempNorm);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

//	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_MEDIUM);
}



BEGIN_CLASS(CNapalmProjectile)
END_CLASS_DEFAULT_FLAGS(CNapalmProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmProjectile::CNapalmProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNapalmProjectile::CNapalmProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_hSound				= 0;

	AddLight(100.0f, 1.0f, 0.75f, 0.0f);
	VEC_INIT(m_vInitScale);

	m_dwTrailFXID	= OBJFX_SMOKETRAIL_2;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CNapalmProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Create a sound attached to the projectile
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\napalm\\projectile.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);
	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CNapalmProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	DDWORD		nType = EXP_NAPALM_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\napalm\\impact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


BEGIN_CLASS(CNapalmAltProjectile)
END_CLASS_DEFAULT_FLAGS(CNapalmAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmAltProjectile::CNapalmAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNapalmAltProjectile::CNapalmAltProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_hSound				= 0;

	AddLight(100.0f, 1.0f, 0.75f, 0.0f);
	VEC_INIT(m_vInitScale);

	m_dwTrailFXID	= OBJFX_SMOKETRAIL_2;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmAltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CNapalmAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Create a sound attached to the projectile
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\napalm\\projectile.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);
	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmAltProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CNapalmAltProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	DDWORD		nType = EXP_NAPALM_ALT;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\napalm\\altimpact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	//***** Make the projectiles that explode from the impact *****//
	CProjectile	*pProjectile = NULL;
	DVector		vFire, vUp, vU, vR, vF;
	DRotation	rRot, tempRot;
	DFLOAT		fVelocity, fDamage = m_fDamage / 3;
	int			nRadius = m_nRadius / 3;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);
	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CNapalmFireball");
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	pServerDE->GetVelocity(m_hObject, &vFire);
	pServerDE->AlignRotation(&rRot, &vNormal, &vUp);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	pServerDE->AlignRotation(&rRot, &vF, &vFire);

	DFLOAT	lowSpread = MATH_PI * 0.15f;
	DFLOAT	hiSpread = MATH_PI * 0.85f;
	fVelocity = 35.0f;

	for(int i = 0; i < 8; i++)
	{
		ROT_COPY(tempRot, rRot);
		pServerDE->RotateAroundAxis(&tempRot, &vNormal, pServerDE->Random(lowSpread, hiSpread));
		pServerDE->GetRotationVectors(&tempRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vFire, vNormal, fVelocity);
		VEC_MULSCALAR(vR, vR, pServerDE->Random(fVelocity * 0.75f, fVelocity * 3.0f));
		VEC_ADD(vFire, vFire, vR);

		VEC_NORM(vFire);
		VEC_MULSCALAR(vFire, vFire, fVelocity);

		if (hClass)
			pProjectile = (CProjectile*)pServerDE->CreateObject(hClass, &ocStruct);

		if(pProjectile)
			pProjectile->Setup(&vFire, m_nWeaponType, fDamage, fVelocity, m_nRadius, m_hFiredFrom);
	}
}



BEGIN_CLASS(CNapalmFireball)
END_CLASS_DEFAULT_FLAGS(CNapalmFireball, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmFireball::CNapalmFireball()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNapalmFireball::CNapalmFireball() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;
	m_fLifeTime				= g_pServerDE->Random(1.5f,2.5f);

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_GRAVITY | FLAG_NOSLIDING;
	m_fStartTime			= g_pServerDE->GetTime();

	m_damage.SetGodMode(DTRUE);

	AddLight(150.0f, 1.0f, 0.75f, 0.0f);
	VEC_INIT(m_vInitScale);

	m_nBounces				= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmFireball::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CNapalmFireball::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale, vVel, vPos;
	DFLOAT		fTime = pServerDE->GetTime();

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);
	pServerDE->ScaleObject(m_hObject, &vScale);

	if(m_nBounces >= 3)
		return DFALSE;

	if(m_bExplode)
	{
		Explode();
		m_bExplode = DFALSE;
	}

	pServerDE->GetVelocity(m_hObject, &vVel);
	pServerDE->GetObjectPos(m_hObject, &vPos);

	if (m_hLight)
		pServerDE->SetObjectPos(m_hLight, &vPos);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmFireball::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CNapalmFireball::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// return if it hit a non solid object
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// return if it hit another of this same class
	if (pServerDE->GetObjectClass(hObj) == pServerDE->GetObjectClass(m_hObject))
		return;

	m_nBounces++;

	m_bExplode = DTRUE;
	m_hHitObject = hObj;

 	// Cast a ray from our last known position to see what we hit
	IntersectQuery iq;
	IntersectInfo  ii;
	DVector vVel, vLift;

	pServerDE->GetVelocity(m_hObject, &vVel);
	VEC_COPY(vLift, vVel);
	pServerDE->GetObjectPos(m_hObject, &m_LastPos);

	VEC_COPY(iq.m_From, m_LastPos);			// Get start point at the last known position.
	VEC_MULSCALAR(iq.m_To, vVel, 1.1f);
	VEC_ADD(iq.m_To, iq.m_To, iq.m_From);	// Get destination point slightly past where we should be
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if(pServerDE->IntersectSegment(&iq, &ii))
	{
		VEC_MULSCALAR(vVel, vVel, 0.35f);
		pServerDE->SetVelocity(m_hObject, &vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNapalmFireball::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CNapalmFireball::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_NAPALM_FIREBALL;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

//	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_MEDIUM);
}



BEGIN_CLASS(CBugSprayProjectile)
END_CLASS_DEFAULT_FLAGS(CBugSprayProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayProjectile::CBugSprayProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBugSprayProjectile::CBugSprayProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_GRAVITY | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_fStartTime			= g_pServerDE->GetTime();
	m_fLifeTime				= 10.0f;
	m_fDelay				= 0.01f;

	VEC_INIT(m_vInitScale);

	m_dwTrailFXID	= OBJFX_BUGSPRAY_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CBugSprayProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CBugSprayProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_BUGSPRAY_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Bugbuster\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CBugSprayAltProjectile)
END_CLASS_DEFAULT_FLAGS(CBugSprayAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayAltProjectile::CBugSprayAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBugSprayAltProjectile::CBugSprayAltProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_FIRE;
	m_fStartTime			= g_pServerDE->GetTime();
	m_dwFlags				= m_dwFlags | FLAG_GRAVITY | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_fLifeTime				= 10.0f;
	m_fDelay				= 0.01f;

	VEC_INIT(m_vInitScale);

	m_dwTrailFXID	= OBJFX_BUGSPRAY_2;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayAltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CBugSprayAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBugSprayAltProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CBugSprayAltProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_BUGSPRAY_ALT;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Bugbuster\\altimpact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CDeathRayProjectile)
END_CLASS_DEFAULT_FLAGS(CDeathRayProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRayProjectile::CDeathRayProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDeathRayProjectile::CDeathRayProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_nDamageType			= DAMAGE_TYPE_ELECTRIC;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE;
	m_fLifeTime				= 0.0f;

	m_fStartTime			= 0.0f;
	m_fBounceDist			= 0.0f;
	m_bFirstUpdate			= DTRUE;
	m_bFirstShot			= DTRUE;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRayProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CDeathRayProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	if(m_bFirstUpdate)
	{
		DRotation	rRot;
		DVector		vTemp;

		// Find a forward vector to fire with...
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vTemp, &vTemp, &m_vBeamDir);

		m_bFirstUpdate = DFALSE;
	}

	// If it's time to reflect off of something... then cast a new ray
	if(pServerDE->GetTime() - m_fStartTime > DEATHRAY_BOUNCE_DELAY)
	{
		IntersectQuery	iq;
		IntersectInfo	ii;
		DVector			vDist;

		// Set the start and dest points for the intersection
		pServerDE->GetObjectPos(m_hObject, &(iq.m_From));
		VEC_MULSCALAR(vDist, m_vBeamDir, 3000.0f)
		VEC_ADD(iq.m_To, iq.m_From, vDist);

		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = LiquidFilterFn;
		iq.m_pUserData = NULL;	

		DBOOL bHit;
		
		if(bHit = pServerDE->IntersectSegment(&iq, &ii))
		{
			if(m_bFirstShot && m_hFiredFrom == ii.m_hObject)
			{
				VEC_COPY(iq.m_From, ii.m_Point);
				bHit = pServerDE->IntersectSegment(&iq, &ii);		
			}
		}

		m_bFirstShot = DFALSE;

		// Test to see if we hit anything...
		if(bHit)
		{
			HCLASS	hObjClass   = pServerDE->GetObjectClass(ii.m_hObject);

			// Create the client side laser effect
			HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
			pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

			pServerDE->WriteToMessageVector(hMessage, &(iq.m_From));
			pServerDE->WriteToMessageVector(hMessage, &(ii.m_Point));
			pServerDE->WriteToMessageByte(hMessage, LASER_GREEN_SMALL);

			pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

			// Add a little effect and damage the object we hit
			AddExplosion(ii.m_Point, ii.m_Plane.m_Normal);
			DamageObject(m_hFiredFrom, this, ii.m_hObject, m_fDamage, vDist, ii.m_Point, m_nDamageType);

			// If we hit a destructable object, don't reflect...
			if(pServerDE->IsKindOf(hObjClass, pServerDE->GetClass("CBaseCharacter")))
				return DFALSE;

			// End the beam if we hit the sky (don't reflect)
			SurfaceType eType = GetSurfaceType(ii.m_hObject, ii.m_hPoly);
			if (eType == SURFTYPE_SKY)
				return DFALSE;

			// Add the distance to the bounce distance
			VEC_SUB(vDist, ii.m_Point, iq.m_From);
			m_fBounceDist += VEC_MAG(vDist);

			if(m_fBounceDist > DEATHRAY_BOUNCE_DIST)
				return DFALSE;

			// Otherwise, find a new direction to reflect
			pServerDE->TeleportObject(m_hObject, &ii.m_Point);

			DVector		vTemp;
			VEC_COPY(vTemp, ii.m_Plane.m_Normal);

			VEC_NORM(vDist);
			VEC_NORM(vTemp);

			// Calculate a reflection vector
			DFLOAT	dot = VEC_DOT(vTemp, vDist);
			VEC_MULSCALAR(m_vBeamDir, vTemp, 2.0f * -dot);
			VEC_ADD(m_vBeamDir, vDist, m_vBeamDir);
			VEC_NORM(m_vBeamDir);

			m_fStartTime = pServerDE->GetTime();
		}
		else
		{
			// We didn't hit anything... so create the beam from start to end
			// Create the client side laser effect
			HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
			pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

			pServerDE->WriteToMessageVector(hMessage, &(iq.m_From));
			pServerDE->WriteToMessageVector(hMessage, &(iq.m_To));
			pServerDE->WriteToMessageByte(hMessage, LASER_GREEN_SMALL);

			pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
			return DFALSE;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRayProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CDeathRayProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_DEATHRAY_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\laser\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CLeechPrimeProjectile)
END_CLASS_DEFAULT_FLAGS(CLeechPrimeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechPrimeProjectile::CLeechPrimeProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CLeechPrimeProjectile::CLeechPrimeProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Sprites\\lleechproj.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_LEECH;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE  | FLAG_NOSLIDING;

	VEC_SET(m_vColor, 1.0f, g_pServerDE->Random(0.25f,1.0f), 0.0f);

	AddLight(100.0f, m_vColor.x, m_vColor.y, m_vColor.z);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_hSound				= 0;

	VEC_INIT(m_vInitScale);

	m_fTrailScale			= g_pServerDE->Random(0.25f, 2.0f);
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_FIRETRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechPrimeProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

DBOOL CLeechPrimeProjectile::InitialUpdate(DVector* vec)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vU, vR, vF;
	DRotation	rRot;

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	pServerDE->RotateAroundAxis(&rRot, &vF, pServerDE->Random(0.0f, MATH_PI));
	pServerDE->SetObjectRotation(m_hObject, &rRot);

	m_fCurve = MATH_PI * pServerDE->Random(0.05f, 0.1f);
	m_fCurve = pServerDE->IntRandom(0,1) ? m_fCurve : -m_fCurve;
	m_fAmp = pServerDE->Random(5.0f, 20.0f);

	DBOOL bRet = CProjectile::InitialUpdate(vec);
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechPrimeProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CLeechPrimeProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT		fTime = pServerDE->GetTime();
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	// Start a sound from the projectile
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\lifeleech\\projectile.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		Explode();
		return DFALSE;
	}

	// Set the scale and color
	DVector		vScale;
	VEC_SET(vScale, 0.2f, 0.2f, 0.2f);

	if(fTime - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);

	// Change the object's velocity to move sorta weird like
	DVector		vVel, vU, vR, vF;
	DRotation	rRot;

	pServerDE->GetVelocity(m_hObject, &vVel);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	pServerDE->RotateAroundAxis(&rRot, &vF, m_fCurve);
	pServerDE->SetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vU, vU, m_fAmp);
	VEC_ADD(vVel, vVel, vU);

	pServerDE->SetVelocity(m_hObject, &vVel);

	// Move the light to the object's position
	if(m_hLight)
	{
		DVector		vPos;
		pServerDE->GetObjectPos(m_hObject, &vPos);
		pServerDE->SetObjectPos(m_hLight, &vPos);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechPrimeProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CLeechPrimeProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_LIFELEECH_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
}



BEGIN_CLASS(CLeechAltProjectile)
END_CLASS_DEFAULT_FLAGS(CLeechAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechAltProjectile::CLeechAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CLeechAltProjectile::CLeechAltProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DTRUE;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeechAltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CLeechAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vNormal;
	DDWORD		nType = EXP_LIFELEECH_ALT;

	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);
	pServerDE->GetObjectPos(m_hFiredFrom, &vPos);

	//********************************************************************
	// Create the explosion type
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	//********************************************************************
	// Move all the objects within the radius back and randomly rotate them a little

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead)) return DFALSE;

	// Work with the squares of the distances so as to not have to get a square root.
	DVector		vObjPos, vObjVel, vUp;
	DRotation	rObjRot;
	HOBJECT		hObj = 0;
	DFLOAT		mag, randRot = MATH_PI * LIFELEECH_ROTATE_AMOUNT;
	short		objType;
	DDWORD		dwUserFlags;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		pLink = pLink->m_pNext;
		if(!hObj)	break;

		objType = pServerDE->GetObjectType(hObj);
		dwUserFlags = pServerDE->GetObjectUserFlags(hObj);
		pServerDE->GetObjectPos(hObj, &vObjPos);

		if(hObj == m_hFiredFrom) continue;
		if((objType != OT_MODEL) || !(dwUserFlags & USRFLG_SINGULARITY_ATTRACT)) continue;
//		if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter"))) continue;
		if(VEC_DISTSQR(vObjPos, vPos) > LIFELEECH_PUSH_RADIUS * LIFELEECH_PUSH_RADIUS) continue;

		// Get all the object information
		pServerDE->GetObjectRotation(hObj, &rObjRot);
		pServerDE->GetVelocity(hObj, &vObjVel);

		// Randomly rotate the object a little
		pServerDE->RotateAroundAxis(&rObjRot, &vUp, pServerDE->Random(-randRot, randRot));
		pServerDE->SetObjectRotation(hObj, &rObjRot);

		// Get a vector from the position to the object and scale it to adjust the velocity
		VEC_SUB(vObjPos, vObjPos, vPos);
		mag = LIFELEECH_PUSH_RADIUS - VEC_MAG(vObjPos);
		if(mag < 0.0f)	mag = 0.0f;
		VEC_NORM(vObjPos);
		VEC_MULSCALAR(vObjPos, vObjPos, mag * 7.5f);

		// Offset the objects velocity by a fraction of the gravity and the scaled vector
		pServerDE->GetGlobalForce(&vUp);
		VEC_MULSCALAR(vUp, vUp, -0.25f);
		VEC_ADD(vObjVel, vObjVel, vUp);
		VEC_ADD(vObjVel, vObjVel, vObjPos);

		// Set the new object velocity, and damage it a little
		pServerDE->SetVelocity(hObj, &vObjVel);
		pServerDE->GetObjectPos(hObj, &vObjPos);
		DamageObject(m_hFiredFrom, this, hObj, m_fDamage, vObjVel, vObjPos, DAMAGE_TYPE_EXPLODE);
	}

	return DFALSE;
}



BEGIN_CLASS(CFlareProjectile)
END_CLASS_DEFAULT_FLAGS(CFlareProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareProjectile::CFlareProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlareProjectile::CFlareProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename = "Sprites\\lensflare32.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_FIRE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_nColor				= g_pServerDE->IntRandom(0,3);
	switch(m_nColor)
	{
		case	0:	VEC_SET(m_vColor, 1.0f, 0.0f, 0.0f);	 break;
		case	1:	VEC_SET(m_vColor, 1.0f, 0.25f, 0.0f);	 break;
		case	2:	VEC_SET(m_vColor, 1.0f, 1.0f, 0.0f);	 break;
		case	3:	VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);	 break;
		default:	VEC_SET(m_vColor, 1.0f, 1.0f, 0.0f);	 break;
	}

	AddLight(200.0f, m_vColor.x, m_vColor.y, m_vColor.z);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;
	m_bTrackObj				= DFALSE;

	m_fLastDamageTime		= 0.0f;
	m_fDamageTime			= 1.0f;

	VEC_INIT(m_vInitScale);

	m_hSound = 0;

	m_dwTrailFXID	= OBJFX_SPARKS_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CFlareProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\flare\\projectile.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);

	DFLOAT fTime = pServerDE->GetTime();
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		if(m_bTrackObj)
		{
			DVector vPos, vUp;
			g_pServerDE->GetObjectPos(m_hObject, &vPos);
			VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
			AddExplosion(vPos, vUp);
		}
		else
			Explode();

		return DFALSE;
	}

	if (m_fLifeTime > 0.0f)
	{
		if(fTime > (m_fStartTime + m_fLifeTime))
		{
			if(m_hSound)
				{ pServerDE->KillSound(m_hSound); m_hSound = 0; }
			return DFALSE;
		}
	}

	DVector vVel, vPos;

	if(m_hHitObject)
	{
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hHitObject), pServerDE->GetClass("CBaseCharacter")))
		{
			CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(m_hHitObject);

			if(pObj->IsDead())
				m_bExplode = DTRUE;
		}

		// If we hit something that the flare should stick to... copy it's movement
		if(m_bTrackObj)
		{
			VEC_SET(vVel, 0.0f, 0.0f, 0.0f);
			pServerDE->SetVelocity(m_hObject, &vVel);

			pServerDE->GetObjectPos(m_hHitObject, &vPos);
			VEC_ADD(vPos, vPos, m_vObjOffset);
			pServerDE->SetObjectPos(m_hObject, &vPos);

			if(fTime > m_fLastDamageTime + m_fDamageTime)
			{
				m_fLastDamageTime = fTime;
				DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, vVel, vPos, DAMAGE_TYPE_FIRE);
			}
		}
	}
	else
	{
		// If the object no longer exists (has died or something)... remove the projectile
		if(m_bTrackObj)
		{
			if(m_hSound)
				{ pServerDE->KillSound(m_hSound); m_hSound = 0; }
			return DFALSE;
		}

		pServerDE->GetObjectPos(m_hObject, &vPos);
	}

	if (m_hLight)
		pServerDE->SetObjectPos(m_hLight, &vPos);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CFlareProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return;

	if(hObj == m_hFiredFrom) return; // Let it get out of our bounding box...

	// return if it hit a non solid object
	if(hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// return if it hit another of this same class
	if(pServerDE->GetObjectClass(hObj) == pServerDE->GetObjectClass(m_hObject))
		return;

	// Turn off the sound
	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	HCLASS	hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS	hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS	hCorpse		= pServerDE->GetClass("CCorpse");
	HCLASS	hObjClass   = pServerDE->GetObjectClass(hObj);

	if(pServerDE->IsKindOf(hObjClass, hCharacter) || pServerDE->IsKindOf(hObjClass, hPlayer) || pServerDE->IsKindOf(hObjClass, hCorpse))
	{
		DVector		objPos, flarePos;

		g_pServerDE->GetObjectPos(hObj, &objPos);
		g_pServerDE->GetObjectPos(m_hObject, &flarePos);
		VEC_SUB(m_vObjOffset, flarePos, objPos);

		m_bTrackObj = DTRUE;
		m_hHitObject = hObj;
		m_fLifeTime = 5.0f;
		m_fStartTime = pServerDE->GetTime(); 
	}
	else
	{
		m_hHitObject = hObj;
		m_bExplode = DTRUE;
	}

	DDWORD flags = pServerDE->GetObjectFlags(m_hObject);
	flags &= ~FLAG_TOUCH_NOTIFY;
	pServerDE->SetObjectFlags(m_hObject, flags);

	// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
	pServerDE->CreateInterObjectLink(m_hObject, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CFlareProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_FLARE_FIZZLE;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\flare\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CFlareAltProjectile)
END_CLASS_DEFAULT_FLAGS(CFlareAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareAltProjectile::CFlareAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlareAltProjectile::CFlareAltProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename = "Sprites\\lensflare32.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	switch(g_pServerDE->IntRandom(0,3))
	{
		case	0:	VEC_SET(m_vColor, 1.0f, 0.0f, 0.0f);	 break;
		case	1:	VEC_SET(m_vColor, 1.0f, 0.25f, 0.0f);	 break;
		case	2:	VEC_SET(m_vColor, 1.0f, 1.0f, 0.0f);	 break;
		case	3:	VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);	 break;
		default:	VEC_SET(m_vColor, 1.0f, 1.0f, 0.0f);	 break;
	}

	AddLight(200.0f, m_vColor.x, m_vColor.y, m_vColor.z);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fBurstTime			= 0.1f;
	m_fDelay				= 0.01f;
	m_nExpType				= EXP_FLARE_FRAG;
	m_bGetVelocity			= DTRUE;

	VEC_INIT(m_vInitScale);

//	m_dwTrailFXID	= OBJFX_SPARKS_2;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareAltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CFlareAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT		fTime = pServerDE->GetTime();
	DVector		vScale;
	VEC_SET(vScale, 0.5f, 0.5f, 0.5f);

	if(m_bGetVelocity)
	{
		pServerDE->GetVelocity(m_hObject, &m_vVel);
		m_bGetVelocity = DFALSE;
	}

	if(fTime - m_fStartTime < m_fDelay)
	{
		VEC_INIT(vScale);
	}

	if(fTime - m_fStartTime > m_fBurstTime)
	{
		DVector		vFire, vTemp, vU, vR, vF;
		DRotation	rRot;
		DFLOAT		mag, hOffset, vOffset;

		ROT_INIT(rRot);
		VEC_SET(vTemp, 0.0f, 1.0f, 0.0f);
		pServerDE->AlignRotation(&rRot, &m_vVel, &vTemp);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		mag = VEC_MAG(m_vVel);

		//***** Make the projectiles that explode from the flare *****//
		CProjectile	*pProjectile = NULL;

		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);
		pServerDE->GetObjectPos(m_hObject, &ocStruct.m_Pos);

		HCLASS hClass = pServerDE->GetClass("CFlareBurstProjectile");

		for(int i = 0; i < 8; i++)
		{
			VEC_COPY(vFire, m_vVel);
			VEC_NORM(vFire);

			switch(i)
			{
				case	7:	hOffset = 0.06f; vOffset = 0.06f;	break;
				case	6:	hOffset = -0.06f; vOffset = -0.06f;	break;
				case	5:	hOffset = -0.06f; vOffset = 0.06f;	break;
				case	4:	hOffset = 0.06f; vOffset = -0.06f;	break;
				case	3:	hOffset = 0.0f; vOffset = 0.135f;	break;
				case	2:	hOffset = 0.0f; vOffset = -0.135f;	break;
				case	1:	hOffset = -0.135f; vOffset = 0.0f;	break;
				case	0:	hOffset = 0.135f; vOffset = 0.0f;	break;
				default:	hOffset = 0.0f; vOffset = 0.0f;		break;
			}

			VEC_MULSCALAR(vTemp, vR, hOffset)
			VEC_ADD(vFire, vFire, vTemp)

			VEC_MULSCALAR(vTemp, vU, vOffset)
			VEC_ADD(vFire, vFire, vTemp)

			VEC_NORM(vFire)

			if (hClass)
				pProjectile = (CProjectile*)pServerDE->CreateObject(hClass, &ocStruct);

			if(pProjectile)
			{
				pProjectile->Setup(&vFire, m_nWeaponType, m_fDamage * 0.5f, mag, m_nRadius, m_hFiredFrom);
				((CFlareBurstProjectile*)pProjectile)->SetColor(&m_vColor);
			}
		}

		DVector vPos;
		g_pServerDE->GetObjectPos(m_hObject, &vPos);

		AddImpact(vPos, vPos, m_hHitObject);

		m_nExpType = EXP_FLARE_BURST;
		CProjectile::Update(pMovement);
		return	DFALSE;
	}

	pServerDE->ScaleObject(m_hObject, &vScale);
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);

	DBOOL bRet = CProjectile::Update(pMovement);
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareAltProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CFlareAltProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = m_nExpType;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\flare\\impact.wav", 500.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CFlareBurstProjectile)
END_CLASS_DEFAULT_FLAGS(CFlareBurstProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareBurstProjectile::CFlareBurstProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlareBurstProjectile::CFlareBurstProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename = "Sprites\\lensflare32.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_fStartTime = g_pServerDE->GetTime();
	VEC_INIT(m_vInitScale);
	m_damage.SetGodMode(DTRUE);

	m_dwTrailFXID	= OBJFX_SPARKS_3;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareBurstProjectile::SetColor()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

void CFlareBurstProjectile::SetColor(DVector *vColor)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	VEC_COPY(m_vColor, *vColor);
//	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);
	AddLight(75.0f, m_vColor.x, m_vColor.y, m_vColor.z);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareBurstProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CFlareBurstProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT		fTime = pServerDE->GetTime();
	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	pServerDE->ScaleObject(m_hObject, &vScale);
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);

	DBOOL bRet = CProjectile::Update(pMovement);
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareBurstProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CFlareBurstProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_FLARE_FRAG;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\flare\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(COrbProjectile)
END_CLASS_DEFAULT_FLAGS(COrbProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::COrbProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

COrbProjectile::COrbProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Ammo\\orb.abc";
	m_pProjectileSkin		= "Skins\\Powerups\\orb_pu.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_hTrackObj				= DNULL;
	m_fRotateAmount			= 0.0f;
	m_fHitTime				= 0.0f;
	m_fLastDamageTime		= 0.0f;
	m_bState				= THEORB_STATE_TRACKING;

	m_hSound				= 0;

	VEC_SET(m_vInitScale, 0.5f, 0.5f, 0.5f);

	m_dwTrailFXID			= OBJFX_ORBTRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL COrbProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vU, vR, vF;
	DRotation	rRot;
	DFLOAT		fMag;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	// Play a sound if we don't have one playing already
	if(!m_hSound)
	{
		if(m_bState == THEORB_STATE_ATTACHED)
			m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\orb\\loopdrilling.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
		else
			m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\orb\\loop.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
	}

	// If the orb hit a non-player object, make it break into pieces
	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		Explode();
		return DFALSE;
	}

	if(m_bState == THEORB_STATE_TRACKING)
	{
		m_hTrackObj = FindTrackObj();
		if(m_hTrackObj)		TrackObjInRange();

		//*******************************************************************
		// Adjust the velocity according to the rotation
		pServerDE->GetVelocity(m_hObject, &vF);
		fMag = VEC_MAG(vF);

		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vF, vF, fMag);
		pServerDE->SetVelocity(m_hObject, &vF);
		//*******************************************************************
	}
	else if(m_bState == THEORB_STATE_ATTACHED)
	{
		if(!m_hHitObject || (pServerDE->GetTime() - m_fHitTime > THEORB_DAMAGE_TIME))
		{
			m_bExplode = DTRUE;
		}
		else if(m_hHitObject)
		{
			if(pServerDE->GetModelNodeTransform(m_hHitObject, "head", &vPos, &rRot))
			{
				// Set the orb in front of the players head that was hit
				pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

				VEC_MULSCALAR(vF, vF, THEORB_HEAD_OFFSET);
				VEC_ADD(vPos, vPos, vF);
				pServerDE->SetObjectPos(m_hObject, &vPos);

				pServerDE->RotateAroundAxis(&rRot, &vU, MATH_PI);
				pServerDE->SetObjectRotation(m_hObject, &rRot);

				// Damage the player who it's attached to
				if(pServerDE->GetTime() - m_fLastDamageTime > THEORB_DAMAGE_DELAY)
				{
					m_fLastDamageTime = pServerDE->GetTime();
					DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, vF, vPos, m_nDamageType);
					AddBloodImpact(&vPos, &vF, m_fDamage);
				}
			}

			if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hHitObject), pServerDE->GetClass("CPlayerObj")))
			{
				CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hHitObject);

				if(pObj->IsDead())
					m_bExplode = DTRUE;
			}
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void COrbProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hHitObject) return;
	if(hObj == m_hFiredFrom) return;

	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	if(m_bState == THEORB_STATE_TRACKING)
	{
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			DVector		vVel;
			VEC_INIT(vVel);
			pServerDE->SetVelocity(m_hObject, &vVel);

			m_hHitObject = hObj;
			m_fHitTime = pServerDE->GetTime();
			m_bState = THEORB_STATE_ATTACHED;

			if(m_hSound)
				{ pServerDE->KillSound(m_hSound); m_hSound = 0; }
		}
		else
			m_bExplode = DTRUE;
	}

	// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
	pServerDE->CreateInterObjectLink(m_hObject, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void COrbProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_ORB_BREAK;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\orb\\explode.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::FindTrackObj()
//
//	PURPOSE:	Find any object within the range of the orb that has a head and is moving
//
// ----------------------------------------------------------------------- //

HOBJECT COrbProjectile::FindTrackObj()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead)) return DNULL;

	// Work with the squares of the distances so as to not have to get a square root.
	HOBJECT		hObj, hTempObj	= DNULL;
	DFLOAT		fRadiusSqr		= THEORB_DETECT_RADIUS * THEORB_DETECT_RADIUS;
	DFLOAT		fHalfRadiusSqr	= fRadiusSqr / 2;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;

		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			DVector		vDir, vOrbPos, vHeadPos;
			DRotation	rRot;
			DFLOAT		fDistSqr;

			if(hObj == m_hFiredFrom)
				{ pLink = pLink->m_pNext; continue; }

			// Handle the motion detection
			pServerDE->GetVelocity(hObj, &vDir);

			if((vDir.x < 0.1f) && (vDir.y < 0.1f) && (vDir.z < 0.1f))
				{ pLink = pLink->m_pNext; continue; }

			// If the object has a 'head' node... get the dist from the orb to the head
			if(pServerDE->GetModelNodeTransform(hObj, "head", &vHeadPos, &rRot))
			{
				pServerDE->GetObjectPos(m_hObject, &vOrbPos);
				VEC_SUB(vDir, vHeadPos, vOrbPos);

				fDistSqr = VEC_MAGSQR(vDir);
				if(fDistSqr <= fRadiusSqr)
				{
					fRadiusSqr = fDistSqr;
					hTempObj = hObj;
				}
			}
		}

		pLink = pLink->m_pNext;
	}

	return hTempObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbProjectile::TrackObjInRange()
//
//	PURPOSE:	Track the selected object
//
// ----------------------------------------------------------------------- //

void COrbProjectile::TrackObjInRange()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return;

	DVector		vDir, vOrbPos, vHeadPos, vUp;
	DRotation	rTemp, rOrbRot, rHeadRot;
	DFLOAT		fTemp;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	// If the object is dead or something, stop tracking
	if(!m_hTrackObj)	return;

	// If the object has a 'head' node... get the dist from the orb to the head
	if(pServerDE->GetModelNodeTransform(m_hTrackObj, "head", &vHeadPos, &rHeadRot))
	{
		// Get the current position and rotation
		pServerDE->GetObjectPos(m_hObject, &vOrbPos);
		pServerDE->GetObjectRotation(m_hObject, &rOrbRot);

		// Create the destination rotation
		VEC_SUB(vDir, vHeadPos, vOrbPos);
		pServerDE->AlignRotation(&rHeadRot, &vDir, &vUp);

		fTemp = VEC_MAG(vDir);
		fTemp = 1.0f - (fTemp / THEORB_DETECT_RADIUS);

		// Try to get a rotation inbetween the current and the destination and set it
		pServerDE->InterpolateRotation(&rTemp, &rOrbRot, &rHeadRot, fTemp);
		pServerDE->SetObjectRotation(m_hObject, &rTemp);
	}
	else
		return;
}




BEGIN_CLASS(COrbAltProjectile)
END_CLASS_DEFAULT_FLAGS(COrbAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbAltProjectile::COrbAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

COrbAltProjectile::COrbAltProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Ammo\\orb.abc";
	m_pProjectileSkin		= "Skins\\Powerups\\orb_pu.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE | FLAG_NOSLIDING;

	m_fHitTime				= 0.0f;
	m_fLastDamageTime		= 0.0f;
	m_bState				= THEORB_STATE_TRACKING;
	m_bSwitchView			= DFALSE;

	m_bCreateCamera			= DTRUE;
	m_hSound				= 0;

	VEC_SET(m_vInitScale, 0.5f, 0.5f, 0.5f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbAltProjectile::InitialUpdate(DVector *pMovement)
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DBOOL COrbAltProjectile::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	if(m_dwClientID)
	{
		HMESSAGEWRITE hMsg = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMsg, SFX_ORBCAM_ID);
		pServerDE->WriteToMessageDWord(hMsg, m_dwClientID);
		pServerDE->WriteToMessageByte(hMsg, DTRUE);
		pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
	}

	return CProjectile::InitialUpdate(pMovement);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbAltProjectile::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL COrbAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vU, vR, vF;
	DRotation	rRot;
	DFLOAT		fMag;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	// Play a sound if we don't have one playing already
	if(!m_hSound)
	{
		if(m_bState == THEORB_STATE_ATTACHED)
			m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\orb\\loopdrilling.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
		else
			m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\orb\\altloop.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
	}

	if(m_bCreateCamera)
	{
		if(IsPlayer(m_hFiredFrom))
		{
    		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);
			if(pObj)
				pObj->SetOrbObject(m_hObject);
		}

		m_bCreateCamera = DFALSE;
	}

	// If the orb hit a non-player object, make it break into pieces
	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		// Change the view back to the character
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hFiredFrom), pServerDE->GetClass("CPlayerObj")))
		{
    		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);

			if(m_dwClientID)
			{
				HMESSAGEWRITE hMsg = pServerDE->StartSpecialEffectMessage(this);
				pServerDE->WriteToMessageByte(hMsg, SFX_ORBCAM_ID);
				pServerDE->WriteToMessageDWord(hMsg, m_dwClientID);
				pServerDE->WriteToMessageByte(hMsg, DFALSE);
				pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
			}

/*			if(!m_bSwitchView)
			{
				HMESSAGEWRITE hMsg = pServerDE->StartMessage(pObj->GetClient(), SMSG_ORBCAM);
				pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
				m_bSwitchView = DTRUE;
			}
*/		}

		Explode();
		return DFALSE;
	}

	if(m_bState == THEORB_STATE_TRACKING)
	{
		// Adjust the velocity according to the rotation of the character who shot it
		pServerDE->GetVelocity(m_hObject, &vF);
		fMag = VEC_MAG(vF);

		// Follow the same rotation as the character's facing
	    if(IsPlayer(m_hFiredFrom))
    	{
    		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);

			if(pObj->IsDead())
			{
				m_bExplode = DTRUE;
				return DTRUE;
			}

			if(pObj->IsActivated())
			{
				pObj->GetPlayerRotation(&rRot);
				pServerDE->SetObjectRotation(m_hObject, &rRot);
			}
		}
		else
			pServerDE->GetObjectRotation(m_hObject, &rRot);

		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vF, vF, fMag);
		pServerDE->SetVelocity(m_hObject, &vF);
	}
	else if(m_bState == THEORB_STATE_ATTACHED)
	{
		if(!m_hHitObject || (pServerDE->GetTime() - m_fHitTime > THEORB_DAMAGE_TIME))
		{
			m_bExplode = DTRUE;
		}
		else if(m_hHitObject)
		{
			if(pServerDE->GetModelNodeTransform(m_hHitObject, "head", &vPos, &rRot))
			{
				// Set the orb in front of the players head that was hit
				pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

				VEC_MULSCALAR(vF, vF, THEORB_HEAD_OFFSET);
				VEC_ADD(vPos, vPos, vF);
				pServerDE->SetObjectPos(m_hObject, &vPos);

				pServerDE->RotateAroundAxis(&rRot, &vU, MATH_PI);
				pServerDE->SetObjectRotation(m_hObject, &rRot);

				// Damage the player who it's attached to
				if(pServerDE->GetTime() - m_fLastDamageTime > THEORB_DAMAGE_DELAY)
				{
					m_fLastDamageTime = pServerDE->GetTime();
					DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, vF, vPos, m_nDamageType);
					AddBloodImpact(&vPos, &vF, m_fDamage);
				}
			}

			if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hHitObject), pServerDE->GetClass("CPlayerObj")))
			{
				CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hHitObject);

				if(pObj->IsDead())
					m_bExplode = DTRUE;
			}
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbAltProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void COrbAltProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hHitObject) return;
	if(hObj == m_hFiredFrom) return;

	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// Change the view back to the character if we hit anything
	if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hFiredFrom), pServerDE->GetClass("CPlayerObj")))
    {
    	CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);

		if(m_dwClientID)
		{
			HMESSAGEWRITE hMsg = pServerDE->StartSpecialEffectMessage(this);
			pServerDE->WriteToMessageByte(hMsg, SFX_ORBCAM_ID);
			pServerDE->WriteToMessageDWord(hMsg, m_dwClientID);
			pServerDE->WriteToMessageByte(hMsg, DFALSE);
			pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
		}

/*		if(!m_bSwitchView)
		{
			HMESSAGEWRITE hMsg = pServerDE->StartMessage(pObj->GetClient(), SMSG_ORBCAM);
			pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
			m_bSwitchView = DTRUE;
		}
*/	}

	if(m_bState == THEORB_STATE_TRACKING)
	{
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			DVector		vPos, vObj;
			DRotation	rRot;

			if(pServerDE->GetModelNodeTransform(hObj, "head", &vPos, &rRot))
			{
				pServerDE->GetObjectPos(m_hObject, &vObj);
				VEC_SUB(vPos, vPos, vObj);
				DFLOAT	fMag = VEC_MAG(vPos);

				if(fMag <= THEORB_ATTACH_RANGE_ALT)
				{
					DVector		vVel;
					VEC_INIT(vVel);
					pServerDE->SetVelocity(m_hObject, &vVel);

					m_hHitObject = hObj;
					m_fHitTime = pServerDE->GetTime();
					m_bState = THEORB_STATE_ATTACHED;

					if(m_hSound)
						{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

					if(IsPlayer(m_hFiredFrom))
					{
    					CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);
						if(pObj)
							pObj->SetOrbObject(DNULL);
					}
				}
				else
					m_bExplode = DTRUE;
			}
			else
				m_bExplode = DTRUE;
		}
		else
			m_bExplode = DTRUE;
	}

	// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
	pServerDE->CreateInterObjectLink(m_hObject, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COrbAltProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void COrbAltProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_ORB_BREAK;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\orb\\explode.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CTeslaProjectile)
END_CLASS_DEFAULT_FLAGS(CTeslaProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaProjectile::CTeslaProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeslaProjectile::CTeslaProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Sprites\\Teslaprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	AddLight(100.0f, 0.65f, 0.65f, 1.0f);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_hSound				= 0;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CTeslaProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.175f, 0.175f, 0.175f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\tesla\\projectile.wav", 600.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	pServerDE->ScaleObject(m_hObject, &vScale);
	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CTeslaProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	DDWORD		nType = EXP_TESLA_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


BEGIN_CLASS(CTeslaBoltProjectile)
END_CLASS_DEFAULT_FLAGS(CTeslaBoltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBoltProjectile::CTeslaBallProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeslaBoltProjectile::CTeslaBoltProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
//	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Ammo\\Lightining.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\Pulsebeam.dtx";

	m_nDamageType			= DAMAGE_TYPE_ELECTRIC;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	AddLight(100.0f, 0.65f, 0.65f, 1.0f);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBoltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CTeslaBoltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	IntersectQuery iq;
	IntersectInfo  ii;
	DRotation rRot;
	DVector vFrom, vTo, vDist, vFire;

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	// Get the forward vector
	pServerDE->GetRotationVectors(&rRot, &vTo, &vTo, &vFire);

    pServerDE->GetObjectPos(m_hObject, &vFrom);
	VEC_COPY(vTo, vFrom);
	VEC_INIT(vDist);
    
// Add distance to rotation    
	VEC_MULSCALAR(vDist, vFire, 2000.0f)
    
// Get distination point    
	VEC_ADD(vTo, vTo, vDist);

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);
    
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	DBOOL bHit;
	
	if(bHit = pServerDE->IntersectSegment(&iq, &ii))
	{
		if(m_hFiredFrom == ii.m_hObject)
		{
			VEC_COPY(iq.m_From, ii.m_Point);
			bHit = pServerDE->IntersectSegment(&iq, &ii);		
		}
	}

	// Test to see if we hit anything...
	if(bHit)
	{
		pServerDE->TeleportObject(m_hObject, &ii.m_Point);

		AddImpact(ii.m_Point, ii.m_Plane.m_Normal, ii.m_hObject);
		if(m_nRadius) 
			DamageObjectsWithinRadius();
	}

	return DFALSE;
}


BEGIN_CLASS(CTeslaBallProjectile)
END_CLASS_DEFAULT_FLAGS(CTeslaBallProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBallProjectile::CTeslaBallProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeslaBallProjectile::CTeslaBallProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
//	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Ammo\\Lightining.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\Pulsebeam.dtx";

	m_nDamageType			= DAMAGE_TYPE_ELECTRIC;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	AddLight(100.0f, 0.65f, 0.65f, 1.0f);

	m_fAttractRadius		= TESLABALL_ATTRACT_MIN;
	m_fStartTime			= 0.0f;
	m_fLastFireTime			= 0.0f;
	m_hLastObj				= 0;

	m_bArmed				= 0;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBallProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CTeslaBallProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, 0.1f);

	if(m_bArmed)
	{
		if(pServerDE->GetTime() - m_fStartTime > TESLABALL_ATTRACT_TIME)
			return DFALSE;

		if(pServerDE->GetTime() - m_fLastFireTime > TESLABALL_ATTRACT_DELAY)
			FireBolt();
	}
	else
	{
		// Check to see where the lightning hit and create an explosion
		IntersectQuery iq;
		IntersectInfo  ii;
		DRotation rRot;
		DVector vFrom, vTo, vDist, vFire;

		pServerDE->GetObjectRotation(m_hObject, &rRot);
		// Get the forward vector
		pServerDE->GetRotationVectors(&rRot, &vTo, &vTo, &vFire);

		pServerDE->GetObjectPos(m_hObject, &vFrom);
		VEC_COPY(vTo, vFrom);
		VEC_INIT(vDist);
    
		VEC_MULSCALAR(vDist, vFire, 2000.0f)
    
		VEC_ADD(vTo, vTo, vDist);

		VEC_COPY(iq.m_From, vFrom);
		VEC_COPY(iq.m_To, vTo);
    
		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = LiquidFilterFn;
		iq.m_pUserData = NULL;	

		DBOOL bHit;
		
		if(bHit = pServerDE->IntersectSegment(&iq, &ii))
		{
			if(m_hFiredFrom == ii.m_hObject)
			{
				VEC_COPY(iq.m_From, ii.m_Point);
				bHit = pServerDE->IntersectSegment(&iq, &ii);		
			}
		}

		// Test to see if we hit anything...
		if(bHit)
		{
			VEC_COPY(vTo, ii.m_Point);
			VEC_COPY(vDist, ii.m_Plane.m_Normal);
		}
		else
		{
			VEC_SET(vDist, 0.0f, 1.0f, 0.0f);
			VEC_COPY(vTo, iq.m_To);
		}

		m_bArmed		= DTRUE;
		m_fStartTime	= g_pServerDE->GetTime();

		pServerDE->TeleportObject(m_hObject, &vTo);
		AddImpact(vTo, vDist, ii.m_hObject);
		if(m_nRadius)	DamageObjectsWithinRadius();

		VEC_SET(vFrom, 0.0f, 0.0f, 0.0f)
		pServerDE->SetVelocity(m_hObject, &vFrom);
		pServerDE->SetAcceleration(m_hObject, &vFrom);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBallProjectile::FireBolt()
//
//	PURPOSE:	Fire lightning from the center of the explosion
//
// ----------------------------------------------------------------------- //

void CTeslaBallProjectile::FireBolt()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;

	DFLOAT	timeRatio = (pServerDE->GetTime() - m_fStartTime) / TESLABALL_ATTRACT_TIME;
	m_fAttractRadius = TESLABALL_ATTRACT_MIN + (TESLABALL_ATTRACT_RANGE * timeRatio);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return;

	HOBJECT		hObj = 0;
	DVector		vTarget;
	HCLASS		objClass = 0;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		pLink = pLink->m_pNext;
		if(!hObj)	break;

		objClass = pServerDE->GetObjectClass(hObj);
		pServerDE->GetObjectPos(hObj, &vTarget);

		// If the object is not a character or is not close enough, then skip it
		if(!pServerDE->IsKindOf(objClass, pServerDE->GetClass("CBaseCharacter"))) continue;
		if(VEC_DISTSQR(vTarget, vPos) > m_fAttractRadius * m_fAttractRadius) continue;
		if(hObj == m_hLastObj) continue;

		// Align to target and fire a lightning bolt
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_LIGHTNING_ID);

		pServerDE->WriteToMessageVector(hMessage, &vPos);
		pServerDE->WriteToMessageVector(hMessage, &vTarget);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_SHAPE_BOXED);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_FORM_WIDE2THIN);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_TYPE_HIGHSEG);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

		// Damage the sucker who got hit
		DamageObjectsInRadius(m_hFiredFrom, this, vTarget, TESLABOLT_DAMAGERADIUS, TESLABOLT_DAMAGE, DAMAGE_TYPE_ELECTRIC);

		switch(pServerDE->IntRandom(0,2))
		{
			case	0: PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\althit1.wav", 1500.0f, SOUNDPRIORITY_MISC_MEDIUM); break;
			case	1: PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\althit2.wav", 1500.0f, SOUNDPRIORITY_MISC_MEDIUM); break;
			case	2: PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\althit3.wav", 1500.0f, SOUNDPRIORITY_MISC_MEDIUM); break;
		}

		m_fLastFireTime = pServerDE->GetTime();
		m_hLastObj = hObj;
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaBallProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CTeslaBallProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_TESLA_ALT;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\altimpact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


BEGIN_CLASS(CSingularityProjectile)
END_CLASS_DEFAULT_FLAGS(CSingularityProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularityProjectile::CSingularityProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSingularityProjectile::CSingularityProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_SINGULARITY;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_fStartTime			= 0.0f;
	m_bArmed				= 0;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularityProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CSingularityProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, 0.1f);

	if(m_bArmed)
	{
		if(pServerDE->GetTime() - m_fStartTime > SINGULARITY_ATTRACT_TIME)
			return DFALSE;

		// Get the head of the list of destructable objects
		DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
		if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DTRUE;

		// Suck stuff in...
		DVector		vPos, vObjPos, vVel;
		DDWORD		dwUserFlags = 0;
		DFLOAT		magSqr;
		HOBJECT		hObj = 0;

		pServerDE->GetObjectPos(m_hObject, &vPos);

		while(pLink != &CDestructable::m_DestructableHead)
		{
			hObj = ((CDestructable*)pLink->m_pData)->GetObject();
			pLink = pLink->m_pNext;
			if(!hObj)	break;

			dwUserFlags = pServerDE->GetObjectUserFlags(hObj);

			// Check to see if we should just skip this object
			if(hObj == m_hObject) continue;
			if(pServerDE->GetObjectType(hObj) != OT_MODEL) continue;
			if(!(dwUserFlags & USRFLG_SINGULARITY_ATTRACT)) continue;

			pServerDE->GetObjectPos(hObj, &vObjPos);
			magSqr = VEC_DISTSQR(vObjPos, vPos);

			// Check to see if it's too far away
			if(magSqr > SINGULARITY_ATTRACT_RADIUS * SINGULARITY_ATTRACT_RADIUS) continue;

			// If it's in the kill radius... remove it or damage it a lot
			if(magSqr <= SINGULARITY_KILL_RADIUS * SINGULARITY_KILL_RADIUS)
			{
				VEC_INIT(vVel)
				pServerDE->SetVelocity(hObj, &vVel);
				pServerDE->MoveObject(hObj, &vPos);

				// If it's not a character object... remove it and do a cool effect
				if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter"))) 
				{
					pServerDE->RemoveObject(hObj);

					// Create an effect at the source of the blackhole
					DVector		offset;
					VEC_SET(offset, 0.0f, 0.0f, 0.0f);

					HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
					pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

					pServerDE->WriteToMessageObject(hMessage, m_hObject);
					pServerDE->WriteToMessageVector(hMessage, &offset);
					pServerDE->WriteToMessageFloat(hMessage, 1.0f);
					pServerDE->WriteToMessageDWord(hMessage, 0);
					pServerDE->WriteToMessageDWord(hMessage, OBJFX_SINGULARITY_1);
					pServerDE->WriteToMessageDWord(hMessage, 0);

					pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
				}
				else
				{
					DamageObject(m_hFiredFrom, this, hObj, m_fDamage * 2.0f, m_vDir, vPos, m_nDamageType);
				}
			}
			else
			{
				// If we're half way to the black hole... start damaging the object
				if(magSqr <= (SINGULARITY_ATTRACT_RADIUS * SINGULARITY_ATTRACT_RADIUS) / 2.0f)
				{
					if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
						DamageObject(m_hFiredFrom, this, hObj, m_fDamage, m_vDir, vPos, m_nDamageType);
				}

				DFLOAT	fAccel = SINGULARITY_ATTRACT_RADIUS - (magSqr / SINGULARITY_ATTRACT_RADIUS);

				pServerDE->GetVelocity(hObj, &vVel);

				VEC_SUB(vObjPos, vPos, vObjPos);
				VEC_NORM(vObjPos);

				VEC_MULSCALAR(vObjPos, vObjPos, fAccel * 0.75f);
				VEC_ADD(vVel, vVel, vObjPos);
				pServerDE->SetVelocity(hObj, &vVel);
			}
		}
	}
	else
	{
		//*************************************************************
		// Check to see where the beam hit and create an explosion
		//*************************************************************

		IntersectQuery iq;
		IntersectInfo  ii;
		DRotation rRot;
		DVector vFrom, vTo, vDist, vFire;

		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vTo, &vTo, &vFire);

		pServerDE->GetObjectPos(m_hObject, &vFrom);
		VEC_COPY(vTo, vFrom);
		VEC_INIT(vDist);
    
		VEC_MULSCALAR(vDist, vFire, 3000.0f)
    
		VEC_ADD(vTo, vTo, vDist);

		VEC_COPY(iq.m_From, vFrom);
		VEC_COPY(iq.m_To, vTo);
    
		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = LiquidFilterFn;
		iq.m_pUserData = NULL;	

		DBOOL bHit;
		
		if(bHit = pServerDE->IntersectSegment(&iq, &ii))
		{
			if(m_hFiredFrom == ii.m_hObject)
			{
				VEC_COPY(iq.m_From, ii.m_Point);
				bHit = pServerDE->IntersectSegment(&iq, &ii);		
			}
		}

		// Test to see if we hit anything...
		if(bHit)
		{
			VEC_COPY(vTo, ii.m_Point);
			VEC_COPY(vDist, ii.m_Plane.m_Normal);
		}
		else
		{
			VEC_SET(vDist, 0.0f, 1.0f, 0.0f);
			VEC_COPY(vTo, iq.m_To);
		}

		m_bArmed		= DTRUE;
		m_fStartTime	= g_pServerDE->GetTime();

		VEC_MULSCALAR(vDist, vDist, 25.0f);
		VEC_ADD(vTo, vTo, vDist);
		VEC_NORM(vDist);

		pServerDE->TeleportObject(m_hObject, &vTo);
		AddExplosion(vTo, vDist);

		VEC_SET(vFrom, 0.0f, 0.0f, 0.0f)
		pServerDE->SetVelocity(m_hObject, &vFrom);
		pServerDE->SetAcceleration(m_hObject, &vFrom);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularityProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CSingularityProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_SINGULARITY_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Singularity\\impact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CSingularityAltProjectile)
END_CLASS_DEFAULT_FLAGS(CSingularityAltProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularityAltProjectile::CSingularityAltProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSingularityAltProjectile::CSingularityAltProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Ammo\\Lightining.abc";
	m_pProjectileSkin		= "Skins\\Ammo\\Pulsebeam.dtx";

	m_nDamageType			= DAMAGE_TYPE_SINGULARITY;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_fStartTime			= 0.0f;
	m_bArmed				= 0;

	VEC_INIT(m_vInitScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularityAltProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CSingularityAltProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, 0.1f);

	if(m_bArmed)
	{
		if(pServerDE->GetTime() - m_fStartTime > SINGULARITY_ATTRACT_TIME * 2.0f)
			return DFALSE;

		// Get the head of the list of destructable objects
		DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
		if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DTRUE;

		// Suck stuff in...
		DVector		vPos, vObjPos, vVel;
		DDWORD		dwUserFlags = 0;
		DFLOAT		magSqr;
		HOBJECT		hObj = 0;

		pServerDE->GetObjectPos(m_hFiredFrom, &vPos);

		while(pLink != &CDestructable::m_DestructableHead)
		{
			hObj = ((CDestructable*)pLink->m_pData)->GetObject();
			pLink = pLink->m_pNext;
			if(!hObj)	break;

			dwUserFlags = pServerDE->GetObjectUserFlags(hObj);

			// Check to see if we should just skip this object
			if(hObj == m_hObject) continue;
			if(hObj == m_hFiredFrom) continue;
			if(pServerDE->GetObjectType(hObj) != OT_MODEL) continue;
			if(!(dwUserFlags & USRFLG_SINGULARITY_ATTRACT)) continue;

			pServerDE->GetObjectPos(hObj, &vObjPos);
			magSqr = VEC_DISTSQR(vObjPos, vPos);

			// Check to see if it's too far away
			if(magSqr > SINGULARITY_ATTRACT_ALT_RADIUS * SINGULARITY_ATTRACT_ALT_RADIUS) continue;

			// If it's in the kill radius... remove it or damage it a lot
			if(magSqr <= SINGULARITY_KILL_RADIUS * SINGULARITY_KILL_RADIUS)
			{
				VEC_INIT(vVel)
				pServerDE->SetVelocity(hObj, &vVel);
				pServerDE->MoveObject(hObj, &vPos);

				// If it's not a character object... remove it and do a cool effect
				if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter"))) 
				{
					pServerDE->RemoveObject(hObj);

					// Create an effect at the source of the blackhole
					DVector		offset;
					VEC_SET(offset, 0.0f, 0.0f, 0.0f);

					HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
					pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

					pServerDE->WriteToMessageObject(hMessage, m_hObject);
					pServerDE->WriteToMessageVector(hMessage, &offset);
					pServerDE->WriteToMessageFloat(hMessage, 1.0f);
					pServerDE->WriteToMessageDWord(hMessage, 0);
					pServerDE->WriteToMessageDWord(hMessage, OBJFX_SINGULARITY_1);
					pServerDE->WriteToMessageDWord(hMessage, 0);

					pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
				}
			}
			else
			{
				// If we're half way to the black hole... start damaging the object
				if(magSqr <= (SINGULARITY_ATTRACT_ALT_RADIUS * SINGULARITY_ATTRACT_ALT_RADIUS) / 2.0f)
				{
					if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
						DamageObject(m_hFiredFrom, this, hObj, m_fDamage, m_vDir, vPos, m_nDamageType);
				}

				DFLOAT	fAccel = SINGULARITY_ATTRACT_ALT_RADIUS - (magSqr / SINGULARITY_ATTRACT_ALT_RADIUS);

				pServerDE->GetVelocity(hObj, &vVel);

				VEC_SUB(vObjPos, vPos, vObjPos);
				VEC_NORM(vObjPos);

				VEC_MULSCALAR(vObjPos, vObjPos, fAccel * 0.5f);
				VEC_ADD(vVel, vVel, vObjPos);
				pServerDE->SetVelocity(hObj, &vVel);
			}
		}
	}
	else
	{
		// Create a semi-blackhole effect
		DVector		offset;
		VEC_SET(offset, 0.0f, 0.0f, 0.0f);

		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hFiredFrom);
		pServerDE->WriteToMessageVector(hMessage, &offset);
		pServerDE->WriteToMessageFloat(hMessage, 1.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, OBJFX_SINGULARITY_2);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

		// Create a semi-blackhole effect
		hMessage = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hFiredFrom);
		pServerDE->WriteToMessageVector(hMessage, &offset);
		pServerDE->WriteToMessageFloat(hMessage, 1.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, OBJFX_SINGULARITY_3);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

		m_bArmed		= DTRUE;
		m_fStartTime	= g_pServerDE->GetTime();
	}

	return DTRUE;
}



BEGIN_CLASS(CEnergyBlastProjectile)
END_CLASS_DEFAULT_FLAGS(CEnergyBlastProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlastProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CEnergyBlastProjectile::CEnergyBlastProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;
	m_fLifeTime				= 10.0f;

	m_hstrShockwaveFilename	= g_pServerDE->CreateString("");
	// m_pImpactFilename		= "Sprites\\Explosn.spr";

	m_pProjectileFilename	= "Sprites\\yellowflare2.spr";

	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_bZealot				= DFALSE;
	m_bDivine				= DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlastProjectile::Init
//
//	PURPOSE:	Initialize a projectile
//
// ----------------------------------------------------------------------- //

void CEnergyBlastProjectile::Setup(DVector *vDir, DBYTE nType, DFLOAT fDamage, DFLOAT fVelocity,
									int nRadius, HOBJECT hFiredFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hFiredFrom) return;

	HCLASS hZealot = pServerDE->GetClass("ZealotAI");
	HCLASS hClass = pServerDE->GetObjectClass(hFiredFrom);

	m_bZealot = pServerDE->IsKindOf(hClass, hZealot);

	if(m_bZealot)
	{
		ZealotAI* pZealot = (ZealotAI*)pServerDE->HandleToObject(hFiredFrom);

		m_bDivine = pZealot->IsDivine();
	}

	CProjectile::Setup(vDir, nType, fDamage, fVelocity, nRadius, hFiredFrom);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlastProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

DBOOL CEnergyBlastProjectile::InitialUpdate(DVector* vec)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	if (m_bZealot)
	{
		if(m_bDivine)
		{
			pServerDE->SetModelFilenames(m_hObject, "Sprites\\divineflare.spr", DNULL);
			AddLight(150.0f, 1.0f, 0.0f, 0.0f);
			m_dwTrailFXID = OBJFX_FIRETRAIL_1;
		}
		else
		{
			pServerDE->SetModelFilenames(m_hObject, "Sprites\\pulseflare.spr", DNULL);
			AddLight(150.0f, 0.0f, 0.0f, 1.0f);
			m_dwTrailFXID = OBJFX_FIRETRAIL_2;
		}
	}
	else
		m_dwTrailFXID = OBJFX_FIRETRAIL_2;

	m_fTrailScale = 0.5f;
	m_dwTrailScaleFlags = OBJFX_SCALELIFETIME;

	DBOOL bRet = CProjectile::InitialUpdate(vec);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlastProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CEnergyBlastProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlastProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CEnergyBlastProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_TESLA_PRIMARY;

	if (m_bZealot)
	{
		if(m_bDivine)
			nType = EXP_FLAME_SMALL;
		else
			nType = EXP_TESLA_PRIMARY;
	}

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}




BEGIN_CLASS(CFireballProjectile)
END_CLASS_DEFAULT_FLAGS(CFireballProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireballProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFireballProjectile::CFireballProjectile() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;
	m_fLifeTime				= 10.0f;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	AddLight(150.0f, 1.0f, 0.75f, 0.0f);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_fTrailScale			= 0.5f;
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_FIRETRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireballProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CFireballProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}

BEGIN_CLASS(CGroundStrikeProjectile)
END_CLASS_DEFAULT_FLAGS(CGroundStrikeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGroundStrikeProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGroundStrikeProjectile::CGroundStrikeProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DFALSE;
	m_fLifeTime				= 10.0f;

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE | FLAG_NOSLIDING;

	AddLight(150.0f, 1.0f, 0.75f, 0.0f);
	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_fTrailScale			= 0.5f;
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_GROUNDFLAME_2;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGroundStrikeProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CGroundStrikeProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireballProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CFireballProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_FLAME_SMALL;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}



BEGIN_CLASS(CShockwaveProjectile)
END_CLASS_DEFAULT_FLAGS(CShockwaveProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShockwaveProjectile::CShockwaveProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CShockwaveProjectile::CShockwaveProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DTRUE;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShockwaveProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CShockwaveProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vNormal;
	DDWORD nType = EXP_DIVINE_SHOCKBALL;
	
	HCLASS hClass = pServerDE->GetClass("ZealotAI");

	if(pServerDE->IsKindOf(hClass, pServerDE->GetObjectClass(m_hFiredFrom)))
	{
		ZealotAI* pZealot = (ZealotAI*)pServerDE->HandleToObject(m_hFiredFrom);

		if(!pZealot->IsDivine())
			nType = EXP_SHOCKBALL;
		else
			nType = EXP_DIVINE_SHOCKBALL;
	}

	VEC_SET(vNormal, 0.0f, -1000.0f, 0.0f);
	pServerDE->GetObjectPos(m_hFiredFrom, &vPos);

	//********************************************************************
 	// Get the normal of the plane that the person who fired the shot is standing on
	IntersectQuery	iq;
	IntersectInfo	ii;

	VEC_COPY(iq.m_From, vPos);
	VEC_ADD(iq.m_To, iq.m_To, vNormal);
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if(pServerDE->IntersectSegment(&iq, &ii))
	{
		VEC_COPY(vNormal, ii.m_Plane.m_Normal);
		VEC_COPY(vPos, ii.m_Point);
	}

	//********************************************************************
	// Create the explosion type
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	//********************************************************************
	// Move all the objects within the radius back and randomly rotate them a little

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DFALSE;

	// Work with the squares of the distances so as to not have to get a square root.
	DVector		vObjPos, vObjVel, vUp;
	DRotation	rObjRot;
	HOBJECT		hObj = 0;
	DFLOAT		mag, randRot = MATH_PI * LIFELEECH_ROTATE_AMOUNT;
	short		objType;
	DDWORD		dwUserFlags;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		pLink = pLink->m_pNext;
		if(!hObj)	break;

		objType = pServerDE->GetObjectType(hObj);
		dwUserFlags = pServerDE->GetObjectUserFlags(hObj);
		pServerDE->GetObjectPos(hObj, &vObjPos);

		if(hObj == m_hFiredFrom) continue;
		if((objType != OT_MODEL) || !(dwUserFlags & USRFLG_SINGULARITY_ATTRACT)) continue;
//		if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter"))) continue;
		if(VEC_DISTSQR(vObjPos, vPos) > LIFELEECH_PUSH_RADIUS * LIFELEECH_PUSH_RADIUS) continue;

		// Get all the object information
		pServerDE->GetObjectRotation(hObj, &rObjRot);
		pServerDE->GetVelocity(hObj, &vObjVel);

		// Randomly rotate the object a little
//		pServerDE->RotateAroundAxis(&rObjRot, &vUp, pServerDE->Random(-randRot, randRot));
//		pServerDE->SetObjectRotation(hObj, &rObjRot);

		// Get a vector from the position to the object and scale it to adjust the velocity
		VEC_SUB(vObjPos, vObjPos, vPos);
		mag = LIFELEECH_PUSH_RADIUS - VEC_MAG(vObjPos);
		if(mag < 0.0f)	mag = 0.0f;
		VEC_NORM(vObjPos);
		VEC_MULSCALAR(vObjPos, vObjPos, mag * 7.5f);

		// Offset the objects velocity by a fraction of the gravity and the scaled vector
		pServerDE->GetGlobalForce(&vUp);
		VEC_MULSCALAR(vUp, vUp, -0.25f);
		VEC_ADD(vObjVel, vObjVel, vUp);
		VEC_ADD(vObjVel, vObjVel, vObjPos);

		// Set the new object velocity, and damage it a little
//		pServerDE->SetVelocity(hObj, &vObjVel);
		pServerDE->GetObjectPos(hObj, &vObjPos);
		DamageObject(m_hFiredFrom, this, hObj, m_fDamage, vObjVel, vObjPos, DAMAGE_TYPE_EXPLODE);
	}

	return DFALSE;
}



BEGIN_CLASS(CBehemothShockwaveProjectile)
END_CLASS_DEFAULT_FLAGS(CBehemothShockwaveProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBehemothShockwaveProjectile::CBehemothShockwaveProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBehemothShockwaveProjectile::CBehemothShockwaveProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DTRUE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DTRUE;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBehemothShockwaveProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CBehemothShockwaveProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vNormal;

	VEC_SET(vNormal, 0.0f, -1000.0f, 0.0f);
	pServerDE->GetObjectPos(m_hFiredFrom, &vPos);

	//********************************************************************
 	// Get the normal of the plane that the person who fired the shot is standing on
	IntersectQuery	iq;
	IntersectInfo	ii;

	VEC_COPY(iq.m_From, vPos);
	VEC_ADD(iq.m_To, iq.m_To, vNormal);
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if(pServerDE->IntersectSegment(&iq, &ii))
	{
		VEC_COPY(vNormal, ii.m_Plane.m_Normal);
		VEC_COPY(vPos, ii.m_Point);
	}

	//********************************************************************
	// Create the explosion type
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, EXP_SHOCKBALL_LARGE);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	//********************************************************************
	// Damage the surrounding objects
	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DFALSE;

	HOBJECT		hObj;

	// Go through the list of destructable objects and look for people within your FOV
	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;

		if(hObj == m_hFiredFrom)
			{ pLink = pLink->m_pNext; continue; }

		DVector	vDir, vObjPos;
		DFLOAT	fDist = 0.0f;

		// Test the angle between this object and us
		pServerDE->GetObjectPos(hObj, &vObjPos);
		VEC_SUB(vDir, vObjPos, vPos);
		fDist = VEC_MAG(vDir);

		// If the target is too far away... skip it
		if(fDist > LIFELEECH_PUSH_RADIUS)
			{ pLink = pLink->m_pNext; continue; }

		DamageObject(m_hFiredFrom, this, hObj, m_fDamage, vDir, vObjPos, DAMAGE_TYPE_EXPLODE);
		pLink = pLink->m_pNext;
	}

//	DamageObjectsInRadius(m_hFiredFrom, this, vPos, LIFELEECH_PUSH_RADIUS,
//						  m_fDamage, DAMAGE_TYPE_EXPLODE);

	return DFALSE;
}


BEGIN_CLASS(CShikariLoogieProjectile)
END_CLASS_DEFAULT_FLAGS(CShikariLoogieProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShikariLoogieProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CShikariLoogieProjectile::CShikariLoogieProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DFALSE;
	m_fImpactScaleMin		= 1.0f;
	m_fImpactScaleMax		= 5.0f;
	m_fImpactDuration		= 1.0f;
	m_fLifeTime				= 10.0f;

	m_hstrShockwaveFilename	= g_pServerDE->CreateString("");
	// m_pImpactFilename		= "Sprites\\Explosn.spr";

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_ACID;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE | FLAG_NOSLIDING;

	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_fTrailScale			= 0.25f;
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_BUGSPRAY_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShikariLoogieProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CShikariLoogieProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}

void CShikariLoogieProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Placeholder until acid explosion is ready.
	DDWORD		nType = EXP_FLAME_SMALL;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


BEGIN_CLASS(CNagaSpikeProjectile)
END_CLASS_DEFAULT_FLAGS(CNagaSpikeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaSpikeProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNagaSpikeProjectile::CNagaSpikeProjectile() : CProjectile(OT_MODEL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;
	m_fImpactScaleMin		= 1.0f;
	m_fImpactScaleMax		= 5.0f;
	m_fImpactDuration		= 1.0f;
	m_fLifeTime				= 10.0f;

	m_pProjectileFilename	= "Models\\Ammo\\Nagaspike.abc";
	m_pProjectileSkin		= "Skins\\Powerups\\Nagaspike_pu.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_fStartTime			= g_pServerDE->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaSpikeProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CNagaSpikeProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaSpikeProjectile::AddExplosion()
//
//	PURPOSE:	Handle the explosion creation
//
// ----------------------------------------------------------------------- //

void CNagaSpikeProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_NAGA_SPIKE;
	DVector		vTemp;

	VEC_SET(vTemp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vTemp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\Tesla\\altimpact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


BEGIN_CLASS(CNagaDebrisProjectile)
END_CLASS_DEFAULT_FLAGS(CNagaDebrisProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaDebrisProjectile::CNagaDebrisProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNagaDebrisProjectile::CNagaDebrisProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DTRUE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DTRUE;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaDebrisProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CNagaDebrisProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	int i = pServerDE->IntRandom(3,5);

	DVector vTargetPos;

#define NAGA_DEBRIS_RADIUS		150.0f
#define NAGA_DEBRIS_MINRADIUS	0.5f

	HCLASS hClass = pServerDE->GetClass("AI_Mgr");
	HCLASS hFired = pServerDE->GetObjectClass(m_hFiredFrom);

	// Make sure we're an AI and that we can get a target position,
	// otherwise use the explosion position. :)
	//
	// This shouldn't really be necessary, since only the Naga should use
	// this weapon, but still, better safe than sorry if someone wants to use
	// this in a mod somewhere... :)
	//    --Loki
	if(pServerDE->IsKindOf(hFired, hClass))
	{
		AI_Mgr *pAI;
		pAI = (AI_Mgr*)(pServerDE->HandleToObject(m_hFiredFrom));
		pServerDE->GetObjectPos(pAI->GetTarget(),&vTargetPos);
	}
	else
	{
		pServerDE->GetObjectPos(m_hObject,&vTargetPos);
	}

	// Get a position in between the Naga's hands to explode from
	DVector		vHand, vUp;
	DRotation	rHand;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	if(pServerDE->GetModelNodeTransform(m_hFiredFrom, "l_hand", &vHand, &rHand))
		AddExplosion(vHand, vUp);

	if(pServerDE->GetModelNodeTransform(m_hFiredFrom, "r_hand", &vHand, &rHand))
		AddExplosion(vHand, vUp);

	// Do nasty stuff to make debris
	for (int loop = 0; loop < i; loop++)
	{
		DVector vPos;
		DFLOAT	fTemp;

		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_ObjectType = OT_MODEL;

		VEC_COPY(vPos, vTargetPos);

		fTemp = pServerDE->Random(NAGA_DEBRIS_MINRADIUS, NAGA_DEBRIS_RADIUS);
		fTemp = (pServerDE->IntRandom(0,1)) ? fTemp : -fTemp;

		vPos.x += fTemp;

		fTemp = pServerDE->Random(NAGA_DEBRIS_MINRADIUS, NAGA_DEBRIS_RADIUS);
		fTemp = (pServerDE->IntRandom(0,1)) ? fTemp : -fTemp;

		vPos.z += fTemp;

		IntersectQuery iq;
		IntersectInfo ii;

		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		iq.m_FilterFn = DNULL; //LiquidFilterFn;
		iq.m_pUserData = DNULL;

		VEC_COPY(iq.m_From, vPos);
		VEC_COPY(iq.m_To, vPos);

		iq.m_To.y += 1000;

		if(pServerDE->IntersectSegment(&iq, &ii))
			{ VEC_COPY(ocStruct.m_Pos,ii.m_Point); }
		else
			{ VEC_COPY(ocStruct.m_Pos,iq.m_To); }

		ocStruct.m_Pos.y -= 50;

		HCLASS hClass = pServerDE->GetClass("NagaCeilingDebris");
		BaseClass *pObj = pServerDE->CreateObject(hClass, &ocStruct);

		// Random rotation
		DRotation rRot;
		HOBJECT hObj = pServerDE->ObjectToHandle(pObj);

		pServerDE->GetObjectRotation(hObj, &rRot);
		pServerDE->EulerRotateX(&rRot, pServerDE->Random(-MATH_PI, MATH_PI));
		pServerDE->EulerRotateY(&rRot, pServerDE->Random(-MATH_PI, MATH_PI));
		pServerDE->EulerRotateZ(&rRot, pServerDE->Random(-MATH_PI, MATH_PI));
		pServerDE->SetObjectRotation(hObj, &rRot);
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaDebrisProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CNagaDebrisProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_NAGA_POUND_GROUND;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	// Make the screen shake...
/*	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientWeaponSFX");

	CClientWeaponSFX *pWeaponFX = DNULL;

	if(hClass)
		pWeaponFX = (CClientWeaponSFX*)pServerDE->CreateObject(hClass, &theStruct);
*/
	DVector	vTarget;
	DDWORD	nFX = WFX_SCREENSHAKE;
	DDWORD	nExtras = WFX_EXTRA_DAMAGE;
	WeaponFXExtras	ext;
	ext.fDamage = m_fDamage;

	// Get a location to shake the screen from (the target for maximum shakage)! :)
	if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hFiredFrom), pServerDE->GetClass("AI_Mgr")))
	{
		AI_Mgr *pAI;
		pAI = (AI_Mgr*)(pServerDE->HandleToObject(m_hFiredFrom));
		pServerDE->GetObjectPos(pAI->GetTarget(), &vTarget);
	}
	else
		pServerDE->GetObjectPos(m_hObject, &vTarget);

//	if(pWeaponFX)
	SendWeaponSFXMessage(&vTarget, &vTarget, &vNormal, &vNormal, nFX, nExtras, &ext);
}


BEGIN_CLASS(CVomitProjectile)
END_CLASS_DEFAULT_FLAGS(CVomitProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVomitProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVomitProjectile::CVomitProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DFALSE;
	m_fImpactScaleMin		= 1.0f;
	m_fImpactScaleMax		= 5.0f;
	m_fImpactDuration		= 1.0f;
	m_fLifeTime				= 10.0f;

	m_hstrShockwaveFilename	= g_pServerDE->CreateString("");
	// m_pImpactFilename		= "Sprites\\Explosn.spr";

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE | FLAG_NOSLIDING;

	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_fTrailScale			= 0.25f;
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_BUGSPRAY_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVomitProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CVomitProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vScale;
	VEC_SET(vScale, 0.25f, 0.25f, 0.25f);

	if(pServerDE->GetTime() - m_fStartTime < m_fDelay)
		VEC_INIT(vScale);

	pServerDE->ScaleObject(m_hObject, &vScale);

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}



BEGIN_CLASS(CGooProjectile)
END_CLASS_DEFAULT_FLAGS(CGooProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGooProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGooProjectile::CGooProjectile() : CProjectile(OT_NORMAL)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DFALSE;
	m_bClientFX				= DFALSE;
	m_fImpactScaleMin		= 1.0f;
	m_fImpactScaleMax		= 5.0f;
	m_fImpactDuration		= 1.0f;
	m_fLifeTime				= 10.0f;

	m_hstrShockwaveFilename	= g_pServerDE->CreateString("");
	// m_pImpactFilename		= "Sprites\\Explosn.spr";

	m_pProjectileFilename	= "Sprites\\napalmprime.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_FORCECLIENTUPDATE | FLAG_NOSLIDING;

	m_fStartTime			= g_pServerDE->GetTime();
	m_fDelay				= 0.01f;

	m_fTrailScale			= 0.4f;
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_BUGSPRAY_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGooProjectile::Update()
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

DBOOL CGooProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = CProjectile::Update(pMovement);

	return bRet;
}


void CGooProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct ocStruct;

	INIT_OBJECTCREATESTRUCT(ocStruct);
	VEC_COPY(ocStruct.m_Pos,vPos);
	
	HCLASS hClass = pServerDE->GetClass("GooSplat");
	pServerDE->CreateObject(hClass,&ocStruct);
}

BEGIN_CLASS(CSkullProjectile)
END_CLASS_DEFAULT_FLAGS(CSkullProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::COrbProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSkullProjectile::CSkullProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;

	m_pProjectileFilename	= "Models\\Powerups\\skull.abc";
	m_pProjectileSkin		= "Skins\\Powerups\\skull.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;

	m_hTrackObj				= DNULL;
	m_fRotateAmount			= 0.0f;
	m_fHitTime				= 0.0f;
	m_fLastDamageTime		= 0.0f;
	m_bState				= SKULL_STATE_TRACKING;

	m_hSound				= 0;

	VEC_SET(m_vInitScale, 0.5f, 0.5f, 0.5f);

	m_dwTrailFXID			= OBJFX_FIRETRAIL_1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL CSkullProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vU, vR, vF;
	DRotation	rRot;
	DFLOAT		fMag;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	// Play a sound if we don't have one playing already
	if(!m_hSound)
	{
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Enemies\\deathshroud\\flyskull.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

		//fade the deathshroud
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
		fAlpha = 0.60f;
		pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);
	}

	// If the orb hit a non-player object, make it break into pieces
	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		Explode();
		return DFALSE;
	}

	if(m_bState == THEORB_STATE_TRACKING)
	{
		m_hTrackObj = FindTrackObj();
		if(m_hTrackObj)		TrackObjInRange();

		//*******************************************************************
		// Adjust the velocity according to the rotation
		pServerDE->GetVelocity(m_hObject, &vF);
		fMag = VEC_MAG(vF);

		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vF, vF, fMag);
		pServerDE->SetVelocity(m_hObject, &vF);
		//*******************************************************************
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CSkullProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hHitObject) return;
	if(hObj == m_hFiredFrom) return;

	// return if it hit a non solid object [gjk]
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	if(m_bState == THEORB_STATE_TRACKING)
	{
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			DVector		vVel, vPos, vU,vR,vF;
			VEC_INIT(vVel);
			pServerDE->SetVelocity(m_hObject, &vVel);

			m_hHitObject = hObj;
			m_fHitTime = pServerDE->GetTime();

			DRotation rRot;

			if(pServerDE->GetModelNodeTransform(m_hHitObject, "head", &vPos, &rRot))
			{
				// Set the orb in front of the players head that was hit
				pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

				VEC_MULSCALAR(vF, vF, SKULL_HEAD_OFFSET);
				VEC_ADD(vPos, vPos, vF);
				pServerDE->SetObjectPos(m_hObject, &vPos);

				pServerDE->RotateAroundAxis(&rRot, &vU, MATH_PI);
				pServerDE->SetObjectRotation(m_hObject, &rRot);

				DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, vF, vPos, m_nDamageType);
			}
		}

		m_bExplode = DTRUE;
	}

	// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
	pServerDE->CreateInterObjectLink(m_hObject, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CSkullProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_FLARE_BURST;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\howitzer\\impact.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::FindTrackObj()
//
//	PURPOSE:	Find any object within the range of the orb that has a head and is moving
//
// ----------------------------------------------------------------------- //

HOBJECT CSkullProjectile::FindTrackObj()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead)) return DNULL;

	// Work with the squares of the distances so as to not have to get a square root.
	HOBJECT		hObj, hTempObj	= DNULL;
	DFLOAT		fRadiusSqr		= SKULL_DETECT_RADIUS * SKULL_DETECT_RADIUS;
	DFLOAT		fHalfRadiusSqr	= fRadiusSqr / 2;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;

		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			DVector		vDir, vOrbPos, vHeadPos;
			DRotation	rRot;
			DFLOAT		fDistSqr;

			if(hObj == m_hFiredFrom)
				{ pLink = pLink->m_pNext; continue; }

			// Handle the motion detection
			pServerDE->GetVelocity(hObj, &vDir);
			if((vDir.x == 0.0f) && (vDir.y == 0.0f) && (vDir.z == 0.0f))
				{ pLink = pLink->m_pNext; continue; }

			// If the object has a 'head' node... get the dist from the orb to the head
			if(pServerDE->GetModelNodeTransform(hObj, "head", &vHeadPos, &rRot))
			{
				pServerDE->GetObjectPos(m_hObject, &vOrbPos);
				VEC_SUB(vDir, vHeadPos, vOrbPos);

				fDistSqr = VEC_MAGSQR(vDir);
				if(fDistSqr <= fRadiusSqr)
				{
					fRadiusSqr = fDistSqr;
					hTempObj = hObj;
				}
			}
		}

		pLink = pLink->m_pNext;
	}

	return hTempObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkullProjectile::TrackObjInRange()
//
//	PURPOSE:	Track the selected object
//
// ----------------------------------------------------------------------- //

void CSkullProjectile::TrackObjInRange()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return;

	DVector		vDir, vOrbPos, vHeadPos, vUp;
	DRotation	rTemp, rOrbRot, rHeadRot;
	DFLOAT		fTemp;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	// If the object is dead or something, stop tracking
	if(!m_hTrackObj)	return;

	// If the object has a 'head' node... get the dist from the orb to the head
	if(pServerDE->GetModelNodeTransform(m_hTrackObj, "head", &vHeadPos, &rHeadRot))
	{
		// Get the current position and rotation
		pServerDE->GetObjectPos(m_hObject, &vOrbPos);
		pServerDE->GetObjectRotation(m_hObject, &rOrbRot);

		// Create the destination rotation
		VEC_SUB(vDir, vHeadPos, vOrbPos);
		pServerDE->AlignRotation(&rHeadRot, &vDir, &vUp);

		fTemp = VEC_MAG(vDir);
		fTemp = 1.0f - (fTemp / SKULL_DETECT_RADIUS);

		// Try to get a rotation inbetween the current and the destination and set it
		pServerDE->InterpolateRotation(&rTemp, &rOrbRot, &rHeadRot, fTemp);
		pServerDE->SetObjectRotation(m_hObject, &rTemp);
	}
	else
		return;
}

#ifdef _ADD_ON

BEGIN_CLASS(CFlayerChain)
END_CLASS_DEFAULT_FLAGS(CFlayerChain, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::CFlayerChain()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlayerChain::CFlayerChain() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_bClientFX				= DFALSE;
	m_fLifeTime				= 6.5f;

	m_pProjectileFilename	= "Models_ao\\Ammo_ao\\chainhook.abc";
	m_pProjectileSkin		= "Skins_ao\\Ammo_ao\\chainhook.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_hSound				= 0;

	VEC_SET(m_vInitScale, 3.0f, 3.0f, 3.0f);
	m_damage.SetGodMode(DTRUE);

	m_hTrackObj				= DNULL;
	m_fHitTime				= 0.0f;
	m_fLastDamageTime		= 0.0f;
	m_fRetractTime			= 0.0f;
	m_bState				= FLAYER_STATE_TRACKING;

	VEC_INIT(m_vHookPos1);
	VEC_INIT(m_vHookPos2);

	m_byNumLinks			= 25;
	m_fScale				= 4.0;
	m_fStretchedLength		= m_byNumLinks * m_fScale;

	m_hGibAttachment		= DNULL;
	m_hGibObj				= DNULL;
	m_bFirstUpdate			= DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

DBOOL CFlayerChain::InitialUpdate(DVector* vec)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Do the normal initial update
	DBOOL bRet = CProjectile::InitialUpdate(vec);

	// Create a chain from the hook of the flayer
	HMESSAGEWRITE hMessage2 = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage2, SFX_FLAYER_CHAIN_ID);
	pServerDE->WriteToMessageByte(hMessage2, m_byNumLinks);
	pServerDE->WriteToMessageFloat(hMessage2, m_fScale);
	pServerDE->WriteToMessageFloat(hMessage2, 1.0f);
	pServerDE->WriteToMessageByte(hMessage2, 1);
	pServerDE->WriteToMessageByte(hMessage2, 0x01);

	pServerDE->EndMessage2(hMessage2, MESSAGE_GUARANTEED);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL CFlayerChain::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	////////////////////////////////////////////////////////////////////////
	if(m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;

		// Check to see if there's a target straight in front of us...
		IntersectQuery iq;
		IntersectInfo  ii;
		DRotation rRot;
		DVector vU, vR, vF;

		pServerDE->GetObjectPos(m_hObject, &(iq.m_From));
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_MULSCALAR(vF, vF, 3000.0f);
		VEC_ADD(iq.m_To, iq.m_From, vF);

		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = LiquidFilterFn;
		iq.m_pUserData = NULL;	

		DBOOL bHit = DFALSE;

		// Don't shoot the sap who fired this.
		if(bHit = pServerDE->IntersectSegment(&iq, &ii))
		{
			if(m_hFiredFrom == ii.m_hObject)
			{
				VEC_COPY(iq.m_From, ii.m_Point);
				bHit = pServerDE->IntersectSegment(&iq, &ii);		
			}
		}

		if(bHit && pServerDE->IsKindOf(pServerDE->GetObjectClass(ii.m_hObject), pServerDE->GetClass("CBaseCharacter")))
		{
			m_hTrackObj	= ii.m_hObject;
			pServerDE->CreateInterObjectLink(m_hObject, m_hTrackObj);
		}

		// Create a little warp explosion effect at the source
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
		pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

		pServerDE->WriteToMessageVector(hMessage, &(iq.m_From));
		pServerDE->WriteToMessageVector(hMessage, &vU);
		pServerDE->WriteToMessageDWord(hMessage, EXP_FLAYER_PRIMARY);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
	}

	////////////////////////////////////////////////////////////////////////
	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds_ao\\Weapons\\Flayer\\chainloop.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	// If the flayer hit a non-player object, make it break into pieces
	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		DeleteAttachedGIB();
		Explode();
		return DFALSE;
	}

	////////////////////////////////////////////////////////////////////////
	// Set a wiggle time
/*	if(!m_fWiggleStartTime)
	{
		if((pServerDE->GetTime() - m_fStartTime) > m_fWiggleDelay)
			m_fWiggleStartTime = pServerDE->GetTime();
	}
*/
	////////////////////////////////////////////////////////////////////////
	// If there was a character to track... then follow it
	if(m_hTrackObj)
	{
		// Attach to the character and hold him there for a while
		if(m_bState == FLAYER_STATE_ATTACHED)
			DamageAttached();
		else
		{
			TrackObject();
			AdjustVelocity();
		}
	}
	// else fly straight until we hit someone or something
	else
	{
		// Retract into the portal with a chunk of the creature
		if(m_bState == FLAYER_STATE_RETRACT)
		{
			DFLOAT	fTime = pServerDE->GetTime();

			if(fTime < m_fRetractTime + FLAYER_RETRACT_TIME)
			{
				DVector	vDir;

				VEC_SUB(vDir, m_vHookPos2, m_vHookPos1);
				VEC_MULSCALAR(vDir, vDir, (fTime - m_fRetractTime) / FLAYER_RETRACT_TIME);
				VEC_ADD(vDir, m_vHookPos1, vDir);

				pServerDE->SetObjectPos(m_hObject, &vDir);
			}
			else
			{
				DeleteAttachedGIB();
				return DFALSE;
			}
		}
		else
		{
			if((m_bState == FLAYER_STATE_ATTACHED) && !m_hTrackObj)
				ReleaseChain();
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CFlayerChain::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(!hObj) return;
	if(m_hHitObject) return;
	if(hObj == m_hFiredFrom) return;
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	if(!hObj) return;
	if(hObj == m_hFiredFrom) return;
	if(m_bState == FLAYER_STATE_TRACKING)
	{
		if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
		{
			// Set the appropriate variables for attachment
			m_bState = FLAYER_STATE_ATTACHED;
			m_fHitTime = pServerDE->GetTime();
			m_hHitObject = hObj;
			m_hTrackObj = hObj;

			// Stop the flayer where it hit
			DVector	vVel;
			VEC_SET(vVel, 0.0f, 0.0f, 0.0f);
			pServerDE->SetVelocity(m_hObject, &vVel);

			// Trap the character from the flayer (remove gravity if needed)
			CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(m_hTrackObj);
			pObj->Trap(DTRUE, DTRUE);

			// Create the portal and change the chain links to the attached mode
			HandleAttachedEffects();

			// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
			pServerDE->CreateInterObjectLink(m_hObject, hObj);
		}
		else
			m_bExplode = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CFlayerChain::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_FLAYER_SHATTER;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds_ao\\Weapons\\Flayer\\chainbreak.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::TrackObject()
//
//	PURPOSE:	Turn towards the tracked character to follow it
//
// ----------------------------------------------------------------------- //

void CFlayerChain::TrackObject()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE) return;

	DVector		vObjPos, vFlayerPos, vTemp, vU, vR, vF;
	DRotation	rFlayerRot, rNewFlayerRot, rTemp;

	// Get location and rotation information
	pServerDE->GetObjectPos(m_hTrackObj, &vObjPos);
	pServerDE->GetObjectPos(m_hObject, &vFlayerPos);
	pServerDE->GetObjectRotation(m_hObject, &rFlayerRot);
	pServerDE->GetRotationVectors(&rFlayerRot, &vU, &vR, &vF);

	// Get a direction vector and rotation between the flayer and track object
	VEC_SUB(vTemp, vObjPos, vFlayerPos);
	pServerDE->AlignRotation(&rNewFlayerRot, &vTemp, &vU);

	// Try to get a rotation inbetween the current and the destination and set it
	pServerDE->InterpolateRotation(&rTemp, &rFlayerRot, &rNewFlayerRot, 0.2f);
	pServerDE->SetObjectRotation(m_hObject, &rTemp);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::AdjustVelocity()
//
//	PURPOSE:	Adjust the velocity according to the rotation
//
// ----------------------------------------------------------------------- //

void CFlayerChain::AdjustVelocity()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vU, vR, vF;
	DRotation rTemp;
	DFLOAT fTemp = 0.0f;

	pServerDE->GetObjectRotation(m_hObject, &rTemp);
	pServerDE->GetVelocity(m_hObject, &vF);
	fTemp = VEC_MAG(vF);

	pServerDE->GetRotationVectors(&rTemp, &vU, &vR, &vF);
	VEC_MULSCALAR(vF, vF, fTemp);
	pServerDE->SetVelocity(m_hObject, &vF);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::DamageAttached()
//
//	PURPOSE:	Damage the character the flayer is attached to
//
// ----------------------------------------------------------------------- //

DBOOL CFlayerChain::DamageAttached()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT	fTime = pServerDE->GetTime();

	// See if we should let go of the character
	CBaseCharacter *pObj = DNULL;
	if(m_hTrackObj)
		pObj = (CBaseCharacter*)pServerDE->HandleToObject(m_hTrackObj);

	if(!m_hTrackObj || !m_hHitObject || (pObj && pObj->IsDead()) || (fTime > m_fHitTime + FLAYER_TRAP_TIME))
	{
		ReleaseChain();

		// Free the character from the flayer
		if(pObj)
		{
			if(fTime > m_fHitTime + FLAYER_TRAP_TIME)
				pObj->Trap(DFALSE, DTRUE);
			else
				pObj->Trap(-1, DTRUE);
		}

		return DFALSE;
	}

	// See if we should damage the character again
	if(m_hHitObject && fTime > (m_fLastDamageTime + FLAYER_DAMAGE_INTERVAL))
	{
		DVector vDir, vObjPos;
		VEC_SET(vDir, 0.0f, 0.0f, 0.0f);
		pServerDE->GetObjectPos(m_hHitObject, &vObjPos);
		DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, vDir, vObjPos, DAMAGE_TYPE_NORMAL);

		m_fLastDamageTime = fTime;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::ReleaseChain()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CFlayerChain::ReleaseChain()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT	fTime = pServerDE->GetTime();

	m_hTrackObj = DNULL;
	m_hHitObject = DNULL;
	m_bState = FLAYER_STATE_RETRACT;
	m_fRetractTime = fTime;

	// Create a gib at the end of the hook
	CreateAttachedGIB();

	// Switch sounds to the retract sound
	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds_ao\\Weapons\\Flayer\\return.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::HandleAttachedEffects()
//
//	PURPOSE:	Create a portal and change the chain effect to attached mode
//
// ----------------------------------------------------------------------- //

void CFlayerChain::HandleAttachedEffects()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DRotation rRot;
	DVector	vU, vR, vF;

	// Set the retraction positions
	pServerDE->GetObjectPos(m_hObject, &m_vHookPos1);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, -m_fStretchedLength);
	VEC_ADD(m_vHookPos2, m_vHookPos1, vF);

	// TODO: Create the portal effect at HookPos2 and call the chain SFX with new mode
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vHookPos2);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vHookPos2);
	pServerDE->WriteToMessageVector(hMessage, &vU);
	pServerDE->WriteToMessageDWord(hMessage, EXP_FLAYER_RETRACT);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	// Switch sounds to the attached sound
	if(m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds_ao\\Weapons\\Flayer\\loophold.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::CreateAttachedGIB()
//
//	PURPOSE:	Create a fleshy GIB on the end of the hook
//
// ----------------------------------------------------------------------- //

void CFlayerChain::CreateAttachedGIB()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector		vOffset;
	DRotation	rRotOffset;

	// Create the gib model and attach it
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL; 
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE;
	pServerDE->GetObjectPos(m_hObject, &(ocStruct.m_Pos));
	strcpy(ocStruct.m_Filename, "models\\gibs\\flesh\\gib5.abc");
	strcpy(ocStruct.m_SkinName, "skins\\gibs\\flesh\\gib7.dtx");
	VEC_SET(ocStruct.m_Scale, g_pServerDE->Random(0.5f,0.75f), g_pServerDE->Random(0.5f,0.75f), g_pServerDE->Random(0.4f,0.6f));
	ocStruct.m_NextUpdate = 0.01f;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	m_hGibObj = pServerDE->CreateObject(hClass, &ocStruct);

	if(m_hGibObj && m_hGibObj->m_hObject)
	{
		VEC_SET(vOffset, 0.0f, 0.0f, 5.0f);
		ROT_INIT(rRotOffset);
		pServerDE->EulerRotateX(&rRotOffset, pServerDE->Random(0, 360));
		pServerDE->EulerRotateY(&rRotOffset, pServerDE->Random(0, 360));
		pServerDE->CreateAttachment(this->m_hObject, m_hGibObj->m_hObject, DNULL, &vOffset, &rRotOffset, &m_hGibAttachment);

		// Create a blood trail from the end of the gib
		VEC_INIT(vOffset);
		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(m_hGibObj);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hGibObj->m_hObject);
		pServerDE->WriteToMessageVector(hMessage, &vOffset);
		pServerDE->WriteToMessageFloat(hMessage, 1.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, OBJFX_BLOOD_TRAIL_1);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::DeleteAttachedGIB()
//
//	PURPOSE:	Remove the gib
//
// ----------------------------------------------------------------------- //

void CFlayerChain::DeleteAttachedGIB()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(m_hGibAttachment)
	{
		pServerDE->RemoveAttachment(m_hGibAttachment);
		m_hGibAttachment = DNULL;
	}

	if(m_hGibObj)
	{
		pServerDE->RemoveObject(m_hGibObj->m_hObject);
		m_hGibObj = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerChain::BreakLink
//
//	PURPOSE:	Breaks a link of an object
//
// ----------------------------------------------------------------------- //

void CFlayerChain::BreakLink(HOBJECT hObj)
{
	if(hObj == m_hTrackObj)
		m_hTrackObj = DNULL;
	else
		CProjectile::BreakLink(hObj);
}


BEGIN_CLASS(CFlayerPortal)
END_CLASS_DEFAULT_FLAGS(CFlayerPortal, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerPortal::CFlayerPortal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlayerPortal::CFlayerPortal() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fLifeTime				= 6.5f;

	m_pProjectileFilename	= 0;
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_NORMAL;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_hSound				= 0;

	VEC_SET(m_vInitScale, 0.0f, 0.0f, 0.0f);

	m_fSearchTime			= 0.0f;
	m_fLastChainTime		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerPortal::InitialUpdate(DVector *vec)
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DBOOL CFlayerPortal::InitialUpdate(DVector *vec)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector		vPos, vNormal;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.1);

	// Create a little warp explosion effect at the source
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, EXP_FLAYER_ALT);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	// Init the time that we started
	m_fSearchTime = pServerDE->GetTime();
	m_fLastChainTime = m_fSearchTime + FLAYER_ALT_CHAIN_INTERVAL;

	return CProjectile::InitialUpdate(vec);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayerPortal::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL CFlayerPortal::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT	fTime = pServerDE->GetTime();
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.1);

	////////////////////////////////////////////////////////////////////////
	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds_ao\\Weapons\\Flayer\\chainwarp.wav", 1000.0f, SOUNDPRIORITY_MISC_HIGH, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	// If we ran out of time... kill it
	if(fTime > (m_fSearchTime + FLAYER_ALT_SEARCH_TIME))
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		return DFALSE;
	}

	// See if we should let out another chain
	if(fTime > (m_fLastChainTime + FLAYER_ALT_CHAIN_INTERVAL))
	{
		// Reset the last chain time
		m_fLastChainTime = fTime;

		// Get the head of the list of destructable objects
		DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
		if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DTRUE;

		HOBJECT		hObj = DNULL, hTarget = DNULL;
		HCLASS		hCharacter	= pServerDE->GetClass("CBaseCharacter");
		HCLASS		hPlayer		= pServerDE->GetClass("CPlayerObj");
		HCLASS		hObjClass   = DNULL;

		// Get a forward vector and position of the player who shot the voodoo doll
		DVector		vPortalPos;
		pServerDE->GetObjectPos(m_hObject, &vPortalPos);

		// Go through the list of destructable objects and look for people within range
		while(pLink != &CDestructable::m_DestructableHead)
		{
			hObj = ((CDestructable*)pLink->m_pData)->GetObject();
			if(!hObj)	break;

			if(hObj == m_hFiredFrom)
				{ pLink = pLink->m_pNext; continue; }

			hObjClass = pServerDE->GetObjectClass(hObj);

			if(pServerDE->IsKindOf(hObjClass, hCharacter))
			{
				DVector		vObjPos, vDir;
				DFLOAT		fDist;

				// Test the dist between this object and us
				pServerDE->GetObjectPos(hObj, &vObjPos);
				VEC_SUB(vDir, vObjPos, vPortalPos);
				fDist = VEC_MAG(vDir);

				// If the target is too far away... skip it
				if(fDist > FLAYER_ALT_SEARCH_RANGE)
					{ pLink = pLink->m_pNext; continue; }

				hTarget = hObj;
				break;
			}

			pLink = pLink->m_pNext;
		}

		// Create a chain with a target
		if(hTarget)
		{
			DVector vDir;
			CFlayerChain *pChain = NULL;
			HCLASS hClass = pServerDE->GetClass("CFlayerChain");

			VEC_SET(vDir, pServerDE->Random(0.0f, 1.0f), 1.0f, pServerDE->Random(0.0f, 1.0f));
			VEC_NORM(vDir);

			// Create a new chain object
			ObjectCreateStruct ocStruct;
			INIT_OBJECTCREATESTRUCT(ocStruct);
			VEC_COPY(ocStruct.m_Pos, vPortalPos);
			pServerDE->AlignRotation(&(ocStruct.m_Rotation), &vDir, &vDir);

			if(hClass)
				pChain = (CFlayerChain*)pServerDE->CreateObject(hClass, &ocStruct);

			if(pChain)
			{
				pChain->Setup(&vDir, m_nWeaponType, 10.0f, 500.0f, 0, m_hFiredFrom);
				pChain->SetTarget(hTarget);
			}
		}
	}

	return DTRUE;
}


BEGIN_CLASS(CRockProjectile)
END_CLASS_DEFAULT_FLAGS(CRockProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRockProjectile::CRockProjectile()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRockProjectile::CRockProjectile() : CProjectile()
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fLifeTime				= 6.5f;
	m_fFadeTime				= 1.5f;

	m_pProjectileFilename	= "Models_ao\\Powerups_ao\\rock1_pu.abc";
	m_pProjectileSkin		= "Skins_ao\\Powerups_ao\\rock1_pu.dtx";

	m_nDamageType			= DAMAGE_TYPE_NORMAL;

	m_dwFlags = m_dwFlags | FLAG_GRAVITY;

	// Set up angular velocities
	m_fPitchVel = m_fYawVel = 0.0f;
	m_fPitch	= m_fYaw = 0.0f;
	m_nBounceCount = 5;

	m_hSound		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRockProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CRockProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Save position for bounce calculations
	pServerDE->GetObjectPos(m_hObject, &m_LastPos);
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	if (m_fLifeTime > 0)
	{
		DFLOAT fTime = pServerDE->GetTime();

		if(fTime > (m_fStartTime + m_fLifeTime))
		{
			return DFALSE;
		}
		else if(fTime > (m_fStartTime + m_fLifeTime - m_fFadeTime))
		{
			DFLOAT	alpha = 1.0f - ((fTime - (m_fStartTime + m_fLifeTime - m_fFadeTime)) / m_fFadeTime);
			DFLOAT	r, g, b, a;

			pServerDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			pServerDE->SetObjectColor(m_hObject, r, g, b, alpha);
		}
	}

	DVector vVel;
	DRotation rRot;

	pServerDE->GetVelocity( m_hObject ,&vVel );
	pServerDE->GetObjectRotation( m_hObject, &rRot );

	// If velocity slows enough, just stop bouncing and just wait to expire.
	if (VEC_MAG(vVel) < 5.0)
	{
		m_fPitchVel = 0;
		m_fYawVel = 0;

		// Stop the spinning
		pServerDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRockProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CRockProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if(hObj == m_hFiredFrom) return;	// Let it get out of our bounding box...
	if(m_nBounceCount <= 0) return;		// Don't do anything else if it stopped bouncing

	// return if it hit a non solid object
	if(hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// damage a player if it hit one
	if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter")))
	{
		DVector vDir, vObjPos;
		DFLOAT	fNewDamage = m_fDamage * ((DFLOAT)m_nBounceCount / 5.0f);

		pServerDE->GetObjectPos(m_hObject, &vObjPos);
		pServerDE->GetVelocity(m_hObject, &vDir);

		DamageObject(m_hFiredFrom, this, hObj, fNewDamage, vDir, vObjPos, DAMAGE_TYPE_NORMAL);
	}

 	// Cast a ray from our last known position to see what we hit
	DVector vVel, vPos;

	pServerDE->GetVelocity(m_hObject, &vVel);
	pServerDE->GetObjectPos( m_hObject, &vPos );

	CollisionInfo colInfo;
	pServerDE->GetLastCollision( &colInfo );

	// Compute new velocity reflected off of the surface.
	DVector vNormal;
	if( colInfo.m_hPoly )
	{
		VEC_COPY(vNormal, colInfo.m_Plane.m_Normal);

		DFLOAT r = ( VEC_DOT(vVel, vNormal) * 0.3f );
		VEC_MULSCALAR(vNormal, vNormal, r);

		// Play a bounce sound...
		PlaySoundFromObject(m_hObject, "Sounds\\Bounce\\basketball.wav", 500, SOUNDPRIORITY_MISC_MEDIUM);

		VEC_SUB(vVel, vVel, vNormal);

		// Adjust the bouncing..
		m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);
		m_fYawVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE);

		VEC_MULSCALAR(vVel, vVel, 0.5f);	// Lose some energy in the bounce.
		pServerDE->SetVelocity(m_hObject, &vVel);

	}
	else
	{
		VEC_INIT( vVel );
		pServerDE->SetVelocity( m_hObject, &vVel );

		// Play a bounce sound...
		PlaySoundFromObject(m_hObject, "Sounds\\Bounce\\basketball.wav", 500, SOUNDPRIORITY_MISC_MEDIUM);

	}

	m_nBounceCount--;
}

BEGIN_CLASS(CNightmareFireball)
END_CLASS_DEFAULT_FLAGS(CNightmareFireball, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::CNightmareFireball()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNightmareFireball::CNightmareFireball() : CProjectile(OT_SPRITE)
{
	m_bShockwave			= DFALSE;
	m_bExplosion			= DTRUE;
	m_fLifeTime				= 6.5f;

	m_pProjectileFilename	= "Sprites\\lleechproj.spr";
	m_pProjectileSkin		= 0;

	m_nDamageType			= DAMAGE_TYPE_EXPLODE;
	m_dwFlags				= m_dwFlags | FLAG_POINTCOLLIDE | FLAG_NOSLIDING;
	m_hSound				= 0;

	AddLight(100.0f, 1.0f, g_pServerDE->Random(0.25f,1.0f), 0.0f);

	VEC_SET(m_vInitScale, 0.3f, 0.3f, 0.3f);
	m_damage.SetGodMode(DTRUE);

	m_hTrackObj				= DNULL;

	m_fTrailScale			= g_pServerDE->Random(0.25f, 2.0f);
	m_dwTrailScaleFlags		= OBJFX_SCALELIFETIME;
	m_dwTrailFXID			= OBJFX_FIRETRAIL_1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

DBOOL CNightmareFireball::InitialUpdate(DVector* vec)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	// Do the normal initial update
	DBOOL bRet = CProjectile::InitialUpdate(vec);

	// Check to see if there's a target straight in front of us...
	IntersectQuery iq;
	IntersectInfo  ii;
	DRotation rRot;
	DVector vU, vR, vF;

    pServerDE->GetObjectPos(m_hObject, &(iq.m_From));
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	VEC_MULSCALAR(vF, vF, 3000.0f);
	VEC_ADD(iq.m_To, iq.m_From, vF);

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	DBOOL bHit = DFALSE;

	// Don't shoot the sap who fired this.
	if(bHit = pServerDE->IntersectSegment(&iq, &ii))
	{
		if(m_hFiredFrom == ii.m_hObject)
		{
			VEC_COPY(iq.m_From, ii.m_Point);
			bHit = pServerDE->IntersectSegment(&iq, &ii);		
		}
	}

	if(bHit && pServerDE->IsKindOf(pServerDE->GetObjectClass(ii.m_hObject), pServerDE->GetClass("CBaseCharacter")))
	{
		m_hTrackObj	= ii.m_hObject;
		pServerDE->CreateInterObjectLink(m_hObject, m_hTrackObj);
	}

	VEC_COPY(m_vHitLocation, ii.m_Point);

	// Randomly rotate a little to make it arc
	DFLOAT		fRotAmount = MATH_PI / 4.0f;
	fRotAmount = pServerDE->Random(-fRotAmount, fRotAmount);

	// Get location and rotation information
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	pServerDE->RotateAroundAxis(&rRot, &vU, fRotAmount);
	pServerDE->SetObjectRotation(m_hObject, &rRot);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::Update(DVector *pMovement)
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

DBOOL CNightmareFireball::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	////////////////////////////////////////////////////////////////////////
	// Play a sound if we don't have one playing already
	if(!m_hSound)
		m_hSound = PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\napalm\\projectile.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE, DFALSE, 100, DFALSE, DFALSE);

	if(m_bExplode || m_damage.IsDead())
	{
		if(m_hSound)
			{ pServerDE->KillSound(m_hSound); m_hSound = 0; }

		Explode();
		return DFALSE;
	}

	////////////////////////////////////////////////////////////////////////
	// If there was a character to track... then follow it
	if(m_hTrackObj)
	{
		TrackObject();
		AdjustVelocity();
	}
	else
	{
		TrackLocation();
		AdjustVelocity();
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::TrackObject()
//
//	PURPOSE:	Turn towards the tracked character to follow it
//
// ----------------------------------------------------------------------- //

void CNightmareFireball::TrackObject()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector		vObjPos, vPos, vTemp, vU, vR, vF;
	DRotation	rRot, rNewRot, rTemp;

	// Get location and rotation information
	pServerDE->GetObjectPos(m_hTrackObj, &vObjPos);
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Get a direction vector and rotation between the flayer and track object
	VEC_SUB(vTemp, vObjPos, vPos);
	pServerDE->AlignRotation(&rNewRot, &vTemp, &vU);

	// Try to get a rotation inbetween the current and the destination and set it
	DFLOAT fRotate = 0.075f;
	DFLOAT fDist = VEC_MAG(vTemp);

	if(fDist < 750.0f)
	{
		fRotate = 1.0f - (fDist / 750.0f);
		fRotate = (0.175f * fRotate) + 0.075f;
	}

	pServerDE->InterpolateRotation(&rTemp, &rRot, &rNewRot, fRotate);
	pServerDE->SetObjectRotation(m_hObject, &rTemp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::TrackLocation()
//
//	PURPOSE:	Turn towards the tracked location to follow it
//
// ----------------------------------------------------------------------- //

void CNightmareFireball::TrackLocation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector		vPos, vTemp, vU, vR, vF;
	DRotation	rRot, rNewRot, rTemp;

	// Get location and rotation information
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Get a direction vector and rotation between the flayer and track object
	VEC_SUB(vTemp, m_vHitLocation, vPos);
	pServerDE->AlignRotation(&rNewRot, &vTemp, &vU);

	// Try to get a rotation inbetween the current and the destination and set it
	pServerDE->InterpolateRotation(&rTemp, &rRot, &rNewRot, 0.125f);
	pServerDE->SetObjectRotation(m_hObject, &rTemp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::AdjustVelocity()
//
//	PURPOSE:	Adjust the velocity according to the rotation
//
// ----------------------------------------------------------------------- //

void CNightmareFireball::AdjustVelocity()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vU, vR, vF;
	DRotation rTemp;
	DFLOAT fTemp = 0.0f;

	pServerDE->GetObjectRotation(m_hObject, &rTemp);
	pServerDE->GetVelocity(m_hObject, &vF);
	fTemp = VEC_MAG(vF);

	pServerDE->GetRotationVectors(&rTemp, &vU, &vR, &vF);
	VEC_MULSCALAR(vF, vF, fTemp);
	pServerDE->SetVelocity(m_hObject, &vF);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CNightmareFireball::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_LIFELEECH_PRIMARY;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\napalm\\impact.wav", 2000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareFireball::BreakLink
//
//	PURPOSE:	Breaks a link of an object
//
// ----------------------------------------------------------------------- //

void CNightmareFireball::BreakLink(HOBJECT hObj)
{
	if(hObj == m_hTrackObj)
		m_hTrackObj = DNULL;
	else
		CProjectile::BreakLink(hObj);
}

#endif  // From _ADD_ON check at beginning of Flayer projectiles