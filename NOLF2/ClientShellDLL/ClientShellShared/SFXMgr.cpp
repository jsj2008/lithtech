// ----------------------------------------------------------------------- //
//
// MODULE  : CSFXMgr.cpp
//
// PURPOSE : Special FX Mgr	- Implementation
//
// CREATED : 10/24/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SFXMgr.h"
#include "SpecialFX.h"
#include "PolyGridFX.h"
#include "ParticleTrailFX.h"
#include "ParticleSystemFX.h"
#include "MarkSFX.h"
#include "ParticleShowerFX.h"
#include "TracerFX.h"
#include "WeaponFX.h"
#include "DynamicLightFX.h"
#include "ParticleTrailSegmentFX.h"
#include "SmokeFX.h"
#include "BulletTrailFX.h"
#include "VolumeBrushFX.h"
#include "ShellCasingFX.h"
#include "CommonUtilities.h"
#include "WeaponFXTypes.h"
#include "CameraFX.h"
#include "ParticleExplosionFX.h"
#include "DebrisFX.h"
#include "DeathFX.h"
#include "GibFX.h"
#include "ProjectileFX.h"
#include "LightFX.h"
#include "PickupItemFX.h"
#include "DoomsDayPieceFX.h"
#include "GadgetTargetFX.h"
#include "GameClientShell.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "PlayerSoundFX.h"
#include "RandomSparksFX.h"
#include "NodeLinesFX.h"
#include "WeatherFX.h"
#include "LightningFX.h"
#include "sprinklesfx.h"
#include "FireFX.h"
#include "LensFlareFX.h"
#include "MuzzleFlashFX.h"
#include "SearchLightFX.h"
#include "PolyDebrisFX.h"
#include "SteamFX.h"
#include "BaseScaleFX.h"
#include "ExplosionFX.h"
#include "PolyLineFX.h"
#include "LaserTriggerFX.h"
#include "MineFX.h"
#include "BeamFX.h"
#include "PlayerVehicleFX.h"
#include "ObjSpriteFX.h"
#include "VarTrack.h"
#include "DebugLineFX.h"
#include "JumpVolumeFX.h"
#include "CMoveMgr.h"
#include "PlayerLureFX.h"
#include "SnowFX.h"
#include "ScatterFX.h"
#include "DynamicOccluderVolumeFX.h"
#include "TriggerFX.h"
#include "RadarObjectFX.h"
#include "ActivateObjectFX.h"

// NOTE:  These indexes should map EXACTLY to the SFX ids defined
// in SFXMsgIds.h...

static unsigned int s_nDynArrayMaxNums[DYN_ARRAY_SIZE] =
{
	10,		// General fx (Unused?)
	50,		// Polygrid
	1,		// Particle trails - never added to sfx mgr
	50,		// Particle systems
	50,		// Particle shower
	50,		// Tracers
	1,		// Weapons - uses a single static weapon fx
	50,		// Dynamic lights
	50,		// Particle trail segments
	50,		// Smoke
	50,		// Bullet trail
	100,	// Volume brush
	200,	// Shell casings
	1,		// Camera - Unused, it has its own list
	20,		// Particle explosions
	200,	// Sprites/Models (base scale)
	100,	// Debris
	50,		// Death
	50,		// Gibs
	50,		// Projectile
	100,	// Marks - bullet holes
	100,	// Light
	50,		// Random sparks
	200,	// Pickup item
	200,	// Character
	20,		// Player sounds
	1000,	// Node Lines (used for AI nodes)
	100,	// Weather
	50,		// Lightning
	10,		// Sprinkles
	100,	// Fire
	50,		// Lens Flares
	20,		// Muzzle flash
	100,	// Search lights
	100,	// Polygon debris
	100,	// Steam,
	100,	// Explosion
	200,	// PolyLine
	200,	// Body
	100,	// Laser trigger
	100,	// Mines
	50,		// Beams
	10,		// Player vehicles
	10,		// Sound fx (not currently used)
	10,		// Objective sprites
	1,		// Light groups
	500,     // Debug line systems
	1,		// Texture FX, has its own list
	50,		// Snow
	20,		// JumpVolumes
	5,		// PlayerLure
	50,		// GadgetTargets
	1,		// DisplayTimer, doesn't need a list.
	50,		// Dynamic Occluder volumes
	50,		// Scatter
	50,		// Trigger
	20,		// RadarObject
	512,	// ActivateObject
	3,		// DoomsDay pieces
};


static CWeaponFX s_WeaponFX;

// The special FX banks...
CBankedList<CMuzzleFlashFX> g_SFXBank_MuzzleFlash;
CBankedList<CPlayerSoundFX> g_SFXBank_PlayerSound;
CBankedList<CBaseScaleFX> g_SFXBank_BaseScale;
CBankedList<CParticleShowerFX> g_SFXBank_ParticleShower;
CBankedList<CMarkSFX> g_SFXBank_Mark;
CBankedList<CShellCasingFX> g_SFXBank_ShellCasing;
CBankedList<CTracerFX> g_SFXBank_Tracer;
CBankedList<CPolygonDebrisFX> g_SFXBank_PolygonDebris;
CBankedList<CProjectileFX> g_SFXBank_Projectile;
CBankedList<CBeamFX> g_SFXBank_Beam;
CBankedList<CParticleTrailSegmentFX> g_SFXBank_ParticleTrailSegment;
CBankedList<CDebrisFX> g_SFXBank_Debris;
CBankedList<CDynamicLightFX> g_SFXBank_DynamicLight;
CBankedList<CFireFX> g_SFXBank_Fire;
CBankedList<CLensFlareFX> g_SFXBank_LensFlare;
CBankedList<CSmokeFX> g_SFXBank_Smoke;
CBankedList<CCameraFX> g_SFXBank_Camera;
CBankedList<CPolyGridFX> g_SFXBank_PolyGrid;
CBankedList<CExplosionFX> g_SFXBank_Explosion;
CBankedList<CVolumeBrushFX> g_SFXBank_VolumeBrush;
CBankedList<CLightFX> g_SFXBank_Light;
CBankedList<CSteamFX> g_SFXBank_Steam;
CBankedList<CBodyFX> g_SFXBank_Body;
CBankedList<CCharacterFX> g_SFXBank_Character; // Note : This one's big.  CCharacterFX is 9k
CBankedList<CLightningFX> g_SFXBank_Lightning;
CBankedList<CPolyLineFX> g_SFXBank_PolyLine;
CBankedList<CSearchLightFX> g_SFXBank_SearchLight;
CBankedList<CLaserTriggerFX> g_SFXBank_LaserTrigger;
CBankedList<CMineFX> g_SFXBank_Mine;
CBankedList<CRandomSparksFX> g_SFXBank_RandomSparks;
CBankedList<CBulletTrailFX> g_SFXBank_BulletTrail;
CBankedList<CParticleExplosionFX> g_SFXBank_ParticleExplosion;
CBankedList<CGibFX> g_SFXBank_Gib;
CBankedList<CDeathFX> g_SFXBank_Death;
CBankedList<CWeatherFX> g_SFXBank_Weather;
CBankedList<CParticleSystemFX> g_SFXBank_ParticleSystem;
CBankedList<CPickupItemFX> g_SFXBank_PickupItem;
CBankedList<CGadgetTargetFX> g_SFXBank_GadgetTarget;
CBankedList<CNodeLinesFX> g_SFXBank_NodeLines;
CBankedList<SprinklesFX> g_SFXBank_Sprinkles;
CBankedList<CPlayerVehicleFX> g_SFXBank_PlayerVehicle;
CBankedList<CObjSpriteFX> g_SFXBank_ObjSprite;
CBankedList<CDebugLineFX> g_SFXBank_DebugLine;
CBankedList<CSnowFX> g_SFXBank_Snow;
CBankedList<CJumpVolumeFX> g_SFXBank_JumpVolume;
CBankedList<CDynamicOccluderVolumeFX> g_SFXBank_DynamicOccluderVolume;
CBankedList<CScatterFX> g_SFXBank_Scatter;
CBankedList<CTriggerFX> g_SFXBank_Trigger;
CBankedList<CRadarObjectFX> g_SFXBank_RadarObject;
CBankedList<CActivateObjectFX> g_SFXBank_ActivateObject;
CBankedList<CDoomsdayPieceFX> g_SFXBank_DoomsdayPiece;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::Init()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSFXMgr::Init(ILTClient *pClientDE)
{
    if (!g_pLTClient) return LTFALSE;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		if (!m_dynSFXLists[i].Create(GetDynArrayMaxNum(i))) return LTFALSE;
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

void CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read *pMsg)
{
    uint8 nId = pMsg->Readuint8();

	switch (nId)
	{
		case SFX_WEAPON_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_PLAYERLURE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_PLAYERSOUND_ID :
		{
			PLAYERSOUNDCREATESTRUCT pscs;

			pscs.hServerObj	= hObj;
			pscs.nType		= pMsg->Readuint8();
			pscs.nWeaponId	= pMsg->Readuint8();
			pscs.nClientId	= pMsg->Readuint8();
			pscs.vPos		= pMsg->ReadCompPos();

			CreateSFX(nId, &pscs);
		}
		break;

		case SFX_PROJECTILE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_MUZZLEFLASH_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_DEBRIS_ID :
		{
			DEBRISCREATESTRUCT debris;

			debris.hServerObj	= hObj;
			debris.rRot			= pMsg->ReadLTRotation();
			debris.vPos			= pMsg->ReadCompPos();
			debris.nDebrisId	= pMsg->Readuint8();

			CreateSFX(nId, &debris);
		}
		break;

		case SFX_RANDOMSPARKS_ID :
		{
			RANDOMSPARKSCREATESTRUCT rs;

			rs.hServerObj	= hObj;
			rs.vPos			= pMsg->ReadCompPos();
			rs.vDir			= pMsg->ReadLTVector();
			rs.nSparks		= pMsg->Readuint8();
			rs.fDuration	= pMsg->Readuint16();

			CreateSFX(nId, &rs);
		}
		break;

		case SFX_DEATH_ID :
		{
			DEATHCREATESTRUCT d;

			d.hServerObj	= hObj;
			d.eModelId		= (ModelId)pMsg->Readuint8();
			d.nDeathType	= pMsg->Readuint8();
			d.vPos			= pMsg->ReadLTVector();
			d.vDir			= pMsg->ReadLTVector();

			CreateSFX(nId, &d);
		}
		break;

		case SFX_EXPLOSION_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			VBCREATESTRUCT vb;
			vb.Read(pMsg);
			vb.hServerObj = hObj;

			CreateSFX(nId, &vb);
		}
		break;

		case SFX_WEATHER_ID :
		{
			WFXCREATESTRUCT weather;

			weather.Read(pMsg);
			weather.hServerObj = hObj;

			CreateSFX(nId, &weather);
		}
		break;

		case SFX_CAMERA_ID :
		{
			CAMCREATESTRUCT cam;

			cam.hServerObj			 = hObj;
            cam.bAllowPlayerMovement = (LTBOOL) pMsg->Readuint8();
			cam.nCameraType			 = pMsg->Readuint8();
            cam.bIsListener          = (LTBOOL) pMsg->Readuint8();
			cam.fFovX				 = pMsg->Readfloat();
			cam.fFovY				 = pMsg->Readfloat();

			CreateSFX(nId, &cam);
		}
		break;

		case SFX_TRACER_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_POLYGRID_ID :
		{
			PGCREATESTRUCT pg;

			pg.hServerObj = hObj;

			pg.vDims = pMsg->ReadLTVector();
			uint16 wColor = pMsg->Readuint16();
			Color255WordToVector(wColor, &(pg.vColor1));
			wColor = pMsg->Readuint16();
			Color255WordToVector(wColor, &(pg.vColor2));
			pg.fXScaleMin = pMsg->Readfloat();
			pg.fXScaleMax = pMsg->Readfloat();
			pg.fYScaleMin = pMsg->Readfloat();
			pg.fYScaleMax = pMsg->Readfloat();
			pg.fXScaleDuration = pMsg->Readfloat();
			pg.fYScaleDuration = pMsg->Readfloat();
			pg.fXPan = pMsg->Readfloat();
			pg.fYPan = pMsg->Readfloat();
			pg.fAlpha = pMsg->Readfloat();
			pg.fTimeScale = pMsg->Readfloat();
			pg.fDampenScale = pMsg->Readfloat();
			pg.fSpringCoeff	= pMsg->Readfloat();
			pg.fModelDisplace = pMsg->Readfloat();
			pg.fMinFrameRate = pMsg->Readfloat();
			pg.fBaseReflection = pMsg->Readfloat();
			pg.fVolumeIOR = pMsg->Readfloat();

			char szString[256];
			pMsg->ReadString( szString, ARRAY_LEN( szString ));
			pg.sSurfaceSprite = szString;

			pMsg->ReadString( szString, ARRAY_LEN( szString ));
			pg.sSurfaceEnvMap = szString;

			pMsg->ReadString( szString, ARRAY_LEN( szString ));
			pg.sDampenImage = szString;

            pg.dwNumPoliesX = (uint32)pMsg->Readuint16();
			pg.dwNumPoliesY = (uint32)pMsg->Readuint16();
			pg.nNumStartupFrames = (uint32)pMsg->Readuint16();
            pg.bAdditive = (LTBOOL)pMsg->Readbool();
            pg.bMultiply = (LTBOOL)pMsg->Readbool();
			pg.bFresnel = (LTBOOL)pMsg->Readbool();
			pg.bBackfaceCull = (LTBOOL)pMsg->Readbool();
			pg.bRenderEarly = (LTBOOL)pMsg->Readbool();
			pg.bNormalMapSprite = (LTBOOL)pMsg->Readbool();
			pg.nPlasmaType = pMsg->Readuint8();
			pg.nRingRate[0] = pMsg->Readuint8();
			pg.nRingRate[1] = pMsg->Readuint8();
			pg.nRingRate[2] = pMsg->Readuint8();
			pg.nRingRate[3] = pMsg->Readuint8();

			//read in our modifier data
			pg.nActiveModifiers = pMsg->Readuint8();
			for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
			{
				pg.fAccelAmount[nCurrMod] = pMsg->Readfloat();
				pg.nNumAccelPoints[nCurrMod] = pMsg->Readuint16();				
				pg.nXMin[nCurrMod] = pMsg->Readuint16();
				pg.nYMin[nCurrMod] = pMsg->Readuint16();
				pg.nXMax[nCurrMod] = pMsg->Readuint16();
				pg.nYMax[nCurrMod] = pMsg->Readuint16();
			}

			pg.hVolumeBrush = pMsg->ReadObject();

			CreateSFX(nId, &pg);
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_MARK_ID :
		{
			MARKCREATESTRUCT mark;

			mark.hServerObj = hObj;
			mark.m_Rotation		= pMsg->ReadLTRotation();
			mark.m_vPos			= pMsg->ReadLTVector();
			mark.m_fScale		= pMsg->Readfloat();
			mark.nAmmoId		= pMsg->Readuint8();
			mark.nSurfaceType	= pMsg->Readuint8();

			CreateSFX(nId, &mark);
		}
		break;

		case SFX_LIGHTNING_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_POLYLINE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_FIRE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_LENSFLARE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_LASERTRIGGER_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_MINE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_BEAM_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_LIGHT_ID :
		{
			LIGHTCREATESTRUCT light;

			light.hServerObj	= hObj;

			light.vColor = pMsg->ReadLTVector();
			light.dwLightFlags = pMsg->Readuint32();
			light.fIntensityMin = pMsg->Readfloat();
			light.fIntensityMax = pMsg->Readfloat();
			light.nIntensityWaveform = pMsg->Readuint8();
			light.fIntensityFreq = pMsg->Readfloat();
			light.fIntensityPhase = pMsg->Readfloat();
			light.fRadiusMin = pMsg->Readfloat();
			light.fRadiusMax = pMsg->Readfloat();
			light.nRadiusWaveform = pMsg->Readuint8();
			light.fRadiusFreq = pMsg->Readfloat();
			light.fRadiusPhase = pMsg->Readfloat();
			light.hstrRampUpSound = pMsg->ReadHString();
			light.hstrRampDownSound = pMsg->ReadHString();

			CreateSFX(nId, &light);
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			PICKUPITEMCREATESTRUCT pickupitem;

			pickupitem.hServerObj = hObj;
            pickupitem.bRotate = (LTBOOL)pMsg->Readbool();
            pickupitem.bBounce = (LTBOOL)pMsg->Readbool();
			
			char szTemp[256] = {0};
			pMsg->ReadString( szTemp, ARRAY_LEN(szTemp) );
			pickupitem.sClientFX = szTemp;

			pickupitem.m_nTeamId = pMsg->Readuint8( );

			CreateSFX(nId, &pickupitem);
		}
		break;

		case SFX_DOOMSDAYPIECE_ID :
		{
			DOOMSDAYPIECECREATESTRUCT DoomsdayPieceCS;

			DoomsdayPieceCS.hServerObj = hObj;
            DoomsdayPieceCS.eType = (DDPieceType)pMsg->Readuint8();
			DoomsdayPieceCS.bCarried = pMsg->Readbool();
			DoomsdayPieceCS.nTeam = pMsg->Readuint8();
			DoomsdayPieceCS.bPlanted = pMsg->Readbool();

			CreateSFX(nId, &DoomsdayPieceCS);
		}
		break;

		case SFX_GADGETTARGET_ID :
		{
			GADGETTARGETCREATESTRUCT GadgetTarget;

			GadgetTarget.hServerObj = hObj;
            GadgetTarget.eType = (GadgetTargetType)pMsg->Readuint8();
			GadgetTarget.bSwitchWeapons = pMsg->Readbool();
			GadgetTarget.bPowerOn = pMsg->Readbool();
			GadgetTarget.nTeamID = pMsg->Readuint8();
			CreateSFX(nId, &GadgetTarget);
		}
		break;

		case SFX_CHARACTER_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_BODY_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_NODELINES_ID :
		{
			NLCREATESTRUCT w;

			w.hServerObj	= hObj;

			w.vSource		= pMsg->ReadLTVector();
			w.vDestination	= pMsg->ReadLTVector();

			CreateSFX(nId, &w);
		}
		break;

		case SFX_SPRINKLES_ID:
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_STEAM_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_PLAYERVEHICLE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_OBJSPRITE_ID :
		{
            CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_LIGHTGROUP_ID :
		{
			m_cLightGroupFXMgr.HandleSFXMsg(pMsg);
		}
		break;
		
		case SFX_DEBUGLINE_ID:
		{
			CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_TEXTUREFX_ID:
		{
			m_cTextureFXMgr.HandleSFXMsg(pMsg);
		}
		break;

		case SFX_DISPLAYTIMER_ID:
		{
			g_pInterfaceMgr->HandleDisplayTimerMsg( *pMsg );
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
			snow.vAmbientColor = pMsg->ReadLTVector();
			snow.bUseLighting = pMsg->Readuint8() > 0;
			snow.bUseSaturate = pMsg->Readuint8() > 0;
			snow.hstrTextureName = pMsg->ReadHString();

			CreateSFX( nId, &snow );
		}
		break;

		case SFX_JUMPVOLUME_ID:
		{
			CreateSFX(nId, LTNULL, pMsg, hObj);
		}
		break;

		case SFX_DYNAMIC_OCCLUDER_ID:
		{
			CreateSFX(nId, LTNULL, pMsg, hObj);
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
			scatter.fTilt = pMsg->Readfloat();
			scatter.fWaveRate = pMsg->Readfloat();
			scatter.fWaveDist = pMsg->Readfloat();
			scatter.fMaxDrawDist = pMsg->Readfloat();
			scatter.hstrTextureName = pMsg->ReadHString();
			scatter.bUseSaturate = pMsg->Readuint8() > 0;

			CreateSFX( nId, &scatter );
		}
		break;
		
		case SFX_TRIGGER_ID:
		{
			CreateSFX( nId, LTNULL, pMsg, hObj );
		}
		break;

		case SFX_RADAROBJECT_ID:
		{
			CreateSFX( nId, LTNULL, pMsg, hObj );
		}
		break;

		case SFX_ACTIVATEOBJECT_ID:
		{
			CreateSFX( nId, LTNULL, pMsg, hObj );
		}
		break;

		default : break;
	}
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

		case SFX_MUZZLEFLASH_ID :
		{
			g_SFXBank_MuzzleFlash.Delete((CMuzzleFlashFX*)pFX);
		}
		break;

		case SFX_SCALE_ID :
		{
			g_SFXBank_BaseScale.Delete((CBaseScaleFX*)pFX);
		}
		break;

		case SFX_PARTICLESHOWER_ID :
		{
			g_SFXBank_ParticleShower.Delete((CParticleShowerFX*)pFX);
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

		case SFX_TRACER_ID :
		{
			g_SFXBank_Tracer.Delete((CTracerFX*)pFX);
		}
		break;

		case SFX_POLYDEBRIS_ID :
		{
			g_SFXBank_PolygonDebris.Delete((CPolygonDebrisFX*)pFX);
		}
		break;

		case SFX_PROJECTILE_ID :
		{
			g_SFXBank_Projectile.Delete((CProjectileFX*)pFX);
		}
		break;

		case SFX_BEAM_ID :
		{
			g_SFXBank_Beam.Delete((CBeamFX*)pFX);
		}
		break;

		case SFX_PARTICLETRAILSEG_ID :
		{
			g_SFXBank_ParticleTrailSegment.Delete((CParticleTrailSegmentFX*)pFX);
		}
		break;

		case SFX_DEBRIS_ID:
		{
			g_SFXBank_Debris.Delete((CDebrisFX*)pFX);
		}
		break;

		case SFX_DYNAMICLIGHT_ID:
		{
			g_SFXBank_DynamicLight.Delete((CDynamicLightFX*)pFX);
		}
		break;

		case SFX_FIRE_ID :
		{
			g_SFXBank_Fire.Delete((CFireFX*)pFX);
		}
		break;

		case SFX_LENSFLARE_ID :
		{
			g_SFXBank_LensFlare.Delete((CLensFlareFX*)pFX);
		}
		break;

		case SFX_SMOKE_ID :
		{
			g_SFXBank_Smoke.Delete((CSmokeFX*)pFX);
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

		case SFX_LIGHT_ID :
		{
			g_SFXBank_Light.Delete((CLightFX*)pFX);
		}
		break;

		case SFX_STEAM_ID :
		{
			g_SFXBank_Steam.Delete((CSteamFX*)pFX);
		}
		break;

		case SFX_BODY_ID :
		{
			g_SFXBank_Body.Delete((CBodyFX*)pFX);
		}
		break;

		case SFX_CHARACTER_ID :
		{
			g_SFXBank_Character.Delete((CCharacterFX*)pFX);
		}
		break;

		case SFX_LIGHTNING_ID :
		{
			g_SFXBank_Lightning.Delete((CLightningFX*)pFX);
		}
		break;

		case SFX_POLYLINE_ID :
		{
			g_SFXBank_PolyLine.Delete((CPolyLineFX*)pFX);
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
			g_SFXBank_SearchLight.Delete((CSearchLightFX*)pFX);
		}
		break;

		case SFX_LASERTRIGGER_ID :
		{
			g_SFXBank_LaserTrigger.Delete((CLaserTriggerFX*)pFX);
		}
		break;

		case SFX_MINE_ID :
		{
			g_SFXBank_Mine.Delete((CMineFX*)pFX);
		}
		break;

		case SFX_RANDOMSPARKS_ID :
		{
			g_SFXBank_RandomSparks.Delete((CRandomSparksFX*)pFX);
		}
		break;

		case SFX_BULLETTRAIL_ID :
		{
			g_SFXBank_BulletTrail.Delete((CBulletTrailFX*)pFX);
		}
		break;

		case SFX_PARTICLEEXPLOSION_ID :
		{
			g_SFXBank_ParticleExplosion.Delete((CParticleExplosionFX*)pFX);
		}
		break;

		case SFX_GIB_ID :
		{
			g_SFXBank_Gib.Delete((CGibFX*)pFX);
		}
		break;

		case SFX_DEATH_ID :
		{
			g_SFXBank_Death.Delete((CDeathFX*)pFX);
		}
		break;

		case SFX_WEATHER_ID :
		{
			g_SFXBank_Weather.Delete((CWeatherFX*)pFX);
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
			g_SFXBank_ParticleSystem.Delete((CParticleSystemFX*)pFX);
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			g_SFXBank_PickupItem.Delete((CPickupItemFX*)pFX);
		}
		break;

		case SFX_DOOMSDAYPIECE_ID :
		{
			g_SFXBank_DoomsdayPiece.Delete((CDoomsdayPieceFX*)pFX);
		}
		break;

		case SFX_GADGETTARGET_ID :
		{
			g_SFXBank_GadgetTarget.Delete((CGadgetTargetFX*)pFX);
		}
		break;

		case SFX_NODELINES_ID :
		{
			g_SFXBank_NodeLines.Delete((CNodeLinesFX*)pFX);
		}
		break;

		case SFX_SPRINKLES_ID :
		{
			g_SFXBank_Sprinkles.Delete((SprinklesFX*)pFX);
		}
		break;

		case SFX_PLAYERVEHICLE_ID :
		{
			g_SFXBank_PlayerVehicle.Delete((CPlayerVehicleFX*)pFX);
		}
		break;

		case SFX_OBJSPRITE_ID :
		{
			g_SFXBank_ObjSprite.Delete((CObjSpriteFX*)pFX);
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

		case SFX_DYNAMIC_OCCLUDER_ID :
		{
			g_SFXBank_DynamicOccluderVolume.Delete((CDynamicOccluderVolumeFX*)pFX);
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

		case SFX_RADAROBJECT_ID :
		{
			g_SFXBank_RadarObject.Delete((CRadarObjectFX*)pFX);
		}
		break;

		case SFX_ACTIVATEOBJECT_ID :
		{
			g_SFXBank_ActivateObject.Delete((CActivateObjectFX*)pFX);
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
    CSpecialFX* pSFX = LTNULL;

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

		case SFX_MUZZLEFLASH_ID :
		{
			pSFX = g_SFXBank_MuzzleFlash.New();
		}
		break;

		case SFX_SCALE_ID :
		{
			pSFX = g_SFXBank_BaseScale.New();
		}
		break;

		case SFX_TRACER_ID :
		{
			pSFX = g_SFXBank_Tracer.New();
		}
		break;

		case SFX_LIGHTNING_ID :
		{
			pSFX = g_SFXBank_Lightning.New();
		}
		break;

		case SFX_POLYLINE_ID :
		{
			pSFX = g_SFXBank_PolyLine.New();
		}
		break;

		case SFX_FIRE_ID :
		{
			pSFX = g_SFXBank_Fire.New();
		}
		break;

		case SFX_LENSFLARE_ID :
		{
			pSFX = g_SFXBank_LensFlare.New();
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
			pSFX = g_SFXBank_SearchLight.New();
		}
		break;

		case SFX_LASERTRIGGER_ID :
		{
			pSFX = g_SFXBank_LaserTrigger.New();
		}
		break;

		case SFX_MINE_ID :
		{
			pSFX = g_SFXBank_Mine.New();
		}
		break;

		case SFX_BEAM_ID :
		{
			pSFX = g_SFXBank_Beam.New();
		}
		break;

		case SFX_PARTICLESHOWER_ID :
		{
			pSFX = g_SFXBank_ParticleShower.New();
		}
		break;

		case SFX_RANDOMSPARKS_ID :
		{
			pSFX = g_SFXBank_RandomSparks.New();
		}
		break;

		case SFX_PARTICLETRAILSEG_ID :
		{
			pSFX = g_SFXBank_ParticleTrailSegment.New();
		}
		break;

		case SFX_BULLETTRAIL_ID :
		{
			pSFX = g_SFXBank_BulletTrail.New();
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

		case SFX_PARTICLEEXPLOSION_ID :
		{
			pSFX = g_SFXBank_ParticleExplosion.New();
		}
		break;

		case SFX_DEBRIS_ID :
		{
			pSFX = g_SFXBank_Debris.New();
		}
		break;

		case SFX_POLYDEBRIS_ID :
		{
			pSFX = g_SFXBank_PolygonDebris.New();
		}
		break;

		case SFX_GIB_ID :
		{
			pSFX = g_SFXBank_Gib.New();
		}
		break;

		case SFX_DYNAMICLIGHT_ID :
		{
			pSFX = g_SFXBank_DynamicLight.New();
		}
		break;

		case SFX_SMOKE_ID :
		{
			pSFX = g_SFXBank_Smoke.New();
		}
		break;

		case SFX_DEATH_ID :
		{
			pSFX = g_SFXBank_Death.New();
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
                            pSFX = LTNULL;
						}
					}
					else
					{
						DeleteSFX(pSFX);
                        pSFX = LTNULL;
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

		case SFX_WEATHER_ID :
		{
			pSFX = g_SFXBank_Weather.New();
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
			pSFX = g_SFXBank_ParticleSystem.New();
		}
		break;

		case SFX_LIGHT_ID :
		{
			pSFX = g_SFXBank_Light.New();
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			pSFX = g_SFXBank_PickupItem.New();
		}
		break;

		case SFX_DOOMSDAYPIECE_ID :
		{
			pSFX = g_SFXBank_DoomsdayPiece.New();
		}
		break;

		case SFX_GADGETTARGET_ID :
		{
			pSFX = g_SFXBank_GadgetTarget.New();
		}
		break;

		case SFX_CHARACTER_ID :
		{
			pSFX = g_SFXBank_Character.New();
		}
		break;

		case SFX_BODY_ID :
		{
			pSFX = g_SFXBank_Body.New();
		}
		break;

		case SFX_NODELINES_ID :
		{
			pSFX = g_SFXBank_NodeLines.New();
		}
		break;

		case SFX_SPRINKLES_ID:
		{
			pSFX = g_SFXBank_Sprinkles.New();
		}
		break;

		case SFX_STEAM_ID:
		{
			pSFX = g_SFXBank_Steam.New();
		}
		break;

		case SFX_PLAYERVEHICLE_ID:
		{
			pSFX = g_SFXBank_PlayerVehicle.New();
		}
		break;

		case SFX_OBJSPRITE_ID:
		{
			pSFX = g_SFXBank_ObjSprite.New();
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

		case SFX_DYNAMIC_OCCLUDER_ID:
		{
			pSFX = g_SFXBank_DynamicOccluderVolume.New();
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

		case SFX_RADAROBJECT_ID:
		{
			pSFX = g_SFXBank_RadarObject.New();
		}
		break;

		case SFX_ACTIVATEOBJECT_ID:
		{
			pSFX = g_SFXBank_ActivateObject.New();
		}
		break;

		default :
            return LTNULL;
		break;
	}


	// Initialize the sfx, and add it to the appropriate array...

    if (!pSFX) return LTNULL;


	// First initialize with the create struct...

	if (psfxCreateStruct)
	{
		if (!pSFX->Init(psfxCreateStruct))
		{
			DeleteSFX(pSFX);
            return LTNULL;
		}
	}
	else if (pMsg)  // Initialize using the hMessage
	{
		if (!pSFX->Init(hServerObj, pMsg))
		{
			DeleteSFX(pSFX);
            return LTNULL;
		}
	}
	else
	{
		DeleteSFX(pSFX);
        return LTNULL;
	}

	if (!pSFX->CreateObject(g_pLTClient))
	{
		DeleteSFX(pSFX);
        return LTNULL;
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
    LTFLOAT fTime = g_pLTClient->GetTime();

	// Update the lightgroups
	m_cLightGroupFXMgr.Update();

	// Update dynamic sfx...

	UpdateDynamicSpecialFX();


	// Update camera sfx...

	int nNumSFX = m_cameraSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i])
		{
			if (fTime >= m_cameraSFXList[i]->m_fNextUpdateTime)
			{
				if (!m_cameraSFXList[i]->Update())
				{
					m_cameraSFXList.Remove(m_cameraSFXList[i]);
				}
				else
				{
					m_cameraSFXList[i]->m_fNextUpdateTime = fTime + m_cameraSFXList[i]->GetUpdateDelta();
				}
			}
		}
	}


	// Make sure the global canvase is near the camera...

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (hCamera)
	{
		HOBJECT hObj = CBasePolyDrawFX::GetGlobalCanvaseObj();
		if (hObj)
		{
            LTVector vPos;
			g_pLTClient->GetObjectPos(hCamera, &vPos);
			g_pLTClient->SetObjectPos(hObj, &vPos);
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

	m_cLightGroupFXMgr.Clear();
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
    LTFLOAT fTime = g_pLTClient->GetTime();

	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		int nNumSFX  = m_dynSFXLists[j].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			if (m_dynSFXLists[j][i])
			{
				if (fTime >= m_dynSFXLists[j][i]->m_fNextUpdateTime)
				{
					if (!m_dynSFXLists[j][i]->Update())
					{
						m_dynSFXLists[j].Remove(m_dynSFXLists[j][i]);
					}
					else
					{
						m_dynSFXLists[j][i]->m_fNextUpdateTime = fTime + m_dynSFXLists[j][i]->GetUpdateDelta();
					}
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

    return LTNULL;
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

    uint8 nFXType   = pMsg->Readuint8();
	HOBJECT hObj	= pMsg->ReadObject();

	// Special case for the lightgroups
	switch( nFXType )
	{
		case SFX_LIGHTGROUP_ID:
		{
			m_cLightGroupFXMgr.HandleSFXMsg(pMsg);
			return;
		}
		break;

		// Special case for the texture FX
		case SFX_TEXTUREFX_ID:
		{
			m_cTextureFXMgr.HandleSFXMsg(pMsg);
			return;
		}
		break;

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
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::OnModelKey()
//
//	PURPOSE:	Handle client-side model key
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs) return;

	// Only pass these on to the player (and AI)...

	int nNumSFX = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();

    int i;
    for (i=0; i < nNumSFX; i++)
	{
		if (m_dynSFXLists[SFX_CHARACTER_ID][i] && m_dynSFXLists[SFX_CHARACTER_ID][i]->GetServerObj() == hObj)
		{
			m_dynSFXLists[SFX_CHARACTER_ID][i]->OnModelKey(hObj, pArgs);
		}
	}

	nNumSFX = m_dynSFXLists[SFX_BODY_ID].GetSize();

	for (i=0; i < nNumSFX; i++)
	{
		if (m_dynSFXLists[SFX_BODY_ID][i] && m_dynSFXLists[SFX_BODY_ID][i]->GetServerObj() == hObj)
		{
			m_dynSFXLists[SFX_BODY_ID][i]->OnModelKey(hObj, pArgs);
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

	return LTNULL;
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

	return LTNULL;
}


CBodyFX* CSFXMgr::GetBodyFX(HOBJECT hObject)
{
	CBodyFX* pBodyFX = NULL;
	int cBodyFX  = m_dynSFXLists[SFX_BODY_ID].GetSize();

	for ( int iBodyFX = 0 ; iBodyFX < cBodyFX ; iBodyFX++ )
	{
		pBodyFX = (CBodyFX*)m_dynSFXLists[SFX_BODY_ID][iBodyFX];
		if (pBodyFX && pBodyFX->GetServerObj() == hObject)
		{
			return pBodyFX;
		}
	}

	return LTNULL;
}


CBodyFX* CSFXMgr::GetBodyFromHitBox(HOBJECT hHitBox)
{
	CBodyFX* pBodyFX = NULL;
	int cBodyFX  = m_dynSFXLists[SFX_BODY_ID].GetSize();

	for ( int iBodyFX = 0 ; iBodyFX < cBodyFX ; iBodyFX++ )
	{
		pBodyFX = (CBodyFX*)m_dynSFXLists[SFX_BODY_ID][iBodyFX];
		if (pBodyFX)
		{
			if (pBodyFX->GetHitBox() == hHitBox)
			{
				return pBodyFX;
			}
		}
	}

	return LTNULL;
}

#ifdef __PSX2
int CSFXMgr::GetCharactersInRadius(const LTVector& vecOrigin, float fRadius, CPtrList &lstResult,
									LTBOOL bSearchGood, LTBOOL bSearchNeutral, LTBOOL bSearchBad)
{
	int count = 0;
	LTVector vDims;
	LTVector vecObject;
	float fVal;
	LTVector vRadiusBoxMin, vRadiusBoxMax, vObjBoxMin, vObjBoxMax, vTestBoxMin, vTestBoxMax;
	LTVector vPts[8];
	int i,nChar;
	LTVector vecDiff;
	vRadiusBoxMin.Init( fRadius, fRadius, fRadius );
	vRadiusBoxMax = vecOrigin + vRadiusBoxMin;
	vRadiusBoxMin = vecOrigin - vRadiusBoxMin;
	BOOL bOK;

	CCharacterFX* pChar;
	int nNumSFX  = m_dynSFXLists[SFX_CHARACTER_ID].GetSize();
	for (nChar=0; nChar < nNumSFX; nChar++)
	{
		pChar = (CCharacterFX*)m_dynSFXLists[SFX_CHARACTER_ID][nChar];
		if (pChar && !pChar->m_cs.bIsPlayer)
		{
			// Check alignment
			bOK = FALSE;
			if(bSearchBad)
			{
				if(pChar->m_cs.eCrosshairCharacterClass == BAD)
				{
					bOK = TRUE;
				}
			}
			if(bSearchNeutral)
			{
				if(pChar->m_cs.eCrosshairCharacterClass == NEUTRAL)
				{
					bOK = TRUE;
				}
			}
			if(bSearchGood)
			{
				if(pChar->m_cs.eCrosshairCharacterClass == GOOD)
				{
					bOK = TRUE;
				}
			}

			if(bOK)
			{
				g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vecObject);
				g_pPhysicsLT->GetObjectDims(pChar->GetServerObj(), &vDims);
				vDims.x = 23;
				vDims.y = 54;
				vDims.z = 23;

				vObjBoxMin = vecObject - vDims;
				vObjBoxMax = vecObject + vDims;

				// Check if boxes intersect.
				if( BoxesIntersect( vRadiusBoxMin, vRadiusBoxMax, vObjBoxMin, vObjBoxMax ))
				{
					// Get the intersecting box.
					VEC_MIN( vTestBoxMax, vRadiusBoxMax, vObjBoxMin );
					VEC_MAX( vTestBoxMin, vRadiusBoxMin, vObjBoxMax );

					// Consider the object to be centered on the intersecting box.
					vDims = vTestBoxMax - vTestBoxMin;
					vecObject = vDims + vTestBoxMin;
					vDims *= 0.5f;

					// Check each point to see if it's in the radius.
					SetupBoxPoints( vPts, vecObject, vDims );
					for( i = 0; i < 8; i++ )
					{
						vecDiff = vPts[i] - vecOrigin;
						fVal = fRadius + vDims.Mag( );
						if( vecDiff.MagSqr( ) < fVal * fVal )
						{
							// Add the object to the list
							lstResult.AddTail(pChar);

							++count;
							break;
						}
					}
				}
			}
		}
	}
	
	return count;
}

int CSFXMgr::GetCharactersInCone(const LTVector& vecOrigin, const LTVector& vecForward, float fAngle, float fDist, CPtrList &lstResult,
									LTBOOL bSearchGood, LTBOOL bSearchNeutral, LTBOOL bSearchBad)
{
	// convert angle to radians
	float cosOfAngle, radAngle;


	radAngle = (float)(fAngle * MATH_PI / 180); //angle;
	
	// divide by 2 because we're taking the half angle left or right of the forward vector
	cosOfAngle = (float)cos(radAngle/2); //angle / 2.0f;
	
	POSITION pos, nextPos;

	if( GetCharactersInRadius( vecOrigin, fDist, lstResult, bSearchGood, bSearchNeutral, bSearchBad ))
	{
		CCharacterFX* pChar;

		float fOffset = 300.0f;
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("AutoTargetOffset");
		if(hVar)
		{
			fOffset = g_pLTClient->GetVarValueFloat(hVar);
		}
		
		LTVector vNewOrigin = vecOrigin + (vecForward * -fOffset);

		pos = lstResult.GetHeadPosition( );
		while( pos )
		{
			nextPos = pos;
			pChar = (CCharacterFX*)lstResult.GetNext( nextPos );

			LTVector vecTargetPos;
			g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vecTargetPos);
			LTVector vecDiff = (vecTargetPos - LTVector(0.0f, -32.0f, 0.0f)) - vNewOrigin;

			LTVector vecTmpForward = vecForward;
			vecDiff.Normalize( );

			if (vecTmpForward.Dot(vecDiff) < cosOfAngle)
				lstResult.RemoveAt( pos );

			pos = nextPos;
		}
		/*while( pos )
		{
			nextPos = pos;
			pChar = (CCharacterFX*)lstResult.GetNext( nextPos );

			LTVector vecTargetPos;
			g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vecTargetPos);
			LTVector vecDiff = vecTargetPos - vecOrigin;

			fMagSqr = vecDiff.MagSqr();
			if(fMagSqr > 10000.0f)
			{
				LTVector vecTmpForward = vecForward;
				vecDiff.Normalize( );
	
				if (vecTmpForward.Dot(vecDiff) < cosOfAngle)
					lstResult.RemoveAt( pos );
			}
			else
			{
				// They're close, make sure they're in front of us
				
				float fSign = vecForward.Dot(vecDiff);
				if (fSign < 0.0f)
				{
					// They were behind us, throw em out
					lstResult.RemoveAt(pos);
				}
			}

			pos = nextPos;
		}*/
	}

	return lstResult.GetCount();
}
#endif
