// ----------------------------------------------------------------------- //
//
// MODULE  : CSFXMgr.cpp
//
// PURPOSE : Special FX Mgr	- Implementation
//
// CREATED : 10/24/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SFXMgr.h"
#include "SpecialFX.h"
#include "PolyGridFX.h"
#include "RenderTargetFX.h"
#include "RenderTargetGroupFX.h"
#include "MarkSFX.h"
#include "WeaponFX.h"
#include "VolumeBrushFX.h"
#include "ShellCasingFX.h"
#include "CommonUtilities.h"
#include "WeaponFXTypes.h"
#include "CameraFX.h"
#include "ProjectileFX.h"
#include "PickupItemFX.h"
#include "GameClientShell.h"
#include "CharacterFX.h"
#include "PlayerSoundFX.h"
#include "SearchLightFX.h"
#include "ExplosionFX.h"
#include "VarTrack.h"
#include "DebugLineFX.h"
#include "JumpVolumeFX.h"
#include "CMoveMgr.h"
#include "PlayerLureFX.h"
#include "SnowFX.h"
#include "ScatterFX.h"
#include "DynamicSectorVolumeFX.h"
#include "TriggerFX.h"
#include "AimMagnetFX.h"
#include "NavMarkerFX.h"
#include "LadderFX.h"
#include "SpecialMoveFX.h"
#include "FinishingMoveFX.h"
#include "ForensicObjectFX.h"
#include "NavMarkerTypeDB.h"
#include "TurretFX.h"
#include "ClientSoundNonPointFX.h"
#include "VolumetricLightFX.h"
#include "EntryToolLockFX.h"
#include "ScreenEffectFX.h"
#include "ActivateObjectFX.h"
#include "iperformancemonitor.h"
#include "ClientWeaponMgr.h"
#include "CTFFlagBaseSFX.h"
#include "CTFFlagSFX.h"
#include "TeamClientFXSFX.h"
#include "PhysicsConstraintFX.h"
#include "PhysicsCollisionSystemFX.h"
#include "SpecialFXNotifyMessageHandler.h"
#include "ControlPointSFX.h"

//our object used for tracking performance for SFX updating
static CTimedSystem g_tsClientSFXUpdate("GameClient_SFX_Update", "GameClient");

// NOTE:  These indexes should map EXACTLY to the SFX ids defined
// in SFXMsgIds.h...

static unsigned int s_nDynArrayMaxNums[DYN_ARRAY_SIZE] =
{
	50,		// Polygrid
	MAX_RENDERTARGETFX_OBJECTS,	// Render Targets
	MAX_RENDERTARGETGROUP_OBJECTS,	// Render Target Groups
	1,		// Particle trails - never added to sfx mgr
	1,		// Weapons - uses a single static weapon fx
	100,	// Volume brush
	200,	// Shell casings
	1,		// Camera - Unused, it has its own list
	50,		// Projectile
	100,	// Marks - bullet holes
	200,	// Pickup item
	200,	// Character
	20,		// Player sounds
	100,	// Search lights
	100,	// Polygon debris
	100,	// Explosion
	10,		// Sound fx (not currently used)
	MAX_DEBUG_LINE_SYSTEMS,	// Debug line systems
	50,		// Snow
	20,		// JumpVolumes
	5,		// PlayerLure
	1,		// DisplayTimer, doesn't need a list.
	50,		// Dynamic sector volumes
	50,		// Scatter
	50,		// Trigger
	MAX_AIM_MAGNETS, // AutoAim Magnets
	MAX_NAV_MARKERS, // Navigation Markers
	50,		// Ladder
	120,	// SpecialMove
	10,		// FinishingMove
	12,		// ForensicObjects
	20,		// Turret
	50,		// SoundNonPoint
	MAX_VOLUMETRICLIGHTFX_OBJECTS,		// VolumetricLight
	15,		// EntryToolLocks
	1,		// ScreenEffect
	MAX_ACTIVATEOBJECTFX_OBJECTS, // Activate-able Objects
	4,		// CTFFlagBase
	4,		// CTFFlag
	100,	// TeamClientFXSFX
	200,	// Physics Constraints
	100,	// Physics Collision System
	MAX_CONTROLPOINT_OBJECTS,	// ControlPoint
};


static CWeaponFX s_WeaponFX;

// The special FX banks...
CBankedList<CPlayerSoundFX> g_SFXBank_PlayerSound;
CBankedList<CMarkSFX> g_SFXBank_Mark;
CBankedList<CShellCasingFX> g_SFXBank_ShellCasing;
CBankedList<CProjectileFX> g_SFXBank_Projectile;
CBankedList<CCameraFX> g_SFXBank_Camera;
CBankedList<CPolyGridFX> g_SFXBank_PolyGrid;
CBankedList<CRenderTargetFX> g_SFXBank_RenderTarget;
CBankedList<CRenderTargetGroupFX> g_SFXBank_RenderTargetGroup;
CBankedList<CExplosionFX> g_SFXBank_Explosion;
CBankedList<CVolumeBrushFX> g_SFXBank_VolumeBrush;
CBankedList<CCharacterFX> g_SFXBank_Character; // Note : This one's big.  CCharacterFX is 9k
CBankedList<CSearchLightFX> g_SFXBank_SearchLight;
CBankedList<CPickupItemFX> g_SFXBank_PickupItem;
CBankedList<CDebugLineFX> g_SFXBank_DebugLine;
CBankedList<CSnowFX> g_SFXBank_Snow;
CBankedList<CJumpVolumeFX> g_SFXBank_JumpVolume;
CBankedList<CDynamicSectorVolumeFX> g_SFXBank_DynamicSectorVolume;
CBankedList<CScatterFX> g_SFXBank_Scatter;
CBankedList<CTriggerFX> g_SFXBank_Trigger;
CBankedList<CAimMagnetFX> g_SFXBank_AimMagnet;
CBankedList<CNavMarkerFX> g_SFXBank_NavMarker;
CBankedList<CLadderFX> g_SFXBank_Ladder;
CBankedList<CSpecialMoveFX> g_SFXBank_SpecialMove;
CBankedList<CFinishingMoveFX> g_SFXBank_FinishingMove;
CBankedList<CForensicObjectFX> g_SFXBank_ForensicObject;
CBankedList<CTurretFX> g_SFXBank_Turret;
CBankedList<CSoundNonPointFX> g_SFXBank_SoundNonPoint;
CBankedList<CVolumetricLightFX> g_SFXBank_VolumetricLight;
CBankedList<CEntryToolLockFX> g_SFXBank_EntryToolLock;
CBankedList<CScreenEffectFX> g_SFXBank_ScreenEffect;
CBankedList<CActivateObjectFX> g_SFXBank_ActivateObject;
CBankedList<TeamClientFXSFX> g_SFXBank_TeamClientFx;
CBankedList<CPhysicsConstraintFX> g_SFXBank_PhysicsConstraintFX;
CBankedList<CPhysicsCollisionSystemFX> g_SFXBank_PhysicsCollisionSystemFX;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::Init()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

bool CSFXMgr::Init(ILTClient *pClientDE)
{
    if (!g_pLTClient) return false;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		if (!m_dynSFXLists[i].Create(GetDynArrayMaxNum(i))) return false;
	}

	return m_cameraSFXList.Create(CAMERA_LIST_SIZE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::HandleSFXMsg()
//
//	PURPOSE:	Handle a special fx message
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read *pMsg)
{
    uint8 nId = pMsg->Readuint8();

	switch( nId )
	{
		case SFX_WEAPON_ID:
		case SFX_PLAYERLURE_ID:
		case SFX_PROJECTILE_ID:
		case SFX_EXPLOSION_ID:
		case SFX_SEARCHLIGHT_ID:
		case SFX_PICKUPITEM_ID:
		case SFX_CHARACTER_ID:
		case SFX_DEBUGLINE_ID:
		case SFX_JUMPVOLUME_ID:
		case SFX_DYNAMIC_SECTOR_ID:
		case SFX_TRIGGER_ID:
		case SFX_LADDER_ID:
		case SFX_SPECIALMOVE_ID:
		case SFX_FINISHINGMOVE_ID:
		case SFX_FORENSICOBJECT_ID:
		case SFX_TURRET_ID:
		case SFX_CTFFLAGBASE_ID:
		case SFX_CTFFLAG_ID:
		case SFX_CONTROLPOINT_ID:
		case SFX_SOUND_NONPOINT_ID:
		case SFX_ENTRYTOOLLOCK_ID:
		case SFX_SCREENEFFECT_ID:
		case SFX_ACTIVATEOBJECT_ID:
		case SFX_TEAMCLIENTFX_ID:
		case SFX_PHYSICS_CONSTRAINT_ID:
		case SFX_PHYSICS_COLLISION_SYSTEM_ID:
		{
			return CreateSFX( nId, NULL, pMsg, hObj );
		}
		break;

		case SFX_PLAYERSOUND_ID :
		{
			PLAYERSOUNDCREATESTRUCT pscs;

			pscs.hServerObj	= hObj;
			pscs.nType		= pMsg->Readuint8();
			pscs.hWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
			pscs.nClientId	= pMsg->Readuint8();
			pscs.vPos		= pMsg->ReadCompPos();

			return CreateSFX(nId, &pscs);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			VBCREATESTRUCT vb;
			vb.Read(pMsg);
			vb.hServerObj = hObj;

			return CreateSFX(nId, &vb);
		}
		break;

		case SFX_CAMERA_ID :
		{
			CAMCREATESTRUCT cam;

			cam.hServerObj				= hObj;
			cam.bAllowPlayerMovement	= !!pMsg->Readuint8();
			cam.nCameraType				= pMsg->Readuint8();
			cam.bIsListener				= !!pMsg->Readuint8();
			cam.fFovY					= pMsg->Readfloat();
			cam.fFovAspectScale			= pMsg->Readfloat();

			return CreateSFX(nId, &cam);
		}
		break;

		case SFX_POLYGRID_ID :
		{
			PGCREATESTRUCT pg;

			pg.Read(pMsg);
			pg.hServerObj = hObj;

			return CreateSFX(nId, &pg);
		}
		break;

		case SFX_RENDERTARGET_ID :
		{
			RENDERTARGETCREATESTRUCT cs;

			cs.hServerObj = hObj;

			cs.m_nRenderTargetGroupID	= pMsg->Readuint16();
			cs.m_nRenderTargetLOD		= pMsg->Readuint8();

			cs.m_nUpdateFrequency	= pMsg->Readuint16();
			cs.m_nUpdateOffset		= pMsg->Readuint16();

			cs.m_vFOV.x = pMsg->Readfloat();
			cs.m_vFOV.y = pMsg->Readfloat();

			cs.m_bMirror			= pMsg->Readbool();
			cs.m_bRefraction		= pMsg->Readbool();
			cs.m_fRefractionClipPlaneBias = pMsg->Readfloat();

			char pszBuffer[MAX_PATH + 1];
			pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
			cs.m_sMaterial = pszBuffer;

			pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
			cs.m_sParam = pszBuffer;

			return CreateSFX(nId, &cs);
		}
		break;

		case SFX_RENDERTARGETGROUP_ID :
		{
			RENDERTARGETGROUPCREATESTRUCT cs;

			cs.hServerObj = hObj;

			cs.m_nUniqueGroupID = pMsg->Readuint16();

			for(uint32 nCurrDims = 0; nCurrDims < LTARRAYSIZE(cs.m_nDimensions); nCurrDims++)
			{
				cs.m_nDimensions[nCurrDims].x = pMsg->Readuint16();
				cs.m_nDimensions[nCurrDims].y = pMsg->Readuint16();
			}

			cs.m_bCubeMap			= pMsg->Readbool();
			cs.m_bMipMap			= pMsg->Readbool();
			cs.m_bFogVolumes		= pMsg->Readbool();
			cs.m_bLastFrameEffects	= pMsg->Readbool();
			cs.m_bCurrFrameEffects	= pMsg->Readbool();

			return CreateSFX(nId, &cs);
		}
		break;

		case SFX_MARK_ID :
		{
			MARKCREATESTRUCT mark;

			mark.hServerObj				= hObj;
			mark.m_hParent				= pMsg->ReadObject();
			mark.m_tTransform.m_rRot	= pMsg->ReadLTRotation();
			mark.m_tTransform.m_vPos	= pMsg->ReadLTVector();
			mark.hAmmo					= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
			mark.nSurfaceType			= pMsg->Readuint8();

			return CreateSFX(nId, &mark);
		}
		break;

		case SFX_AIMMAGNET_ID :
		{
			AIMMAGNETCREATESTRUCT AimMagnet;

			AimMagnet.hServerObj = hObj;
			AimMagnet.m_nTeamId = pMsg->Readuint8( );
			AimMagnet.m_hTarget = pMsg->ReadObject( );

			return CreateSFX(nId, &AimMagnet);
		}
		break;

		case SFX_NAVMARKER_ID :
		{
			NAVMARKERCREATESTRUCT NavMarker;

			NavMarker.hServerObj = hObj;
			NavMarker.Read(pMsg);
			return CreateSFX(nId, &NavMarker);
		}
		break;

		case SFX_DISPLAYTIMER_ID:
		{
			g_pInterfaceMgr->HandleDisplayTimerMsg( *pMsg );
			return NULL;
		}
		break;

		case SFX_SNOW_ID:
		{
			SNOWCREATESTRUCT snow;

			snow.hServerObj = hObj;

			snow.vDims = pMsg->ReadLTVector();
			snow.fDensity = pMsg->Readfloat();
			snow.fParticleRadius = pMsg->Readfloat();
			snow.fFallRate = pMsg->Readfloat();
			snow.fTumbleRate = pMsg->Readfloat();
			snow.fTumbleRadius = pMsg->Readfloat();
			snow.fMaxDrawDist = pMsg->Readfloat();
			snow.bTranslucent = pMsg->Readbool();
			snow.bTranslucentLight = pMsg->Readbool();
			snow.bBackFaces = pMsg->Readbool();
			snow.nBaseColor = pMsg->Readuint32();

			char pszMaterialBuff[MAX_PATH];
			pMsg->ReadString( pszMaterialBuff, LTARRAYSIZE(pszMaterialBuff) );
			snow.sMaterialName = pszMaterialBuff;

			return CreateSFX( nId, &snow );
		}
		break;

		case SFX_SCATTER_ID:
		{
			SCATTERCREATESTRUCT scatter;

			scatter.hServerObj = hObj;

			scatter.vDims = pMsg->ReadLTVector();
			scatter.nBlindDataIndex = pMsg->Readuint32();
			scatter.fHeight = pMsg->Readfloat();
			scatter.fWidth = pMsg->Readfloat();
			scatter.fMaxScale = pMsg->Readfloat();
			scatter.fWaveRate = pMsg->Readfloat();
			scatter.fWaveDist = pMsg->Readfloat();
			scatter.fMaxDrawDist = pMsg->Readfloat();
			
			char pszMaterialBuff[MAX_PATH];
			pMsg->ReadString( pszMaterialBuff, LTARRAYSIZE(pszMaterialBuff) );
			scatter.sMaterialName = pszMaterialBuff;

			scatter.bTranslucent = pMsg->Readbool();
			scatter.bTranslucentLight = pMsg->Readbool();
			scatter.bBackFaces = pMsg->Readbool();
			scatter.nBaseColor = pMsg->Readuint32();
			scatter.nNumImages = pMsg->Readuint8();

			return CreateSFX( nId, &scatter );
		}
		break;

		case SFX_VOLUMETRICLIGHT_ID:
		{
			VOLUMETRICLIGHTCREATESTRUCT cs;
			cs.hServerObj = hObj;
			cs.Read( pMsg );
			return CreateSFX( nId, &cs );
		}
		break;

		default : break;
	}

	return NULL;
}


void CSFXMgr::DeleteSFX(CSpecialFX* pFX)
{
	// Make sure we've got a valid pointer
	if (!pFX)
		return;

	// Make sure we don't try to delete the static fx(s)...
	if (pFX == &s_WeaponFX)
		return;

	// Alrighty, get the ID from the effect...
	switch(pFX->GetSFXID())
	{
		// Delete it from the right place...

		case SFX_PLAYERSOUND_ID :
		{
			g_SFXBank_PlayerSound.Delete((CPlayerSoundFX*)pFX);
		}
		break;

		case SFX_MARK_ID :
		{
			g_SFXBank_Mark.Delete((CMarkSFX*)pFX);
		}
		break;

		case SFX_SHELLCASING_ID :
		{
			g_SFXBank_ShellCasing.Delete((CShellCasingFX*)pFX);
		}
		break;

		case SFX_PROJECTILE_ID :
		{
			g_SFXBank_Projectile.Delete((CProjectileFX*)pFX);
		}
		break;

		case SFX_CAMERA_ID :
		{
			g_SFXBank_Camera.Delete((CCameraFX*)pFX);
		}
		break;

		case SFX_POLYGRID_ID :
		{
			g_SFXBank_PolyGrid.Delete((CPolyGridFX*)pFX);
		}
		break;

		case SFX_RENDERTARGET_ID :
		{
			g_SFXBank_RenderTarget.Delete((CRenderTargetFX*)pFX);
		}
		break;

		case SFX_RENDERTARGETGROUP_ID :
		{
			g_SFXBank_RenderTargetGroup.Delete((CRenderTargetGroupFX*)pFX);
		}
		break;

		case SFX_EXPLOSION_ID :
		{
			g_SFXBank_Explosion.Delete((CExplosionFX*)pFX);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			g_SFXBank_VolumeBrush.Delete((CVolumeBrushFX*)pFX);
		}
		break;

		case SFX_CHARACTER_ID :
		{
			g_SFXBank_Character.Delete((CCharacterFX*)pFX);
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
			g_SFXBank_SearchLight.Delete((CSearchLightFX*)pFX);
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			g_SFXBank_PickupItem.Delete((CPickupItemFX*)pFX);
		}
		break;

		case SFX_AIMMAGNET_ID :
		{
			g_SFXBank_AimMagnet.Delete((CAimMagnetFX*)pFX);
		}
		break;

		case SFX_NAVMARKER_ID :
		{
			g_SFXBank_NavMarker.Delete((CNavMarkerFX*)pFX);
		}
		break;

		case SFX_LADDER_ID :
		{
			g_SFXBank_Ladder.Delete((CLadderFX*)pFX);
		}
		break;

		case SFX_SPECIALMOVE_ID :
		{
			g_SFXBank_SpecialMove.Delete((CSpecialMoveFX*)pFX);
		}
		break;

		case SFX_FINISHINGMOVE_ID :
		{
			g_SFXBank_FinishingMove.Delete((CFinishingMoveFX*)pFX);
		}
		break;

		case SFX_DEBUGLINE_ID :
		{
			g_SFXBank_DebugLine.Delete((CDebugLineFX*)pFX);
		}
		break;

		case SFX_SNOW_ID :
		{
			g_SFXBank_Snow.Delete((CSnowFX*)pFX);
		}
		break;

		case SFX_JUMPVOLUME_ID :
		{
			g_SFXBank_JumpVolume.Delete((CJumpVolumeFX*)pFX);
		}
		break;

		case SFX_DYNAMIC_SECTOR_ID :
		{
			g_SFXBank_DynamicSectorVolume.Delete((CDynamicSectorVolumeFX*)pFX);
		}
		break;

		case SFX_SCATTER_ID :
		{
			g_SFXBank_Scatter.Delete((CScatterFX*)pFX);
		}
		break;

		case SFX_TRIGGER_ID :
		{
			g_SFXBank_Trigger.Delete((CTriggerFX*)pFX);
		}
		break;

		case SFX_FORENSICOBJECT_ID :
		{
			g_SFXBank_ForensicObject.Delete((CForensicObjectFX*)pFX);
		}
		break;

		case SFX_TURRET_ID:
		{
			g_SFXBank_Turret.Delete( (CTurretFX*)pFX );
		}
		break;

		case SFX_TEAMCLIENTFX_ID:
		{
			g_SFXBank_TeamClientFx.Delete( (TeamClientFXSFX*)pFX );
		}
		break;

		case SFX_CTFFLAGBASE_ID:
		case SFX_CTFFLAG_ID:
		case SFX_CONTROLPOINT_ID:
		{
			delete pFX;
		}
		break;

		case SFX_SOUND_NONPOINT_ID:
		{
			g_SFXBank_SoundNonPoint.Delete( (CSoundNonPointFX*)pFX );
		}
		break;

		case SFX_VOLUMETRICLIGHT_ID:
		{
			g_SFXBank_VolumetricLight.Delete( (CVolumetricLightFX*)pFX );
		}
		break;

		case SFX_ENTRYTOOLLOCK_ID:
		{
			g_SFXBank_EntryToolLock.Delete( (CEntryToolLockFX*)pFX );
		}
		break;

		case SFX_SCREENEFFECT_ID:
		{
			g_SFXBank_ScreenEffect.Delete( (CScreenEffectFX*)pFX );
		}
		break;

		case SFX_ACTIVATEOBJECT_ID :
		{
			g_SFXBank_ActivateObject.Delete( (CActivateObjectFX*)pFX);
		}
		break;

		case SFX_PHYSICS_CONSTRAINT_ID:
		{
			g_SFXBank_PhysicsConstraintFX.Delete( (CPhysicsConstraintFX*)pFX );
		}
		break;

		case SFX_PHYSICS_COLLISION_SYSTEM_ID:
		{
			g_SFXBank_PhysicsCollisionSystemFX.Delete( (CPhysicsCollisionSystemFX*)pFX );
		}
		break;

		// Ok, it's not a banked effect
		default:
		{
			debug_delete(pFX);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::CreateSFX()
//
//	PURPOSE:	Create the special fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct,
							   ILTMessage_Read *pMsg, HOBJECT hServerObj)
{
    CSpecialFX* pSFX = NULL;

	switch (nId)
	{
		case SFX_WEAPON_ID :
		{
			pSFX = &s_WeaponFX;
		}
		break;

		case SFX_PLAYERLURE_ID :
		{
			pSFX = debug_new( PlayerLureFX );
		}
		break;

		case SFX_PLAYERSOUND_ID :
		{
			pSFX = g_SFXBank_PlayerSound.New();
		}
		break;

		case SFX_PROJECTILE_ID :
		{
			pSFX = g_SFXBank_Projectile.New();
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
			pSFX = g_SFXBank_SearchLight.New();
		}
		break;

		case SFX_MARK_ID :
		{
			pSFX = g_SFXBank_Mark.New();
		}
		break;

		case SFX_SHELLCASING_ID :
		{
			pSFX = g_SFXBank_ShellCasing.New();
		}
		break;

		case SFX_CAMERA_ID :
		{
			pSFX = g_SFXBank_Camera.New();
			if (pSFX)
			{
				if (pSFX->Init(psfxCreateStruct))
				{
					if (g_pLTClient->IsConnected())
					{
						if (pSFX->CreateObject(g_pLTClient))
						{
							m_cameraSFXList.Add(pSFX);
						}
						else
						{
							DeleteSFX(pSFX);
                            pSFX = NULL;
						}
					}
					else
					{
						DeleteSFX(pSFX);
                        pSFX = NULL;
					}
				}
			}

			return pSFX;
		}
		break;

		case SFX_POLYGRID_ID :
		{
			pSFX = g_SFXBank_PolyGrid.New();
		}
		break;

		case SFX_RENDERTARGET_ID :
		{
			pSFX = g_SFXBank_RenderTarget.New();
		}
		break;

		case SFX_RENDERTARGETGROUP_ID :
		{
			pSFX = g_SFXBank_RenderTargetGroup.New();
		}
		break;

		case SFX_EXPLOSION_ID :
		{
			pSFX = g_SFXBank_Explosion.New();
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			pSFX = g_SFXBank_VolumeBrush.New();
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			pSFX = g_SFXBank_PickupItem.New();
		}
		break;

		case SFX_AIMMAGNET_ID :
		{
			pSFX = g_SFXBank_AimMagnet.New();
		}
		break;

		case SFX_NAVMARKER_ID :
		{
			pSFX = g_SFXBank_NavMarker.New();
		}
		break;

		case SFX_LADDER_ID :
		{
			pSFX = g_SFXBank_Ladder.New();
		}
		break;

		case SFX_SPECIALMOVE_ID :
		{
			pSFX = g_SFXBank_SpecialMove.New();
		}
		break;

		case SFX_FINISHINGMOVE_ID :
		{
			pSFX = g_SFXBank_FinishingMove.New();
		}
		break;

		case SFX_CHARACTER_ID :
		{
			pSFX = g_SFXBank_Character.New();
		}
		break;

		case SFX_DEBUGLINE_ID:
		{
			pSFX = g_SFXBank_DebugLine.New();
		}
		break;

		case SFX_SNOW_ID:
		{
			pSFX = g_SFXBank_Snow.New();
		}
		break;
 
		case SFX_JUMPVOLUME_ID:
		{
			pSFX = g_SFXBank_JumpVolume.New();
		}
		break;

		case SFX_DYNAMIC_SECTOR_ID:
		{
			pSFX = g_SFXBank_DynamicSectorVolume.New();
		}
		break;

		case SFX_SCATTER_ID:
		{
			pSFX = g_SFXBank_Scatter.New();
		}
		break;

		case SFX_TRIGGER_ID:
		{
			pSFX = g_SFXBank_Trigger.New();
		}
		break;

		case SFX_FORENSICOBJECT_ID :
		{
			pSFX = g_SFXBank_ForensicObject.New();
		}
		break;

		case SFX_TURRET_ID:
		{
			pSFX = g_SFXBank_Turret.New( );
		}
		break;

		case SFX_TEAMCLIENTFX_ID:
		{
			pSFX = g_SFXBank_TeamClientFx.New( );
		}
		break;

		case SFX_CTFFLAGBASE_ID:
		{
			LT_MEM_TRACK_ALLOC(pSFX = new CTFFlagBaseSFX, LT_MEM_TYPE_GAMECODE);
		}
		break;

		case SFX_CTFFLAG_ID:
		{
			LT_MEM_TRACK_ALLOC(pSFX = new CTFFlagSFX, LT_MEM_TYPE_GAMECODE);
		}
		break;

		case SFX_CONTROLPOINT_ID:
		{
			LT_MEM_TRACK_ALLOC(pSFX = new ControlPointSFX, LT_MEM_TYPE_GAMECODE);
		}
		break;

		case SFX_SOUND_NONPOINT_ID :
		{
			pSFX = g_SFXBank_SoundNonPoint.New();
		}
		break;

		case SFX_VOLUMETRICLIGHT_ID :
		{
			pSFX = g_SFXBank_VolumetricLight.New();
		}
		break;

		case SFX_ENTRYTOOLLOCK_ID :
		{
			pSFX = g_SFXBank_EntryToolLock.New();
		}
		break;

		case SFX_SCREENEFFECT_ID :
		{
			pSFX = g_SFXBank_ScreenEffect.New();
		}
		break;

		case SFX_ACTIVATEOBJECT_ID:
		{
			pSFX = g_SFXBank_ActivateObject.New();
		}
		break;

		case SFX_PHYSICS_CONSTRAINT_ID:
		{
			pSFX = g_SFXBank_PhysicsConstraintFX.New( );
		}
		break;

		case SFX_PHYSICS_COLLISION_SYSTEM_ID:
		{
			pSFX = g_SFXBank_PhysicsCollisionSystemFX.New( );
		}
		break;

		default :
            return NULL;
		break;
	}


	// Initialize the sfx, and add it to the appropriate array...

    if (!pSFX) return NULL;


	// First initialize with the create struct...

	if (psfxCreateStruct)
	{
		if (!pSFX->Init(psfxCreateStruct))
		{
			DeleteSFX(pSFX);
            return NULL;
		}
	}
	else if (pMsg)  // Initialize using the hMessage
	{
		if (!pSFX->Init(hServerObj, pMsg))
		{
			DeleteSFX(pSFX);
            return NULL;
		}
	}
	else
	{
		DeleteSFX(pSFX);
        return NULL;
	}

	if (!pSFX->CreateObject(g_pLTClient))
	{
		DeleteSFX(pSFX);
        return NULL;
	}

	if( !AddDynamicSpecialFX(pSFX, nId))
	{
		DeleteSFX( pSFX );
		return NULL;
	}

	return pSFX;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::UpdateSpecialFX()
//
//	PURPOSE:	Update any the special FX
//
// ----------------------------------------------------------------------- //

void CSFXMgr::UpdateSpecialFX()
{
	if( g_pGameClientShell->IsServerPaused( ))
		return;

	// Update dynamic sfx...

	UpdateDynamicSpecialFX();


	// Update camera sfx...

	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i])
		{
			if (!m_cameraSFXList[i]->Update())
			{
				m_cameraSFXList.Remove(m_cameraSFXList[i]);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RenderFX()
//
//	PURPOSE:	Render any the special FX
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RenderFX(HOBJECT hCamera)
{
	// Right now only the DebugLineFX && CCharacterFX need rendering, so just walk thruogh those.
	CSpecialFXList* const pDebugLineList = GetFXList(SFX_DEBUGLINE_ID);

	ASSERT( pDebugLineList );
	if( pDebugLineList )
	{

		CSpecialFXList & debug_lines = *pDebugLineList;

		// Try to save ourselves from going through _every_
		// possible item if we can.
		if( debug_lines.GetNumItems() > 0 )
		{
			// Go through ever possible line system and render the
			// non-null ones.
			const int nSize = debug_lines.GetSize();
			for (int i = 0; i < nSize; ++i )
			{
				if ( debug_lines[i] )
				{
					debug_lines[i]->Render(hCamera);
				}
			}
		}
	}

	CSpecialFXList* const pCharList = GetFXList(SFX_CHARACTER_ID);

	ASSERT( pCharList );
	if( pCharList )
	{

		CSpecialFXList & chars = *pCharList;

		// Try to save ourselves from going through _every_
		// possible item if we can.
		if( chars.GetNumItems() > 0 )
		{
			// Go through ever possible line system and render the
			// non-null ones.
			const int nSize = chars.GetSize();
			for (int i = 0; i < nSize; ++i )
			{
				if ( chars[i] )
				{
					chars[i]->Render(hCamera);
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::UpdateRenderTargets()
//
//	PURPOSE:	this should be called within a begin3d/end3d block to run through 
//				and update all render targets that need to be updated
//
// ----------------------------------------------------------------------- //

void CSFXMgr::UpdateRenderTargets(const LTRigidTransform& tCamera, const LTVector2& vCameraFOV)
{
	//run through all of our render targets and tell them to update
	CSpecialFXList* pRenderTargetList = GetFXList(SFX_RENDERTARGET_ID);
	if(pRenderTargetList && (pRenderTargetList->GetNumItems() > 0))
	{
		//run through the list finding those that are initialized
		for(uint32 nCurrRenderTarget = 0; nCurrRenderTarget < (uint32)pRenderTargetList->GetSize(); nCurrRenderTarget++)
		{
			CSpecialFX* pEffect = (*pRenderTargetList)[nCurrRenderTarget];
			if(pEffect)
			{
				//and now that we have found one, tell it to update
				((CRenderTargetFX*)pEffect)->UpdateRenderTarget(tCamera, vCameraFOV);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::DirtyVisibleRenderTargets()
//
//	PURPOSE:	calling this will dirty all render targets that were visible. This is primarily needed
//				for rendering multiple frames within a single update
//
// ----------------------------------------------------------------------- //

void CSFXMgr::DirtyVisibleRenderTargets()
{
	//run through all of our render targets and tell them to update
	CSpecialFXList* pRenderTargetList = GetFXList(SFX_RENDERTARGET_ID);
	if(pRenderTargetList && (pRenderTargetList->GetNumItems() > 0))
	{
		//run through the list finding those that are initialized
		for(uint32 nCurrRenderTarget = 0; nCurrRenderTarget < (uint32)pRenderTargetList->GetSize(); nCurrRenderTarget++)
		{
			CSpecialFX* pEffect = (*pRenderTargetList)[nCurrRenderTarget];
			if(pEffect)
			{
				//and now that we have found one, tell it to dirty
				((CRenderTargetFX*)pEffect)->DirtyIfVisible();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveSpecialFX()
//
//	PURPOSE:	Remove the specified special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveSpecialFX(HLOCALOBJ hObj)
{
	// Remove the dynamic special fx associated with this object..

	RemoveDynamicSpecialFX(hObj);


	// See if this was a camera...

	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i] && m_cameraSFXList[i]->GetServerObj() == hObj)
		{
			m_cameraSFXList[i]->WantRemove();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveAll()
//
//	PURPOSE:	Remove all the special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveAll()
{
	RemoveAllDynamicSpecialFX();

	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		m_cameraSFXList.Remove(m_cameraSFXList[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::AddDynamicSpecialFX()
//
//	PURPOSE:	Add a dyanamic special fx to our lists
//
// ----------------------------------------------------------------------- //

bool CSFXMgr::AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId)
{
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		return !!m_dynSFXLists[nIndex].Add(pSFX);
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::UpdateDynamicSpecialFX()
//
//	PURPOSE:	Update the dyanamic special fxs
//
// ----------------------------------------------------------------------- //

void CSFXMgr::UpdateDynamicSpecialFX()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientSFXUpdate);

	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		int nNumSFX  = m_dynSFXLists[j].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			CSpecialFX* pSFX = m_dynSFXLists[j][i];
			if( pSFX )
			{
				if (!pSFX->Update())
				{
					m_dynSFXLists[j].Remove(pSFX);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveDynamicSpecialFX()
//
//	PURPOSE:	Remove the specified special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveDynamicSpecialFX(HOBJECT hObj)
{
	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		int nNumSFX  = m_dynSFXLists[j].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			// More than one sfx may have the same server handle, so let them
			// all have an opportunity to remove themselves...

			if (m_dynSFXLists[j][i] && m_dynSFXLists[j][i]->GetServerObj() == hObj)
			{
				m_dynSFXLists[j][i]->WantRemove();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::RemoveAllDynamicSpecialFX()
//
//	PURPOSE:	Remove all dynamic special fx
//
// ----------------------------------------------------------------------- //

void CSFXMgr::RemoveAllDynamicSpecialFX()
{
	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		int nNumSFX  = m_dynSFXLists[j].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			m_dynSFXLists[j].Remove(m_dynSFXLists[j][i]);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::FindSpecialFX()
//
//	PURPOSE:	Find the specified special fx type associated with the
//				object (see SFXMsgIds.h for valid types)
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::FindSpecialFX(uint8 nType, HLOCALOBJ hObj)
{
	if (0 <= nType && nType < DYN_ARRAY_SIZE)
	{
		int nNumSFX  = m_dynSFXLists[nType].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			if (m_dynSFXLists[nType][i] && m_dynSFXLists[nType][i]->GetServerObj() == hObj)
			{
				return m_dynSFXLists[nType][i];
			}
		}
	}

    return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayIndex()
//
//	PURPOSE:	Get the array index associated with a particular type of
//				dynamic special fx
//
// ----------------------------------------------------------------------- //

int CSFXMgr::GetDynArrayIndex(uint8 nFXId)
{
	// All valid fxids should map directly to the array index...If this is
	// an invalid id, use the general fx index (i.e., 0)...

	if (nFXId < 0 || nFXId >= DYN_ARRAY_SIZE)
	{
		return 0;
	}

	return nFXId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayMaxNum()
//
//	PURPOSE:	Find a dyanamic special fx associated with an object
//
// ----------------------------------------------------------------------- //

unsigned int CSFXMgr::GetDynArrayMaxNum(uint8 nIndex)
{
	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		// Use detail setting for bullet holes...

		if (nIndex == SFX_MARK_ID)
		{
			CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
			if (pSettings)
			{
				return int(pSettings->NumBulletHoles() + 1);
			}
		}

		return s_nDynArrayMaxNums[nIndex];
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnObjectRotate()
//
//	PURPOSE:	Handle an object rotation.
//				Some FX may want to modify the rotation.
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnObjectRotate( HOBJECT hObj, bool bTeleport, LTRotation *pNewRot )
{
	if( !hObj || ! pNewRot )
		return;

	// If it's a CharacterFX we might want to modify the rotation...

	CSpecialFX *pFX = FindSpecialFX( SFX_CHARACTER_ID, hObj );
	if( pFX )
	{
		pFX->OnObjectRotate( pNewRot );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnTouchNotify()
//
//	PURPOSE:	Handle client-side touch notify
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	if (!hMain) return;

	// See if this is the move-mgr's object...

	if (hMain == g_pPlayerMgr->GetMoveMgr()->GetObject())
	{
		g_pPlayerMgr->GetMoveMgr()->OnTouchNotify(pInfo, forceMag);
	}
	else
	{
		// Okay see if this is a special fx...

		CSpecialFX* pFX = FindSpecialFX(SFX_PROJECTILE_ID, hMain);

		if (pFX)
		{
			pFX->HandleTouch(pInfo);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnSFXMessage()
//
//	PURPOSE:	Handle server-to-sfx messages
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnSFXMessage(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	uint32 nInitialMsgPos = pMsg->Tell( );

    uint8 nFXType   = pMsg->Readuint8();
	HOBJECT hObj	= pMsg->ReadObject();

	switch( nFXType )
	{
		// Special case for the camera
		case SFX_CAMERA_ID:
		{
			// Find the camera sfx...

			int nNumSFX = m_cameraSFXList.GetSize();

			for( int i = 0; i < nNumSFX; ++i )
			{
				if( m_cameraSFXList[i] && m_cameraSFXList[i]->GetServerObj() == hObj )
				{
					m_cameraSFXList[i]->OnServerMessage( pMsg );
				}
			}

			return;
		}
		break;

		// Special case for the texture FX
		case SFX_DISPLAYTIMER_ID:
		{
			g_pInterfaceMgr->HandleDisplayTimerMsg( *pMsg );
			return;
		}
		break;

	}


	if (0 <= nFXType && nFXType < DYN_ARRAY_SIZE && hObj)
	{
		CSpecialFX* pFX = FindSpecialFX(nFXType, hObj);

		if (pFX)
		{
			pFX->OnServerMessage(pMsg);
		}
		// If the object is waiting to be created, then append the message.
		else
{
			// Need to save the message from the beginning.
			uint32 nCurPos = pMsg->Tell( );
			pMsg->SeekTo( nInitialMsgPos );

			SpecialFXNotifyMessageHandler::Instance().AppendMessage( *pMsg, hObj );

			pMsg->SeekTo( nCurPos );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnSFXMessageOverride()
//
//	PURPOSE:	Handle server-to-sfx messages
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnSFXMessageOverride(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	HOBJECT hObj	= pMsg->ReadObject();

	SpecialFXNotifyMessageHandler::Instance().ChangeMessage( *pMsg, hObj );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnModelKey()
//
//	PURPOSE:	Handle client-side model key
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID hTrackerID)
{
	if (!hObj || !pArgs) return;

	// Only pass these on to the player (and AI)...

	int nNumSFX = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();

    int i;
    for (i=0; i < nNumSFX; i++)
	{
		if (m_dynSFXLists[SFX_CHARACTER_ID][i] && m_dynSFXLists[SFX_CHARACTER_ID][i]->GetServerObj() == hObj)
		{
			m_dynSFXLists[SFX_CHARACTER_ID][i]->OnModelKey(hObj, pArgs, hTrackerID);
		}
	}
}

CCharacterFX* CSFXMgr::GetCharacterFX(HOBJECT hObject)
{
	CCharacterFX* pCharacterFX = NULL;
	int cCharacterFX  = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();

	for ( int iCharacterFX = 0 ; iCharacterFX < cCharacterFX ; iCharacterFX++ )
	{
		pCharacterFX = (CCharacterFX*)m_dynSFXLists[SFX_CHARACTER_ID][iCharacterFX];
		if (pCharacterFX && pCharacterFX->GetServerObj() == hObject)
		{
			return pCharacterFX;
		}
	}

	return NULL;
}


CCharacterFX* CSFXMgr::GetCharacterFromHitBox(HOBJECT hHitBox)
{
	CCharacterFX* pCharacterFX = NULL;
	int cCharacterFX  = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();

	for ( int iCharacterFX = 0 ; iCharacterFX < cCharacterFX ; iCharacterFX++ )
	{
		pCharacterFX = (CCharacterFX*)m_dynSFXLists[SFX_CHARACTER_ID][iCharacterFX];
		if (pCharacterFX)
		{
			if (pCharacterFX->GetHitBox() == hHitBox)
			{
				return pCharacterFX;
			}
		}
	}

	return NULL;
}

CCharacterFX* CSFXMgr::GetCharacterFromClientID(uint32 nClientId)
{
	CCharacterFX* pCharacterFX = NULL;
	int cCharacterFX  = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();

	for ( int iCharacterFX = 0 ; iCharacterFX < cCharacterFX ; iCharacterFX++ )
	{
		pCharacterFX = (CCharacterFX*)m_dynSFXLists[SFX_CHARACTER_ID][iCharacterFX];
		if (pCharacterFX)
		{
			if (pCharacterFX->m_cs.bIsPlayer && pCharacterFX->m_cs.nClientID == nClientId)
			{
				return pCharacterFX;
			}
		}
	}

	return NULL;
}


CLadderFX* CSFXMgr::GetLadderFX(HOBJECT hObject)
{
	CLadderFX* pLadderFX = NULL;
	int cLadderFX  = m_dynSFXLists[SFX_LADDER_ID].GetSize();

	for ( int iLadderFX = 0 ; iLadderFX < cLadderFX ; iLadderFX++ )
	{
		pLadderFX = (CLadderFX*)m_dynSFXLists[SFX_LADDER_ID][iLadderFX];
		if (pLadderFX && pLadderFX->GetServerObj() == hObject)
		{
			return pLadderFX;
		}
	}

	return NULL;
}

CSpecialMoveFX* CSFXMgr::GetSpecialMoveFX(HOBJECT hObject)
{
	CSpecialMoveFX* pSpecialMoveFX = NULL;
	int cSpecialMoveFX  = m_dynSFXLists[SFX_SPECIALMOVE_ID].GetSize();

	for ( int iSpecialMoveFX = 0 ; iSpecialMoveFX < cSpecialMoveFX ; iSpecialMoveFX++ )
	{
		pSpecialMoveFX = (CSpecialMoveFX*)m_dynSFXLists[SFX_SPECIALMOVE_ID][iSpecialMoveFX];
		if (pSpecialMoveFX && pSpecialMoveFX->GetServerObj() == hObject)
		{
			return pSpecialMoveFX;
		}
	}

	CFinishingMoveFX* pFinishingMoveFX = NULL;
	int cFinishingMoveFX  = m_dynSFXLists[SFX_FINISHINGMOVE_ID].GetSize();

	for ( int iFinishingMoveFX = 0 ; iFinishingMoveFX < cFinishingMoveFX ; iFinishingMoveFX++ )
	{
		pFinishingMoveFX = (CFinishingMoveFX*)m_dynSFXLists[SFX_FINISHINGMOVE_ID][iFinishingMoveFX];
		if (pFinishingMoveFX && pFinishingMoveFX->GetServerObj() == hObject)
		{
			return pFinishingMoveFX;
		}
	}

	CEntryToolLockFX* pEntryToolLockFX = NULL;
	int cEntryToolLockFX  = m_dynSFXLists[SFX_ENTRYTOOLLOCK_ID].GetSize();

	for ( int iEntryToolLockFX = 0 ; iEntryToolLockFX < cEntryToolLockFX ; iEntryToolLockFX++ )
	{
		pEntryToolLockFX = (CEntryToolLockFX*)m_dynSFXLists[SFX_ENTRYTOOLLOCK_ID][iEntryToolLockFX];
		if (pEntryToolLockFX && pEntryToolLockFX->GetServerObj() == hObject)
		{
			return pEntryToolLockFX;
		}
	}

	// Check for evidence only if we're holding the proper tool
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		uint8 nActivateType = pWeapon->GetActivationType();
		if (IS_ACTIVATE_FORENSIC(nActivateType))
		{
			CForensicObjectFX* pForensicObjectFX = NULL;
			int cForensicObjectFX  = m_dynSFXLists[SFX_FORENSICOBJECT_ID].GetSize();

			for ( int iForensicObjectFX = 0 ; iForensicObjectFX < cForensicObjectFX ; iForensicObjectFX++ )
			{
				pForensicObjectFX = (CForensicObjectFX*)m_dynSFXLists[SFX_FORENSICOBJECT_ID][iForensicObjectFX];
				if (pForensicObjectFX && pForensicObjectFX->GetServerObj() == hObject)
				{
					return pForensicObjectFX;
				}
			}
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetTurretFX()
//
//	PURPOSE:	If the given object is associated with a TurretFX return the TurretFX...
//
// ----------------------------------------------------------------------- //

CTurretFX* CSFXMgr::GetTurretFX( HOBJECT hObject )
{
	// Don't bother searching if the object is invalid or the list is empty...
	if( !hObject || (m_dynSFXLists[SFX_TURRET_ID].GetNumItems( ) == 0) )
		return NULL;

	CTurretFX *pTurretFX = NULL;
	int cTurretFX = m_dynSFXLists[SFX_TURRET_ID].GetSize( );

	// Search the 
	for( int nTurret = 0; nTurret < cTurretFX; ++nTurret )
	{
		pTurretFX = (CTurretFX*)m_dynSFXLists[SFX_TURRET_ID][nTurret];
		if( pTurretFX && pTurretFX->GetServerObj( ) == hObject )
		{
			// Success...
			return pTurretFX;
		}
	}

	// Not found...
	return NULL;

}

