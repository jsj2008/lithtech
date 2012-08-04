// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 2/22/98
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "MarkSFX.h"
#include "MsgIDs.h"
#include "ShellCasingFX.h"
#include "iltphysics.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "ClientConnectionMgr.h"
#include "ClientWeapon.h"
#include "ClientWeaponMgr.h"
#include "PlayerCamera.h"
#include "FXDB.h"
#include "PolyGridFX.h"
#include "ModelDecalMgr.h"
#include "CharacterFx.h"
#include "ClientWeaponUtils.h"
#include "PhysicsUtilities.h"
#include "LTEulerAngles.h"

static uint32 s_nNumShells = 0;

VarTrack	g_cvarShowFirePath;
VarTrack	g_cvarFlyByRadius;
VarTrack	g_cvarFlyByImpactRadiusFactor;
VarTrack	g_cvarFlyByWithImpactChance;
VarTrack	g_vtWeaponFXMinImpactDot;
VarTrack	g_vtWeaponFXMinFireDot;
VarTrack	g_vtWeaponFXUseFOVPerformance;
VarTrack	g_vtWeaponFXMaxFireDist;
VarTrack	g_vtWeaponFXMaxImpactDist;
VarTrack	g_vtWeaponFXMaxMultiImpactDist;
VarTrack	g_vtMultiDing;

VarTrack	g_vtPolygridImpactPerturbRadius;
VarTrack	g_vtPolygridImpactPerturbForce;

VarTrack	g_vtCreateImpactFXOnHitNode;
VarTrack	g_vtProtrudeFXOffset;

#define		WFX_DEFAULT_POLY_PERTURB_FORCE	1000.0f
#define		WFX_DEFAULT_POLY_PERTURB_RADIUS	30.0f

bool		g_bCanSeeImpactPos	= true;
bool		g_bCanSeeFirePos	= true;
bool		g_bDistantFirePos	= false;
bool		g_bDistantImpactPos	= false;

void	DebugPrintFXStruct(const CLIENTFX_CREATESTRUCT& cs,const char* szFun)
{
	DebugCPrint(0,"**************");
	if (!LTStrEmpty(szFun))
	{
		DebugCPrint(0,"FX Struct info: %s",szFun);
	}
	else
	{
		DebugCPrint(0,"FX Struct info:");
	}

	DebugCPrint(0,"   Name: %s",cs.m_sName);
	DebugCPrint(0,"   Flags: %x",cs.m_dwFlags);
	DebugCPrint(0,"   m_hParentObject: %x",cs.m_hParentObject);
	DebugCPrint(0,"   m_hParentRigidBody: %x",cs.m_hParentRigidBody);
	DebugCPrint(0,"   m_hNode: %x",cs.m_hNode);
	DebugCPrint(0,"   m_hSocket: %x",cs.m_hSocket);
	DebugCPrint(0,"   m_tTransform.m_vPos.x: %0.2f",cs.m_tTransform.m_vPos.x);
	DebugCPrint(0,"   m_tTransform.m_vPos.y: %0.2f",cs.m_tTransform.m_vPos.y);
	DebugCPrint(0,"   m_tTransform.m_vPos.z: %0.2f",cs.m_tTransform.m_vPos.z);

	EulerAngles EA = Eul_FromQuat( cs.m_tTransform.m_rRot, EulOrdYXZr );
	float fYaw		= EA.x;
	float fPitch	= EA.y;
	float fRoll		= EA.z;

	DebugCPrint(0,"   m_tTransform Yaw: %0.2f",fYaw);
	DebugCPrint(0,"   m_tTransform Pitch: %0.2f",fPitch);
	DebugCPrint(0,"   m_tTransform Roll: %0.2f",fRoll);

	DebugCPrint(0,"   m_bUseTargetData: %s", (cs.m_bUseTargetData ? "true" : "false") );
	if (cs.m_bUseTargetData)
	{
		DebugCPrint(0,"   m_hTargetObject: %x",cs.m_hTargetObject);
		DebugCPrint(0,"   m_vTargetOffset.x: %0.2f",cs.m_vTargetOffset.x);
		DebugCPrint(0,"   m_vTargetOffset.y: %0.2f",cs.m_vTargetOffset.y);
		DebugCPrint(0,"   m_vTargetOffset.z: %0.2f",cs.m_vTargetOffset.z);
	}
	


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

bool CWeaponFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return false;
    if (!pMsg) return false;

	WCREATESTRUCT w;

	w.hServerObj	= hServObj;
    
	bool bShooterIDSent = pMsg->Readbool( );
	if( bShooterIDSent )
	{
		w.nShooterId    = pMsg->Readuint8( );

		// Search through the CharacterFX for the player with this ID...
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while( iter != CCharacterFX::GetCharFXList( ).end( )) 
		{
			if( (*iter)->m_cs.nClientID == w.nShooterId )
			{
				w.hFiredFrom = (*iter)->GetServerObj( );
			}

			++iter;
		}
	}
	else
	{
		w.hFiredFrom    = pMsg->ReadObject( );
	}

	w.hWeapon = NULL;
	w.hAmmo = NULL;

	// If the weapon record was written in the message it needs to be retrieved.
	// Otherwise the weapon can be obtained from the character that fired the weapon...
	bool bReadWeaponRecord = pMsg->Readbool( );
	if( bReadWeaponRecord )
	{
		w.hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	}
	else
	{
		// Get the current weapon of the firing character...
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while( iter != CCharacterFX::GetCharFXList( ).end( )) 
		{
			if( (*iter)->GetServerObj( ) == w.hFiredFrom )
			{
				w.hWeapon = (*iter)->m_cs.hCurWeaponRecord;
			}

			++iter;
		}
	}
	
	if( !w.hWeapon )
	{
		LTERROR( "Invalid weapon record, can't initialize WeaponFX!" );
		return false;
	}
	
	// If the ammo record was written in the message it needs to be retrieved.
	// Otherwise the ammo used will be the weapons default...
	bool bReadAmmoRecord = pMsg->Readbool( );
	if( bReadAmmoRecord )
	{
		w.hAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	}
	else
	{
		bool bIsAI = pMsg->Readbool();
		HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData(w.hWeapon, bIsAI);
		w.hAmmo = g_pWeaponDB->GetRecordLink(hWeaponData, WDB_WEAPON_rAmmoName);
	}

	if( !w.hAmmo )
	{
		LTERROR( "Invalid ammo record, can't initialize WeaponFX!" );
		return false;
	}

	w.wIgnoreFX			= pMsg->Readuint16();

	// Read if they hit the main world or another object.
	if( pMsg->Readbool( ))
	{
		w.hObjectHit		= g_pLTBase->GetMainWorldModel();
	}
	else
	{
		w.hObjectHit		= pMsg->ReadObject();
	}
	w.nSurfaceType		= pMsg->Readuint8();
    w.vFirePos			= pMsg->ReadCompLTVector();
	w.bFXAtFlashSocket	= pMsg->Readbool();
	w.vPos				= pMsg->ReadCompLTVector();
    w.vSurfaceNormal	= pMsg->ReadCompLTPolarCoord();
    bool bNodeHit		= pMsg->Readbool();
    w.hNodeHit	 		= bNodeHit ? pMsg->Readuint8() : INVALID_MODEL_NODE;

  	return Init(&w);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

bool CWeaponFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	WCREATESTRUCT* pCS = (WCREATESTRUCT*)psfxCreateStruct;

	m_hWeapon		= pCS->hWeapon;
	m_hAmmo			= pCS->hAmmo;
	m_eSurfaceType	= (SurfaceType)pCS->nSurfaceType;
	m_wIgnoreFX		= pCS->wIgnoreFX;

	m_hObjectHit	 = pCS->hObjectHit;
    m_hFiredFrom     = pCS->hFiredFrom; // NULL
	m_vFirePos		 = pCS->vFirePos;
	m_bFXAtFlashSocket = pCS->bFXAtFlashSocket;
	m_vPos			 = pCS->vPos;
	m_vSurfaceNormal = pCS->vSurfaceNormal;
	m_bLeftHandWeapon = pCS->bLeftHandWeapon;
	if( m_hObjectHit )
	{
		LTASSERT( m_vSurfaceNormal.MagSqr() > 0.0f, "Invalid surface normal." );
		m_vSurfaceNormal.Normalize();
	}

	m_eCode			= CC_NO_CONTAINER;
	m_eFirePosCode	= CC_NO_CONTAINER;

    if( !m_hAmmo || !m_hWeapon )
		return false;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	m_fInstDamage   = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstDamage );
    m_fAreaDamage   = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamage );

	m_nShooterId	= pCS->nShooterId;
	
	// Node that got hit
	m_hNodeHit		= pCS->hNodeHit;

	// ModelDB::Node that got hit..
	// (if it exists..)
	// only handle this if we have a valid m_hObjectHit
	m_hModelDBNode = NULL;

	if (( m_hNodeHit != INVALID_MODEL_NODE ) && (m_hObjectHit))
	{
		char szName[64] = "";

		CCharacterFX *pImpactedCharacterFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hObjectHit );
		if ( pImpactedCharacterFX )
		{
			g_pLTClient->GetModelLT()->GetNodeName(m_hObjectHit, m_hNodeHit, szName, LTARRAYSIZE(szName) );
			m_hModelDBNode = g_pModelsDB->GetSkeletonNode(pImpactedCharacterFX->GetModelSkeleton(), szName);
		}
	}
	else
	{
		m_hModelDBNode = NULL;
	}

	if (!g_cvarShowFirePath.IsInitted())
	{
		g_cvarShowFirePath.Init(g_pLTClient, "ShowFirePath", NULL, -1.0f);
    }

	if (!g_cvarFlyByRadius.IsInitted())
	{
		g_cvarFlyByRadius.Init(g_pLTClient, "FlyByRadius", NULL, 600.0f);
	}
	if (!g_cvarFlyByImpactRadiusFactor.IsInitted())
	{
		g_cvarFlyByImpactRadiusFactor.Init(g_pLTClient, "FlyByImpactRadiusFactor", NULL, 0.75f);
	}
	if (!g_cvarFlyByWithImpactChance.IsInitted())
	{
		g_cvarFlyByWithImpactChance.Init(g_pLTClient, "FlyByWithImpactChance", NULL, 0.25f);
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
		g_vtMultiDing.Init(g_pLTClient, "WeaponFXMultiImpactDing", NULL, 0.0f);
	}

	if (!g_vtPolygridImpactPerturbRadius.IsInitted())
	{
		g_vtPolygridImpactPerturbRadius.Init(g_pLTClient, "PolygridImpactPerturbRadius", NULL, WFX_DEFAULT_POLY_PERTURB_RADIUS);
	}

	if (!g_vtPolygridImpactPerturbForce.IsInitted())
	{
		g_vtPolygridImpactPerturbForce.Init(g_pLTClient, "PolygridImpactPerturbForce", NULL, WFX_DEFAULT_POLY_PERTURB_FORCE);
	}

	if (!g_vtCreateImpactFXOnHitNode.IsInitted())
	{
		g_vtCreateImpactFXOnHitNode.Init(g_pLTClient, "ImpactFXOnHitNode", NULL, 1.0f);
	}
	
	if( !g_vtProtrudeFXOffset.IsInitted( ))
	{
		g_vtProtrudeFXOffset.Init( g_pLTClient, "ProtrudeFXOffset", NULL, 10.0f );
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

bool CWeaponFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return false;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return false;

	// Make sure the parent has updated their attachments.
	g_pLTClient->ProcessAttachments( m_hFiredFrom );

	// Set up our data members...

	// Set the local client id...

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);
    m_nLocalId = (uint8)dwId;

	m_bLocalClientFired = (m_nLocalId == m_nShooterId && m_nLocalId >= 0);

	m_nDetailLevel = pSettings->SpecialFXSetting();

	// Fire pos may get tweaked a little...
	m_vOriginalFirePos = m_vFirePos;
	if( m_bFXAtFlashSocket )
	{
		m_vFirePos = CalcFirePos(m_vFirePos,m_bLeftHandWeapon);
	}

	m_vDir = m_vPos - m_vFirePos;
	m_fFireDistance = m_vDir.Mag();
	m_vDir.Normalize();

	m_rDirRot = LTRotation(m_vDir, LTVector(0.0f, 1.0f, 0.0f));
	if( m_hObjectHit )
		m_rSurfaceRot = LTRotation(m_vSurfaceNormal, LTVector(0.0f, 1.0f, 0.0f));
	else
		m_rSurfaceRot = m_rDirRot;

	SetupExitInfo();


	// Calculate if the camera can see the fire position and the impact
	// position...

	g_bCanSeeImpactPos	= true;
	g_bCanSeeFirePos	= true;
	g_bDistantImpactPos	= false;
	g_bDistantFirePos	= false;

#pragma MESSAGE( "[RP] 07/16/03 - All WeaponFX are now currently visible but performance will suffer." )
// This should be reevaluated.  Possibly give control to artists in FXEdit so they can specify which FX will
// always show or within a certain radius.
/*
	if (g_vtWeaponFXUseFOVPerformance.GetFloat())
	{
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		LTVector vCameraPos, vF, vDir;
		LTRotation rCameraRot;
		g_pLTClient->GetObjectPos(hCamera, &vCameraPos);
		g_pLTClient->GetObjectRotation(hCamera, &rCameraRot);
		vF = rCameraRot.Forward();

		vDir = m_vPos - vCameraPos;
		float fImpactDist = vDir.Mag();

		if (fImpactDist > g_vtWeaponFXMaxImpactDist.GetFloat())
		{
			g_bDistantImpactPos = true;
		}

		vDir.Normalize();

		float fMul = vDir.Dot(vF);
		g_bCanSeeImpactPos = (fMul < g_vtWeaponFXMinImpactDot.GetFloat() ? false : true);

		// In multiplayer we need to account for impacts that occur around
		// our camera that we didn't cause (this is also an issue in single
		// player, but due to the singler player gameplay dynamics it isn't
		// as noticeable)...

		if (!g_bCanSeeImpactPos && IsMultiplayerGameClient())
		{
			// Somebody else shot this...if the impact is close enough, we 
			// "saw" it...
			if (m_nLocalId != m_nShooterId && fImpactDist <= g_vtWeaponFXMaxMultiImpactDist.GetFloat())
			{
				g_bCanSeeImpactPos = true;
			}
		}

		vDir = m_vFirePos - vCameraPos;

		if (vDir.Mag() > g_vtWeaponFXMaxFireDist.GetFloat())
		{
			g_bDistantFirePos = true;
		}

		vDir.Normalize();

		fMul = vDir.Dot(vF);
		g_bCanSeeFirePos = (fMul < g_vtWeaponFXMinFireDot.GetFloat() ? false : true);
	}
*/


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

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	if (IsLiquid(m_eCode))
	{
		HRECORD hUWImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sUWImpactFX ));
		m_wImpactFX	= hUWImpactFX ? WFX_MARK : 0;
	}
	else
	{
		HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sImpactFX ));
		m_wImpactFX	= hImpactFX ? WFX_MARK : 0;
	}

	HRECORD hFireFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFireFX ));
	m_wFireFX = g_pFXDB->GetFireFlags(hFireFX);

	// Assume alt-fire, silenced, and tracer...these will be cleared by
	// IgnoreFX if not used...

	m_wFireFX |= WFX_ALTFIRESND | WFX_SILENCED | WFX_TRACER | WFX_MUZZLE;

	// Assume impact ding, it will be cleared if not used...

	m_wImpactFX |= WFX_IMPACTDING;

	// Clear all the fire fx we want to ignore...

	m_wFireFX &= ~m_wIgnoreFX;
	m_wImpactFX &= ~m_wIgnoreFX;


	// See if this is a redundant weapon fx (i.e., this client shot the
	// weapon so they've already seen this fx)...

	// only play the the weapon ding in response to the server message
	if (g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
	{
		if( (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ) != PROJECTILE)
		{
			if ( m_bLocalClientFired )
			{
				if (m_wImpactFX & WFX_IMPACTDING)
				{
					if (g_vtMultiDing.GetFloat())
					{
						PlayImpactDing();
					}
				}
			}
		}
	}


	// Show the fire path...(debugging...)

	if (g_cvarShowFirePath.GetFloat() > 0)
	{
#pragma MESSAGE( "CPolyLineFX was removed. We will need to implement this another way once other systems are in place." )
	}


	// Create impact related fx...

	CreateWeaponImpactFX();

	if (g_bCanSeeImpactPos)
	{
		if (ShowsClientFX(m_eSurfaceType) && GetConsoleInt("MarkShow", 1))
		{
			bool bCreateMark = true;
			if (g_bDistantImpactPos && m_bLocalClientFired)
			{
				// Assume we'll see the mark if we're zoomed in ;)
				bCreateMark = g_pPlayerMgr->GetPlayerCamera()->IsZoomed();
			}

			if (bCreateMark)
			{
				CreateMark(m_vPos, m_vSurfaceNormal, m_rSurfaceRot, m_eSurfaceType);
			}
		}

		CreateSurfaceSpecificFX();

		CreatePhysicsImpulseForce( );
	}


	// No tracers under water...

	if (GetConsoleInt("Tracers", 1) && (m_wFireFX & WFX_TRACER) && !IsLiquid(m_eCode))
	{
		CreateTracer();
	}

	CreateWeaponBeamFX();

	if (g_bCanSeeFirePos)
	{
		if( m_wFireFX & WFX_MUZZLE )
			CreateMuzzleFX();

		if( !m_bLocalClientFired && !g_bDistantFirePos && GetConsoleInt("ShellCasings", 1) && (m_wFireFX & WFX_SHELL) )
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

	if (IsBulletTrailWeapon() && !m_bLocalClientFired)
	{
		PlayBulletFlyBySound();
	}


    return false;  // Just delete me, I'm done :)
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

    if (g_pLTClient->IntersectSegment(qInfo, &iInfo))
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
//	ROUTINE:	CWeaponFX::CreateExitMark
//
//	PURPOSE:	Create any exit surface marks
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExitMark()
{
	if (m_eExitSurface != ST_UNKNOWN && ShowsClientFX(m_eExitSurface))
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
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sImpactFX ));

	if (IsLiquid(m_eCode))
	{
		hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sUWImpactFX ));
	}

	if (!hImpactFX) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Randomly rotate the bullet hole...
	LTRotation rRandomRot = rRot;
	rRandomRot.Rotate(vNorm, GetRandom(0.0f, MATH_CIRCLE));

	MARKCREATESTRUCT mark;
	mark.hAmmo			= m_hAmmo;
	mark.nSurfaceType   = eType;
	mark.m_tTransform.Init(vPos, rRandomRot);

	//setup the mark in the parent object's space
	if(m_hObjectHit)
	{
		LTRigidTransform tObjHitTransform;
		g_pLTClient->GetObjectTransform(m_hObjectHit, &tObjHitTransform);

		mark.m_hParent		= m_hObjectHit;
		mark.m_tTransform	= tObjHitTransform.GetInverse() * LTRigidTransform(vPos, rRandomRot);
	}

	psfxMgr->CreateSFX(SFX_MARK_ID, &mark);
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
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	HRECORD hTracerFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sTracerFX );
	if( !m_hAmmo || !hTracerFX ) return;

	//create a clientFX that will be positioned at the fire position at the appropriate
	//orientation
	CLIENTFX_CREATESTRUCT fxCS( g_pFXDB->GetString(hTracerFX,FXDB_sFXName), 0, LTRigidTransform(m_vFirePos, m_rDirRot) );
	fxCS.m_bUseTargetData	= true;
	fxCS.m_hTargetObject	= NULL;
	fxCS.m_vTargetOffset	= m_vPos;
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateWeaponBeamFX()
//
//	PURPOSE:	Create the weapon beam fx...
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateWeaponBeamFX()
{
	// Do fire fx beam fx...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	HRECORD hFireFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFireFX ));
	if( hFireFX)
	{
		const char* pszBeamFX = (IsLiquid(m_eCode) ? g_pFXDB->GetString( hFireFX, FXDB_sUWBeamFXName ) : g_pFXDB->GetString( hFireFX, FXDB_sBeamFXName ));
		if (!LTStrEmpty( pszBeamFX ))
		{
			CLIENTFX_CREATESTRUCT fxInit( pszBeamFX, 0, LTRigidTransform(m_vFirePos, m_rDirRot) );
			fxInit.m_bUseTargetData = true;
			fxInit.m_hTargetObject	= NULL;
			fxInit.m_vTargetOffset	= m_vPos;
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );	
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateWeaponImpactFX()
//
//	PURPOSE:	Create weapon specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateWeaponImpactFX()
{
	// Only do impact fx if the client can see the impact position
	// or the impact fx may last a little while...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);

	if( g_bCanSeeImpactPos ||
		g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamage ) > 0.0f ||
		g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamage ) > 0.0f )
	{
		IFXCS cs;
		cs.eCode		= m_eCode;
		cs.eSurfType	= m_eSurfaceType;
		cs.rSurfRot		= m_rSurfaceRot;
		cs.vDir			= m_vDir;
		cs.vPos			= m_vPos;
		cs.vSurfNormal	= m_vSurfaceNormal;

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

		HRECORD hMoveableImpactOverrideFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sMovableImpactOverrideFX ));
		if( bHitMovable && hMoveableImpactOverrideFX )
		{
			// Create the model hit effect
			g_pFXDB->CreateImpactFX( hMoveableImpactOverrideFX, cs );
		}
		else if (IsLiquid(m_eCode))
		{
			HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sUWImpactFX ));
			// Create underwater weapon fx...
			g_pFXDB->CreateImpactFX( hImpactFX, cs );
		}
		else
		{
			HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sImpactFX ));
			// Create a ricochet weapon fx...
			g_pFXDB->CreateImpactFX( hImpactFX, cs);
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
	// Only check for visibility if we actually have an object...
	if( m_hObjectHit )
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( m_hObjectHit, OFT_Flags, dwFlags );
		
		// Only do the surface FX on invisible objects if it's the local player
		// or a container object...
		if( !(dwFlags & FLAG_VISIBLE) )
		{	
			uint32 dwObjectType;
			g_pCommonLT->GetObjectType( m_hObjectHit, &dwObjectType );
			if( (dwObjectType != OT_CONTAINER) && (m_hObjectHit != g_pLTClient->GetClientObject( )) )
				return;
		}

		// Perturb polygrids associated with the object hit...
		PerturbImpactedPolygrid();
		
		// Apply model decals
		ApplyModelDecal();
	}

	if ((m_wFireFX & WFX_EXITMARK) && ShowsClientFX(m_eExitSurface))
	{
		CreateExitMark();
	}

	// If we don't have a surface fx type, we're done...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	const char *pszSurfaceFXType = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sSurfaceFXType );
	if( !pszSurfaceFXType[0] || LTStrIEquals( pszSurfaceFXType, "None" ))
	{
		return;
	}

	// Create the surface specific fx...

	HSURFACE hSurf = g_pSurfaceDB->GetSurface(m_eSurfaceType);
	if (hSurf)
	{
		HSRF_IMPACT hSurfImp = g_pSurfaceDB->GetSurfaceImpactFX(hSurf, pszSurfaceFXType);
		bool hNodeOverrideWasUsed = false;
		bool bIsLiquid = IsLiquid(m_eCode);

		if (m_hModelDBNode != NULL)
		{
			hNodeOverrideWasUsed = CreateSurfaceSpecificImpactFX_NormalNode(m_hModelDBNode, hAmmoData, bIsLiquid);
		}

		if (hSurfImp)
		{
			//CreateSurfaceSpecificImpactFX(hSurf, hSurfImp, IsLiquid(m_eCode));
			if (!hNodeOverrideWasUsed)
			{
				CreateSurfaceSpecificImpactFX_Normal(hSurf, hSurfImp, bIsLiquid);
			}
			CreateSurfaceSpecificImpactFX_Outgoing(hSurf, hSurfImp, bIsLiquid);
			CreateSurfaceSpecificImpactFX_ToViewer(hSurf, hSurfImp, bIsLiquid);
			CreateSurfaceSpecificImpactFX_ToSource(hSurf, hSurfImp, bIsLiquid);
			CreateSurfaceSpecificImpactFX_OnSource(hSurf, hSurfImp, bIsLiquid);
			CreateSurfaceSpecificImpactFX_Protruding( hSurf, hSurfImp, bIsLiquid );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PerturbImpactedPolygrid()
//
//	PURPOSE:	Perturb polygrid if one is associated with the object
//				that was impacted.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PerturbImpactedPolygrid()
{
	if( !m_hObjectHit )
		return;

	// See if this object is liquid...
	uint16 dwCode;
	if (g_pLTClient->GetContainerCode(m_hObjectHit, &dwCode))
	{
		if(!IsLiquid((ContainerCode)dwCode))
		{
			return;
		}
	}
	else
	{
		return;
	}


	// See if there are any polygrids to be perturbed...

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_POLYGRID_ID );
	if( pList )
	{
		// Try and find a polygrid that is the surface of our hit object...

		int nNumPGrids = pList->GetSize();
		for( int i = 0; i < nNumPGrids; ++i )
		{
			if( (*pList)[i] )
			{
				CPolyGridFX* pPGrid= (CPolyGridFX*)(*pList)[i];

				LTVector vIntersectPos = m_vPos;
				if( PathIntersectsPolygrid(pPGrid, vIntersectPos) )
				{
					float fRadius = g_vtPolygridImpactPerturbRadius.GetFloat();
					float fForce = g_vtPolygridImpactPerturbForce.GetFloat();

					// Use the force associated with the ammo if we're not tweaking the values...
					if (fForce == WFX_DEFAULT_POLY_PERTURB_FORCE)
					{
						HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
						fForce = g_pWeaponDB->GetFloat(hAmmoData, WDB_AMMO_fInstDamageImpulseForce);
					}

					pPGrid->CreateDisturbance(vIntersectPos, fRadius, fForce);
					return; // Only perturb one polygrid
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PathIntersectsPolygrid()
//
//	PURPOSE:	Determine if our fire path intersected the specified
//				polygrid.  Returns the intersected position in vIntersectPos
//				if return value is true.
//
// ----------------------------------------------------------------------- //

bool CWeaponFX::PathIntersectsPolygrid(const CPolyGridFX* pPGrid, LTVector & vIntersectPos)
{
	if (!pPGrid) return false;

	// First off, determine the intersection with the plane of the polygrid...

	HLOCALOBJ hPolygrid = pPGrid->GetObject();
	if (!hPolygrid) return false;

	LTVector vStart = m_vFirePos;
	LTVector vEnd = m_vPos + (5.0f * m_vDir); // Test a little ways in...

	LTRigidTransform tTrans;
	g_pLTClient->GetObjectTransform(hPolygrid, &tTrans);

	LTVector vPolyCenter = tTrans.m_vPos;
	LTRotation rRot	= tTrans.m_rRot;
	LTVector vU = rRot.Up();

	// Move the segment into the polygrid’s offset space...
	LTVector vSO = vStart - vPolyCenter;
	LTVector vEO = vEnd - vPolyCenter;

	float fDistStart = vSO.Dot(vU);
	float fDistEnd = vEO.Dot(vU);

	// If both distances are of the same sign, the fire path didn't intersect the plane of the
	// polygrid, so return...
	if ((fDistStart >= 0.0f && fDistEnd >= 0.0f) || 
		(fDistStart < 0.0f && fDistEnd < 0.0f))
		return false;

	// Find the point on the plane
	vIntersectPos = vSO + ((fDistStart / (fDistStart - fDistEnd)) * (vEO - vSO));

	// And now determine if it is within the rectangle
	float fXVal = vIntersectPos.Dot(rRot.Right());
	float fZVal = vIntersectPos.Dot( rRot.Forward());
	if ( (LTAbs(fXVal) >= pPGrid->GetDims().x) || (LTAbs(fZVal) >= pPGrid->GetDims().z) )
		return false;

	// Add back the center of the polygrid to get it back into world space
	vIntersectPos += vPolyCenter;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX()
//
//	PURPOSE:	Create surface specific fx impact fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSurfaceSpecificImpactFX(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	// note: just calls the specific calls. Maintained for history's sake. - Terry
	CreateSurfaceSpecificImpactFX_Normal(hSurf, hImpact, bUnderWater);
	CreateSurfaceSpecificImpactFX_Outgoing(hSurf, hImpact, bUnderWater);
	CreateSurfaceSpecificImpactFX_ToViewer(hSurf, hImpact, bUnderWater);
	CreateSurfaceSpecificImpactFX_ToSource(hSurf, hImpact, bUnderWater);
	CreateSurfaceSpecificImpactFX_OnSource(hSurf, hImpact, bUnderWater);
	CreateSurfaceSpecificImpactFX_Protruding( hSurf, hImpact, bUnderWater );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_Normal()
//
//	PURPOSE:	Create surface specific NORMAL fx impact fx
//
// ----------------------------------------------------------------------- //
void	CWeaponFX::CreateSurfaceSpecificImpactFX_Normal(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	if (!hSurf || !hImpact) return;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_NormalFX : SrfDB_Imp_sNormalFX;

	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpact,pszFXAttribName);
	if( !nNumFx )
		return;

	//create a normal facing orientation with a random rotation
	LTRotation rSurface = m_rSurfaceRot;
	rSurface.Rotate(m_vSurfaceNormal, GetRandom(0.0f, MATH_CIRCLE));

	bool bCreateOnHitNode = (g_vtCreateImpactFXOnHitNode.GetFloat() && (m_hNodeHit != INVALID_MODEL_NODE) && (m_hObjectHit));
	// Setup the fx struct minus the fxname since it will be the same for each fx.
	CLIENTFX_CREATESTRUCT fxCS( "", 0);
	if( bCreateOnHitNode )
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hObjectHit, m_hNodeHit, tNode, true );

		LTRigidTransform trNode(tNode.m_vPos, tNode.m_rRot);
		LTRigidTransform trSurface(tNode.m_vPos, rSurface);

		fxCS.m_hParentObject = m_hObjectHit;
		fxCS.m_hNode	  = m_hNodeHit;
		fxCS.m_tTransform = (trNode.GetInverse() * trSurface);
	}
	else
	{
		LTRigidTransform trSource( m_vPos, rSurface );
/*
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			fxCS.m_tTransform = (trObject.GetInverse( ) * trSource );
		}
		else
*/
		{
			fxCS.m_tTransform = trSource;
		}

		// move the hit point out a little bit from the surface along
		// the normal. Sometimes, the point was slightly inside the surface,
		// cause the system to occlude the sounds. -- Terry
		fxCS.m_tTransform.m_vPos += 0.3f * m_vSurfaceNormal;
	}

	// Create all the fx specified.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
/*
		DebugPrintFXStruct(fxCS,__FUNCTION__);
		DebugCPrint(0,"   m_vPos.x: %0.2f",m_vPos.x);
		DebugCPrint(0,"   m_vPos.y: %0.2f",m_vPos.y);
		DebugCPrint(0,"   m_vPos.z: %0.2f",m_vPos.z);
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			DebugCPrint(0,"   trObject.m_vPos.x: %0.2f",trObject.m_vPos.x);
			DebugCPrint(0,"   trObject.m_vPos.y: %0.2f",trObject.m_vPos.y);
			DebugCPrint(0,"   trObject.m_vPos.z: %0.2f",trObject.m_vPos.z);
		}
*/
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_NormalNode()
//
//	PURPOSE:	Create NODE specific NORMAL NODE fx impact fx
//
// ----------------------------------------------------------------------- //
bool	CWeaponFX::CreateSurfaceSpecificImpactFX_NormalNode(ModelsDB::HNODE hNode, HAMMODATA hAmmoData, bool bUnderWater)
{
	if (!hNode) return false;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_NormalFX : SrfDB_Imp_sNormalFX;

	// get the node-override weaponFX
	HRECORD hNodeOverride = g_pModelsDB->GetNodeOverrideWeaponFX( hNode );
	if (hNodeOverride == NULL)
	{
		return false;
	}

	// Get the ammo type..
	const char *pszSurfaceFXType = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sSurfaceFXType );
	if( !pszSurfaceFXType[0] || LTStrIEquals( pszSurfaceFXType, "None" ))
	{
		return false;
	}

	// get the impact effect for the weapon used.
	HRECORD hImpactEffect = g_pWeaponDB->GetRecordLink(hNodeOverride, pszSurfaceFXType);
	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpactEffect,pszFXAttribName);
	if( !nNumFx )
		return false;

	//create a normal facing orientation with a random rotation
	LTRotation rSurface = m_rSurfaceRot;
	rSurface.Rotate(m_vSurfaceNormal, GetRandom(0.0f, MATH_CIRCLE));

	// Create the init struct minus the fxname.
	CLIENTFX_CREATESTRUCT fxCS( "", 0, LTRigidTransform(m_vPos, rSurface) );

	// Create all the fx specified.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpactEffect, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_Outgoing()
//
//	PURPOSE:	Create surface specific OUTGOING fx impact fx
//
// ----------------------------------------------------------------------- //
void	CWeaponFX::CreateSurfaceSpecificImpactFX_Outgoing(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	if (!hSurf || !hImpact) return;

	// Use underwater or regular FX.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_OutgoingFX : SrfDB_Imp_sOutgoingFX;

	// Create all the fx specified.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpact,pszFXAttribName);
	if( !nNumFx )
		return;

	// Calculate outgoing rotation...
	LTRotation rOutgoing;
	if (g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bCanShootThrough))
	{
		// Outgoing is same as fire direction...
		rOutgoing = m_rDirRot;
		rOutgoing.Rotate(m_vDir, GetRandom(0.0f, MATH_CIRCLE));
	}
	else
	{
		// Outgoing is reflection of incoming on surface...
		LTVector vAlignTo = m_vDir + (-2.0f * m_vDir.Dot(m_vSurfaceNormal)) * m_vSurfaceNormal;
		vAlignTo.Normalize();

		LTRotation rHitRot(vAlignTo, LTVector(0.0f, 1.0f, 0.0f));
		rHitRot.Rotate(vAlignTo, GetRandom(0.0f, MATH_CIRCLE));

		rOutgoing = rHitRot;
	}

	bool bCreateOnHitNode = (g_vtCreateImpactFXOnHitNode.GetFloat() && (m_hNodeHit != INVALID_MODEL_NODE) && (m_hObjectHit));
	// Setup the fx struct minus the fxname since it will be the same for each fx.
	CLIENTFX_CREATESTRUCT fxCS( "", 0);
	if( bCreateOnHitNode )
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hObjectHit, m_hNodeHit, tNode, true );

		LTRigidTransform trNode(tNode.m_vPos, tNode.m_rRot);
		LTRigidTransform trOutgoing(tNode.m_vPos, rOutgoing);

		fxCS.m_hParentObject = m_hObjectHit;
		fxCS.m_hNode	  = m_hNodeHit;
		fxCS.m_tTransform = (trNode.GetInverse() * trOutgoing);
	}
	else
	{
		LTRigidTransform trSource( m_vPos, rOutgoing );
/*
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			fxCS.m_tTransform = (trObject.GetInverse( ) * trSource );
		}
		else
*/
		{
			fxCS.m_tTransform = trSource;
		}
	}

	// Create all the fx.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_ToViewer()
//
//	PURPOSE:	Create surface specific TO VIEWER fx impact fx
//
// ----------------------------------------------------------------------- //
void	CWeaponFX::CreateSurfaceSpecificImpactFX_ToViewer(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	if (!hSurf || !hImpact) return;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_ToViewerFX : SrfDB_Imp_sToViewerFX;

	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpact,pszFXAttribName);
	if( !nNumFx )
		return;

	// See if the camera is close enough to create the to viewer fx...
	LTVector const& vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );
	LTVector vAlignTo = vCameraPos - m_vPos;

	float fDistToViewer = vAlignTo.Mag();
	float fRadius = (bUnderWater ? g_pSurfaceDB->GetFloat(hImpact,SrfDB_Imp_fUW_ToViewerRadius) : g_pSurfaceDB->GetFloat(hImpact,SrfDB_Imp_fToViewerRadius));

	// Check if we're outside the radius.
	if (fDistToViewer > fRadius)
		return;

	// Calculate to viewer rotation...
	vAlignTo.Normalize();
	LTRotation rToViewer(vAlignTo, LTVector(0.0f, 1.0f, 0.0f));
	rToViewer.Rotate(vAlignTo, GetRandom(0.0f, MATH_CIRCLE));


	bool bCreateOnHitNode = (g_vtCreateImpactFXOnHitNode.GetFloat() && (m_hNodeHit != INVALID_MODEL_NODE) && (m_hObjectHit));
	// Setup the fx struct minus the fxname since it will be the same for each fx.
	CLIENTFX_CREATESTRUCT fxCS( "", 0);
	if( bCreateOnHitNode )
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hObjectHit, m_hNodeHit, tNode, true );

		LTRigidTransform trNode(tNode.m_vPos, tNode.m_rRot);
		LTRigidTransform trToViewer(tNode.m_vPos, rToViewer);

		fxCS.m_hParentObject = m_hObjectHit;
		fxCS.m_hNode	  = m_hNodeHit;
		fxCS.m_tTransform = (trNode.GetInverse() * trToViewer);
	}
	else
	{
		LTRigidTransform trSource( m_vPos, rToViewer );
/*
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			fxCS.m_tTransform = (trObject.GetInverse( ) * trSource );
		}
		else
*/
		{
			fxCS.m_tTransform = trSource;
		}


	}

	// Create all the fx specified.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
/*
		DebugPrintFXStruct(fxCS,__FUNCTION__);
		DebugCPrint(0,"   m_vPos.x: %0.2f",m_vPos.x);
		DebugCPrint(0,"   m_vPos.y: %0.2f",m_vPos.y);
		DebugCPrint(0,"   m_vPos.z: %0.2f",m_vPos.z);
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			DebugCPrint(0,"   trObject.m_vPos.x: %0.2f",trObject.m_vPos.x);
			DebugCPrint(0,"   trObject.m_vPos.y: %0.2f",trObject.m_vPos.y);
			DebugCPrint(0,"   trObject.m_vPos.z: %0.2f",trObject.m_vPos.z);
		}
*/
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_ToSource()
//
//	PURPOSE:	Create surface specific TO SOURCE fx impact fx
//
// ----------------------------------------------------------------------- //
void	CWeaponFX::CreateSurfaceSpecificImpactFX_ToSource(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	if (!hSurf || !hImpact) return;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_ToSourceFX : SrfDB_Imp_sToSourceFX;

	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpact,pszFXAttribName);
	if( !nNumFx )
		return;

	LTVector vDir = m_vDir;
	if( m_bFXAtFlashSocket )
	{
		// Calculate the original fire direction if m_vDir was modified to come from the flash socket...
		vDir = m_vPos - m_vOriginalFirePos;
		vDir.Normalize( );
	}

	// Calculate to viewer rotation...
	LTVector vAlignTo = -vDir;
	LTRotation rToSource(vAlignTo, LTVector(0.0f, 1.0f, 0.0f));
	rToSource.Rotate(vAlignTo, GetRandom(0.0f, MATH_CIRCLE));

	bool bCreateOnHitNode = (g_vtCreateImpactFXOnHitNode.GetFloat() && (m_hNodeHit != INVALID_MODEL_NODE) && (m_hObjectHit));
	// Setup the fx struct minus the fxname since it will be the same for each fx.
	CLIENTFX_CREATESTRUCT fxCS( "", 0);
	if( bCreateOnHitNode )
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hObjectHit, m_hNodeHit, tNode, true );

		LTRigidTransform trNode(tNode.m_vPos, tNode.m_rRot);
		LTRigidTransform trSource(tNode.m_vPos, rToSource);

		fxCS.m_hParentObject = m_hObjectHit;
		fxCS.m_hNode	  = m_hNodeHit;
		fxCS.m_tTransform = (trNode.GetInverse() * trSource);
	}
	else
	{
		LTRigidTransform trSource( m_vPos, rToSource );
/*
		if (m_hObjectHit)
		{
			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			fxCS.m_tTransform = (trObject.GetInverse( ) * trSource );
		}
		else
*/
		{
			fxCS.m_tTransform = trSource;
		}

	}

	// Create all the fx specified.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_OnSource()
//
//	PURPOSE:	Create surface specific ON SOURCE fx impact fx
//
// ----------------------------------------------------------------------- //
void	CWeaponFX::CreateSurfaceSpecificImpactFX_OnSource(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater)
{
	if (!hSurf || !hImpact) return;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_OnSourceFX : SrfDB_Imp_sOnSourceFX;

	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hImpact,pszFXAttribName);
	if( !nNumFx )
		return;

	LTRigidTransform tImpact;
	tImpact.Init( );

	HOBJECT hFiredFrom = (m_bLocalClientFired ? g_pMoveMgr->GetObject() : m_hFiredFrom);
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;

	// Check the ammo for a socket to create the FX at...
	const char *pszSocket = g_pSurfaceDB->GetString( g_pWeaponDB->GetAmmoData( m_hAmmo ), WDB_AMMO_sSourceImpactSocket );
	if( pszSocket && pszSocket[0] )
	{
		if( g_pModelLT->GetSocket( hFiredFrom, pszSocket, hSocket ) == LT_OK )
		{
			LTTransform tSocket;
			g_pModelLT->GetSocketTransform( hFiredFrom, hSocket, tSocket, false );

			tImpact.m_vPos = tSocket.m_vPos;
			tImpact.m_rRot = tSocket.m_rRot;
		}
	}

	CLIENTFX_CREATESTRUCT fxCS( "", 0, hFiredFrom, tImpact );

	// Create all the fx specified.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificImpactFX_OnSource()
//
//	PURPOSE:	Create surface specific ON SOURCE fx impact fx
//
// ----------------------------------------------------------------------- //
void CWeaponFX::CreateSurfaceSpecificImpactFX_Protruding( HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater )
{
	if( !hSurf || !hImpact )
		return;

	// If the local client is the source don't do the FX.
	if( m_hObjectHit == g_pLTClient->GetClientObject( ))
		return;

	// Use underwater or regular normalfx.
	char const* pszFXAttribName = bUnderWater ? SrfDB_Imp_sUW_ProtrudingFX : SrfDB_Imp_sProtrudingFX;

	// Get the number of fx.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues( hImpact,pszFXAttribName );
	if( !nNumFx )
		return;

	// Calculate protruding rotation...
	LTVector vDir = m_vDir;
	if( m_bFXAtFlashSocket )
	{
		// Calculate the original fire direction if m_vDir was modified to come from the flash socket...
		vDir = m_vPos - m_vOriginalFirePos;
		vDir.Normalize( );
	}

	// Calculate to viewer rotation...
	LTVector vAlignTo = -vDir;
	LTRotation rProtruding( vAlignTo, LTVector(0.0f, 1.0f, 0.0f) );
	rProtruding.Rotate( vAlignTo, GetRandom(0.0f, MATH_CIRCLE) );

	bool bCreateOnHitNode = (g_vtCreateImpactFXOnHitNode.GetFloat() && (m_hNodeHit != INVALID_MODEL_NODE) && (m_hObjectHit));
	// Setup the fx struct minus the fxname since it will be the same for each fx.
	CLIENTFX_CREATESTRUCT fxCS( "", 0, m_hObjectHit );
	if( bCreateOnHitNode )
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform( m_hObjectHit, m_hNodeHit, tNode, true );

		LTRigidTransform trNode(tNode.m_vPos, tNode.m_rRot);
		LTRigidTransform trProtruding(tNode.m_vPos + (vDir * -g_vtProtrudeFXOffset.GetFloat( )), rProtruding);

		fxCS.m_hNode	  = m_hNodeHit;
		fxCS.m_tTransform = (trNode.GetInverse() * trProtruding);
	}
	else
	{
		LTRigidTransform trSource( m_vPos, rProtruding );
		if (m_hObjectHit)
		{
			// Don't do protruding FX for characters if not created on a node.
			uint32 dwUserFlags = 0;
			g_pCommonLT->GetObjectFlags( m_hObjectHit, OFT_User, dwUserFlags );
			if( (dwUserFlags & (USRFLG_CHARACTER | USRFLG_HITBOX)) != 0 )
				return;

			LTRigidTransform trObject;
			g_pLTClient->GetObjectTransform( m_hObjectHit, &trObject );
			fxCS.m_tTransform = (trObject.GetInverse( ) * trSource );
		}
		else
		{
			fxCS.m_tTransform = trSource;
		}


	}

	// Create all the fx.
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		const char *pszFxName = g_pSurfaceDB->GetString( hImpact, pszFXAttribName, nFxIndex );
		if( !pszFxName || !pszFxName[0] )
			continue;

		LTStrCpy(fxCS.m_sName, pszFxName, LTARRAYSIZE(fxCS.m_sName));
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::IsBulletTrailWeapon()
//
//	PURPOSE:	See if this weapon creates bullet trails in liquid
//
// ----------------------------------------------------------------------- //

bool CWeaponFX::IsBulletTrailWeapon()
{
	return (g_pWeaponDB->GetAmmoInstDamageType( m_hAmmo ) == DT_BULLET);
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
	if (m_bLocalClientFired)
	{
		//This is the player we are creating the effect for, so we only need to create
		//the ammo specific effects to give them feedback. So if it exists, create
		//that effect
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
		HRECORD hFireFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFireFX ));
		if( hFireFX && !LTStrEmpty(g_pFXDB->GetString(hFireFX,FXDB_sFXName)) )
		{
			CLIENTFX_CREATESTRUCT	fxcs( g_pFXDB->GetString(hFireFX,FXDB_sFXName), 0, m_hFiredFrom, LTRigidTransform(m_vFirePos, m_rDirRot) );
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxcs, true );
		}
	}
	else
	{
		//This is the AI shooting, we just need to create an effect as specified
		//by the hand held position
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, USE_AI_DATA);
		const char* pszHHMuzzleFxName = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHHMuzzleFX );
		if( pszHHMuzzleFxName[0])
		{
			CLIENTFX_CREATESTRUCT	fxcs( pszHHMuzzleFxName, 0, LTRigidTransform(m_vFirePos, m_rDirRot) );
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxcs, true );
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
	sc.vStartPos	= CalcBreachPos(m_vFirePos,m_bLeftHandWeapon);
	sc.hWeapon		= m_hWeapon;
	sc.hAmmo		= m_hAmmo;
    sc.b3rdPerson	= true;


	// See if this is our local client who fired and if we're in first
	// person...

	if( m_bLocalClientFired &&
		(g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson) )
	{
		sc.b3rdPerson = false;

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
//	ROUTINE:	CWeaponFX::PlayFireSound
//
//	PURPOSE:	Play the fire sound
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayFireSound()
{
	if (m_bLocalClientFired)
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

	::PlayWeaponSound( m_hWeapon, m_bLocalClientFired, m_vFirePos, eSoundId );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CalcFirePos
//
//	PURPOSE:	Calculate the fire position based on the FireFrom object
//
// ----------------------------------------------------------------------- //

LTVector CWeaponFX::CalcFirePos( const LTVector &vFirePos, bool bLeftHand )
{
	LTVector vPos = vFirePos;

	if( m_bLocalClientFired )
	{
		CClientWeapon *pCurWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
		if( pCurWeapon )
		{
			vPos = pCurWeapon->GetFlashPos( );
		}
	}
	else if( m_hFiredFrom )
	{
        LTRotation rRot;

		CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_hFiredFrom);
		if (pCharacter && pCharacter->IsOperatingTurret()) 
		{
			CTurretFX* pTurret = pCharacter->GetTurret();
			GetAttachmentSocketTransform( pTurret->GetTurretWeapon(), "Flash", vPos, rRot );
		}
		else
		{
			if( !GetAttachmentSocketTransform( m_hFiredFrom, (m_bLeftHandWeapon ? "Left_Flash" : "Flash"), vPos, rRot ) )
				GetAttachmentSocketTransform( m_hFiredFrom, (m_bLeftHandWeapon ? "LeftHand" : "RightHand"), vPos, rRot );
		}	
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

LTVector CWeaponFX::CalcBreachPos(const LTVector &vBreachPos, bool bLeftHand)
{
	if (!m_hFiredFrom) return vBreachPos;

	if( m_bFailedToFindBreach ) return vBreachPos;

	// See if this is our local client who fired, and if so
	// only calculate fire position if we are in 3rd person...

	LTRotation rRot;
    LTVector vPos;
	if( m_bLocalClientFired &&
		(g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson) )
	{
		CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
  		if ( pClientWeapon )
  		{
			LTRigidTransform rBreachTransform;
  			pClientWeapon->GetBreachTransform( rBreachTransform );
			return rBreachTransform.m_vPos;
  		}
	}


	if (m_hFiredFrom)
	{
		CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_hFiredFrom);
		if (pCharacter && pCharacter->IsOperatingTurret()) 
		{
			CTurretFX* pTurret = pCharacter->GetTurret();
			if (!GetAttachmentSocketTransform(pTurret->GetTurretWeapon(), "Breach", vPos, rRot))
			{
				// [jeffo 02/24/04] Cache the failure so that we don't search
				// for the socket every time a shell casing is created.
				// This prevents the engine from spamming the console with
				// an error from ILTModel::GetSocket().
				m_bFailedToFindBreach = true;
				vPos = vBreachPos;
			}
		}
		else
		{
			if (!GetAttachmentSocketTransform(m_hFiredFrom, (m_bLeftHandWeapon ? "Left_Breach" : "Breach"), vPos, rRot))
			{
				// [jeffo 02/24/04] Cache the failure so that we don't search
				// for the socket every time a shell casing is created.
				// This prevents the engine from spamming the console with
				// an error from ILTModel::GetSocket().
				m_bFailedToFindBreach = true;
				vPos = vBreachPos;
			}
		}
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
	if( !m_hWeapon || !m_hAmmo )
		return;
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	if( (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ) != VECTOR)
		return;

	// Camera pos
	LTVector vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	// Check if we'll be hearing an impact sound.

	HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sImpactFX ));
  	if( hImpactFX )
  	{
		// The impact sound is considered heard if it's within the factor's soundradius.
		float fFlyByImpactRadiusFactor = g_cvarFlyByImpactRadiusFactor.GetFloat( );
  		LTVector vDist = m_vPos - vPos;
  		if ( vDist.Mag() < (fFlyByImpactRadiusFactor * float(g_pFXDB->GetInt32(hImpactFX,FXDB_nAISoundRadius))) ) 
		{
			// We're going to hear the impact.  We still have a chance
			// to hear the flyby.
			float fFlyByWithImpactChance = g_cvarFlyByWithImpactChance.GetFloat( );
			if( fFlyByWithImpactChance < 1.0f && GetRandom( 0.0f, 1.0f ) >= fFlyByWithImpactChance )
			{
				// Don't hear the flyby.
                return;
			}
		}
  	}


	// See if the camera is close enough to the bullet path to hear the
	// bullet...

	float fRadius = g_cvarFlyByRadius.GetFloat();

	LTVector vDir = m_vDir;

	const LTVector vRelativePos = vPos - m_vFirePos;
    const float fRayDist = vDir.Dot(vRelativePos);
	LTVector vBulletDir = (vDir*fRayDist - vRelativePos);

    const float fDistSqr = vBulletDir.MagSqr();

	if (fDistSqr < fRadius*fRadius)
	{
		// Play the fly by sound bute....
		//vPos += vBulletDir;   // old way of getting pos...

		LTVector vFlyByVel;
		LTVector vMovePos;

		// determine flyby velocity..
		vFlyByVel = vRelativePos;	// direction is towards player from gun
		vFlyByVel.Normalize();		// normalize to get direction only.
		vFlyByVel *= 1000.0f;		// now set velocity (1000 is arbitrary and sounds good)
									// I'm thinking I need a set of general sound properties
									// for the DB that I can put this and other things into..

		vMovePos = vFlyByVel;		// now to move position.. get velocity..
		vMovePos *= -0.25;			// move about 1/4 that backwards so it passes by player..
		vPos += vMovePos;

		HRECORD hFlybysound;
					   
		hFlybysound = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFlyBySound );

		if (hFlybysound)
		{
			g_pClientSoundMgr->PlayDBSoundFromPosWithPath(vPos, vFlyByVel, hFlybysound, 
				SMGR_INVALID_RADIUS, SOUNDPRIORITY_MISC_LOW,
				0, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS);
		}
		else
		{
			// keeping this for a little while until the flyby records are filled out
			// in all the ammo types. Think of it as a hardcoded 'default' until we're
			// sure they're all set. -- Terry
			g_pClientSoundMgr->PlayDBSoundFromPosWithPath(vPos, vFlyByVel, g_pSoundDB->GetSoundDBRecord("BulletFlyBy"), 
				SMGR_INVALID_RADIUS, SOUNDPRIORITY_MISC_LOW,
				0, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS);
		}
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
	if (!IsMultiplayerGameClient()) return;

	CCharacterFX* pCharFX = g_pGameClientShell->GetLocalCharacterFX();
	if (pCharFX)
	{
		pCharFX->PlayDingSound();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::ApplyModelDecal()
//
//	PURPOSE:	Put a decal on an impacted character if appropriate
//
// ----------------------------------------------------------------------- //

void CWeaponFX::ApplyModelDecal()
{
	// Models only
	uint32 nObjectType;
	g_pCommonLT->GetObjectType(m_hObjectHit, &nObjectType);
	if( nObjectType != OT_MODEL )
		return;
		
	// Don't apply decals to the main character
	CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hObjectHit );
	if( pCharacter == g_pGameClientShell->GetLocalCharacterFX() )
		return;

	// Do non-node hits relative to the root node
	HMODELNODE hNodeHit = m_hNodeHit;
	if( hNodeHit == INVALID_MODEL_NODE )
	{
		g_pModelLT->GetRootNode(m_hObjectHit, hNodeHit);
	}

	// Add a decal
	char szNodeName[64];
	ModelsDB::HMODEL hModel = pCharacter ? pCharacter->GetModel() : NULL;
	if (hModel && g_pLTClient->GetModelLT()->GetNodeName(m_hObjectHit, hNodeHit, szNodeName, LTARRAYSIZE(szNodeName)) == LT_OK)
	{
		HRECORD hDamageType = g_pWeaponDB->GetAmmoInstDamageTypeRecord(m_hAmmo);	//!!ARL: Might be a better way to get this rather than assuming inst damage.
		bool bLowGore = !g_pProfileMgr->GetCurrentProfile()->m_bGore;
		HRECORD hDecal = g_pModelsDB->GetDamageSpecificDecalRecord(hModel, szNodeName, hDamageType, bLowGore);
		uint32 nDecalType = g_pModelDecalMgr->GetDecalType(hDecal);
		g_pModelDecalMgr->AddDecal(m_hObjectHit, hNodeHit, nDecalType, m_vPos, m_vDir);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreatePhysicsImpulseForce()
//
//	PURPOSE:	Apply a physics impulse force at the impact position...
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreatePhysicsImpulseForce( )
{
	if( !m_hObjectHit )
		return;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( m_hAmmo );

	// Apply a physical force to the object that was hit...
	float fImpulse = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstDamageImpulseForce );

	// Adjust the amount of force applied based on distance away...
	float fDist = (m_vPos - m_vFirePos).Mag( );
	float fDamageFactor = g_pWeaponDB->GetEffectiveVectorRangeDamageFactor( m_hWeapon, fDist, false );
	fImpulse *= fDamageFactor;

	PhysicsUtilities::ApplyPhysicsImpulseForce( m_hObjectHit, fImpulse, m_vDir, m_vPos, false );
}

// EOF
