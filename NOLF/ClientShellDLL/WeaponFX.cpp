// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 2/22/98
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "MarkSFX.h"
#include "ParticleShowerFX.h"
#include "DynamicLightFX.h"
#include "BulletTrailFX.h"
#include "MsgIDs.h"
#include "ShellCasingFX.h"
#include "ParticleExplosionFX.h"
#include "BaseScaleFX.h"
#include "DebrisFX.h"
#include "CMoveMgr.h"
#include "RandomSparksFX.h"
#include "iltphysics.h"
#include "iltcustomdraw.h"
#include "MuzzleFlashFX.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "PolyDebrisFX.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;

static uint32 s_nNumShells = 0;

VarTrack	g_cvarShowFirePath;
VarTrack	g_cvarLightBeamColorDelta;
VarTrack	g_cvarImpactPitchShift;
VarTrack	g_cvarFlyByRadius;
VarTrack	g_cvarFlyBySoundRadius;
VarTrack	g_vtBloodSplatsMinNum;
VarTrack	g_vtBloodSplatsMaxNum;
VarTrack	g_vtBloodSplatsMinLifetime;
VarTrack	g_vtBloodSplatsMaxLifetime;
VarTrack	g_vtBloodSplatsMinScale;
VarTrack	g_vtBloodSplatsMaxScale;
VarTrack	g_vtBloodSplatsRange;
VarTrack	g_vtBloodSplatsPerturb;
VarTrack	g_vtBigBloodSizeScale;
VarTrack	g_vtBigBloodLifeScale;
VarTrack	g_vtCreatePolyDebris;
VarTrack	g_vtWeaponFXMinImpactDot;
VarTrack	g_vtWeaponFXMinFireDot;
VarTrack	g_vtWeaponFXUseFOVPerformance;
VarTrack	g_vtWeaponFXMaxFireDist;
VarTrack	g_vtWeaponFXMaxImpactDist;
VarTrack	g_vtWeaponFXMaxMultiImpactDist;
VarTrack	g_vtMultiDing;

LTBOOL		g_bCanSeeImpactPos	= LTTRUE;
LTBOOL		g_bCanSeeFirePos	= LTTRUE;
LTBOOL		g_bDistantFirePos	= LTFALSE;
LTBOOL		g_bDistantImpactPos	= LTFALSE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	WCREATESTRUCT w;

	w.hServerObj	= hServObj;
    w.hFiredFrom    = g_pLTClient->ReadFromMessageObject(hMessage);
    w.nWeaponId     = g_pLTClient->ReadFromMessageByte(hMessage);
    w.nAmmoId       = g_pLTClient->ReadFromMessageByte(hMessage);
    w.nSurfaceType  = g_pLTClient->ReadFromMessageByte(hMessage);
    w.wIgnoreFX     = g_pLTClient->ReadFromMessageWord(hMessage);
    w.nShooterId    = g_pLTClient->ReadFromMessageByte(hMessage);
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vFirePos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vSurfaceNormal));
	// This doesn't always give the correct values...
    //g_pLTClient->ReadFromMessageCompPosition(hMessage, &(w.vFirePos));
    //g_pLTClient->ReadFromMessageCompPosition(hMessage, &(w.vPos));
    //g_pLTClient->ReadFromMessageCompPosition(hMessage, &(w.vSurfaceNormal));

	return Init(&w);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	WCREATESTRUCT* pCS = (WCREATESTRUCT*)psfxCreateStruct;

	m_nWeaponId		= pCS->nWeaponId;
	m_nAmmoId		= pCS->nAmmoId;
	m_eSurfaceType	= (SurfaceType)pCS->nSurfaceType;
	m_wIgnoreFX		= pCS->wIgnoreFX;

    m_hFiredFrom     = pCS->hFiredFrom; // LTNULL
	m_vFirePos		 = pCS->vFirePos;
	m_vPos			 = pCS->vPos;
	m_vSurfaceNormal = pCS->vSurfaceNormal;
	m_vSurfaceNormal.Norm();

	m_eCode			= CC_NO_CONTAINER;
	m_eFirePosCode	= CC_NO_CONTAINER;

	m_pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
    if (!m_pAmmo) return LTFALSE;

	m_pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
    if (!m_pWeapon) return LTFALSE;

    m_fInstDamage   = (LTFLOAT) m_pAmmo->nInstDamage;
    m_fAreaDamage   = (LTFLOAT) m_pAmmo->nAreaDamage;

	m_nShooterId	= pCS->nShooterId;
	m_bLocal		= pCS->bLocal;

	if (!g_cvarShowFirePath.IsInitted())
	{
		g_cvarShowFirePath.Init(g_pLTClient, "ShowFirePath", NULL, -1.0f);
    }

	if (!g_cvarLightBeamColorDelta.IsInitted())
	{
		g_cvarLightBeamColorDelta.Init(g_pLTClient, "LightBeamColorDelta", NULL, 50.0f);
	}

	if (!g_cvarImpactPitchShift.IsInitted())
	{
		g_cvarImpactPitchShift.Init(g_pLTClient, "PitchShiftImpact", NULL, -1.0f);
	}

	if (!g_cvarFlyByRadius.IsInitted())
	{
		g_cvarFlyByRadius.Init(g_pLTClient, "FlyByRadius", NULL, 300.0f);
	}

	if (!g_cvarFlyBySoundRadius.IsInitted())
	{
		g_cvarFlyBySoundRadius.Init(g_pLTClient, "FlyBySoundRadius", NULL, 500.0f);
	}

	if (!g_vtBloodSplatsMinNum.IsInitted())
	{
		g_vtBloodSplatsMinNum.Init(g_pLTClient, "BloodSplatsMinNum", NULL, 3.0f);
	}

	if (!g_vtBloodSplatsMaxNum.IsInitted())
	{
		g_vtBloodSplatsMaxNum.Init(g_pLTClient, "BloodSplatsMaxNum", NULL, 10.0f);
	}

	if (!g_vtBloodSplatsMinLifetime.IsInitted())
	{
		g_vtBloodSplatsMinLifetime.Init(g_pLTClient, "BloodSplatsMinLifetime", NULL, 5.0f);
	}

	if (!g_vtBloodSplatsMaxLifetime.IsInitted())
	{
		g_vtBloodSplatsMaxLifetime.Init(g_pLTClient, "BloodSplatsMaxLifetime", NULL, 10.0f);
	}

	if (!g_vtBloodSplatsMinScale.IsInitted())
	{
		g_vtBloodSplatsMinScale.Init(g_pLTClient, "BloodSplatsMinScale", NULL, 0.01f);
	}

	if (!g_vtBloodSplatsMaxScale.IsInitted())
	{
		g_vtBloodSplatsMaxScale.Init(g_pLTClient, "BloodSplatsMaxScale", NULL, 0.05f);
	}

	if (!g_vtBloodSplatsRange.IsInitted())
	{
		g_vtBloodSplatsRange.Init(g_pLTClient, "BloodSplatsRange", NULL, 500.0f);
	}

	if (!g_vtBloodSplatsPerturb.IsInitted())
	{
		g_vtBloodSplatsPerturb.Init(g_pLTClient, "BloodSplatsPerturb", NULL, 100.0f);
	}

	if (!g_vtBigBloodSizeScale.IsInitted())
	{
		g_vtBigBloodSizeScale.Init(g_pLTClient, "BigBloodSizeScale", NULL, 5.0f);
	}

	if (!g_vtBigBloodLifeScale.IsInitted())
	{
		g_vtBigBloodLifeScale.Init(g_pLTClient, "BigBloodLifeScale", NULL, 3.0f);
	}

	if (!g_vtCreatePolyDebris.IsInitted())
	{
		g_vtCreatePolyDebris.Init(g_pLTClient, "CreatePolyDebris", NULL, 1.0f);
	}

	if (!g_vtWeaponFXMinFireDot.IsInitted())
	{
		g_vtWeaponFXMinFireDot.Init(g_pLTClient, "WeaponFXMinFireDot", NULL, 0.6f);
	}

	if (!g_vtWeaponFXMinImpactDot.IsInitted())
	{
		g_vtWeaponFXMinImpactDot.Init(g_pLTClient, "WeaponFXMinImpactDot", NULL, 0.6f);
	}

	if (!g_vtWeaponFXUseFOVPerformance.IsInitted())
	{
		g_vtWeaponFXUseFOVPerformance.Init(g_pLTClient, "WeaponFXUseFOVPerformance", NULL, 1.0f);
	}

	if (!g_vtWeaponFXMaxFireDist.IsInitted())
	{
		g_vtWeaponFXMaxFireDist.Init(g_pLTClient, "WeaponFXMaxFireDist", NULL, 1000.0f);
	}

	if (!g_vtWeaponFXMaxImpactDist.IsInitted())
	{
		g_vtWeaponFXMaxImpactDist.Init(g_pLTClient, "WeaponFXMaxImpactDist", NULL, 1000.0f);
	}

	if (!g_vtWeaponFXMaxMultiImpactDist.IsInitted())
	{
		g_vtWeaponFXMaxMultiImpactDist.Init(g_pLTClient, "WeaponFXMaxMultiImpactDist", NULL, 300.0f);
	}

	if (!g_vtMultiDing.IsInitted())
	{
		g_vtMultiDing.Init(g_pLTClient, "WeaponFXMultiImpactDing", NULL, 1.0f);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pWeaponMgr) return LTFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTFALSE;

	// Set up our data members...

	// Set the local client id...

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);
    m_nLocalId = (uint8)dwId;


	m_nDetailLevel = pSettings->SpecialFXSetting();

	// Fire pos may get tweaked a little...

	m_vFirePos = CalcFirePos(m_vFirePos);

	m_vDir = m_vPos - m_vFirePos;
	m_fFireDistance = m_vDir.Mag();
	m_vDir.Norm();

    g_pLTClient->AlignRotation(&m_rSurfaceRot, &m_vSurfaceNormal, LTNULL);
    g_pLTClient->AlignRotation(&m_rDirRot, &m_vDir, LTNULL);

	SetupExitInfo();



	// Calculate if the camera can see the fire position and the impact
	// position...

	g_bCanSeeImpactPos	= LTTRUE;
	g_bCanSeeFirePos	= LTTRUE;
	g_bDistantImpactPos	= LTFALSE;
	g_bDistantFirePos	= LTFALSE;

	if (g_vtWeaponFXUseFOVPerformance.GetFloat())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCamera();
		LTVector vCameraPos, vU, vR, vF, vDir;
		LTRotation rCameraRot;
		g_pLTClient->GetObjectPos(hCamera, &vCameraPos);
		g_pLTClient->GetObjectRotation(hCamera, &rCameraRot);
		g_pLTClient->GetRotationVectors(&rCameraRot, &vU, &vR, &vF);

		vDir = m_vPos - vCameraPos;
		LTFLOAT fImpactDist = vDir.Mag();

		if (fImpactDist > g_vtWeaponFXMaxImpactDist.GetFloat())
		{
			g_bDistantImpactPos = LTTRUE;
		}

		vDir.Norm();

		LTFLOAT fMul = VEC_DOT(vDir, vF);
		g_bCanSeeImpactPos = (fMul < g_vtWeaponFXMinImpactDot.GetFloat() ? LTFALSE : LTTRUE);

		// In multiplayer we need to account for impacts that occur around
		// our camera that we didn't cause (this is also an issue in single
		// player, but due to the singler player gameplay dynamics it isn't
		// as noticeable)...

		if (!g_bCanSeeImpactPos && IsMultiplayerGame())
		{
			// Somebody else shot this...if the impact is close enough, we 
			// "saw" it...
			if (m_nLocalId != m_nShooterId && fImpactDist <= g_vtWeaponFXMaxMultiImpactDist.GetFloat())
			{
				g_bCanSeeImpactPos = LTTRUE;
			}
		}

		vDir = m_vFirePos - vCameraPos;

		if (vDir.Mag() > g_vtWeaponFXMaxFireDist.GetFloat())
		{
			g_bDistantFirePos = LTTRUE;
		}

		vDir.Norm();

		fMul = VEC_DOT(vDir, vF);
		g_bCanSeeFirePos = (fMul < g_vtWeaponFXMinFireDot.GetFloat() ? LTFALSE : LTTRUE);
	}



	// Determine what container the sfx is in...

	HLOCALOBJ objList[1];
    LTVector vTestPos = m_vPos + m_vSurfaceNormal;  // Test a little closer...
    uint32 dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				m_eCode = (ContainerCode)dwCode;
			}
		}
	}

	// Determine if the fire point is in liquid

	vTestPos = m_vFirePos + m_vDir;  // Test a little further in...
    dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				m_eFirePosCode = (ContainerCode)dwCode;
			}
		}
	}


	if (IsLiquid(m_eCode))
	{
		m_wImpactFX	= m_pAmmo->pUWImpactFX ? m_pAmmo->pUWImpactFX->nFlags : 0;
	}
	else
	{
		m_wImpactFX	= m_pAmmo->pImpactFX ? m_pAmmo->pImpactFX->nFlags : 0;
	}

	m_wFireFX = m_pAmmo->pFireFX ? m_pAmmo->pFireFX->nFlags : 0;

	// Assume alt-fire, silenced, and tracer...these will be cleared by
	// IgnoreFX if not used...

	m_wFireFX |= WFX_ALTFIRESND | WFX_SILENCED | WFX_TRACER;

	// Assume impact ding, it will be cleared if not used...

	m_wImpactFX |= WFX_IMPACTDING;

	// Clear all the fire fx we want to ignore...

	m_wFireFX &= ~m_wIgnoreFX;
	m_wImpactFX &= ~m_wIgnoreFX;


	// See if this is a redundant weapon fx (i.e., this client shot the
	// weapon so they've already seen this fx)...

	if (g_pGameClientShell->IsMultiplayerGame())
	{
		if (m_pAmmo->eType != PROJECTILE)
		{
			if (!m_bLocal && m_nLocalId >= 0 && m_nLocalId == m_nShooterId)
			{
				if (m_wImpactFX & WFX_IMPACTDING)
				{
					if (g_vtMultiDing.GetFloat())
					{
						PlayImpactDing();
					}
				}

                return LTFALSE;
			}
		}
	}


	// Show the fire path...(debugging...)

	if (g_cvarShowFirePath.GetFloat() > 0)
	{
		PLFXCREATESTRUCT pls;

		pls.vStartPos			= m_vFirePos;
		pls.vEndPos				= m_vPos;
        pls.vInnerColorStart    = LTVector(GetRandom(127.0f, 255.0f), GetRandom(127.0f, 255.0f), GetRandom(127.0f, 255.0f));
		pls.vInnerColorEnd		= pls.vInnerColorStart;
        pls.vOuterColorStart    = LTVector(0, 0, 0);
        pls.vOuterColorEnd      = LTVector(0, 0, 0);
		pls.fAlphaStart			= 1.0f;
		pls.fAlphaEnd			= 1.0f;
		pls.fMinWidth			= 0;
		pls.fMaxWidth			= 10;
		pls.fMinDistMult		= 1.0f;
		pls.fMaxDistMult		= 1.0f;
		pls.fLifeTime			= 10.0f;
		pls.fAlphaLifeTime		= 10.0f;
		pls.fPerturb			= 0.0f;
        pls.bAdditive           = LTFALSE;
		pls.nWidthStyle			= PLWS_CONSTANT;
		pls.nNumSegments		= 2;

		CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYLINE_ID, &pls);
		if (pFX) pFX->Update();
	}


	// If the surface is the sky, don't create any impact related fx...

	if (m_eSurfaceType != ST_SKY || (m_wImpactFX & WFX_IMPACTONSKY))
	{
		CreateWeaponSpecificFX();

		if (g_bCanSeeImpactPos)
		{
			if ((m_wImpactFX & WFX_MARK) && ShowsMark(m_eSurfaceType) && (LTBOOL)GetConsoleInt("MarkShow", 1))
			{
				LTBOOL bCreateMark = LTTRUE;
				if (g_bDistantImpactPos && m_nLocalId == m_nShooterId)
				{
					// Assume we'll see the mark if we're zoomed in ;)
					bCreateMark = g_pGameClientShell->IsZoomed();
				}

				if (bCreateMark)
				{
					CreateMark(m_vPos, m_vSurfaceNormal, m_rSurfaceRot, m_eSurfaceType);
				}
			}

			CreateSurfaceSpecificFX();
		}

		PlayImpactSound();
	}


	if (IsBulletTrailWeapon())
	{
		if (IsLiquid(m_eFirePosCode))
		{
			if (m_nDetailLevel != RS_LOW)
			{
				CreateBulletTrail(&m_vFirePos);
			}
		}
	}


	// No tracers under water...

	if ((LTBOOL)GetConsoleInt("Tracers", 1) && (m_wFireFX & WFX_TRACER) && !IsLiquid(m_eCode))
	{
		CreateTracer();
	}

	if (g_bCanSeeFirePos)
	{
		// Only do muzzle fx for the client (not for AIs)...

		if ((m_wFireFX & WFX_MUZZLE) && (m_nLocalId == m_nShooterId))
		{
			CreateMuzzleFX();
		}

		if (!g_bDistantFirePos &&
			(LTBOOL)GetConsoleInt("ShellCasings", 1) &&
			(m_wFireFX & WFX_SHELL))
		{
			CreateShell();
		}

		if ((m_wFireFX & WFX_LIGHT))
		{
			CreateMuzzleLight();
		}
	}

	if ((m_wFireFX & WFX_FIRESOUND) || (m_wFireFX & WFX_ALTFIRESND) || (m_wFireFX & WFX_SILENCED))
	{
		PlayFireSound();
	}

	// Only do fly-by sounds for weapons that leave bullet trails...

	if (IsBulletTrailWeapon())
	{
		PlayBulletFlyBySound();
	}


    return LTFALSE;  // Just delete me, I'm done :)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::SetupExitInfo
//
//	PURPOSE:	Setup our exit info
//
// ----------------------------------------------------------------------- //

void CWeaponFX::SetupExitInfo()
{
	m_eExitSurface	= ST_UNKNOWN;
	m_vExitPos		= m_vFirePos;
	m_vExitNormal	= m_vDir;
	m_eExitCode		= CC_NO_CONTAINER;

	if (m_nDetailLevel == RS_LOW) return;

	// Determine if there is an "exit" surface...

	IntersectQuery qInfo;
	IntersectInfo iInfo;

	qInfo.m_From = m_vFirePos + m_vDir;
	qInfo.m_To   = m_vFirePos - m_vDir;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		m_eExitSurface	= GetSurfaceType(iInfo);
		m_vExitNormal	= iInfo.m_Plane.m_Normal;
		m_vExitPos		= iInfo.m_Point + m_vDir;

		// Determine what container the sfx is in...

		HLOCALOBJ objList[1];
        LTVector vTestPos = m_vExitPos + m_vExitNormal;  // Test a little closer...
        uint32 dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

		if (dwNum > 0 && objList[0])
		{
            uint32 dwUserFlags;
            g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

			if (dwUserFlags & USRFLG_VISIBLE)
			{
                uint16 dwCode;
                if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
				{
					m_eExitCode = (ContainerCode)dwCode;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateExitDebris
//
//	PURPOSE:	Create any exit debris
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExitDebris()
{
	int i;

	// Create the surface specific exit fx...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eExitSurface);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	if (IsLiquid(m_eExitCode))
	{
		// Create underwater fx...

		// Create any exit particle shower fx associated with this surface...

		for (i=0; i < pSurf->nNumUWExitPShowerFX; i++)
		{
			CPShowerFX* pPShowerFX = g_pSurfaceMgr->GetPShowerFX(pSurf->aUWExitPShowerFXIds[i]);
			g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vExitPos, m_vExitNormal, m_vSurfaceNormal);
		}
	}
	else
	{
		// Create normal fx...

		// Create any exit scale fx associated with this surface...

		for (i=0; i < pSurf->nNumExitScaleFX; i++)
		{
			CScaleFX* pScaleFX = g_pSurfaceMgr->GetScaleFX(pSurf->aExitScaleFXIds[i]);
			g_pFXButeMgr->CreateScaleFX(pScaleFX, m_vExitPos, m_vExitNormal, &m_vExitNormal, &m_rSurfaceRot);
		}

		// Create any exit particle shower fx associated with this surface...

		for (i=0; i < pSurf->nNumExitPShowerFX; i++)
		{
			CPShowerFX* pPShowerFX = g_pSurfaceMgr->GetPShowerFX(pSurf->aExitPShowerFXIds[i]);
			g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vExitPos, m_vExitNormal, m_vSurfaceNormal);
		}

		// Create any exit poly debris fx associated with this surface...

		for (i=0; i < pSurf->nNumExitPolyDebrisFX; i++)
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pSurfaceMgr->GetPolyDebrisFX(pSurf->aExitPolyDebrisFXIds[i]);
			g_pFXButeMgr->CreatePolyDebrisFX(pPolyDebrisFX, m_vExitPos, m_vExitNormal, m_vExitNormal);
		}
	}


	// Determine if we should create a beam of light through the surface...

	CreateLightBeamFX(pSurf);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLightBeamFX
//
//	PURPOSE:	Create a light beam (if appropriate)
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLightBeamFX(SURFACE* pSurf)
{
	if (!pSurf) return;

    LTVector vEnterColor, vExitColor;
    if (g_pLTClient->GetPointShade(&m_vExitPos, &vExitColor) == LT_OK)
	{
		// Get the EnterColor value...

        LTVector vEnterPos = m_vExitPos - (m_vExitNormal * float(pSurf->nMaxShootThroughThickness + 1));

        if (g_pLTClient->GetPointShade(&vEnterPos, &vEnterColor) == LT_OK)
		{
			// Calculate the difference in light value...

            LTFLOAT fMaxEnter = Max(vEnterColor.x, vEnterColor.y);
			fMaxEnter = Max(fMaxEnter, vEnterColor.z);

            LTFLOAT fMaxExit = Max(vExitColor.x, vExitColor.y);
			fMaxExit = Max(fMaxExit, vExitColor.z);

			if (fabs((double)(fMaxExit - fMaxEnter)) >= g_cvarLightBeamColorDelta.GetFloat())
			{
                LTVector vStartPoint, vDir;
				if (fMaxEnter > fMaxExit)
				{
					vStartPoint = m_vExitPos;
					vDir = m_vDir;
				}
				else
				{
					vStartPoint = vEnterPos;
					vDir = -m_vDir;
				}

				PLFXCREATESTRUCT pls;

				pls.vStartPos			= vStartPoint;
				pls.vEndPos				= vStartPoint + (vDir * GetRandom(100.0, 150.0f));
                pls.vInnerColorStart    = LTVector(230, 230, 230);
				pls.vInnerColorEnd		= pls.vInnerColorStart;
                pls.vOuterColorStart    = LTVector(0, 0, 0);
                pls.vOuterColorEnd      = LTVector(0, 0, 0);
				pls.fAlphaStart			= 0.5f;
				pls.fAlphaEnd			= 0.0f;
				pls.fMinWidth			= 0;
				pls.fMaxWidth			= 10;
				pls.fMinDistMult		= 1.0f;
				pls.fMaxDistMult		= 1.0f;
				pls.fLifeTime			= 10.0f;
				pls.fAlphaLifeTime		= 10.0f;
				pls.fPerturb			= 0.0f;
                pls.bAdditive           = LTFALSE;
				pls.nWidthStyle			= PLWS_CONSTANT;
				pls.nNumSegments		= 1;

				CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYLINE_ID, &pls);
				if (pFX) pFX->Update();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateExitMark
//
//	PURPOSE:	Create any exit surface marks
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExitMark()
{
	if (m_eExitSurface != ST_UNKNOWN && ShowsMark(m_eExitSurface))
	{
        LTRotation rNormRot;
        g_pLTClient->AlignRotation(&rNormRot, &m_vExitNormal, LTNULL);

		CreateMark(m_vExitPos, m_vExitNormal, rNormRot, m_eExitSurface);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMark
//
//	PURPOSE:	Create a mark fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMark(LTVector vPos, LTVector vNorm, LTRotation rRot,
						   SurfaceType eType)
{
	IMPACTFX* pImpactFX = m_pAmmo->pImpactFX;

	if (IsLiquid(m_eCode))
	{
		pImpactFX = m_pAmmo->pUWImpactFX;
	}

	if (!pImpactFX) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	MARKCREATESTRUCT mark;

	mark.m_vPos = vPos;
	mark.m_Rotation = rRot;

	// Randomly rotate the bullet hole...

    g_pLTClient->RotateAroundAxis(&mark.m_Rotation, &vNorm, GetRandom(0.0f, MATH_CIRCLE));

	mark.m_fScale		= pImpactFX->fMarkScale;
	mark.nAmmoId		= m_nAmmoId;
	mark.nSurfaceType   = eType;

	psfxMgr->CreateSFX(SFX_MARK_ID, &mark);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBulletTrail
//
//	PURPOSE:	Create a bullet trail fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBulletTrail(LTVector *pvStartPos)
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr || !pvStartPos) return;

    LTVector vColor1, vColor2;
	vColor1.Init(200.0f, 200.0f, 200.0f);
	vColor2.Init(255.0f, 255.0f, 255.0f);

	BTCREATESTRUCT bt;

	bt.vStartPos		= *pvStartPos;
	bt.vDir				= m_vDir;
	bt.vColor1			= vColor1;
	bt.vColor2			= vColor2;
	bt.fLifeTime		= 0.5f;
	bt.fFadeTime		= 0.3f;
	bt.fRadius			= 400.0f;
	bt.fGravity			= 0.0f;
	bt.fNumParticles	= (m_nDetailLevel == RS_MED) ? 15.0f : 30.0f;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_BULLETTRAIL_ID, &bt);

	// Let each bullet trail do its initial update...

	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateTracer
//
//	PURPOSE:	Create a tracer fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateTracer()
{
	if (!m_pAmmo || !m_pAmmo->pTracerFX) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_nDetailLevel != RS_HIGH && GetRandom(1, 2) == 1) return;

	// Create tracer...

	if (m_fFireDistance > 100.0f)
	{
		TRCREATESTRUCT tracer;

		// Make tracer start in front of gun a little ways...

		tracer.vStartPos	= m_vFirePos; // + (m_vDir * 25.0f);
		tracer.vEndPos		= m_vPos;
		tracer.pTracerFX	= m_pAmmo->pTracerFX;

		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_TRACER_ID, &tracer);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateWeaponSpecificFX()
//
//	PURPOSE:	Create weapon specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateWeaponSpecificFX()
{
	// Do fire fx beam fx...

	if (m_pAmmo->pFireFX && m_pAmmo->pFireFX->nNumBeamFX > 0)
	{
		for (int i=0; i < m_pAmmo->pFireFX->nNumBeamFX; i++)
		{
			g_pFXButeMgr->CreateBeamFX(m_pAmmo->pFireFX->pBeamFX[i],
				m_vFirePos, m_vPos);
		}
	}

	// Only do impact fx if the client can see the impact position
	// or the impact fx may last a little while...

	if (g_bCanSeeImpactPos || m_pAmmo->fProgDamage > 0.0f || m_pAmmo->nAreaDamage > 0)
	{
		if (IsLiquid(m_eCode))
		{
			// Create underwater weapon fx...

			IFXCS cs;
			cs.eCode		= m_eCode;
			cs.eSurfType	= m_eSurfaceType;
			cs.rSurfRot		= m_rSurfaceRot;
			cs.vDir			= m_vDir;
			cs.vPos			= m_vPos;
			cs.vSurfNormal	= m_vSurfaceNormal;
			cs.fBlastRadius = (LTFLOAT) m_pAmmo->nAreaDamageRadius;
			cs.fTintRange   = (LTFLOAT) (m_pAmmo->nAreaDamageRadius * 5);

			g_pFXButeMgr->CreateImpactFX(m_pAmmo->pUWImpactFX, cs);
		}
		else
		{
			IFXCS cs;
			cs.eCode		= m_eCode;
			cs.eSurfType	= m_eSurfaceType;
			cs.rSurfRot		= m_rSurfaceRot;
			cs.vDir			= m_vDir;
			cs.vPos			= m_vPos;
			cs.vSurfNormal	= m_vSurfaceNormal;
			cs.fBlastRadius = (LTFLOAT) m_pAmmo->nAreaDamageRadius;
			cs.fTintRange   = (LTFLOAT) (m_pAmmo->nAreaDamageRadius * 5);

			g_pFXButeMgr->CreateImpactFX(m_pAmmo->pImpactFX, cs);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificFX()
//
//	PURPOSE:	Create surface specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSurfaceSpecificFX()
{
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	// Don't do gore fx...

	if (m_eSurfaceType == ST_FLESH)
	{
		if (!pSettings->Gore())
		{
			return;
		}

		if (m_pAmmo->eType == VECTOR && m_pAmmo->eInstDamageType == DT_BULLET)
		{
			CreateBloodSplatFX();
		}
	}

	if ((m_wFireFX & WFX_EXITMARK) && ShowsMark(m_eExitSurface))
	{
		CreateExitMark();
	}

	if (m_wFireFX & WFX_EXITDEBRIS)
	{
		CreateExitDebris();
	}

	if (!m_pAmmo || !m_pAmmo->pImpactFX) return;
	if (!m_pAmmo->pImpactFX->bDoSurfaceFX) return;


	// Create the surface specific fx...

	int i;
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eSurfaceType);
	if (pSurf)
	{
		if (IsLiquid(m_eCode))
		{
			// Create underwater fx...

			// Create any impact particle shower fx associated with this surface...

			for (i=0; i < pSurf->nNumUWImpactPShowerFX; i++)
			{
				CPShowerFX* pPShowerFX = g_pSurfaceMgr->GetPShowerFX(pSurf->aUWImpactPShowerFXIds[i]);
				g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vPos, m_vDir, m_vSurfaceNormal);
			}
		}
		else
		{
			// Create normal fx...

			// Create any impact scale fx associated with this surface...

			for (i=0; i < pSurf->nNumImpactScaleFX; i++)
			{
				CScaleFX* pScaleFX = g_pSurfaceMgr->GetScaleFX(pSurf->aImpactScaleFXIds[i]);
				g_pFXButeMgr->CreateScaleFX(pScaleFX, m_vPos, m_vDir, &m_vSurfaceNormal, &m_rSurfaceRot);
			}

			// Create any impact particle shower fx associated with this surface...

			for (i=0; i < pSurf->nNumImpactPShowerFX; i++)
			{
				CPShowerFX* pPShowerFX = g_pSurfaceMgr->GetPShowerFX(pSurf->aImpactPShowerFXIds[i]);
				g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vPos, m_vDir, m_vSurfaceNormal);
			}

			// Create any impact poly debris fx associated with this surface...

			if (g_vtCreatePolyDebris.GetFloat())
			{
				for (i=0; i < pSurf->nNumImpactPolyDebrisFX; i++)
				{
					CPolyDebrisFX* pPolyDebrisFX = g_pSurfaceMgr->GetPolyDebrisFX(pSurf->aImpactPolyDebrisFXIds[i]);
					g_pFXButeMgr->CreatePolyDebrisFX(pPolyDebrisFX, m_vPos, m_vDir, m_vSurfaceNormal);
				}
			}
		}
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleLight()
//
//	PURPOSE:	Create a muzzle light associated with this fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleLight()
{
	// Check to see if we have the silencer...

	if (m_wFireFX & WFX_SILENCED) return;

	if (m_nLocalId != m_nShooterId || !g_pGameClientShell->IsFirstPerson())
	{
		MUZZLEFLASHCREATESTRUCT mf;

		mf.pWeapon	= m_pWeapon;
		mf.hParent	= m_hFiredFrom;
		mf.vPos		= m_vFirePos;
		mf.rRot		= m_rDirRot;

		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		psfxMgr->CreateSFX(SFX_MUZZLEFLASH_ID, &mf);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::IsBulletTrailWeapon()
//
//	PURPOSE:	See if this weapon creates bullet trails in liquid
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::IsBulletTrailWeapon()
{
	return (m_pAmmo->eInstDamageType == DT_BULLET);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayImpactSound()
//
//	PURPOSE:	Play a surface impact sound if appropriate
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayImpactSound()
{
	IMPACTFX* pImpactFX = m_pAmmo->pImpactFX;

	if (IsLiquid(m_eCode))
	{
		pImpactFX = m_pAmmo->pUWImpactFX;
	}

	if (!pImpactFX) return;


	if (m_pAmmo->eType == VECTOR)
	{
		if ((m_nDetailLevel == RS_LOW) && GetRandom(1, 2) != 1) return;
		else if ((m_nDetailLevel == RS_MED) && GetRandom(1, 3) == 1) return;
	}

	char* pSnd = GetImpactSound(m_eSurfaceType, m_nAmmoId);
    LTFLOAT fSndRadius = (LTFLOAT) pImpactFX->nSoundRadius;

	if (pSnd)
	{
		uint32 dwFlags = 0;
		float fPitchShift = 1.0f;
		if (g_cvarImpactPitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
		}

        uint8 nVolume = IsLiquid(m_eCode) ? 50 : 100;
		g_pClientSoundMgr->PlaySoundFromPos(m_vPos, pSnd, fSndRadius,
			SOUNDPRIORITY_MISC_LOW, dwFlags, nVolume, fPitchShift);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleFX()
//
//	PURPOSE:	Create muzzle specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleFX()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;


	char* pTexture = "SFX\\Impact\\Spr\\Smoke.spr";

	if (IsLiquid(m_eFirePosCode))
	{
		pTexture = DEFAULT_BUBBLE_TEXTURE;
	}

	PARTICLESHOWERCREATESTRUCT sp;

	sp.vPos				= m_vFirePos;
	sp.vDir				= m_vSurfaceNormal * 10.0f;
	sp.pTexture			= pTexture;
	sp.nParticles		= 1;
	sp.fRadius			= 400.0f;
	sp.fDuration		= 1.0f;
	sp.fEmissionRadius	= 0.05f;
	sp.fRadius			= 800.0f;
	sp.fGravity			= 0.0f;

	sp.vColor1.Init(100.0f, 100.0f, 100.0f);
	sp.vColor2.Init(125.0f, 125.0f, 125.0f);

	if (IsLiquid(m_eFirePosCode))
	{
		GetLiquidColorRange(m_eFirePosCode, &sp.vColor1, &sp.vColor2);
		sp.fRadius		= 600.0f;
		sp.fGravity		= 50.0f;
	}

	psfxMgr->CreateSFX(SFX_PARTICLESHOWER_ID, &sp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShell()
//
//	PURPOSE:	Create shell casing
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShell()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	SHELLCREATESTRUCT sc;
	sc.rRot		 = m_rDirRot;
	sc.vStartPos = m_vFirePos;
	sc.nWeaponId = m_nWeaponId;
	sc.nAmmoId	 = m_nAmmoId;

    sc.b3rdPerson = LTFALSE;

	// See if this is our local client who fired, and if so are we in 3rd person...

	if (m_nLocalId == m_nShooterId)
	{
		sc.b3rdPerson = !g_pGameClientShell->IsFirstPerson();
	}
	else
	{
        sc.b3rdPerson = LTTRUE;
	}


	// Adjust the shell position based on the hand-held breach offset...

	if (sc.b3rdPerson)
	{
		sc.vStartPos += (m_vDir * m_pWeapon->fHHBreachOffset);
	}
	else  // Get the shell eject pos...
	{
		sc.vStartPos = g_pGameClientShell->GetWeaponModel()->GetShellEjectPos(sc.vStartPos);

		// Add on the player's velocity...

		HOBJECT hObj = g_pGameClientShell->GetMoveMgr()->GetObject();
		if (hObj)
		{
            g_pLTClient->Physics()->GetVelocity(hObj, &sc.vStartVel);
		}
		//sc.dwFlags = FLAG_REALLYCLOSE;
	}


	// Only create every other shell if medium detail set...
	// if (m_nDetailLevel == RS_MED && (++s_nNumShells % 2 == 0)) return;

	psfxMgr->CreateSFX(SFX_SHELLCASING_ID, &sc);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBloodSplatFX
//
//	PURPOSE:	Create the blood splats, etc.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBloodSplatFX()
{
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings || !pSettings->Gore()) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX = LTNULL;

	LTFLOAT fRange = g_vtBloodSplatsRange.GetFloat();

	// See if we should make some blood splats...

	ClientIntersectQuery iQuery;
	IntersectInfo iInfo;

	iQuery.m_From = m_vPos;

	LTVector vDir = m_vDir;

	// Create some blood splats...

	int nNumSplats = GetRandom((int)g_vtBloodSplatsMinNum.GetFloat(), (int)g_vtBloodSplatsMaxNum.GetFloat());

	LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&m_rDirRot, &vU, &vR, &vF);

	for (int i=0; i < nNumSplats; i++)
	{
		LTVector vDir = m_vDir;

		// Perturb direction after first splat...

		if (i > 0)
		{
			float fPerturb = g_vtBloodSplatsPerturb.GetFloat();

			float fRPerturb = (GetRandom(-fPerturb, fPerturb))/1000.0f;
			float fUPerturb = (GetRandom(-fPerturb, fPerturb))/1000.0f;

			vDir += (vR * fRPerturb);
			vDir += (vU * fUPerturb);
		}

		iQuery.m_To = vDir * fRange;
		iQuery.m_To += m_vPos;
		iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_HPOLY;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo) && IsMainWorld(iInfo.m_hObject))
		{
			SurfaceType eType = GetSurfaceType(iInfo);
			if (eType == ST_SKY || eType == ST_INVISIBLE)
			{
				return; // Don't leave blood on the sky
			}


			LTBOOL bBigBlood = (LTBOOL)GetConsoleInt("BigBlood", 0);

			// Create a blood splat...

			BSCREATESTRUCT sc;

			g_pLTClient->AlignRotation(&(sc.rRot), &(iInfo.m_Plane.m_Normal), LTNULL);

			// Randomly rotate the blood splat

			g_pLTClient->RotateAroundAxis(&(sc.rRot), &(iInfo.m_Plane.m_Normal), GetRandom(0.0f, MATH_CIRCLE));

			LTVector vTemp = vDir * -2.0f;
			sc.vPos = iInfo.m_Point + vTemp;  // Off the wall a bit
			sc.vVel.Init(0.0f, 0.0f, 0.0f);

			sc.vInitialScale.Init(1.0f, 1.0f, 1.0f);
			sc.vInitialScale.x	= GetRandom(g_vtBloodSplatsMinScale.GetFloat(), g_vtBloodSplatsMaxScale.GetFloat());

			if (bBigBlood) sc.vInitialScale.x *= g_vtBigBloodSizeScale.GetFloat();

			sc.vInitialScale.y	= sc.vInitialScale.x;
			sc.vFinalScale		= sc.vInitialScale;

			sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
			sc.fLifeTime		= GetRandom(g_vtBloodSplatsMinLifetime.GetFloat(), g_vtBloodSplatsMaxLifetime.GetFloat());

			if (bBigBlood) sc.fLifeTime *= g_vtBigBloodLifeScale.GetFloat();

			sc.fInitialAlpha	= 1.0f;
			sc.fFinalAlpha		= 0.0f;
			sc.nType			= OT_SPRITE;
			sc.bMultiply		= LTTRUE;

			char* pBloodFiles[] =
			{
				"Sfx\\Test\\Spr\\BloodL1.spr",
				"Sfx\\Test\\Spr\\BloodL2.spr",
				"Sfx\\Test\\Spr\\BloodL3.spr",
				"Sfx\\Test\\Spr\\BloodL4.spr"
			};

			sc.pFilename = pBloodFiles[GetRandom(0,3)];

			pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
			if (pFX) pFX->Update();
		}
		else if (i==0)
		{
			// Didn't hit anything straight back, do don't bother to
			// do anymore...

			return;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayFireSound
//
//	PURPOSE:	Play the fire sound
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayFireSound()
{
	if (m_nLocalId >= 0 && m_nLocalId == m_nShooterId)
	{
		return;  // This client already heard the sound ;)
	}

	PlayerSoundId eSoundId = PSI_FIRE;

	if (m_wFireFX & WFX_SILENCED)
	{
		eSoundId = PSI_SILENCED_FIRE;
	}
	else if (m_wFireFX & WFX_ALTFIRESND)
	{
		eSoundId = PSI_ALT_FIRE;
	}

	::PlayWeaponSound(m_pWeapon, m_vFirePos, eSoundId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CalcFirePos
//
//	PURPOSE:	Calculate the fire position based on the FireFrom object
//
// ----------------------------------------------------------------------- //

LTVector CWeaponFX::CalcFirePos(LTVector vFirePos)
{
	if (!m_hFiredFrom) return vFirePos;

	// See if this is our local client who fired, and if so
	// only calculate fire position if we are in 3rd person...

	if (m_nLocalId == m_nShooterId)
	{
		if (g_pGameClientShell->IsFirstPerson()) return vFirePos;
	}

    LTVector vPos;
    LTRotation rRot;
	if (!GetAttachmentSocketTransform(m_hFiredFrom, "Flash", vPos, rRot))
	{
		vPos = vFirePos;
	}

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayBulletFlyBySound()
//
//	PURPOSE:	Play bullet fly by sound (if appropriate)
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayBulletFlyBySound()
{
	if (!m_pWeapon || !m_pAmmo) return;

	if (m_pAmmo->eType != VECTOR) return;

	// Camera pos

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
    LTVector vPos;
    g_pLTClient->GetObjectPos(hCamera, &vPos);

	// We only play the flyby sound if we won't hear an impact or
	// fire sound...

	LTVector vDist = m_vFirePos - vPos;
	if (vDist.Mag() < m_pWeapon->nFireSoundRadius) return;

	if (m_pAmmo->pImpactFX)
	{
		vDist = m_vPos - vPos;
		if (vDist.Mag() < m_pAmmo->pImpactFX->nSoundRadius) return;
	}


	// See if the camera is close enough to the bullet path to hear the
	// bullet...

	LTFLOAT fRadius = g_cvarFlyByRadius.GetFloat();

	LTVector vDir = m_vDir;

	const LTVector vRelativePos = vPos - m_vFirePos;
    const LTFLOAT fRayDist = vDir.Dot(vRelativePos);
	LTVector vBulletDir = (vDir*fRayDist - vRelativePos);

    const LTFLOAT fDistSqr = vBulletDir.MagSqr();

	if (fDistSqr < fRadius*fRadius)
	{
		vPos += vBulletDir;
		g_pClientSoundMgr->PlaySoundFromPos(vPos, "Guns\\Snd\\flyby.wav",
			g_cvarFlyBySoundRadius.GetFloat(), SOUNDPRIORITY_MISC_LOW);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayImpactDing()
//
//	PURPOSE:	Play a impact ding sound if appropriate
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayImpactDing()
{
	if (!IsMultiplayerGame()) return;

	CCharacterFX* pCharFX = g_pGameClientShell->GetMoveMgr()->GetCharacterFX();
	if (pCharFX)
	{
		pCharFX->PlayDingSound();
	}
}