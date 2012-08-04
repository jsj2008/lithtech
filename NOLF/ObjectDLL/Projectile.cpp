// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Projectile.h"
#include "ltengineobjects.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "ServerUtilities.h"
#include "Explosion.h"
#include "Character.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "ClientServerShared.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "Body.h"
#include "AI.h"
#include "CVarTrack.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "GameServerShell.h"
#include "Camera.h"

BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, GameBase, NULL, NULL, CF_HIDDEN)

extern uint16 g_wIgnoreFX;
extern uint8  g_nRandomWeaponSeed;

static LTBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData);

#define MAX_MODEL_NODES			9999
#define UPDATE_DELTA			0.001f
#define MAX_VECTOR_LOOP			20

static CVarTrack g_vtInvisibleMaxThickness;

CVarTrack g_vtNetFriendlyFire;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile() : GameBase(OT_MODEL)
{
	AddAggregate(&m_damage);

    m_hObject               = LTNULL;
	m_fVelocity				= 0.0f;
	m_fInstDamage			= 0.0f;
	m_fProgDamage			= 0.0f;
	m_fMass					= INFINITE_MASS; //0.0f;
	m_fLifeTime				= 5.0f;
	m_fRange				= 10000.0f;
    m_bSilenced             = LTFALSE;

	m_vDims.Init(1.0f, 1.0f, 1.0f);
	m_vFirePos.Init();
	m_vFlashPos.Init();

    m_hFiredFrom            = LTNULL;
	m_eInstDamageType		= DT_BULLET;
	m_eProgDamageType		= DT_UNSPECIFIED;
	m_nWeaponId				= 0;
	m_nAmmoId				= 0;

	m_fStartTime			= 0.0f;
    m_bObjectRemoved        = LTFALSE;

    m_bDetonated            = LTFALSE;

	m_dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING |
		FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT | FLAG_RAYHIT;

	// Make sure we can't be damaged by any of the progressive/volumebrush
	// damage types...
	m_dwCantDamageTypes		= (DamageTypeToFlag(DT_STUN) | DamageTypeToFlag(DT_SLEEPING) |
		DamageTypeToFlag(DT_BURN) | DamageTypeToFlag(DT_FREEZE) | DamageTypeToFlag(DT_ELECTROCUTE) |
		DamageTypeToFlag(DT_POISON) | DamageTypeToFlag(DT_ENDLESS_FALL));

    m_pAmmoData         = LTNULL;
    m_pWeaponData       = LTNULL;

	m_bProcessInvImpact = LTFALSE;
	m_vInvisVel.Init();
	m_vInvisNewPos.Init();

	m_bNumCallsToAddImpact = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = m_dwFlags;

				// This will be set in Setup()...
				SAFE_STRCPY(pStruct->m_Filename, "Models\\Default.abc");

				pStruct->m_NextUpdate = UPDATE_DELTA;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hFiredFrom)
				{
                    m_hFiredFrom = LTNULL;
				}
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    uint32 dwRet = GameBase::ObjectMessageFn(hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (m_damage.IsDead())
			{
                Detonate(LTNULL);
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

void CProjectile::Setup(CWeapon* pWeapon, WFireInfo & info)
{
	if (!pWeapon || !info.hFiredFrom || !g_pWeaponMgr) return;

	if (!g_vtInvisibleMaxThickness.IsInitted())
	{
        g_vtInvisibleMaxThickness.Init(g_pLTServer, "InvisibleMaxThickness", LTNULL, 33.0f);
	}
	if (!g_vtNetFriendlyFire.IsInitted())
	{
        g_vtNetFriendlyFire.Init(g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f);
	}

	m_vDir				= info.vPath;
	m_vDir.Norm();

	m_hFiredFrom		= info.hFiredFrom;
	m_nWeaponId			= pWeapon->GetId();
	m_nAmmoId			= pWeapon->GetAmmoId();
	m_fLifeTime			= pWeapon->GetLifeTime();
	m_fInstDamage		= pWeapon->GetInstDamage();
	m_fProgDamage		= pWeapon->GetProgDamage();

	m_pWeaponData		= pWeapon->GetWeaponData();
	if (!m_pWeaponData) return;

	m_pAmmoData			= pWeapon->GetAmmoData();
	if (!m_pAmmoData) return;

	m_eInstDamageType	= m_pAmmoData->eInstDamageType;
	m_eProgDamageType	= m_pAmmoData->eProgDamageType;

	if (info.bOverrideVelocity)
	{
		m_fVelocity = info.fOverrideVelocity;
	}
	else if (info.bAltFire)
	{
        m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nAltVelocity : 0);
	}
	else
	{
        m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nVelocity : 0);
	}

    m_fRange            = (LTFLOAT) m_pWeaponData->nRange;
	m_bSilenced			= !!(pWeapon->GetSilencer());

	AmmoType eAmmoType  = m_pAmmoData->eType;

	m_vFirePos			= info.vFirePos;
	m_vFlashPos			= info.vFlashPos;

	m_bNumCallsToAddImpact = 0;

	// See if we start inside the test object...

	if (!TestInsideObject(info.hTestObj, eAmmoType))
	{
		if (eAmmoType == PROJECTILE)
		{
			DoProjectile();
		}
		else if (eAmmoType == VECTOR)
		{
			DoVector();
		}
		else if (eAmmoType == GADGET)
		{
			; // Do nothing for now
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestInsideObject
//
//	PURPOSE:	Test to see if the projectile is inside the test object
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType)
{
    if (!hTestObj) return LTFALSE;

	// TO DO???
	// NOTE:  This code may need to be updated to use test the dims
	// of the CharacterHitBox instead of the dims of the object...
	// TO DO???

	// See if we are inside the test object...

    LTVector vTestPos, vTestDims;
    g_pLTServer->GetObjectPos(hTestObj, &vTestPos);
    g_pLTServer->GetObjectDims(hTestObj, &vTestDims);

	if (m_vFirePos.x < vTestPos.x - vTestDims.x ||
		m_vFirePos.x > vTestPos.x + vTestDims.x ||
		m_vFirePos.y < vTestPos.y - vTestDims.y ||
		m_vFirePos.y > vTestPos.y + vTestDims.y ||
		m_vFirePos.z < vTestPos.z - vTestDims.z ||
		m_vFirePos.z > vTestPos.z + vTestDims.z)
	{
        return LTFALSE;
	}


	// We're inside the object, so we automatically hit the object...

	if (eAmmoType == PROJECTILE)
	{
		Detonate(hTestObj);
	}
	else
	{
		if (eAmmoType == VECTOR)
		{
			if (IsCharacter(hTestObj))
			{
                CCharacter *pChar = (CCharacter*) g_pLTServer->HandleToObject(hTestObj);
                if (!pChar) return LTFALSE;

				ModelNode eModelNode = g_pModelButeMgr->GetSkeletonDefaultHitNode(pChar->GetModelSkeleton());

				pChar->SetModelNodeLastHit(eModelNode);

				m_fInstDamage *= pChar->ComputeDamageModifier(eModelNode);
			}

			ImpactDamageObject(m_hFiredFrom, hTestObj);
		}

        LTVector vNormal(0, 1, 0);
		AddImpact(hTestObj, m_vFlashPos, vTestPos, vNormal, GetSurfaceType(hTestObj));
	}

	RemoveObject();

    return LTTRUE;
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
    g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED);

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_MOVEABLE);

	m_damage.Init(m_hObject);
	m_damage.SetMass(m_fMass);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);

	int32 nDamageTypes = m_damage.GetCantDamageTypes();
	m_damage.SetCantDamageTypes(nDamageTypes | m_dwCantDamageTypes);

    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
    g_pLTServer->SetObjectDims(m_hObject, &m_vDims);
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
	// Process moving us through an invisible object if necessary.  This is
	// done here since the position/velocity can't be updated in the touch
	// notify (since the engine is in the process of updating it)...

	if (m_bProcessInvImpact)
	{
		m_bProcessInvImpact = LTFALSE;
	    g_pLTServer->SetVelocity(m_hObject, &m_vInvisVel);
		g_pLTServer->SetObjectPos(m_hObject, &m_vInvisNewPos);
	}

    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);

	// If we didn't hit anything, blow up...

    if (g_pLTServer->GetTime() >= (m_fStartTime + m_fLifeTime))
	{
        Detonate(LTNULL);
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
	if (m_bObjectRemoved) return;

	// Don't process any touches until this has been cleared...

	if (m_bProcessInvImpact) return;

	 // Let it get out of our bounding box...

	if (hObj == m_hFiredFrom) return;

	CCharacterHitBox* pHitBox = LTNULL;

	// If we've hit a character (or body), let its hit box take control...

	if (IsCharacter(hObj))
	{
       CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		if (pChar)
		{
			hObj = pChar->GetHitBox();
		}
	}
	else if (IsBody(hObj))
	{
	    Body* pBody = (Body*)g_pLTServer->HandleToObject(hObj);
		if (pBody)
		{
			hObj = pBody->GetHitBox();
		}
	}


	if (IsCharacterHitBox(hObj))
	{
        pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == m_hFiredFrom) return;
	}


	// Don't hit our own type of projectiles (for multi-projectile weapons
	// and projectiles that stick to objects)...

	if (IsKindOf(hObj, m_hObject))
	{
        CProjectile* pObj = (CProjectile*)g_pLTServer->HandleToObject(hObj);
		if (pObj)
		{
			if (pObj->GetFiredFrom() == m_hFiredFrom)
			{
				return;
			}
		}
	}



	// See if we want to impact on this object...

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

    LTBOOL bIsWorld = IsMainWorld(hObj);


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID))
	{
		if (pHitBox)
		{
			// See if we really impacted on the box...

			if (pHitBox->DidProjectileImpact(this))
			{
				// This is the object that we really hit...

				hObj = pHitBox->GetModelObject();
			}
			else
			{
				return;
			}
		}
		else if (!(dwFlags & FLAG_RAYHIT))
		{
			// If we have ray hit set to true, projectiles should
			// impact on us too...

			return;
		}
	}


	// See if we hit the sky...

	if (bIsWorld || (OT_WORLDMODEL == g_pLTServer->GetObjectType(hObj)))
	{
		CollisionInfo info;
        g_pLTServer->GetLastCollision(&info);

		SurfaceType eType = GetSurfaceType(info);

		if (eType == ST_SKY)
		{
			RemoveObject();
			return;
		}
		else if (eType == ST_INVISIBLE)
		{
			// Update 1.002 [KLS] - If multiplayer and we hit an invisible
			// surface, just treat it like a normal surface...
			if (!IsMultiplayerGame())
			{
				m_bProcessInvImpact = LTTRUE;

				g_pLTServer->GetObjectPos(m_hObject, &m_vInvisNewPos);
				g_pLTServer->GetVelocity(m_hObject, &m_vInvisVel);
				m_vInvisNewPos += (m_vInvisVel * g_pLTServer->GetFrameTime());

				// Make sure this new position is inside the world...else
				// just blow up...

				if (LT_INSIDE == g_pLTServer->Common()->GetPointStatus(&m_vInvisNewPos))
				{
					return;
				}
			}
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
	if (m_bDetonated) return;

	// Make sure we don't detonate if a cinematic is playing (i.e.,
	// make sure the user doesn't disrupt the cinematic)...

	if (Camera::IsActive())
	{
		RemoveObject();
		return;
	}


    m_bDetonated = LTTRUE;

	SurfaceType eType = ST_UNKNOWN;

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...

    LTVector vNormal(0.0f, 1.0f, 0.0f);

	if (hObj)
	{
        if (IsMainWorld(hObj) || g_pLTServer->GetObjectType(hObj) == OT_WORLDMODEL)
		{
			CollisionInfo info;
            g_pLTServer->GetLastCollision(&info);

			if (info.m_hPoly)
			{
				eType = GetSurfaceType(info.m_hPoly);
			}

			LTPlane plane = info.m_Plane;
			vNormal = plane.m_Normal;

			// Calculate where we really hit the plane...

            LTVector vVel, vP0, vP1, vDir;
            g_pLTServer->GetVelocity(m_hObject, &vVel);
			vDir = vVel;
			vDir.Norm();

			vP1 = vPos;
            vVel *= g_pLTServer->GetFrameTime();
			vP0 = vP1 - vVel;
			vP1 += vVel;

			// Make sure we don't tunnel through an object...

			IntersectInfo iInfo;
			IntersectQuery qInfo;

			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

			qInfo.m_From	  = vP0;
			qInfo.m_To		  = vPos;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = m_hObject;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				vPos    = iInfo.m_Point - vDir;
				eType   = GetSurfaceType(iInfo);
				vNormal = iInfo.m_Plane.m_Normal;
			}
			else
			{

				//g_pLTServer->CPrint("P0  = %.2f, %.2f, %.2f", VEC_EXPAND(vP0));
				//g_pLTServer->CPrint("P1  = %.2f, %.2f, %.2f", VEC_EXPAND(vP1));
				//LTVector vDist = vP1 - vP0;
				//g_pLTServer->CPrint("Distance from P0 to P1: %.2f", vDist.Mag());

				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - info.m_Plane.m_Dist;

				if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					//g_pLTServer->CPrint("Percent: %.2f", fPercent);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

            LTRotation rRot;
            g_pLTServer->AlignRotation(&rRot, &vNormal, LTNULL);
            g_pLTServer->SetObjectRotation(m_hObject, &rRot);

			// g_pLTServer->CPrint("Pos = %.2f, %.2f, %.2f", VEC_EXPAND(vPos));
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


	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);


	// Handle impact damage...

	if (hObj)
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;
		ImpactDamageObject(hDamager, hObj);
	}


    //g_pLTServer->CPrint("Server end pos (%.2f, %.2f, %.2f)", vPos.x, vPos.y, vPos.z);
    //g_pLTServer->CPrint("Server fly time %.2f", g_pLTServer->GetTime() - m_fStartTime);

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

void CProjectile::AddImpact(HOBJECT hObj, LTVector vFirePos, LTVector vImpactPos,
                            LTVector vSurfaceNormal, SurfaceType eType)
{
	// Create the client side weapon fx...

	CLIENTWEAPONFX fxStruct;

	fxStruct.hFiredFrom		= m_hFiredFrom;
	fxStruct.vSurfaceNormal	= vSurfaceNormal;
	fxStruct.vFirePos		= vFirePos;
	fxStruct.vPos			= vImpactPos + (m_vDir * -1.0f);
	fxStruct.hObj			= hObj;
	fxStruct.nWeaponId		= m_pWeaponData->nId;
	fxStruct.nAmmoId		= m_pAmmoData->nId;
	fxStruct.nSurfaceType	= eType;
	fxStruct.wIgnoreFX		= g_wIgnoreFX;

	// Always use the flash position for the first call to AddImpact...

	if (m_bNumCallsToAddImpact == 0)
	{
		fxStruct.vFirePos = m_vFlashPos;
	}

	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)

	g_wIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE;


	// Allow exit surface fx on the next call to AddImpact...

	g_wIgnoreFX &= ~WFX_EXITSURFACE;


	if (IsMoveable(hObj))
	{
		// Well, don't do too many exit marks...The server will add one
		// if necessary...

		g_wIgnoreFX |= WFX_EXITMARK;
	}


	// If this is a player object, get the client id...

	if (IsPlayer(m_hFiredFrom))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
            fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	CreateClientWeaponFX(fxStruct);

	// Do the area and progressive (over time) damage...

	if ((m_pAmmoData->nAreaDamage > 0.0f && eType != ST_SKY) ||
		 m_pAmmoData->fProgDamageLifetime > 0.0f)
	{
		AddExplosion(vImpactPos, vSurfaceNormal);
	}


	// Update Character fire info...

	if (m_hFiredFrom && IsCharacter(m_hFiredFrom) && CanSetLastFireInfo())
	{
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pChar)
		{
			CharFireInfo info;
			info.hObject	= hObj;
			info.vFiredPos	= m_vFlashPos;  // Use initial flash pos
			info.vImpactPos = vImpactPos;
			info.nWeaponId  = m_pWeaponData->nId;
			info.nAmmoId	= m_pAmmoData->nId;
            info.fTime      = g_pLTServer->GetTime();
			info.bSilenced  = m_bSilenced;
			info.eSurface	= eType;

			pChar->SetLastFireInfo(&info);
		}
	}

	m_bNumCallsToAddImpact++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(LTVector vPos, LTVector vNormal)
{
    HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Pos = vPos;

    LTRotation rRot;
    g_pLTServer->AlignRotation(&rRot, &vNormal, &vNormal);
    theStruct.m_Rotation = rRot;

    Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);
	if (pExplosion)
	{
		pExplosion->Setup(m_hFiredFrom, m_nAmmoId);
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
	if (!g_pWeaponMgr) return;

	// If this is a player object, get the client id...

    uint8 nShooterId = -1;
	if (IsPlayer(m_hFiredFrom))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
            nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}


	// Create a special fx...

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_PROJECTILE_ID);
    g_pLTServer->WriteToMessageByte(hMessage, m_pWeaponData->nId);
    g_pLTServer->WriteToMessageByte(hMessage, m_pAmmoData->nId);
    g_pLTServer->WriteToMessageByte(hMessage, nShooterId);
    g_pLTServer->EndMessage(hMessage);
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
	if (m_bObjectRemoved) return;

    g_pLTServer->RemoveObject(m_hObject);

    m_bObjectRemoved = LTTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorPolyFilterFn()
//
//	PURPOSE:	Handle filtering out unwanted polies
//
// ----------------------------------------------------------------------- //

LTBOOL DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Don't filter for now...

    return LTTRUE;

	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return LTFALSE;
	}

    return LTTRUE;
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

LTBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
		// CharacterHitBox objects are used for vector impacts, don't
		// impact on the character/body prop object itself....

		if (IsCharacter(hObj) || IsBody(hObj) || IsKindOf(hObj, "Intelligence"))
		{
            return LTFALSE;
		}

		// Check special character hit box cases...

		if (IsCharacterHitBox(hObj))
		{
            CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				// Make sure we don't hit ourself...

				HOBJECT hUs = (HOBJECT)pUserData;

				HOBJECT hTestObj = pCharHitBox->GetModelObject();
                if (!hTestObj) return LTFALSE;

				if (hTestObj == hUs)
				{
                    return LTFALSE;
				}


				// Do special AI hitting AI case...
				if (IsAI(hUs) && IsAI(hTestObj))
				{
                    CAI *pAI = (CAI*) g_pLTServer->HandleToObject(hUs);
                    if (!pAI) return LTFALSE;

					// We can't hit guys we like, unless they're NEUTRAL

                    CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hTestObj);
                    if (!pB) return LTFALSE;

					CharacterClass cc = pB->GetCharacterClass();
					if (cc != NEUTRAL)
					{
						return LIKE != GetAlignement(pAI->GetCharacterClass(), cc);
					}
				}

				// Check for friendly fire
				if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && g_vtNetFriendlyFire.GetFloat() < 1.0f)
				{
					// We can't hit guys on our team unless friendly fire is turned on
					if (IsPlayer(hUs) && IsPlayer(hTestObj))
					{
                        CPlayerObj* pUs = (CPlayerObj*) g_pLTServer->HandleToObject(hUs);
                        if (!pUs) return LTFALSE;


                        CPlayerObj* pThem = (CPlayerObj*) g_pLTServer->HandleToObject(hTestObj);
                        if (!pThem) return LTFALSE;

						if (pUs->GetTeamID() == pThem->GetTeamID())
                            return LTFALSE;
					}

				}
			}
		}

        return LTTRUE;
	}

    return LTFALSE;
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
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX) return;

	// Set up the model...

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	SAFE_STRCPY(createStruct.m_Filename, m_pAmmoData->pProjectileFX->szModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pAmmoData->pProjectileFX->szSkin);

    g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createStruct);
    g_pLTServer->ScaleObject(m_hObject, &(m_pAmmoData->pProjectileFX->vModelScale));

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);


	// Start your engines...

    m_fStartTime = g_pLTServer->GetTime();


	// Make the flash position the same as the fire position...

	m_vFlashPos	= m_vFirePos;


	// If we have a fired from object, make a link to it...

	if (m_hFiredFrom)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hFiredFrom);
	}


	// Set our force ignore limit and mass...

	g_pLTServer->SetBlockingPriority(m_hObject, 0);
	g_pLTServer->SetForceIgnoreLimit(m_hObject, 0.0f);
    g_pLTServer->SetObjectMass(m_hObject, m_fMass);


	// Make sure we are pointing in the direction we are traveling...

    LTRotation rRot;
    g_pLTServer->AlignRotation(&rRot, &m_vDir, LTNULL);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);


	// Make sure we have the correct flags set...

    dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= m_pAmmoData->pProjectileFX->dwObjectFlags;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);


	// And away we go...

    LTVector vVel;
	vVel = m_vDir * m_fVelocity;
    g_pLTServer->SetVelocity(m_hObject, &vVel);


	// Special case of 0 life time...

	if (m_fLifeTime <= 0.0f)
	{
        Detonate(LTNULL);
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
	IntersectInfo iInfo;
	IntersectQuery qInfo;

    LTVector vTo, vFrom, vOriginalFrom;
	vFrom = m_vFirePos;
	vTo	= vFrom + (m_vDir * m_fRange);

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

    LTBOOL bHitSomething = LTFALSE;
    LTBOOL bDone         = LTFALSE;

	int nLoopCount = 0; // No infinite loops thanks.

	HOBJECT hLastHitbox = LTNULL;

	while (!bDone)
	{
		qInfo.m_PolyFilterFn = DoVectorPolyFilterFn;
		qInfo.m_FilterFn	 = DoVectorFilterFn;
		qInfo.m_pUserData	 = m_hFiredFrom;

		qInfo.m_From = vFrom;
		qInfo.m_To   = vTo;

        if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
		{
            HCLASS hClass = g_pLTServer->GetObjectClass(iInfo.m_hObject);

			if ( iInfo.m_hObject == hLastHitbox )
			{
				_ASSERT(LTFALSE);
				g_pLTServer->CPrint("ERROR: vector intersected character hitbox twice");
				bDone = LTTRUE;
				continue;
			}

			if (IsCharacterHitBox(iInfo.m_hObject))
			{
				hLastHitbox = iInfo.m_hObject;

				vOriginalFrom = vFrom;

				if (HandlePotentialHitBoxImpact(iInfo, vFrom))
				{
					HandleVectorImpact(iInfo, vOriginalFrom, vTo);
					return;
				}
			}
			else
			{
				if (HandleVectorImpact(iInfo, vFrom, vTo))
				{
					return;
				}
			}
		}
		else // Didn't hit anything...
		{
            bDone = LTTRUE;
		}


		// Melee weapons can't shoot through objects...

		if (m_pAmmoData->eInstDamageType == DT_MELEE)
		{
			bDone = LTTRUE;
		}


		// Make sure we don't loop forever...

		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
            g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Infinite loop encountered!!!");
            bDone = LTTRUE;
		}
	}


	// Didn't hit anything so just impact at the end pos...

    LTVector vUp(0.0f, 1.0f, 0.0f);
    AddImpact(LTNULL, m_vFlashPos, vTo, vUp, ST_SKY);


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

LTBOOL CProjectile::HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom,
                                     LTVector & vTo)
{
	// Get the surface type...

	SurfaceType eSurfType = GetSurfaceType(iInfo);


	// See if we hit an invisible surface...

	if (eSurfType == ST_INVISIBLE)
	{
		if (!CalcInvisibleImpact(iInfo, eSurfType))
		{
			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
            if (!pSurf) return LTTRUE;

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}
	}


	// See if we hit an object that should be damaged...

    LTBOOL bHitWorld = IsMainWorld(iInfo.m_hObject);

	if (!bHitWorld && eSurfType != ST_LIQUID)
	{
		ImpactDamageObject(m_hFiredFrom, iInfo.m_hObject);
	}


	// If the fire position is the initial fire position, use the flash
	// position when building the impact special fx...

    LTVector vFirePos = (vFrom.Equals(m_vFirePos) ? m_vFlashPos : vFrom);

	AddImpact(iInfo.m_hObject, vFirePos, iInfo.m_Point, iInfo.m_Plane.m_Normal, eSurfType);


	// See if we can shoot through the surface...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
    if (!pSurf) return LTTRUE;  // Done.

	if (pSurf->bCanShootThrough)
	{
		int nMaxThickness = pSurf->nMaxShootThroughThickness;
		if (nMaxThickness == 0)
		{
			// Special case of always being able to shoot through surface...

			// Calculate new values for next DoVector iteration...

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}

		// Test if object/wall intersected is thin enough to be shot
		// through...

		// Test object case first...

		if (!bHitWorld && iInfo.m_hObject)
		{
			// Test to see if we can shoot through the object...

            LTVector vDims;
            g_pLTServer->GetObjectDims(iInfo.m_hObject, &vDims);

			if (vDims.x*2.0f >= nMaxThickness &&  vDims.y*2.0f >= nMaxThickness &&
				vDims.z*2.0f >= nMaxThickness)
			{
				// Can't shoot through this object...
                return LTTRUE;
			}
		}

		// Determine if we shot through the wall/object...

		IntersectInfo iTestInfo;
		IntersectQuery qTestInfo;

        qTestInfo.m_From = iInfo.m_Point + (m_vDir * (LTFLOAT)(nMaxThickness + 1));
		qTestInfo.m_To   = iInfo.m_Point - m_vDir;

		qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

		qTestInfo.m_FilterFn  = DoVectorFilterFn;
		qTestInfo.m_pUserData = m_hFiredFrom;

        if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
		{
			// Calculate new values for next DoVector iteration...

            LTVector vThickness = iTestInfo.m_Point - iInfo.m_Point;
			return UpdateDoVectorValues(*pSurf, vThickness.Mag(), iTestInfo.m_Point, vFrom, vTo);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateVectorValues
//
//	PURPOSE:	Update our DoVector values
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness,
                                        LTVector vImpactPos, LTVector & vFrom, LTVector & vTo)
{
	// See if we've traveled the distance...

    LTVector vDistTraveled = vImpactPos - m_vFirePos;
    LTFLOAT fDist = m_fRange - vDistTraveled.Mag();
    if (fDist < 1.0f) return LTTRUE;

	// Just dampen based on the bute file values, don't worry about the
	// surface thinkness...

	m_fInstDamage *= surf.fBulletDamageDampen;
	fDist *= surf.fBulletRangeDampen;

	int nPerturb = surf.nMaxShootThroughPerturb;

	if (nPerturb)
	{
        LTRotation rRot;
        g_pLTServer->AlignRotation(&rRot, &m_vDir, LTNULL);

        LTVector vU, vR, vF;
        g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

        LTFLOAT fRPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;
        LTFLOAT fUPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		m_vDir += (vR * fRPerturb);
		m_vDir += (vU * fUPerturb);

		m_vDir.Norm();
	}

	// Make sure we move the from position...

	if (vFrom.Equals(vImpactPos, 1.0f))
	{
		vFrom += m_vDir;
	}
	else
	{
		vFrom = vImpactPos;
	}

	vTo = vFrom + (m_vDir * fDist);

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CalcInvisibleImpact
//
//	PURPOSE:	Update the impact value so it ignores invisible surfaces
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType)
{
	// Since we hit an invisible surface try and find a solid surface that
	// is the real surface of impact.  NOTE:  We assume that the solid
	// surface will have a normal facing basically the opposite direction...

	IntersectInfo iTestInfo;
	IntersectQuery qTestInfo;

	qTestInfo.m_From = iInfo.m_Point + (m_vDir * g_vtInvisibleMaxThickness.GetFloat());
	qTestInfo.m_To   = iInfo.m_Point - m_vDir;

	qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	qTestInfo.m_FilterFn  = DoVectorFilterFn;
	qTestInfo.m_pUserData = m_hFiredFrom;

    if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
	{
		eSurfType = GetSurfaceType(iTestInfo);

		// If we hit another invisible surface, we're done...

		if (eSurfType != ST_INVISIBLE)
		{
			iInfo = iTestInfo;
            return LTTRUE;
		}
	}

    return LTFALSE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialHitBoxImpact
//
//	PURPOSE:	Handle a vector hitting a Character's hit box
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandlePotentialHitBoxImpact(IntersectInfo & iInfo,
                                               LTVector & vFrom)
{
    CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(iInfo.m_hObject);
    if (!pCharHitBox) return LTFALSE;

	return pCharHitBox->HandleImpact(this, iInfo, m_vDir, vFrom);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ImpactDamageObject
//
//	PURPOSE:	Handle impact damage to an object
//
// ----------------------------------------------------------------------- //

void CProjectile::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj)
{
	DamageStruct damage;

	damage.hDamager = hDamager;
	damage.vDir		= m_vDir;

	// Do Instant damage...

	if (m_fInstDamage > 0.0f)
	{
		damage.eType	= m_eInstDamageType;
		damage.fDamage	= m_fInstDamage;

		damage.DoDamage(this, hObj);
	}

	// Do Progressive damage...(if the progressive damage is supposed to
	// happen over time, it will be done in the explosion object)....

	if (m_fProgDamage > 0.0f && m_pAmmoData->fProgDamageLifetime <= 0.0f)
	{
		damage.eType	 = m_eProgDamageType;
		damage.fDamage	 = m_fProgDamage;
		damage.fDuration = m_pAmmoData->fProgDamageDuration;

		damage.DoDamage(this, hObj);
	}



	// Update player summary info...

	CPlayerObj* pPlayer;
	if (IsPlayer(hDamager) && IsCharacter(hObj) && IsAccuracyType(m_eInstDamageType))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(hObj);
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer)
		{
			ModelNode eModelNode = pChar->GetModelNodeLastHit();

			if (eModelNode != eModelNodeInvalid)
			{
				HitLocation eLoc = g_pModelButeMgr->GetSkeletonNodeLocation(pChar->GetModelSkeleton(),eModelNode);
				pPlayer->GetPlayerSummaryMgr()->IncNumHits(eLoc);
			}
			else
			{
				pPlayer->GetPlayerSummaryMgr()->IncNumHits(HL_UNKNOWN);
			}
		}
	}

	if (IsPlayer(hObj))
	{
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hObj);
		if (pPlayer)
		{
			pPlayer->GetPlayerSummaryMgr()->IncNumTimesHit();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hFiredFrom);

    g_pLTServer->WriteToMessageVector(hWrite, &m_vFlashPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vFirePos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vDir);
    g_pLTServer->WriteToMessageByte(hWrite, m_bSilenced);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRemoveFromWorld);
    g_pLTServer->WriteToMessageByte(hWrite, m_bObjectRemoved);
    g_pLTServer->WriteToMessageByte(hWrite, m_bDetonated);
    g_pLTServer->WriteToMessageByte(hWrite, m_nWeaponId);
    g_pLTServer->WriteToMessageByte(hWrite, m_nAmmoId);
    g_pLTServer->WriteToMessageByte(hWrite, m_eInstDamageType);
    g_pLTServer->WriteToMessageByte(hWrite, m_eProgDamageType);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fInstDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fStartTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLifeTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fVelocity);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRange);

    g_pLTServer->WriteToMessageByte(hWrite, m_bProcessInvImpact);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vInvisVel);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vInvisNewPos);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hFiredFrom);

    g_pLTServer->ReadFromMessageVector(hRead, &m_vFlashPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vFirePos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vDir);
    m_bSilenced         = g_pLTServer->ReadFromMessageByte(hRead);
    m_bRemoveFromWorld  = g_pLTServer->ReadFromMessageByte(hRead);
    m_bObjectRemoved    = g_pLTServer->ReadFromMessageByte(hRead);
    m_bDetonated        = g_pLTServer->ReadFromMessageByte(hRead);
    m_nWeaponId         = g_pLTServer->ReadFromMessageByte(hRead);
    m_nAmmoId           = g_pLTServer->ReadFromMessageByte(hRead);
    m_eInstDamageType   = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_eProgDamageType   = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_fInstDamage       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fProgDamage       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fStartTime        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLifeTime         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fVelocity         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fRange            = g_pLTServer->ReadFromMessageFloat(hRead);

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData	  = g_pWeaponMgr->GetAmmo(m_nAmmoId);

    m_bProcessInvImpact = g_pLTServer->ReadFromMessageByte(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vInvisVel);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vInvisNewPos);
}