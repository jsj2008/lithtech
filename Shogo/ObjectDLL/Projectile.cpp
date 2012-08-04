// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "Projectile.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "RiotObjectUtilities.h"
#include "Explosion.h"
#include "BaseCharacter.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "Weapon.h"
#include "ClientServerShared.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "ModelNodes.h"
#include "BodyProp.h"
#include "BaseAI.h"
#include "CVarTrack.h"

BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, BaseClass, NULL, NULL, CF_HIDDEN)

extern DBYTE g_nIgnoreFX;
extern DBYTE g_nRandomWeaponSeed;

static DBOOL AttackerLiquidFilterFn(HOBJECT hObj, void *pUserData);
static DBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData);

#define	DEFAULT_SOUND_RADIUS	3000.0f
#define MAX_MODEL_NODES			9999
#define UPDATE_DELTA			0.05f
#define MAX_VECTOR_LOOP			20

static CVarTrack g_MissileSpeedTrack;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile() : BaseClass(OT_NORMAL)
{
	AddAggregate(&m_damage);

	m_fVelocity				= 0.0f;
	m_fDamage				= 0.0f;
	m_fMass					= 0.0f;
	m_fRadius				= 0.0f;
	m_fLifeTime				= 5.0f;
	m_eModelSize			= MS_NORMAL;
	m_fRange				= 10000.0f;
	m_bSilenced				= DFALSE;

	VEC_SET(m_vDims, 1.0f, 1.0f, 1.0f);
	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);

	m_hFiredFrom			= DNULL;
	m_eType					= VECTOR;
	m_eDamageType			= DT_PUNCTURE;
	m_nId					= (RiotWeaponId)0;

	m_fStartTime			= 0.0f;
	m_bObjectRemoved		= DFALSE;

	m_fExplosionDuration	= 0.3f;

	m_pProjectileFilename	= DNULL;
	m_pProjectileSkin		= DNULL;

	m_bDetonated			= DFALSE;

	m_dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_VISIBLE | 
		FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT | FLAG_RAYHIT;

	m_dwCantDamageTypes		= 0;

	// Cool anime snaking stuff...
	
	m_bSnakingOn		= DFALSE;
	m_bFirstSnake		= DTRUE;
	m_fSnakeDir			= 0.0f;
	m_fSnakeUpVel		= 0.0f;


	// Lock on target?

	m_bCanLockOnTarget	= DFALSE;
	m_fLockWaitTime		= 0.5f;
	m_hLockOnTarget		= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CProjectile::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = m_dwFlags | FLAG_FORCECLIENTUPDATE;

				VEC_COPY(pStruct->m_Scale, m_vScale);

				if (m_pProjectileFilename) SAFE_STRCPY(pStruct->m_Filename, m_pProjectileFilename);
				if (m_pProjectileSkin) SAFE_STRCPY(pStruct->m_SkinName, m_pProjectileSkin);
				
				pStruct->m_NextUpdate = UPDATE_DELTA;
			}			
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hLockOnTarget)
				{
					m_hLockOnTarget = DNULL;
				}
				if (hLink == m_hFiredFrom)
				{
					m_hFiredFrom = DNULL;
				}
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CProjectile::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	DDWORD dwRet = BaseClass::ObjectMessageFn(hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) break;

			if (m_damage.IsDead())
			{
				Detonate(DNULL);
			}
		}
		break;

		default : break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile with the information needed
//
// ----------------------------------------------------------------------- //

void CProjectile::Setup(CWeapon* pWeapon, HOBJECT hFiredFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pWeapon || !hFiredFrom) return;

	VEC_COPY(m_vDir, pWeapon->GetLastFirePath());
	VEC_NORM(m_vDir);

	m_fDamage		= pWeapon->GetDamage();
	m_fRadius		= (DFLOAT)pWeapon->GetRadius();
	m_hFiredFrom	= hFiredFrom;
	m_eModelSize	= pWeapon->GetSize();
	m_nId			= pWeapon->GetId();
	m_fVelocity		= GetWeaponVelocity(m_nId);
	m_fRange		= pWeapon->GetRange();
	m_fLifeTime		= pWeapon->GetLifeTime();
	m_eDamageType	= pWeapon->GetDamageType();
	m_bSilenced		= pWeapon->IsZoomed();	// For not silenced if zoomed in
	m_bCanLockOnTarget	= pWeapon->CanLockOnTarget();
	m_eType				= GetWeaponType(m_nId);
	
	pServerDE->GetObjectPos(m_hObject, &m_vFirePos);

	if (m_eType == PROJECTILE)
	{
		DoProjectile();
	}
	else  // Cast a ray...
	{
		DoVector();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

void CProjectile::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED);

	if(!g_MissileSpeedTrack.IsInitted())
		g_MissileSpeedTrack.Init(pServerDE, "MissileSpeed", DNULL, 1.0f);

	if (nInfo == INITIALUPDATE_SAVEGAME) return;


	DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(m_hObject);
	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_MOVEABLE);

	m_damage.Init(m_hObject);
	m_damage.SetMass(m_fMass);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanHeal(DFALSE);
	m_damage.SetCanRepair(DFALSE);
	m_damage.SetApplyDamagePhysics(DFALSE);
	m_damage.SetCantDamageTypes(m_dwCantDamageTypes);

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	pServerDE->SetObjectDims(m_hObject, &m_vDims);

	m_fStartTime  = pServerDE->GetTime();
	m_fSnakeUpVel = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CProjectile::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	DFLOAT fTime = pServerDE->GetTime();

	DVector vVel;
	pServerDE->GetVelocity(m_hObject, &vVel);

	if (m_bCanLockOnTarget && m_hLockOnTarget && fTime > m_fStartTime + m_fLockWaitTime)
	{
		DVector vU, vR, vF;
		DRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_NORM(vU);
		VEC_NORM(vF);

		DVector vTargetPos, vPos;
		pServerDE->GetObjectPos(m_hLockOnTarget, &vTargetPos);
		pServerDE->GetObjectPos(m_hObject, &vPos);

		DVector vDir;
		VEC_SUB(vDir, vTargetPos, vPos);
		VEC_NORM(vDir);
		VEC_MULSCALAR(vVel, vDir, m_fVelocity);

		pServerDE->AlignRotation(&rRot, &vDir, &vU);
		pServerDE->SetObjectRotation(m_hObject, &rRot);
	}



	// See if we want to do some cool Anime snaking :)

	if (m_bSnakingOn)
	{
		DVector vTemp, vU, vR, vF;
		DRotation rRot;
		
		//pServerDE->GetObjectRotation(m_hObject, &rRot);
		rRot = m_SnakingRot;
		
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_NORM(vU);
		VEC_NORM(vF);
		
		if (m_bFirstSnake)
		{
			m_bFirstSnake = DFALSE;
			m_fSnakeDir	  = (g_nRandomWeaponSeed & 1) ? 1.0f : -1.0f;
		}
		else
		{
			// Subtract velocity off current up vector...

			VEC_MULSCALAR(vTemp, vU, m_fSnakeUpVel);
			VEC_SUB(vVel, vVel, vTemp);
		}

		pServerDE->RotateAroundAxis(&rRot, &vF, m_fSnakeDir * 10.0f * pServerDE->GetFrameTime());
		
		//pServerDE->SetObjectRotation(m_hObject, &rRot);
		m_SnakingRot = rRot;
		
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_NORM(vU);

		// Add velocity to new up vector...

		m_fSnakeUpVel = m_fVelocity * 0.67f * pServerDE->GetFrameTime();

		VEC_MULSCALAR(vTemp, vU, m_fSnakeUpVel);
		VEC_ADD(vVel, vVel, vU);

		pServerDE->SetVelocity(m_hObject, &vVel);
	}
	
	
	// If we didn't hit anything, blow up...

	if (fTime >= (m_fStartTime + m_fLifeTime)) 
	{
		Detonate(DNULL);
	} 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bObjectRemoved) return;


	 // Let it get out of our bounding box...

	if (hObj == m_hFiredFrom) return;


	// See if we want to impact on this object...

	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;


	HCLASS hClassMe		= pServerDE->GetObjectClass(m_hObject);
	HCLASS hClassObj	= pServerDE->GetObjectClass(hObj);
	HCLASS hClassWorld  = pServerDE->GetObjectClass(pServerDE->GetWorldObject());


	// Don't impact on non-solid objects...

	DDWORD dwFlags = pServerDE->GetObjectFlags(hObj);
	if (!pServerDE->IsKindOf(hClassObj, hClassWorld) && !(dwFlags & FLAG_SOLID)) return;


	// Don't hit projectiles fired from the same person (e.g., bullgut)...

	if (pServerDE->IsKindOf(hClassObj, hClassMe))
	{
		CProjectile* pObj = (CProjectile*)pServerDE->HandleToObject(hObj);
		if (pObj)
		{
			if (pObj->GetFiredFrom() == m_hFiredFrom)
			{
				return;
			}
		}
	}


	// See if we hit the sky...

	if (pServerDE->IsKindOf(hClassObj, hClassWorld))
	{
		CollisionInfo info;
		pServerDE->GetLastCollision(&info);

		SurfaceType eType = GetSurfaceType(info.m_hPoly);

		if (eType == ST_SKY)
		{
			RemoveObject();
			return;
		}
	}


	HandleImpact(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleImpact()
//
//	PURPOSE:	Allow sub-classes to handle impacts...Default is to
//				go boom.
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleImpact(HOBJECT hObj)
{
	Detonate(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectile::Detonate(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bDetonated) return;

	m_bDetonated = DTRUE;

	SurfaceType eType = ST_UNKNOWN;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...

	DVector vNormal;
	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

	if (hObj)
	{
		HCLASS hClassObj   = pServerDE->GetObjectClass(hObj);
		HCLASS hClassWorld = pServerDE->GetObjectClass(pServerDE->GetWorldObject());

		if (pServerDE->IsKindOf(hClassObj, hClassWorld))
		{
			CollisionInfo info;
			pServerDE->GetLastCollision(&info);

			if (info.m_hPoly)
			{
				eType = GetSurfaceType(info.m_hPoly);
			}

			VEC_COPY(vNormal, info.m_Plane.m_Normal);

			DRotation rRot;
			pServerDE->AlignRotation(&rRot, &vNormal, DNULL);
			pServerDE->SetObjectRotation(m_hObject, &rRot);

			// Calculate where we really hit the plane...

			DVector vVel, vP0, vP1;
			pServerDE->GetVelocity(m_hObject, &vVel);

			VEC_COPY(vP1, vPos);
			VEC_MULSCALAR(vVel, vVel, pServerDE->GetFrameTime());
			VEC_SUB(vP0, vP1, vVel);

			DFLOAT fDot1 = DIST_TO_PLANE(vP0, info.m_Plane);
			DFLOAT fDot2 = DIST_TO_PLANE(vP1, info.m_Plane);

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				VEC_COPY(vPos, vP1);
			}
			else
			{
				DFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
				VEC_LERP(vPos, vP0, vP1, fPercent);
			}
		}
	}
	else
	{
		// Since hObj was null, this means the projectile's lifetime was up,
		// so we just blew-up in the air.

		eType = ST_AIR; 
	}


	if (eType == ST_UNKNOWN)
	{
		eType = GetSurfaceType(hObj);
	}


	AddImpact(hObj, vPos, vNormal, eType);

	
	// See if this projectile does impact damage...

	if (hObj && (m_fDamage > 0.0f && m_fRadius <= 0.0f))
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;
		DamageObject(hDamager, this, hObj, m_fDamage, m_vDir, m_eDamageType);
	}


	// Remove projectile from world...

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddImpact()
//
//	PURPOSE:	Add an impact object.
//
// ----------------------------------------------------------------------- //

void CProjectile::AddImpact(HOBJECT hObj, DVector vPoint, DVector vNormal, 
							SurfaceType eType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vPos, vTemp;

	VEC_COPY(vPos, vPoint);
	VEC_MULSCALAR(vTemp, m_vDir, -1.0f);
	VEC_ADD(vPos, vPos, vTemp);


	// Create the client side weapon fx...

	CLIENTWEAPONFX fxStruct;

	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &vNormal, DNULL);

	VEC_COPY(fxStruct.vPos, vPos);
	ROT_COPY(fxStruct.rRot, rRot);


	// Set the high bit of the weapon type if this is a small weapon...

	DBYTE nId = (m_eModelSize == MS_SMALL) ? (m_nId | MODEL_SMALL_FLAG) : 
				((m_eModelSize == MS_LARGE) ? (m_nId | MODEL_LARGE_FLAG) : m_nId);

	fxStruct.hObj			= hObj;
	fxStruct.nWeaponId		= nId;
	fxStruct.nSurfaceType	= eType;
	fxStruct.nIgnoreFX		= g_nIgnoreFX;


	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)

	g_nIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;


	// If this is a player object, get the client id...

	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId	= (DBYTE) pServerDE->GetClientID(pPlayer->GetClient());
		}
	}

	VEC_COPY(fxStruct.vFirePos, m_vFirePos);

	CreateClientWeaponFX(fxStruct);
	

	if (RadiusDamageType(m_eDamageType) && eType != ST_SKY)
	{
		AddExplosion(vPos, vNormal);
	}


	// Update BaseCharacter fire info...

	if (m_hFiredFrom && IsBaseCharacter(m_hFiredFrom))
	{
		CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject(m_hFiredFrom);
		if (pChar)
		{
			pChar->SetLastFireInfo(&m_vFirePos, &vPos, m_nId, m_bSilenced);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	Explosion* pExplosion = DNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	HCLASS hClass = pServerDE->GetClass("Explosion");

	if (hClass)
	{
		VEC_COPY(theStruct.m_Pos, vPos);

		DRotation rRot;
		pServerDE->AlignRotation(&rRot, &vNormal, &vNormal);
		ROT_COPY(theStruct.m_Rotation, rRot);

		pExplosion = (Explosion*)pServerDE->CreateObject(hClass, &theStruct);
	}
	
	if (pExplosion)
	{
		pExplosion->Setup(this);
		pExplosion->GoBoom();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSpecialFX()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (GetProjectileFX(m_nId) == 0) return;

	DBYTE nId = (m_eModelSize == MS_SMALL) ? (m_nId | MODEL_SMALL_FLAG) : 
				((m_eModelSize == MS_LARGE) ? (m_nId | MODEL_LARGE_FLAG) : m_nId);

	// If this is a player object, get the client id...

	DBYTE nShooterId = -1;
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			nShooterId = (DBYTE) pServerDE->GetClientID(pPlayer->GetClient());
		}
	}

	
	// Create a special fx...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_PROJECTILE_ID);
	pServerDE->WriteToMessageByte(hMessage, nId);
	pServerDE->WriteToMessageByte(hMessage, nShooterId);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::RemoveObject()
//
//	PURPOSE:	Remove the object, and do clean up (isle 4)
//
// ----------------------------------------------------------------------- //

void CProjectile::RemoveObject()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->RemoveObject(m_hObject);	

	m_bObjectRemoved = DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackerLiquidFilterFn()
//
//	PURPOSE:	Filter the attacker out of CastRay and/or 
//				IntersectSegment calls (so you don't shot yourself).
//				However, we want to ignore liquid as well...
//
// ----------------------------------------------------------------------- //

DBOOL AttackerLiquidFilterFn(HOBJECT hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
		return LiquidFilterFn(hObj, pUserData);
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorFilterFn()
//
//	PURPOSE:	Filter the attacker out of IntersectSegment 
//				calls (so you don't shot yourself).  Also handle
//				AIs of the same alignment not shooting eachother
//
// ----------------------------------------------------------------------- //

DBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
		HOBJECT hUs = (HOBJECT)pUserData;
		if (IsAI(hUs) && IsAI(hObj))
		{
			BaseAI *pAI = (BaseAI*) g_pServerDE->HandleToObject(hUs);
			if (!pAI) return DFALSE;

			// We can't hit guys we like...

			if (pAI->CheckAlignment(LIKE, hObj))
			{
				return DFALSE;
			}
		}
		return DTRUE; // LiquidFilterFn(hObj, pUserData);
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::LockOnTarget
//
//	PURPOSE:	Try to lock on a target
//
// ----------------------------------------------------------------------- //

void CProjectile::LockOnTarget()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hFiredFrom) return;

	HCLASS hClassFired = pServerDE->GetObjectClass(m_hFiredFrom);
	HCLASS hClassBase  = pServerDE->GetClass("CBaseCharacter");


	// We can only lock on targets if we have the targeting upgrade item...
	
	if (pServerDE->IsKindOf(hClassFired, hClassBase))
	{
		CBaseCharacter* pBaseChar = (CBaseCharacter*)pServerDE->HandleToObject(m_hFiredFrom);
		if (!pBaseChar || !pBaseChar->HasTargetingUpgrade()) return;
	}


	// Use forward vector of person shooting to make sure we aim straight...

	DRotation rRot;
	DVector vU, vR, vF;
	pServerDE->GetObjectRotation(m_hFiredFrom, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	VEC_NORM(vF);


	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	IntersectInfo iInfo;
	IntersectQuery qInfo;

	DVector vTemp, vPos2;
	VEC_MULSCALAR(vTemp, vF, m_fRange);
	VEC_ADD(vPos2, vPos, vTemp);

	VEC_COPY(qInfo.m_From, vPos);
	VEC_COPY(qInfo.m_To, vPos2);

	qInfo.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn  = SpecificObjectFilterFn;
	qInfo.m_pUserData = m_hFiredFrom;

	if (pServerDE->IntersectSegment(&qInfo, &iInfo))
	{
		HOBJECT hObj = iInfo.m_hObject;
		if (hObj)
		{
			HCLASS hClassHit = pServerDE->GetObjectClass(hObj);
			if (pServerDE->IsKindOf(hClassHit, hClassBase))
			{
				pServerDE->CreateInterObjectLink(m_hObject, hObj);
				m_hLockOnTarget = hObj;

				// Notify player's of locks...

				HCLASS hClassPlayer = pServerDE->GetClass("CPlayerObj");
				if (pServerDE->IsKindOf(hClassHit, hClassPlayer))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hLockOnTarget);
					HCLIENT hClient		= pPlayer->GetClient();

					// Only Mecha's know about locks...

					if (hClient && pPlayer->IsMecha())
					{
						HMESSAGEWRITE hWrite = pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
						pServerDE->WriteToMessageByte(hWrite, IC_ROCKETLOCK_ID);
						pServerDE->WriteToMessageByte(hWrite, 0);		// I'm the target
						pServerDE->WriteToMessageFloat(hWrite, 0.0f);
						pServerDE->EndMessage(hWrite);
					}
				}

				if (pServerDE->IsKindOf(hClassFired, hClassPlayer))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hFiredFrom);
					HCLIENT hClient		= pPlayer->GetClient();

					if (hClient)
					{
						HMESSAGEWRITE hWrite = pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
						pServerDE->WriteToMessageByte(hWrite, IC_ROCKETLOCK_ID);
						pServerDE->WriteToMessageByte(hWrite, 1);		// I fired the thing
						pServerDE->WriteToMessageFloat(hWrite, 0.0f);
						pServerDE->EndMessage(hWrite);
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoProjectile
//
//	PURPOSE:	Do projectile stuff...
//
// ----------------------------------------------------------------------- //

void CProjectile::DoProjectile()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;


	// If we have a fired from object, make a link to it...

	if (m_hFiredFrom)
	{
		pServerDE->CreateInterObjectLink(m_hObject, m_hFiredFrom);
	}


	if (m_eModelSize == MS_SMALL)
	{
		VEC_SET(m_vDims, m_vDims.x/5, m_vDims.y/5, m_vDims.z/5);
		VEC_SET(m_vScale, .333f, .333f, .333f);
		pServerDE->SetObjectDims(m_hObject, &m_vDims);
		pServerDE->ScaleObject(m_hObject, &m_vScale);
	}


	// Set our force ignore limit and mass...

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
	pServerDE->SetObjectMass(m_hObject, m_fMass);


	// Make sure we are pointing in the direction we are traveling...

	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &m_vDir, DNULL);
	pServerDE->SetObjectRotation(m_hObject, &rRot);

	// Store it for snaking.
	m_SnakingRot = rRot;

		
	// And away we go...

	DVector vVel;
	float fMul = 1.0f;

	if(m_nId == GUN_TOW_ID)
		fMul = g_MissileSpeedTrack.GetFloat(1.0f);
								  
	VEC_MULSCALAR(vVel, m_vDir, m_fVelocity * fMul);
	pServerDE->SetVelocity(m_hObject, &vVel);


	// If we're supposed to lock on a target, look for a target...

	if (m_bCanLockOnTarget)
	{
		LockOnTarget();
	}


	// Special case of 0 life time...

	if (m_fLifeTime <= 0.0f) 
	{
		Detonate(DNULL);
	}
	else
	{
		AddSpecialFX();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoVector
//
//	PURPOSE:	Do vector stuff
//
// ----------------------------------------------------------------------- //

void CProjectile::DoVector()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vFrom;
	pServerDE->GetObjectPos(m_hObject, &vFrom);

	IntersectInfo iInfo;
	IntersectQuery qInfo;

	DVector vTemp, vTo;
	VEC_MULSCALAR(vTemp, m_vDir, m_fRange);
	VEC_ADD(vTo, vFrom, vTemp);

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	DBOOL bHitSomething = DTRUE;
	DBOOL bDone			= DFALSE;

	HCLASS hBodyClass = pServerDE->GetClass("BodyProp");

	int nLoopCount = 0; // No infinite loops thanks.

	while (!bDone)
	{
		qInfo.m_FilterFn  = DoVectorFilterFn;
		qInfo.m_pUserData = m_hFiredFrom;

		VEC_COPY(qInfo.m_From, vFrom);
		VEC_COPY(qInfo.m_To, vTo);

		if (pServerDE->IntersectSegment(&qInfo, &iInfo))
		{
			HCLASS hClass = pServerDE->GetObjectClass(iInfo.m_hObject);

			// Shooting AI?...

			if (IsAI(iInfo.m_hObject))
			{
				bDone = HandlePotentialAIImpact(iInfo, vFrom);
			}
			else if (pServerDE->IsKindOf(hClass, hBodyClass))
			{
				bDone = HandlePotentialBodyImpact(iInfo, vFrom);
			}
			else
			{
				bDone = DTRUE;
			}
		}
		else // Didn't hit anything...
		{
			if (m_eType != MELEE) 
			{
				DVector vUp;
				VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
				AddImpact(DNULL, vTo, vUp, ST_SKY);
			}

			bHitSomething = DFALSE;
			bDone		  = DTRUE;
		}


		// Make sure we don't loop forever...

		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
			if (m_eType != MELEE) 
			{
				DVector vUp;
				VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
				AddImpact(DNULL, vTo, vUp, ST_SKY);
			}

			bHitSomething = DFALSE;
			bDone		  = DTRUE;
		}
	}

	
	if (bHitSomething)
	{
		HandleVectorImpact(qInfo, iInfo);
	}


	// Okay, we're all done now...bye, bye...

	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;


	// Get the surface type (check the poly first)...

	SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);
	
	if (eType == ST_UNKNOWN)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}


	if (m_eType != CANNON && eType != ST_LIQUID)
	{
		DamageObject(m_hFiredFrom, this, iInfo.m_hObject, m_fDamage, m_vDir, m_eDamageType);
	}

	if (!(eType == ST_LIQUID && m_eType == CANNON))
	{
		AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
	}

	
	// If we hit liquid, cast another ray that will go through the water...
	
	if (eType == ST_LIQUID)
	{
		qInfo.m_FilterFn = AttackerLiquidFilterFn;

		if (pServerDE->IntersectSegment(&qInfo, &iInfo))
		{
			if (m_eType != CANNON)
			{
				DamageObject(m_hFiredFrom, this, iInfo.m_hObject, m_fDamage, m_vDir, m_eDamageType);
			}

			// Get the surface type (check the poly first)...

			SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);
			
			if (eType == ST_UNKNOWN)
			{
				eType = GetSurfaceType(iInfo.m_hObject);
			}

			AddImpact(iInfo.m_hObject, iInfo.m_Point, 
					  iInfo.m_Plane.m_Normal, eType);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialAIImpact
//
//	PURPOSE:	Handle a vector hitting an AI's bounding box 
//
// ----------------------------------------------------------------------- //

DBOOL CProjectile::HandlePotentialAIImpact(IntersectInfo & iInfo, DVector & vFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	BaseAI *pAI = (BaseAI*) pServerDE->HandleToObject(iInfo.m_hObject);
	if (!pAI) return DFALSE;

	if (!pAI->UsingHitDetection()) return DTRUE;

	DDWORD		nModelId   = pAI->GetModelId();
	ModelSize	eModelSize = pAI->GetModelSize();
	DDWORD		nNode	   = MAX_MODEL_NODES;

	// OPTIMIZATION - timing...
	//DCounter dcounter;
	//pServerDE->StartCounter(&dcounter);
	// OPTIMIZATION - timing...

	DBOOL bHitSomething = DoLocationBasedImpact(iInfo, vFrom, nModelId, eModelSize, nNode);
	
	// OPTIMIZATION - timing...
	//pServerDE->BPrint("DoLocationBasedImpact: %d ticks", pServerDE->EndCounter(&dcounter));
	// OPTIMIZATION - timing...	
		

	// Did we hit something?

	if (bHitSomething && nNode < MAX_MODEL_NODES)
	{
		pAI->SetLastHitModelNode(nNode);
		AdjustDamage(pAI, nNode);
	}

	return bHitSomething;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialBodyImpact
//
//	PURPOSE:	Handle a vector hitting an BodyProp's bounding box 
//
// ----------------------------------------------------------------------- //

DBOOL CProjectile::HandlePotentialBodyImpact(IntersectInfo & iInfo, DVector & vFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	BodyProp* pProp = (BodyProp*)pServerDE->HandleToObject(iInfo.m_hObject);
	if (!pProp) return DFALSE;

	DDWORD	  nModelId   = pProp->GetModelId();
	ModelSize eModelSize = pProp->GetModelSize();
	DDWORD	  nNode;

	return DoLocationBasedImpact(iInfo, vFrom, nModelId, eModelSize, nNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoLocationBasedImpact
//
//	PURPOSE:	Handle a vector hitting an model's bounding box that is
//				using location-specific hit detation 
//				(basic algorithm borrowed from B2)
//
// ----------------------------------------------------------------------- //

DBOOL CProjectile::DoLocationBasedImpact(IntersectInfo & iInfo, DVector & vFrom,
										 DDWORD nModelId, ModelSize eModelSize,
										 DDWORD & nNode)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT fTemp = 0, fDis = 999.0f;
	DVector	vPos, vObjDims, vDir, vTemp;
	DRotation rRot;
	DBOOL bStatus = DFALSE;
	nNode = MAX_MODEL_NODES;

	// SCHLEGZ 3/12/98 3:46:09 AM: Best way to get accurate dims...

	pServerDE->GetModelAnimUserDims(iInfo.m_hObject, &vObjDims, pServerDE->GetModelAnimation(iInfo.m_hObject));

	// This is something I pulled out of my ass but it seems to work better than I thought
	// Basically, I'm trying to carry the shot further inward towards the model to get
	// a more accurate measure of which node was hit

	vTemp.x = (float)fabs(m_vDir.x);
	vTemp.y = (float)fabs(m_vDir.y);
	vTemp.z = (float)fabs(m_vDir.z);

	if (vTemp.x > vTemp.y && vTemp.x > vTemp.z)
	{
		VEC_MULSCALAR(vDir, m_vDir, vObjDims.x/vTemp.x);	
	}
	else if (vTemp.y > vTemp.x  && vTemp.y > vTemp.z)
	{
		VEC_MULSCALAR(vDir, m_vDir, vObjDims.y/vTemp.y);	
	}
	else if (vTemp.z > vTemp.x  && vTemp.z > vTemp.y)
	{
		VEC_MULSCALAR(vDir, m_vDir, vObjDims.z/vTemp.z);	
	}

	VEC_ADD(vFrom, iInfo.m_Point, vDir);

	vDir.x = (float)fabs(vDir.x);
	vDir.y = (float)fabs(vDir.y);
	vDir.z = (float)fabs(vDir.z);

	DDWORD nNumNodes = GetNumModelNodes(nModelId, eModelSize);
	char* pNodeName = DNULL;

	for (DDWORD i = 0; i < nNumNodes; i++)
	{
		pNodeName = GetModelNodeName(i, nModelId, eModelSize);
		if (pNodeName)
		{
			pServerDE->GetModelNodeHideStatus(iInfo.m_hObject, pNodeName, &bStatus);

			if (!bStatus)
			{
				DBOOL bRet = pServerDE->GetModelNodeTransform(iInfo.m_hObject, pNodeName, &vPos, &rRot);

				fTemp = VEC_DIST(vPos, vFrom);
				if (fTemp < fDis && (fTemp <= vDir.x || fTemp <= vDir.y || fTemp <= vDir.z))
				{
					fDis = fTemp;
					nNode = i;
				}
			}
		}
	}

	// Did we hit something?

	if (nNode < nNumNodes)
	{
		return DTRUE;
	}

	nNode = MAX_MODEL_NODES;

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AdjustDamage
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

void CProjectile::AdjustDamage(CBaseCharacter* pChar, DDWORD nNode)
{
	if (!pChar) return;

	DFLOAT fAdjustVal = 1.0f;

	switch (pChar->GetNodeType(nNode))
	{
		case NT_HEAD:
			fAdjustVal = 2.0f;
		break;

		case NT_LARM:
		case NT_RARM:
		case NT_LLEG:
		case NT_RLEG:
			fAdjustVal = 0.5f;
		break;

		case NT_TORSO:
		case NT_PELVIS:
		default:
			fAdjustVal = 1.0f;
		break;
	}

	m_fDamage *= fAdjustVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hFiredFrom);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLockOnTarget);

	pServerDE->WriteToMessageVector(hWrite, &m_vFirePos);
	pServerDE->WriteToMessageVector(hWrite, &m_vDir);
	pServerDE->WriteToMessageByte(hWrite, m_bSilenced);
	pServerDE->WriteToMessageByte(hWrite, m_bRemoveFromWorld);
	pServerDE->WriteToMessageByte(hWrite, m_bObjectRemoved);
	pServerDE->WriteToMessageByte(hWrite, m_bDetonated);
	pServerDE->WriteToMessageByte(hWrite, m_bSnakingOn);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstSnake);
	pServerDE->WriteToMessageByte(hWrite, m_bCanLockOnTarget);
	pServerDE->WriteToMessageByte(hWrite, m_eModelSize);
	pServerDE->WriteToMessageByte(hWrite, m_nId);
	pServerDE->WriteToMessageByte(hWrite, m_eType);
	pServerDE->WriteToMessageByte(hWrite, m_eDamageType);
	pServerDE->WriteToMessageFloat(hWrite, m_fRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fLifeTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fExplosionDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fVelocity);
	pServerDE->WriteToMessageFloat(hWrite, m_fRange);
	pServerDE->WriteToMessageFloat(hWrite, m_fSnakeUpVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fSnakeDir);
	pServerDE->WriteToMessageFloat(hWrite, m_fLockWaitTime);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hFiredFrom);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hLockOnTarget);

	pServerDE->ReadFromMessageVector(hRead, &m_vFirePos);
	pServerDE->ReadFromMessageVector(hRead, &m_vDir);
	m_bSilenced				= pServerDE->ReadFromMessageByte(hRead);
	m_bRemoveFromWorld		= pServerDE->ReadFromMessageByte(hRead);
	m_bObjectRemoved		= pServerDE->ReadFromMessageByte(hRead);
	m_bDetonated			= pServerDE->ReadFromMessageByte(hRead);
	m_bSnakingOn			= pServerDE->ReadFromMessageByte(hRead);
	m_bFirstSnake			= pServerDE->ReadFromMessageByte(hRead);
	m_bCanLockOnTarget		= pServerDE->ReadFromMessageByte(hRead);
	m_eModelSize			= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	m_nId					= (RiotWeaponId) pServerDE->ReadFromMessageByte(hRead);
	m_eType					= (ProjectileType) pServerDE->ReadFromMessageByte(hRead);
	m_eDamageType			= (DamageType) pServerDE->ReadFromMessageByte(hRead);
	m_fRadius				= pServerDE->ReadFromMessageFloat(hRead);
	m_fStartTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamage				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLifeTime				= pServerDE->ReadFromMessageFloat(hRead);
	m_fExplosionDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_fVelocity				= pServerDE->ReadFromMessageFloat(hRead);
	m_fRange				= pServerDE->ReadFromMessageFloat(hRead);
	m_fSnakeUpVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSnakeDir				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLockWaitTime			= pServerDE->ReadFromMessageFloat(hRead);
}