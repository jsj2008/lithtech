// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 7/6/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileFX.h"
#include "GameClientShell.h"
#include "ParticleTrailFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "iltphysics.h"
#include "ClientWeaponUtils.h"
#include "ParticleTrailFX.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "CMoveMgr.h"
#include "ClientMultiplayerMgr.h"
#include "FXButeMgr.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile system fx
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	PROJECTILECREATESTRUCT proj;

	proj.hServerObj = hServObj;
    proj.nWeaponId  = pMsg->Readuint8();
    proj.nAmmoId    = pMsg->Readuint8();
    proj.nShooterId = pMsg->Readuint8();

	return Init(&proj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile fx
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PROJECTILECREATESTRUCT* pCS = (PROJECTILECREATESTRUCT*)psfxCreateStruct;

	m_nWeaponId		= pCS->nWeaponId;
	m_nAmmoId		= pCS->nAmmoId;
	m_nShooterId	= pCS->nShooterId;
	m_bLocal		= pCS->bLocal;
	m_bAltFire		= pCS->bAltFire;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo || !pAmmo->pProjectileFX)
	{
        return LTFALSE;
	}

	m_pProjectileFX = pAmmo->pProjectileFX;
	m_nFX = m_pProjectileFX->nFlags;

	ASSERT( 0 != m_pProjectileFX );
	// the fx associated with this projectile fx
	if ( '\0' != m_pProjectileFX->szFXName[ 0 ] )
	{
		// get the flags for this ClientFX
		uint32 dwFlags = m_pProjectileFX->dwFXFlags;

		// prepare the create struct
		CLIENTFX_CREATESTRUCT fxInit
			(
				m_pProjectileFX->szFXName,
				dwFlags,
				pCS->hServerObj
			);

		// get in initial position for the effect
		g_pLTClient->GetObjectPos( pCS->hServerObj, &fxInit.m_vPos );

		// create the client fx
		CLIENTFX_LINK tmpClientFXLink;
		g_pClientFXMgr->CreateClientFX
			(
				&tmpClientFXLink,
				fxInit,
				true
			);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTFALSE;

    uint8 nDetailLevel = pSettings->SpecialFXSetting();

    LTVector vPos;
    LTRotation rRot;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	//m_pClientDE->CPrint("Client start pos (%.2f, %.2f, %.2f)", vPos.x, vPos.y, vPos.z);
	//m_fStartTime = m_pClientDE->GetTime();

	if (nDetailLevel != RS_LOW)
	{
		if (m_nFX & PFX_SMOKETRAIL)
		{
			CreateSmokeTrail(vPos, rRot);
		}

		if (m_nFX & PFX_LIGHT)
		{
			CreateLight(vPos, rRot);
		}
	}

	if (m_nFX & PFX_FLARE)
	{
		CreateFlare(vPos, rRot);
	}

	if (m_nFX & PFX_FLYSOUND)
	{
		CreateFlyingSound(vPos, rRot);
	}


	// Do client-side projectiles in multiplayer games...

	if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		// Set the velocity of the "server" object if it is really just a local
		// object...

		if (m_bLocal)
		{
			VEC_COPY(m_vFirePos, vPos);

			m_fStartTime = m_pClientDE->GetTime();

            LTVector vVel, vF;
			vF = rRot.Forward();

			m_vPath = vF;

			// Special case of adjusting the projectile speed...

            LTFLOAT fVel = (LTFLOAT) m_pProjectileFX->nVelocity;
			if (m_bAltFire)
			{
                fVel = (LTFLOAT) m_pProjectileFX->nAltVelocity;
			}

            LTFLOAT fMultiplier = 1.0f;
            if (m_pClientDE->GetSConValueFloat("MissileSpeed", fMultiplier) != LT_NOTFOUND)
			{
				fVel *= fMultiplier;
			}

			vVel = vF * fVel;
			g_pPhysicsLT->SetVelocity(m_hServerObject, &vVel);
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileFX::Update()
{
    if (!m_pClientDE) return LTFALSE;


	if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && m_hServerObject)
	{
		// If this is a local fx, we control the position of the "server object"...

		if (m_bLocal)
		{
			if (!MoveServerObj())
			{
                Detonate(LTNULL);

				// Remove the "server" object...

				m_pClientDE->RemoveObject(m_hServerObject);
                m_hServerObject = LTNULL;
                m_bWantRemove = LTTRUE;
			}
		}
	}


	// Update fx positions...

    LTRotation rRot;
    LTVector vPos;

	if (m_hServerObject)
	{
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
	}


	// See if it is time to go away...

	if (m_bWantRemove)
	{
		RemoveFX();
        return LTFALSE;
	}


	if (m_pSmokeTrail)
	{
		m_pSmokeTrail->Update();
	}


	if (m_hFlare)
	{
		g_pLTClient->SetObjectPos(m_hFlare, &vPos);
		g_pLTClient->SetObjectRotation(m_hFlare, &rRot);
	}

	if (m_hLight)
	{
		g_pLTClient->SetObjectPos(m_hLight, &vPos);
		g_pLTClient->SetObjectRotation(m_hLight, &rRot);
	}

	if (m_hFlyingSound)
	{
		((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundPosition(m_hFlyingSound, &vPos);
	}


	// Update this here so m_vLastServPos is updated after we use it...

	CSpecialFX::Update();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::MoveServerObj
//
//	PURPOSE:	Update mover
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectileFX::MoveServerObj()
{
    if (!m_pClientDE || !m_bLocal || !m_hServerObject || !g_pWeaponMgr) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// If we didn't hit anything we're done...

	if (fTime >= (m_fStartTime + m_pProjectileFX->fLifeTime))
	{
        return LTFALSE;
	}

    LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

    LTBOOL bRet = LTTRUE;

	// Zero out the acceleration to start with.

    LTVector zeroVec;
	zeroVec.Init();
	g_pPhysicsLT->SetAcceleration(m_hServerObject, &zeroVec);

	MoveInfo info;

	info.m_hObject  = m_hServerObject;
	info.m_dt		= fFrameTime;
	((ILTClientPhysics*)g_pPhysicsLT)->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
        LTVector vDiff, vNewPos, vCurPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		vNewPos = vCurPos + info.m_Offset;
		g_pPhysicsLT->MoveObject(m_hServerObject, &vNewPos, 0);

		vDiff = vCurPos - vNewPos;
		if (vDiff.MagSqr() < 5.0f)
		{
            bRet = LTFALSE;
		}
	}
	else
	{
        bRet = LTFALSE;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::GetParticleTrailBank
//
//	PURPOSE:	Get the CParticleTrailFX effect bank
//
// ----------------------------------------------------------------------- //

CBankedList<CParticleTrailFX> *CProjectileFX::GetParticleTrailBank()
{
	static CBankedList<CParticleTrailFX> theBank;

	return &theBank;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateSmokeTrail
//
//	PURPOSE:	Create a smoke trail special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateSmokeTrail(const LTVector & vPos, const LTRotation & rRot)
{
	if (!g_pGameClientShell || !m_hServerObject) return;

	//  Particle smoke trail...

	PTCREATESTRUCT pt;

	pt.hServerObj = m_hServerObject;
	pt.nType	  = m_pProjectileFX->nSmokeTrailType;

	m_pSmokeTrail = GetParticleTrailBank()->New();
	if (!m_pSmokeTrail) return;

	m_pSmokeTrail->Init(&pt);
	m_pSmokeTrail->CreateObject(m_pClientDE);
 }


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlare
//
//	PURPOSE:	Create a flare special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlare(const LTVector & vPos, const LTRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	if (!m_pProjectileFX->szFlareSprite[0]) return;

	SAFE_STRCPY(createStruct.m_Filename, m_pProjectileFX->szFlareSprite);
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, vPos);
    createStruct.m_Rotation = rRot;

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
	if (!m_hFlare) return;

    LTFLOAT fScale = m_pProjectileFX->fFlareScale;
	m_pClientDE->SetObjectScale(m_hFlare, &LTVector(fScale, fScale, 1.0f));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateLight
//
//	PURPOSE:	Create a light special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateLight(const LTVector & vPos, const LTRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;
	VEC_COPY(createStruct.m_Pos, vPos);

	m_hLight = m_pClientDE->CreateObject(&createStruct);
	if (!m_hLight) return;

    LTVector vColor = m_pProjectileFX->vLightColor;
    LTFLOAT fRadius = (LTFLOAT) m_pProjectileFX->nLightRadius;

	m_pClientDE->SetLightColor(m_hLight, vColor.x, vColor.y, vColor.z);
	m_pClientDE->SetLightRadius(m_hLight, fRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlyingSound
//
//	PURPOSE:	Create the flying sound
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlyingSound(const LTVector & vPos, const LTRotation & rRot)
{
	if (!m_pClientDE || m_hFlyingSound || !g_pWeaponMgr) return;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

    LTFLOAT fRadius = (LTFLOAT) m_pProjectileFX->nSoundRadius;
	if (m_pProjectileFX->szSound[0] )
	{
		m_hFlyingSound = g_pClientSoundMgr->PlaySoundFromPos((LTVector)vPos, m_pProjectileFX->szSound,
			fRadius, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::RemoveFX()
{
	if (!g_pGameClientShell || !m_pClientDE) return;

	if (m_pSmokeTrail)
	{
		GetParticleTrailBank()->Delete((CParticleTrailFX*)m_pSmokeTrail);
        m_pSmokeTrail = LTNULL;
	}

	if (m_hFlare)
	{
		m_pClientDE->RemoveObject(m_hFlare);
        m_hFlare = LTNULL;
	}

	if (m_hLight)
	{
		m_pClientDE->RemoveObject(m_hLight);
        m_hLight = LTNULL;
	}

	if (m_hFlyingSound)
	{
		m_pClientDE->SoundMgr()->KillSound(m_hFlyingSound);
        m_hFlyingSound = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectileFX::HandleTouch(CollisionInfo *pInfo)
{
	if (!m_pClientDE || !pInfo || !pInfo->m_hObject || !g_pGameClientShell) return;

	 // Let it get out of our bounding box...

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (pMoveMgr)
	{
		// Don't colide with the move mgr object...

		HLOCALOBJ hMoveObj = pMoveMgr->GetObject();
		if (pInfo->m_hObject == hMoveObj) return;

		// Don't colide with the player object...

		HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
		if (pInfo->m_hObject == hPlayerObj) return;
	}


	// See if we want to impact on this object...

    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

    LTBOOL bIsWorld = IsMainWorld(pInfo->m_hObject);

	// Don't impact on non-solid objects...

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID)) return;


	// See if we hit the sky...

	if (bIsWorld)
	{
		SurfaceType eType = GetSurfaceType(pInfo->m_hPoly);

		if (eType == ST_SKY)
		{
            m_bWantRemove = LTTRUE;
			return;
		}
		else if (eType == ST_INVISIBLE)
		{
			// Keep going, ignore this object...
			return;
		}
	}

	Detonate(pInfo);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::Detonate(CollisionInfo* pInfo)
{
	if (!m_pClientDE || m_bDetonated) return;

    m_bDetonated = LTTRUE;

	SurfaceType eType = ST_UNKNOWN;

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	// Determine the normal of the surface we are impacting on...

    LTVector vNormal;
	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

	if (pInfo)
	{
		if (pInfo->m_hObject)
		{
			eType = GetSurfaceType(pInfo->m_hObject);
		}
		else if (pInfo->m_hPoly != INVALID_HPOLY)
		{
			eType = GetSurfaceType(pInfo->m_hPoly);

			VEC_COPY(vNormal, pInfo->m_Plane.m_Normal);

            LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
			m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);

			// Calculate where we really hit the plane...

            LTVector vVel, vP0, vP1;
			g_pPhysicsLT->GetVelocity(m_hServerObject, &vVel);

			VEC_COPY(vP1, vPos);
			VEC_MULSCALAR(vVel, vVel, g_pGameClientShell->GetFrameTime());
			VEC_SUB(vP0, vP1, vVel);

            LTFLOAT fDot1 = VEC_DOT(pInfo->m_Plane.m_Normal, vP0) - pInfo->m_Plane.m_Dist;
            LTFLOAT fDot2 = VEC_DOT(pInfo->m_Plane.m_Normal, vP1) - pInfo->m_Plane.m_Dist;

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				VEC_COPY(vPos, vP1);
			}
			else
			{
                LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
				VEC_LERP(vPos, vP0, vP1, fPercent);
			}
		}
	}
	else
	{
		// Since pInfo was null, this means the projectile's lifetime was up,
		// so we just blow-up in the air.

		eType = ST_AIR;
	}


    HOBJECT hObj = !!pInfo ? pInfo->m_hObject : LTNULL;
	::AddLocalImpactFX(hObj, m_vFirePos, vPos, vNormal, eType, m_vPath,
					   m_nWeaponId, m_nAmmoId, 0);

    m_bWantRemove = LTTRUE;
}
