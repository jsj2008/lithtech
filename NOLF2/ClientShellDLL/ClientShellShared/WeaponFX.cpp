// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 2/22/98
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "RandomSparksFX.h"
#include "iltphysics.h"
#include "MuzzleFlashFX.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "PolyDebrisFX.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "ClientWeaponBase.h"
#include "ClientMultiplayerMgr.h"

static uint32 s_nNumShells = 0;

VarTrack	g_cvarShowFirePath;
VarTrack	g_cvarLightBeamColorDelta;
VarTrack	g_cvarImpactPitchShift;
VarTrack	g_cvarFlyByRadius;
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

//Function to handle filtering of the intersect segment calls needed by the blood splats
bool BloodSplatFilterFn(HOBJECT hTest, void *pUserData)
{
	// Check for the object type. We only want to be blocked by world models 
	uint32 nObjType;
	if(g_pLTClient->Common()->GetObjectType(hTest, &nObjType) != LT_OK)
		return false;

	if(nObjType != OT_WORLDMODEL)
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	WCREATESTRUCT w;

	w.hServerObj	= hServObj;
	w.hObjectHit	= pMsg->ReadObject();
    w.hFiredFrom    = pMsg->ReadObject();
    w.nWeaponId     = pMsg->Readuint8();
    w.nAmmoId       = pMsg->Readuint8();
    w.nSurfaceType  = pMsg->Readuint8();
    w.wIgnoreFX     = pMsg->Readuint16();
    w.nShooterId    = pMsg->Readuint8();
    w.vFirePos		= pMsg->ReadLTVector();
    w.vPos			= pMsg->ReadLTVector();
    w.vSurfaceNormal	= pMsg->ReadLTVector();
    w.eImpactType	= static_cast< IMPACT_TYPE >( pMsg->Readuint8() );

    // make sure the impact type is valid
    ASSERT( ( 0 <= w.eImpactType ) && ( IMPACT_TYPE_COUNT > w.eImpactType ) );

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

	m_hObjectHit	 = pCS->hObjectHit;
    m_hFiredFrom     = pCS->hFiredFrom; // LTNULL
	m_vFirePos		 = pCS->vFirePos;
	m_vPos			 = pCS->vPos;
	m_vSurfaceNormal = pCS->vSurfaceNormal;
	m_vSurfaceNormal.Normalize();

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

	// type of impact FX to play
	m_eImpactType	= pCS->eImpactType;

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
		g_cvarFlyByRadius.Init(g_pLTClient, "FlyByRadius", NULL, 600.0f);
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

	// Make sure the parent has updated their attachments.
	g_pLTClient->ProcessAttachments( m_hFiredFrom );

	// Set up our data members...

	// Set the local client id...

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);
    m_nLocalId = (uint8)dwId;


	m_nDetailLevel = pSettings->SpecialFXSetting();

	// Fire pos may get tweaked a little...

	m_vFirePos = CalcFirePos(m_vFirePos);

	m_vDir = m_vPos - m_vFirePos;
	m_fFireDistance = m_vDir.Length();
	m_vDir.Normalize();

	m_rSurfaceRot = LTRotation(m_vSurfaceNormal, LTVector(0.0f, 1.0f, 0.0f));
	m_rDirRot = LTRotation(m_vDir, LTVector(0.0f, 1.0f, 0.0f));

	SetupExitInfo();



	// Calculate if the camera can see the fire position and the impact
	// position...

	g_bCanSeeImpactPos	= LTTRUE;
	g_bCanSeeFirePos	= LTTRUE;
	g_bDistantImpactPos	= LTFALSE;
	g_bDistantFirePos	= LTFALSE;

	if (g_vtWeaponFXUseFOVPerformance.GetFloat())
	{
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		LTVector vCameraPos, vF, vDir;
		LTRotation rCameraRot;
		g_pLTClient->GetObjectPos(hCamera, &vCameraPos);
		g_pLTClient->GetObjectRotation(hCamera, &rCameraRot);
		vF = rCameraRot.Forward();

		vDir = m_vPos - vCameraPos;
		LTFLOAT fImpactDist = vDir.Length();

		if (fImpactDist > g_vtWeaponFXMaxImpactDist.GetFloat())
		{
			g_bDistantImpactPos = LTTRUE;
		}

		vDir.Normalize();

		LTFLOAT fMul = vDir.Dot(vF);
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

		if (vDir.Length() > g_vtWeaponFXMaxFireDist.GetFloat())
		{
			g_bDistantFirePos = LTTRUE;
		}

		vDir.Normalize();

		fMul = vDir.Dot(vF);
		g_bCanSeeFirePos = (fMul < g_vtWeaponFXMinFireDot.GetFloat() ? LTFALSE : LTTRUE);
	}



	// Determine what container the sfx is in...

	HLOCALOBJ objList[1];
    LTVector vTestPos = m_vPos + m_vSurfaceNormal;  // Test a little closer...
    uint32 dwNum = ::GetPointContainers(vTestPos, objList, 1, ::GetLiquidFlags());

	if (dwNum > 0 && objList[0])
	{
        uint16 dwCode;
        if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
		{
			m_eCode = (ContainerCode)dwCode;
		}
	}

	// Determine if the fire point is in liquid

	vTestPos = m_vFirePos + m_vDir;  // Test a little further in...
    dwNum = ::GetPointContainers(vTestPos, objList, 1, ::GetLiquidFlags());

	if (dwNum > 0 && objList[0])
	{
        uint16 dwCode;
        if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
		{
			m_eFirePosCode = (ContainerCode)dwCode;
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

	if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
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
					bCreateMark = g_pPlayerMgr->IsZoomed();
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
				CreateBulletTrail(m_vFirePos);
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
		CreateMuzzleFX();

		if (!g_bDistantFirePos && (LTBOOL)GetConsoleInt("ShellCasings", 1) && (m_wFireFX & WFX_SHELL))
		{
			CreateShell();
		}

	}

	if ((m_wFireFX & WFX_FIRESOUND) || (m_wFireFX & WFX_ALTFIRESND) || (m_wFireFX & WFX_SILENCED))
	{
		PlayFireSound();
	}

	// Only do fly-by sounds for weapons that leave bullet trails...that
	// we didn't fire ;)

	if (IsBulletTrailWeapon() && (m_nLocalId != m_nShooterId))
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
        uint32 dwNum = ::GetPointContainers(vTestPos, objList, 1, ::GetLiquidFlags());

		if (dwNum > 0 && objList[0])
		{
			uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				m_eExitCode = (ContainerCode)dwCode;
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
	// Create the surface specific exit fx...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eExitSurface);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	if (IsLiquid(m_eExitCode))
	{
		// Create underwater fx...

		CLIENTFX_CREATESTRUCT	fxCS( pSurf->szUWExitFXName, 0, m_vPos );
		fxCS.m_vTargetNorm = m_vSurfaceNormal;
		g_pClientFXMgr->CreateClientFX(NULL, fxCS, LTTRUE );
	}
	else
	{
		// Create normal fx...

		CLIENTFX_CREATESTRUCT	fxCS( pSurf->szExitFXName, 0, m_vPos );
		fxCS.m_vTargetNorm = m_vSurfaceNormal;
		g_pClientFXMgr->CreateClientFX(NULL, fxCS, LTTRUE );
	}


	// Determine if we should create a beam of light through the surface...

	// CreateLightBeamFX(pSurf);
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
        LTRotation rNormRot(m_vExitNormal, LTVector(0.0f, 1.0f, 0.0f));

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

void CWeaponFX::CreateMark(const LTVector &vPos, const LTVector &vNorm, const LTRotation &rRot,
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

	mark.m_Rotation.Rotate(vNorm, GetRandom(0.0f, MATH_CIRCLE));

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

void CWeaponFX::CreateBulletTrail(const LTVector &vStartPos)
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTVector vColor1, vColor2;
	vColor1.Init(200.0f, 200.0f, 200.0f);
	vColor2.Init(255.0f, 255.0f, 255.0f);

	BTCREATESTRUCT bt;

	bt.vStartPos		= vStartPos;
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

	if (m_pAmmo->pFireFX && m_pAmmo->pFireFX->szBeamFXName )
	{
	 	CLIENTFX_CREATESTRUCT fxInit( m_pAmmo->pFireFX->szBeamFXName, 0, m_vFirePos );
		fxInit.m_bUseTargetData = true;
		fxInit.m_vTargetPos = m_vPos;
		g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );	
	}

	// Only do impact fx if the client can see the impact position
	// or the impact fx may last a little while...

	if (g_bCanSeeImpactPos || m_pAmmo->fProgDamage > 0.0f || m_pAmmo->nAreaDamage > 0)
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

		//determine if the object we hit is a model 
		bool bHitMovable = false;

		if(m_hObjectHit)
		{
			//we hit an object, assume it is movable
			bHitMovable = true;

			uint32 nType;
			if(g_pLTClient->Common()->GetObjectType(m_hObjectHit, &nType) == LT_OK)
			{
				//we only want to hit non-movable world models
				if(nType == OT_WORLDMODEL)
				{
					uint32 nUserFlags;
					if(g_pLTClient->Common()->GetObjectFlags(m_hObjectHit, OFT_User, nUserFlags) == LT_OK)
					{
						//see if it is movable
						if(!(nUserFlags & USRFLG_MOVEABLE))
						{
							bHitMovable = false;
						}
					}
				}
			}				
		}


		if(bHitMovable && m_pAmmo->pMoveableImpactOverrideFX)
		{
			// Create the model hit effect
			g_pFXButeMgr->CreateImpactFX(m_pAmmo->pMoveableImpactOverrideFX, cs);
		}
		else if (IsLiquid(m_eCode))
		{
			// Create underwater weapon fx...
			g_pFXButeMgr->CreateImpactFX(m_pAmmo->pUWImpactFX, cs);
		}
		else if ( IMPACT_TYPE_RICOCHET == m_eImpactType )
		{
			// Create a ricochet weapon fx...
			ASSERT( 0 != m_pAmmo->pProjectileFX );
			IMPACTFX *pRicochetFX =
				g_pFXButeMgr->GetImpactFX(
					m_pAmmo->pProjectileFX->szRicochetFXName
				);
			g_pFXButeMgr->CreateImpactFX(pRicochetFX, cs);
		}
		else if ( IMPACT_TYPE_BLOCKED == m_eImpactType )
		{
			// Create a blocked weapon fx...
			g_pFXButeMgr->CreateImpactFX(m_pAmmo->pBlockedFX, cs);
		}
		else if ( IMPACT_TYPE_IMPACT == m_eImpactType )
		{
			// Create a ricochet weapon fx...
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
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return;

	// Only check for visibility if we actually have an object...

	if( m_hObjectHit )
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( m_hObjectHit, OFT_Flags, dwFlags );
		
		// Don't do surface fx on an invisible object...

		if( !(dwFlags & FLAG_VISIBLE) )
			return;
	}

	if (m_eSurfaceType == ST_FLESH)
	{
		// Only do gore if specified...

		if (pProfile->m_bGore)
		{
			if ( m_pAmmo->eType == VECTOR && 
				(m_pAmmo->eInstDamageType == DT_BULLET || 
				 m_pAmmo->eInstDamageType == DT_MELEE ||
				 m_pAmmo->eInstDamageType == DT_SWORD) )
			{
				CreateBloodSplatFX();
			}
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

	if (!m_pAmmo->pImpactFX) return;
	if (!m_pAmmo->pImpactFX->bDoSurfaceFX) return;


	// Create the surface specific fx...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eSurfaceType);
	if (pSurf)
	{
		if (IsLiquid(m_eCode))
		{
			// Create underwater fx...

			CLIENTFX_CREATESTRUCT	fxCS( pSurf->szUWImpactFXName, 0, m_vPos );
			fxCS.m_vTargetNorm = m_vSurfaceNormal;
			g_pClientFXMgr->CreateClientFX(NULL, fxCS, LTTRUE );
		}
		else
		{
			// Create normal fx...

			CLIENTFX_CREATESTRUCT	fxCS( pSurf->szImpactFXName, 0, m_vPos );
			fxCS.m_vTargetNorm = m_vSurfaceNormal;
			g_pClientFXMgr->CreateClientFX(NULL, fxCS, LTTRUE );
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

	if (m_nLocalId != m_nShooterId || !g_pPlayerMgr->IsFirstPerson())
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

	if( pSnd && pSnd[0] )
	{
		uint32 dwFlags = 0;
		float fPitchShift = 1.0f;
		if (g_cvarImpactPitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
		}

        uint8 nVolume = IsLiquid(m_eCode) ? 50 : 100;
		g_pClientSoundMgr->PlaySoundFromPos((LTVector)m_vPos, pSnd, fSndRadius,
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
	// See who we are creating the effect for
	if (m_nLocalId == m_nShooterId)
	{
		//This is the player we are creating the effect for, so we only need to create
		//the ammo specific effects to give them feedback. So if it exists, create
		//that effect
		if( m_pAmmo->pFireFX && m_pAmmo->pFireFX->szFXName[0] )
		{
			CLIENTFX_CREATESTRUCT	fxcs( m_pAmmo->pFireFX->szFXName, 0, m_vFirePos );
			fxcs.m_vTargetNorm = m_vDir;
			fxcs.m_hParent = m_hFiredFrom;
			g_pClientFXMgr->CreateClientFX(NULL, fxcs, LTTRUE );
		}
	}
	else
	{
		//This is the AI shooting, we just need to create an effect as specified
		//by the hand held position
		if(m_pWeapon->szHHMuzzleFxName[0])
		{
			CLIENTFX_CREATESTRUCT	fxcs( m_pWeapon->szHHMuzzleFxName, 0, m_vFirePos );
			fxcs.m_vTargetNorm = m_vDir;
			g_pClientFXMgr->CreateClientFX(NULL, fxcs, LTTRUE );
		}
	}

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
	SHELLCREATESTRUCT sc;
	sc.rRot			= m_rDirRot;
	sc.vStartPos	= CalcBreachPos(m_vFirePos);
	sc.nWeaponId	= m_nWeaponId;
	sc.nAmmoId		= m_nAmmoId;
    sc.b3rdPerson	= LTTRUE;


	// See if this is our local client who fired and if we're in first
	// person...

	if (m_nLocalId == m_nShooterId && g_pPlayerMgr->IsFirstPerson())
	{
		sc.b3rdPerson = LTFALSE;

		// Add on the player's velocity...
		
		HOBJECT hObj = g_pPlayerMgr->GetMoveMgr()->GetObject();
		if (hObj)
		{
			g_pPhysicsLT->GetVelocity(hObj, &sc.vStartVel);
		}
	}

	g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SHELLCASING_ID, &sc);
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
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile || !pProfile->m_bGore) return;

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

	LTVector vU, vR;
	vU = m_rDirRot.Up();
	vR = m_rDirRot.Right();

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

		iQuery.m_To			= vDir * fRange + m_vPos;
		iQuery.m_Flags		= IGNORE_NONSOLID | INTERSECT_HPOLY | INTERSECT_OBJECTS;
		iQuery.m_FilterFn	= BloodSplatFilterFn;
		iQuery.m_pUserData	= NULL;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo) && IsMainWorld(iInfo.m_hObject))
		{
			SurfaceType eType = GetSurfaceType(iInfo);
			SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eType);

			// Don't leave blood on surfaces that can't show marks...
			if (!pSurface || !pSurface->bShowsMark)
			{
				return;
			}

			//find the largest splat we can place and still have it fit onto the polygon
			float fMaxRadius;
			g_pLTClient->GetMaxRadiusInPoly(iInfo.m_hPoly, iInfo.m_Point, fMaxRadius);
			
			//only bother creating it if it is above some size
			if(fMaxRadius > 0.1f)
			{
				LTBOOL bBigBlood = (LTBOOL)GetConsoleInt("BigBlood", 1);

				// Create a blood splat...

				BSCREATESTRUCT sc;

				sc.rRot = LTRotation(iInfo.m_Plane.m_Normal, LTVector(0.0f, 1.0f, 0.0f));

				// Randomly rotate the blood splat

				sc.rRot.Rotate(iInfo.m_Plane.m_Normal, GetRandom(0.0f, MATH_CIRCLE));

				LTVector vTemp = vDir * -2.0f;
				sc.vPos = iInfo.m_Point + vTemp;  // Off the wall a bit
				sc.vVel.Init(0.0f, 0.0f, 0.0f);

				sc.vInitialScale.Init(1.0f, 1.0f, 1.0f);
				sc.vInitialScale.x	= GetRandom(g_vtBloodSplatsMinScale.GetFloat(), g_vtBloodSplatsMaxScale.GetFloat());

				if (bBigBlood) sc.vInitialScale.x *= g_vtBigBloodSizeScale.GetFloat();

				//clamp it to the maximum size
				static const uint32	knBloodSize = 64;
				sc.vInitialScale.x  = LTMIN(sc.vInitialScale.x, fMaxRadius / knBloodSize);

				sc.vInitialScale.y	= sc.vInitialScale.x;
				sc.vFinalScale		= sc.vInitialScale;

				sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
				sc.fLifeTime		= GetRandom(g_vtBloodSplatsMinLifetime.GetFloat(), g_vtBloodSplatsMaxLifetime.GetFloat());

				if (bBigBlood) sc.fLifeTime *= g_vtBigBloodLifeScale.GetFloat();

				sc.fInitialAlpha	= 1.0f;
				sc.fFinalAlpha		= 0.0f;
				sc.nType			= OT_SPRITE;
				sc.bMultiply		= LTTRUE;
				sc.bPausable		= LTTRUE;

				char* pBloodFiles[] =
				{
					"FX\\Test\\Blood\\Spr\\BloodL1.spr",
					"FX\\Test\\Blood\\Spr\\BloodL2.spr",
					"FX\\Test\\Blood\\Spr\\BloodL3.spr",
					"FX\\Test\\Blood\\Spr\\BloodL4.spr"
				};

				sc.pFilename = pBloodFiles[GetRandom(0,3)];

				pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
				if (pFX) pFX->Update();
			}
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

LTVector CWeaponFX::CalcFirePos(const LTVector &vFirePos)
{
	if (!m_hFiredFrom) return vFirePos;

	// See if this is our local client who fired, and if so
	// only calculate fire position if we are in 3rd person...

	if (m_nLocalId == m_nShooterId)
	{
		if (g_pPlayerMgr->IsFirstPerson()) return vFirePos;
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
//	ROUTINE:	CWeaponFX::CalcBreachPos
//
//	PURPOSE:	Calculate the breach position based on the FireFrom object
//
// ----------------------------------------------------------------------- //

LTVector CWeaponFX::CalcBreachPos(const LTVector &vBreachPos)
{
	if (!m_hFiredFrom) return vBreachPos;

	// See if this is our local client who fired, and if so
	// only calculate fire position if we are in 3rd person...

    LTVector vPos;
	if (m_nLocalId == m_nShooterId && g_pPlayerMgr->IsFirstPerson())
	{
 		IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
  		if ( pClientWeapon )
  		{
  			pClientWeapon->GetShellEjectPos(&vPos);
			return vPos;
  		}
	}

    LTRotation rRot;
	if (!GetAttachmentSocketTransform(m_hFiredFrom, "Breach", vPos, rRot))
	{
		vPos = vBreachPos;
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

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	LTVector vPos;
	g_pLTClient->GetObjectPos(hCamera, &vPos);

  	// We only play the flyby sound if we won't hear an impact...
  
  	if (m_pAmmo->pImpactFX)
  	{
  		LTVector vDist = m_vPos - vPos;
  		if ( vDist.Length() < (0.75f * float(m_pAmmo->pImpactFX->nSoundRadius)) ) 
			return;
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
		// Play the fly by sound bute....

		vPos += vBulletDir;
		g_pClientSoundMgr->PlaySoundFromPos(vPos, "BulletFlyBy");
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

	CCharacterFX* pCharFX = g_pPlayerMgr->GetMoveMgr()->GetCharacterFX();
	if (pCharFX)
	{
		pCharFX->PlayDingSound();
	}
}