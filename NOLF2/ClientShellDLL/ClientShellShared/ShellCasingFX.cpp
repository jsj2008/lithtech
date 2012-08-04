// ----------------------------------------------------------------------- //
//
// MODULE  : SHELLCASINGFX.CPP
//
// PURPOSE : defines class for ejected shells
//
// CREATED : 5/1/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// Includes....

#include "stdafx.h"
#include "ShellCasingFX.h"
#include "iltclient.h"
#include "ltlink.h"
#include "ClientUtilities.h"
#include "WeaponMgr.h"
#include "GameClientShell.h"
#include "ClientWeaponUtils.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "FXButeMgr.h"


VarTrack	g_vtShellMaxBounceCountTrack;
VarTrack	g_vtShellMinUpVelocity;
VarTrack	g_vtShellMaxUpVelocity;
VarTrack	g_vtShellMinRightVelocity;
VarTrack	g_vtShellMaxRightVelocity;
VarTrack	g_vtShellMinForwardVelocity;
VarTrack	g_vtShellMaxForwardVelocity;
VarTrack	g_vtShellMinLifetime;
VarTrack	g_vtShellMaxLifetime;
VarTrack	g_vtShellScaleTime;
VarTrack	g_vtShellMaxScale;
VarTrack	g_vtShellMinSpinsPerSecond;
VarTrack	g_vtShellMaxSpinsPerSecond;

#define MAX_BOUNCE_COUNT				3

extern PhysicsState g_normalPhysicsState;
extern PhysicsState g_waterPhysicsState;

//-------------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------------

// this function will return a random value that can be used for a random
// pitch/yaw velocity
float GenerateRandomVelocity()
{
	//find a random number between that range
	float fRand = GetRandom(g_vtShellMinSpinsPerSecond.GetFloat(), g_vtShellMaxSpinsPerSecond.GetFloat());


	//scale it to be either positive or negative
	if(rand() % 2 == 0)
		fRand = -fRand;

	//now map it into rotations
	return /*MATH_CIRCLE * */ fRand;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CShellCasingFX
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CShellCasingFX::CShellCasingFX()
{
    m_rRot.Init();
	m_vStartPos.Init();
	m_vStartVel.Init();
	m_nWeaponId = WMGR_INVALID_ID;
	m_nAmmoId = WMGR_INVALID_ID;
    m_b3rdPerson = LTFALSE;
    m_fPitchVel = 0.0f;
    m_fYawVel = 0.0f;
    m_fPitch = 0.0f;
    m_fYaw = 0.0f;

	m_vInitialScale.Init(1.0f, 1.0f, 1.0f);
	m_vFinalScale.Init(1.0f, 1.0f, 1.0f);

	m_dwFlags		= FLAG_VISIBLE; // | FLAG_NOLIGHT;
	m_fDieTime		= 15.0f;
    m_bResting      = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Init
//
//	PURPOSE:	Create the shell casing
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	SHELLCREATESTRUCT* pShell = (SHELLCREATESTRUCT*)psfxCreateStruct;

    m_rRot = pShell->rRot;
	m_vStartPos = pShell->vStartPos;
	m_nWeaponId = pShell->nWeaponId;
	m_nAmmoId = pShell->nAmmoId;
	m_dwFlags |= pShell->dwFlags;
	m_vStartVel = pShell->vStartVel;
	m_b3rdPerson = pShell->b3rdPerson;

	m_vInitialScale.Init(1.0f, 1.0f, 1.0f);
	m_vFinalScale.Init(1.0f, 1.0f, 1.0f);

	if (!g_vtShellMaxBounceCountTrack.IsInitted())
	{
		g_vtShellMaxBounceCountTrack.Init(g_pLTClient, "ShellMaxBounceCount", NULL, 3.0);
	}
	if (!g_vtShellMinUpVelocity.IsInitted())
	{
		g_vtShellMinUpVelocity.Init(g_pLTClient, "ShellMinUpVelocity", NULL, 30.0f);
	}
	if (!g_vtShellMaxUpVelocity.IsInitted())
	{
		g_vtShellMaxUpVelocity.Init(g_pLTClient, "ShellMaxUpVelocity", NULL, 75.0f);
	}
	if (!g_vtShellMinRightVelocity.IsInitted())
	{
		g_vtShellMinRightVelocity.Init(g_pLTClient, "ShellMinRightVelocity", NULL, 50.0f);
	}
	if (!g_vtShellMaxRightVelocity.IsInitted())
	{
		g_vtShellMaxRightVelocity.Init(g_pLTClient, "ShellMaxRightVelocity", NULL, 75.0f);
	}
	if (!g_vtShellMinForwardVelocity.IsInitted())
	{
		g_vtShellMinForwardVelocity.Init(g_pLTClient, "ShellMinForwardVelocity", NULL, 20.0f);
	}
	if (!g_vtShellMaxForwardVelocity.IsInitted())
	{
		g_vtShellMaxForwardVelocity.Init(g_pLTClient, "ShellMaxForwardVelocity", NULL, 50.0f);
	}
	if (!g_vtShellMinLifetime.IsInitted())
	{
		g_vtShellMinLifetime.Init(g_pLTClient, "ShellMinLifetime", NULL, 10.0f);
	}
	if (!g_vtShellMaxLifetime.IsInitted())
	{
		g_vtShellMaxLifetime.Init(g_pLTClient, "ShellMaxLifetime", NULL, 15.0f);
	}
	if (!g_vtShellScaleTime.IsInitted())
	{
		g_vtShellScaleTime.Init(g_pLTClient, "ShellScaleTime", NULL, 0.5f);
	}
	if (!g_vtShellMaxScale.IsInitted())
	{
		g_vtShellMaxScale.Init(g_pLTClient, "ShellMaxScale", NULL, 2.0f);
	}
	if (!g_vtShellMinSpinsPerSecond.IsInitted())
	{
		g_vtShellMinSpinsPerSecond.Init(g_pLTClient, "ShellMinSpinsPerSecond", NULL, 2.0f);
	}
	if (!g_vtShellMaxSpinsPerSecond.IsInitted())
	{
		g_vtShellMaxSpinsPerSecond.Init(g_pLTClient, "ShellMaxSpinsPerSecond", NULL, 10.0f);
	}

	m_nBounceCount = (int)g_vtShellMaxBounceCountTrack.GetFloat();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CreateObject
//
//	PURPOSE:	Create the model associated with the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Setup the shell...

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
    if (!pAmmo || !pAmmo->pFireFX) return LTFALSE;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, pAmmo->pFireFX->szShellModel);
	SAFE_STRCPY(createStruct.m_SkinName, pAmmo->pFireFX->szShellSkin);

	m_vInitialScale = pAmmo->pFireFX->vShellScale;
	m_vFinalScale = (m_vInitialScale * g_vtShellMaxScale.GetFloat());

	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags = m_dwFlags;
	createStruct.m_Pos = m_vStartPos;
    createStruct.m_Rotation = m_rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);
    if (!m_hObject) return LTFALSE;

	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);

	// User camera rotation if not 3rd person ;)

	if (!m_b3rdPerson)
	{
		g_pPlayerMgr->GetCameraRotation(m_rRot);
	}

	m_vStartVel += (m_rRot.Up() * GetRandom(g_vtShellMinUpVelocity.GetFloat(), g_vtShellMaxUpVelocity.GetFloat()));
	m_vStartVel += (m_rRot.Right() * GetRandom(g_vtShellMinRightVelocity.GetFloat(), g_vtShellMaxRightVelocity.GetFloat()));
	m_vStartVel += (m_rRot.Forward() * GetRandom(g_vtShellMinForwardVelocity.GetFloat(), g_vtShellMaxForwardVelocity.GetFloat()));

	InitMovingObject(&m_movingObj, m_vStartPos, m_vStartVel);;

	m_fElapsedTime = 0.0f;
	m_fDieTime = GetRandom(g_vtShellMinLifetime.GetFloat(), g_vtShellMaxLifetime.GetFloat());

	m_fPitchVel = GenerateRandomVelocity();
	m_fYawVel	= GenerateRandomVelocity();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Update
//
//	PURPOSE:	Update the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Update()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

	if (g_pGameClientShell->IsServerPaused())
	{
		return LTTRUE;
	}

	m_fElapsedTime += g_pGameClientShell->GetFrameTime();
	m_fDieTime -= g_pGameClientShell->GetFrameTime();
	
    if (m_fDieTime <= 0.0f) return LTFALSE;

	// Update object scale if necessary...

	LTVector vScale;
	m_pClientDE->GetObjectScale(m_hObject, &vScale);

	if (vScale != m_vFinalScale)
	{
		if (m_fElapsedTime <= g_vtShellScaleTime.GetFloat())
		{
			LTVector vScaleRange = (m_vFinalScale - m_vInitialScale);

			vScale = m_vInitialScale + (vScaleRange * (m_fElapsedTime/g_vtShellScaleTime.GetFloat()));

			if (vScale > m_vFinalScale)
			{
				vScale = m_vFinalScale;
			}

			m_pClientDE->SetObjectScale(m_hObject, &vScale);
		}
		else
		{
			m_pClientDE->SetObjectScale(m_hObject, &m_vFinalScale);
		}
	}

    if (m_bResting) return LTTRUE;

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hObject, &rRot);

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.

	if (m_movingObj.m_dwPhysicsFlags & MO_RESTING)
	{
        m_bResting = LTTRUE;

		// Stop the spinning...

		rRot.Rotate(rRot.Up(), m_fYaw);
		g_pLTClient->SetObjectRotation(m_hObject, &rRot);

		// Shell is at rest, we can add a check here to see if we really want
		// to keep it around depending on detail settings...

		//HLOCALOBJ hObjs[1];
        //uint32 nNumFound, nBogus;
		//m_pClientDE->FindObjectsInSphere(&m_movingObj.m_vPos, 64.0f, hObjs, 1, &nBogus, &nNumFound);

		// Remove thyself...
        //if (nNumFound > 15) return LTFALSE;
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
            LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;

			rRot.Rotate(rRot.Up(), m_fYaw);
			rRot.Rotate(rRot.Right(), m_fPitch);
			g_pLTClient->SetObjectRotation(m_hObject, &rRot);
		}
	}


    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, &m_movingObj, vNewPos))
	{
		ClientIntersectInfo info;
		LTBOOL bBouncedOnGround = LTFALSE;
        if (BounceMovingObject(LTNULL, &m_movingObj, vNewPos, &info, 
			INTERSECT_HPOLY, true, bBouncedOnGround))
		{
			// If we hit the sky/invisible surface we're done...

			SurfaceType eType = GetSurfaceType(info);
			if (eType == ST_SKY || eType == ST_INVISIBLE)
			{
                return LTFALSE;
			}

			if (m_nBounceCount >= MAX_BOUNCE_COUNT)
			{
				if (!(m_movingObj.m_dwPhysicsFlags & MO_LIQUID))
				{
					SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
					if (pSurf)
					{
						// Play appropriate sound...

						if (pSurf->szShellImpactSnds[0])
						{
							g_pClientSoundMgr->PlaySoundFromPos(vNewPos, pSurf->szShellImpactSnds[0], pSurf->fShellSndRadius,
								SOUNDPRIORITY_MISC_LOW);
						}
					}
				}
			}

			// Adjust the bouncing..

			m_fPitchVel *= 0.75f;
			m_fYawVel	*= -0.75f;

			m_nBounceCount--;

			if (m_nBounceCount <= 0)
			{
				m_movingObj.m_dwPhysicsFlags |= MO_RESTING;
			}
		}

		m_movingObj.m_vPos = vNewPos;

        if (g_pCommonLT->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
            return LTFALSE;
		}

		g_pLTClient->SetObjectPos(m_hObject, &vNewPos);
	}

    return LTTRUE;
}