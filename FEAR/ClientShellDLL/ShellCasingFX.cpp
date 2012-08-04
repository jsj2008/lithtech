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
#include "GameClientShell.h"
#include "ClientWeaponUtils.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "FXDB.h"
#include "PlayerCamera.h"


VarTrack	g_vtShellMaxBounceCount;
VarTrack	g_vtShellMinLifetime;
VarTrack	g_vtShellMaxLifetime;
VarTrack	g_vtShellScaleTime;
VarTrack	g_vtShellMaxScale;
VarTrack	g_vtShellMinSpinsPerSecond;
VarTrack	g_vtShellMaxSpinsPerSecond;

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
	return MATH_TWOPI * fRand;
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
	m_hWeapon = NULL;
	m_hAmmo = NULL;
	m_b3rdPerson = false;
	m_fPitchVel = 0.0f;
	m_fYawVel = 0.0f;

	m_fInitialScale = 1.0f;
	m_fFinalScale	= 1.0f;

	m_dwFlags		= FLAG_VISIBLE; // | FLAG_NOLIGHT;
	m_fDieTime		= 15.0f;
	m_bResting		= false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Init
//
//	PURPOSE:	Create the shell casing
//
// ----------------------------------------------------------------------- //

bool CShellCasingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return false;

	CSpecialFX::Init(psfxCreateStruct);

	SHELLCREATESTRUCT* pShell = (SHELLCREATESTRUCT*)psfxCreateStruct;

    m_rRot = pShell->rRot;
	m_vStartPos = pShell->vStartPos;
	m_hWeapon = pShell->hWeapon;
	m_hAmmo = pShell->hAmmo;
	m_dwFlags |= pShell->dwFlags;
	m_vStartVel = pShell->vStartVel;
	m_b3rdPerson = pShell->b3rdPerson;

	m_fInitialScale	= 1.0f;
	m_fFinalScale	= 1.0f;

	if (!g_vtShellMaxBounceCount.IsInitted())
	{
		g_vtShellMaxBounceCount.Init(g_pLTClient, "ShellMaxBounceCount", NULL, 6.0);
	}
	if (!g_vtShellMinLifetime.IsInitted())
	{
		g_vtShellMinLifetime.Init(g_pLTClient, "ShellMinLifetime", NULL, 5.0f);
	}
	if (!g_vtShellMaxLifetime.IsInitted())
	{
		g_vtShellMaxLifetime.Init(g_pLTClient, "ShellMaxLifetime", NULL, 7.0f);
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
		g_vtShellMinSpinsPerSecond.Init(g_pLTClient, "ShellMinSpinsPerSecond", NULL, 1.0f);
	}
	if (!g_vtShellMaxSpinsPerSecond.IsInitted())
	{
		g_vtShellMaxSpinsPerSecond.Init(g_pLTClient, "ShellMaxSpinsPerSecond", NULL, 10.0f);
	}

	m_nBounceCount = (int)g_vtShellMaxBounceCount.GetFloat();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CreateObject
//
//	PURPOSE:	Create the model associated with the shell
//
// ----------------------------------------------------------------------- //

bool CShellCasingFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return false;

	// Setup the shell...

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	HRECORD hFireFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFireFX );
	if( !m_hAmmo || !hFireFX )
		return false;

	ObjectCreateStruct createStruct;

	createStruct.SetFileName( g_pFXDB->GetString(hFireFX,FXDB_sShellModel));
	createStruct.SetMaterial(0, g_pFXDB->GetString(hFireFX,FXDB_sShellMaterial));

	m_fInitialScale = g_pFXDB->GetFloat(hFireFX,FXDB_fShellScale);
	m_fFinalScale = (m_fInitialScale * g_vtShellMaxScale.GetFloat());

	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags = m_dwFlags;
	createStruct.m_Pos = m_vStartPos;
    createStruct.m_Rotation = m_rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);
    if (!m_hObject) 
		return false;

	//calculate our object's size
	LTVector vDims;
	g_pLTClient->Physics()->GetObjectDims(m_hObject, &vDims);
	m_fModelDims = LTMAX(vDims.x, vDims.y) * 0.5f * m_fInitialScale;

	m_pClientDE->SetObjectScale(m_hObject, m_fInitialScale);

	//setup the shadow LOD for shell casings
	m_pClientDE->SetObjectShadowLOD(m_hObject, eEngineLOD_High);

	// User camera rotation if not 3rd person ;)
	if (!m_b3rdPerson)
	{
		g_pLTClient->GetObjectRotation( g_pPlayerMgr->GetPlayerCamera()->GetCamera(), &m_rRot );
	}

	LTVector vMinEjectVel = g_pFXDB->GetVector3( hFireFX, FXDB_vShellEjectMinVel );
	LTVector vMaxEjectVel = g_pFXDB->GetVector3( hFireFX, FXDB_vShellEjectMaxVel );

	m_vStartVel += (m_rRot.Up() * GetRandom(vMinEjectVel.y, vMaxEjectVel.y));
	m_vStartVel += (m_rRot.Right() * GetRandom(vMinEjectVel.x, vMaxEjectVel.x));
	m_vStartVel += (m_rRot.Forward() * GetRandom(vMinEjectVel.z, vMaxEjectVel.z));

	InitMovingObject(&m_movingObj, m_vStartPos, m_vStartVel);

	// Make sure we're using the player physics if not in 3rd person...
	if (!m_b3rdPerson)
	{
		m_movingObj.m_dwPhysicsFlags |= MO_PLAYERPHYSICS;
	}

	m_fElapsedTime = 0.0f;
	m_fDieTime = GetRandom(g_vtShellMinLifetime.GetFloat(), g_vtShellMaxLifetime.GetFloat());

	m_fPitchVel = GenerateRandomVelocity();
	m_fYawVel	= GenerateRandomVelocity();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::BounceMovingObject
//
//	PURPOSE:	Bounce a moving object
//
// ----------------------------------------------------------------------- //

bool CShellCasingFX::BounceMovingObject(MovingObject *pObject, LTVector &vNewPos, IntersectInfo* pInfo, bool & bBounceOnGround)
{
	bBounceOnGround = false;

	PhysicsState* pState = GetCurPhysicsState(pObject);
	if (!pState) 
		return false;

	//only test against the main world model
	HOBJECT hMainWorld = g_pLTClient->GetMainWorldModel();

	LTVector vDims;
	g_pLTClient->Physics()->GetObjectDims(m_hObject, &vDims);

	float fHeight = m_fModelDims + 0.1f;

	IntersectQuery query;
	query.m_From		= pObject->m_vPos;
	query.m_To			= LTVector(vNewPos.x, vNewPos.y - fHeight, vNewPos.z);
	query.m_Flags		= INTERSECT_HPOLY;

	//test this segment against the world to see what we bounce against
	if (g_pLTClient->IntersectSegmentAgainst(query, pInfo, hMainWorld))
	{
		// Move the dest point a little in front of the plane.
		vNewPos = pInfo->m_Point + pInfo->m_Plane.m_Normal * fHeight;

		//reflect the velcocity vector over the normal of the plane we intersected
		float fDot = pObject->m_vVelocity.Dot(pInfo->m_Plane.m_Normal);
		pObject->m_vVelocity += (pInfo->m_Plane.m_Normal * fDot * -2.0f);

		//apply the coefficient of restitution for this bounce
		pObject->m_vVelocity *= pState->m_fVelocityDampen;

		float fVelMag = pObject->m_vVelocity.Mag();

		//and add a bit of randomness to the bounce
		LTVector vPerp = pInfo->m_Plane.m_Normal.BuildOrthonormal();
		LTVector vUp = vPerp.Cross(pInfo->m_Plane.m_Normal);

		//the maximum amount of random variation to add per bounce
		static const float kfMaxRandomVelScale = 0.3f;

		pObject->m_vVelocity += (GetRandom(-kfMaxRandomVelScale, kfMaxRandomVelScale) * fVelMag) * vUp;
		pObject->m_vVelocity += (GetRandom(-kfMaxRandomVelScale, kfMaxRandomVelScale) * fVelMag) * vPerp;

		//determine if we are bouncing off of a plane close enough to gravity that we should consider it
		//the ground
		const float kfMinGroundDotValue = 0.707f;		//cos(45 degrees)
		bBounceOnGround = (pInfo->m_Plane.m_Normal.y > kfMinGroundDotValue);

		if (bBounceOnGround)
		{
			//align our orientation to the ground
			LTRotation rRot;
			g_pLTClient->GetObjectRotation(m_hObject, &rRot);

			//project the forward into the plane that we collided on
			LTVector vForward = rRot.Forward();
			vForward -= pInfo->m_Plane.m_Normal * vForward.Dot(pInfo->m_Plane.m_Normal);
			float fForwardMag = vForward.Mag();
			if(fForwardMag < 0.001f)
				vForward = pInfo->m_Plane.m_Normal.BuildOrthonormal();
			else
				vForward /= fForwardMag;

			//and setup a new rotation that will lie on the plane
			LTRotation rNewRot(vForward, rRot.Up());
			g_pLTClient->SetObjectRotation(m_hObject, rNewRot);

			//we hit the ground, see if our velocity is small enough that we want to enter a resting
			//state
			const float kfMinDisableVelocitySqr = LTSqr(50.0f);
			if(pObject->m_vVelocity.MagSqr() < kfMinDisableVelocitySqr)
			{
				pObject->m_dwPhysicsFlags |= MO_RESTING;
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Update
//
//	PURPOSE:	Update the shell
//
// ----------------------------------------------------------------------- //

bool CShellCasingFX::Update()
{
    if (!m_hObject || !m_pClientDE) return false;

	if (g_pGameClientShell->IsServerPaused())
	{
		return true;
	}

	// Make sure we're using the correct frame time...
	float fFrameTime = SimulationTimer::Instance().GetTimerElapsedS();
	if (!m_b3rdPerson)
	{
		fFrameTime = g_pGameClientShell->GetPlayerMgr()->GetPlayerTimerElapsedS();
	}

	m_fElapsedTime += fFrameTime;
	m_fDieTime -= fFrameTime;
	
    if (m_fDieTime <= 0.0f) return false;

	float fScale = 1.0f;
	m_pClientDE->GetObjectScale(m_hObject, &fScale);

	// Update object scale if necessary...
	if (fScale != m_fFinalScale)
	{
		if (m_fElapsedTime <= g_vtShellScaleTime.GetFloat())
		{
			float fScaleRange = (m_fFinalScale - m_fInitialScale);

			float fScale = m_fInitialScale + (fScaleRange * (m_fElapsedTime / g_vtShellScaleTime.GetFloat()));
			m_pClientDE->SetObjectScale(m_hObject, fScale);

			//calculate our object's size
			LTVector vDims;
			g_pLTClient->Physics()->GetObjectDims(m_hObject, &vDims);
			m_fModelDims = LTMAX(vDims.x, vDims.y) * 0.5f * fScale;
		}
		else
		{
			m_pClientDE->SetObjectScale(m_hObject, m_fFinalScale);
		}
	}

    if (m_bResting) return true;

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.

	if (m_movingObj.m_dwPhysicsFlags & MO_RESTING)
	{
        m_bResting = true;
	}
	else
	{
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hObject, &rRot);

		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			rRot.Rotate(rRot.Up(), m_fYawVel * fFrameTime);
			rRot.Rotate(rRot.Right(), m_fPitchVel * fFrameTime);

			rRot.Normalize( );
			g_pLTClient->SetObjectRotation(m_hObject, rRot);
		}
	}


    LTVector vNewPos;
    if (UpdateMovingObject(NULL, &m_movingObj, vNewPos))
	{
		IntersectInfo info;
		bool bBouncedOnGround = false;
        if (BounceMovingObject(&m_movingObj, vNewPos, &info, bBouncedOnGround))
		{
			// If we hit the sky/invisible surface we're done...

			SurfaceType eType = GetSurfaceType(info);
			if (eType == ST_SKY || eType == ST_INVISIBLE)
			{
                return false;
			}

			// Only play the bounce sound on the first bounce...

			if (m_nBounceCount >= (int)g_vtShellMaxBounceCount.GetFloat())
			{
				if (!(m_movingObj.m_dwPhysicsFlags & MO_LIQUID))
				{
					HSURFACE hSurf = g_pSurfaceDB->GetSurface(eType);
					if (hSurf)
					{
						// Play appropriate sound...
						HRECORD hSR = g_pSurfaceDB->GetRecordLink(hSurf,SrfDB_Srf_rShellBounceSnd);
						if (hSR)
						{
							g_pClientSoundMgr->PlayDBSoundFromPos(vNewPos, hSR, SMGR_INVALID_RADIUS, SOUNDPRIORITY_MISC_LOW,
								0, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
								DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS);
						}
					}
				}
			}

			// Adjust the bouncing..

			m_fPitchVel *= 0.5f;
			m_fYawVel	*= -0.5f;

			m_nBounceCount--;

			if (m_nBounceCount <= 0)
			{
				m_movingObj.m_dwPhysicsFlags |= MO_RESTING;
			}
		}

		m_movingObj.m_vPos = vNewPos;

        if (g_pCommonLT->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
            return false;
		}

		g_pLTClient->SetObjectPos(m_hObject, vNewPos);
	}

    return true;
}
