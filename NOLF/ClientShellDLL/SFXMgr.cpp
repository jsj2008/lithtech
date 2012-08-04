// ----------------------------------------------------------------------- //
//
// MODULE  : CSFXMgr.cpp
//
// PURPOSE : Special FX Mgr	- Implementation
//
// CREATED : 10/24/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
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
#include "VideoFX.h"

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
	50,		// Volume brush
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
	50,		// Pickup item
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
	10,		// Video FX
};

extern CGameClientShell* g_pGameClientShell;

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
CBankedList<CNodeLinesFX> g_SFXBank_NodeLines;
CBankedList<SprinklesFX> g_SFXBank_Sprinkles;
CBankedList<CPlayerVehicleFX> g_SFXBank_PlayerVehicle;
CBankedList<CObjSpriteFX> g_SFXBank_ObjSprite;
CBankedList<CVideoFX> g_SFXBank_Video;

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

    LTBOOL bRet = LTTRUE;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		bRet = m_dynSFXLists[i].Create(GetDynArrayMaxNum(i));
        if (!bRet) return LTFALSE;
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

void CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
    uint8 nId = g_pLTClient->ReadFromMessageByte(hMessage);

	switch (nId)
	{
		case SFX_WEAPON_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_PLAYERSOUND_ID :
		{
			PLAYERSOUNDCREATESTRUCT pscs;

			pscs.hServerObj	= hObj;
			pscs.nType		= g_pLTClient->ReadFromMessageByte(hMessage);
			pscs.nWeaponId	= g_pLTClient->ReadFromMessageByte(hMessage);
			pscs.nClientId	= g_pLTClient->ReadFromMessageByte(hMessage);
			g_pLTClient->ReadFromMessageCompPosition(hMessage, &(pscs.vPos));

			CreateSFX(nId, &pscs);
		}
		break;

		case SFX_PROJECTILE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_MUZZLEFLASH_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_DEBRIS_ID :
		{
			DEBRISCREATESTRUCT debris;

			debris.hServerObj	= hObj;
			g_pLTClient->ReadFromMessageRotation(hMessage, &(debris.rRot));
			g_pLTClient->ReadFromMessageCompPosition(hMessage, &(debris.vPos));
			debris.nDebrisId	= g_pLTClient->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &debris);
		}
		break;

		case SFX_RANDOMSPARKS_ID :
		{
			RANDOMSPARKSCREATESTRUCT rs;

			rs.hServerObj = hObj;
			g_pLTClient->ReadFromMessageCompPosition(hMessage, &(rs.vPos));
			g_pLTClient->ReadFromMessageVector(hMessage, &(rs.vDir));
			rs.nSparks	 = g_pLTClient->ReadFromMessageByte(hMessage);
			rs.fDuration = g_pLTClient->ReadFromMessageWord(hMessage);

			CreateSFX(nId, &rs);
		}
		break;

		case SFX_DEATH_ID :
		{
			DEATHCREATESTRUCT d;

			d.hServerObj	= hObj;
			d.eModelId		= (ModelId)g_pLTClient->ReadFromMessageByte(hMessage);
			d.nDeathType	= g_pLTClient->ReadFromMessageByte(hMessage);
			d.eModelStyle	= (ModelStyle)g_pLTClient->ReadFromMessageByte(hMessage);
			g_pLTClient->ReadFromMessageVector(hMessage, &(d.vPos));
			g_pLTClient->ReadFromMessageVector(hMessage, &(d.vDir));

			CreateSFX(nId, &d);
		}
		break;

		case SFX_EXPLOSION_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			VBCREATESTRUCT vb;
			vb.Read(hMessage);
			vb.hServerObj = hObj;

			CreateSFX(nId, &vb);
		}
		break;

		case SFX_WEATHER_ID :
		{
			WFXCREATESTRUCT weather;

			weather.Read(hMessage);
			weather.hServerObj = hObj;

			CreateSFX(nId, &weather);
		}
		break;

		case SFX_CAMERA_ID :
		{
			CAMCREATESTRUCT cam;

			cam.hServerObj			 = hObj;
            cam.bAllowPlayerMovement = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
			cam.nCameraType			 = g_pLTClient->ReadFromMessageByte(hMessage);
            cam.bIsListener          = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &cam);
		}
		break;

		case SFX_TRACER_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_POLYGRID_ID :
		{
			PGCREATESTRUCT pg;

			pg.hServerObj = hObj;
			g_pLTClient->ReadFromMessageVector(hMessage, &(pg.vDims));
			uint16 wColor = g_pLTClient->ReadFromMessageWord(hMessage);
			Color255WordToVector(wColor, &(pg.vColor1));
			wColor = g_pLTClient->ReadFromMessageWord(hMessage);
			Color255WordToVector(wColor, &(pg.vColor2));
			pg.fXScaleMin = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fXScaleMax = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fYScaleMin = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fYScaleMax = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fXScaleDuration = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fYScaleDuration = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fXPan = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fYPan = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.fAlpha = g_pLTClient->ReadFromMessageFloat(hMessage);
			pg.hstrSurfaceSprite = g_pLTClient->ReadFromMessageHString(hMessage);
            pg.dwNumPolies = (uint32)g_pLTClient->ReadFromMessageWord(hMessage);
            pg.bAdditive = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
            pg.bMultiply = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
			pg.nPlasmaType = g_pLTClient->ReadFromMessageByte(hMessage);
			pg.nRingRate[0] = g_pLTClient->ReadFromMessageByte(hMessage);
			pg.nRingRate[1] = g_pLTClient->ReadFromMessageByte(hMessage);
			pg.nRingRate[2] = g_pLTClient->ReadFromMessageByte(hMessage);
			pg.nRingRate[3] = g_pLTClient->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &pg);
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_MARK_ID :
		{
			MARKCREATESTRUCT mark;

			mark.hServerObj = hObj;
			g_pLTClient->ReadFromMessageRotation(hMessage, &(mark.m_Rotation));
			g_pLTClient->ReadFromMessageVector(hMessage, &(mark.m_vPos));
			mark.m_fScale		= g_pLTClient->ReadFromMessageFloat(hMessage);
			mark.nAmmoId		= g_pLTClient->ReadFromMessageByte(hMessage);
			mark.nSurfaceType	= g_pLTClient->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &mark);
		}
		break;

		case SFX_LIGHTNING_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_POLYLINE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_FIRE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_LENSFLARE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_SEARCHLIGHT_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_LASERTRIGGER_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_MINE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_BEAM_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_LIGHT_ID :
		{
			LIGHTCREATESTRUCT light;

			light.hServerObj	= hObj;

			g_pLTClient->ReadFromMessageVector(hMessage, &light.vColor);
			light.dwLightFlags = g_pLTClient->ReadFromMessageDWord(hMessage);
			light.fIntensityMin = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.fIntensityMax = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.nIntensityWaveform = g_pLTClient->ReadFromMessageByte(hMessage);
			light.fIntensityFreq = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.fIntensityPhase = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.fRadiusMin = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.fRadiusMax = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.nRadiusWaveform = g_pLTClient->ReadFromMessageByte(hMessage);
			light.fRadiusFreq = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.fRadiusPhase = g_pLTClient->ReadFromMessageFloat(hMessage);
			light.hstrRampUpSound = g_pLTClient->ReadFromMessageHString(hMessage);
			light.hstrRampDownSound = g_pLTClient->ReadFromMessageHString(hMessage);
            LTBOOL bUseLightAnim = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
			if(bUseLightAnim)
			{
				light.m_hLightAnim = (HLIGHTANIM)g_pLTClient->ReadFromMessageDWord(hMessage);
			}
			else
			{
				light.m_hLightAnim = INVALID_LIGHT_ANIM;
			}

			CreateSFX(nId, &light);
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			PICKUPITEMCREATESTRUCT pickupitem;

			pickupitem.hServerObj = hObj;
            pickupitem.bRotate = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
            pickupitem.bBounce = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
			CreateSFX(nId, &pickupitem);
		}
		break;

		case SFX_CHARACTER_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_BODY_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_NODELINES_ID :
		{
			NLCREATESTRUCT w;

			w.hServerObj	= hObj;

			g_pLTClient->ReadFromMessageVector(hMessage, &(w.vSource));
			g_pLTClient->ReadFromMessageVector(hMessage, &(w.vDestination));

			CreateSFX(nId, &w);
		}
		break;

		case SFX_SPRINKLES_ID:
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_STEAM_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_PLAYERVEHICLE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_OBJSPRITE_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
		}
		break;

		case SFX_VIDEO_ID :
		{
            CreateSFX(nId, LTNULL, hMessage, hObj);
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

		case SFX_VIDEO_ID :
		{
			g_SFXBank_Video.Delete((CVideoFX*)pFX);
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
							   HMESSAGEREAD hMessage, HOBJECT hServerObj)
{
    CSpecialFX* pSFX = LTNULL;

	switch (nId)
	{
		case SFX_WEAPON_ID :
		{
			pSFX = &s_WeaponFX;
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

		case SFX_VIDEO_ID :
		{
			pSFX = g_SFXBank_Video.New();
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
	else if (hMessage)  // Initialize using the hMessage
	{
		if (!pSFX->Init(hServerObj, hMessage))
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

	AddDynamicSpecialFX(pSFX, nId);

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

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
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

void CSFXMgr::AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId)
{
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		m_dynSFXLists[nIndex].Add(pSFX);
	}

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
//	ROUTINE:	CSFXMgr::OnTouchNotify()
//
//	PURPOSE:	Handle client-side touch notify
//
// ----------------------------------------------------------------------- //

void CSFXMgr::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	if (!hMain) return;

	// See if this is the move-mgr's object...

	if (hMain == g_pGameClientShell->GetMoveMgr()->GetObject())
	{
		g_pGameClientShell->GetMoveMgr()->OnTouchNotify(pInfo, forceMag);
	}
	else
	{
		// Okay see if this is a special fx...

		CSpecialFX* pFX = FindSpecialFX(SFX_PROJECTILE_ID, hMain);

		if (pFX)
		{
			pFX->HandleTouch(pInfo, forceMag);
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

void CSFXMgr::OnSFXMessage(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

    uint8 nFXType   = g_pLTClient->ReadFromMessageByte(hMessage);
	HOBJECT hObj	= g_pLTClient->ReadFromMessageObject(hMessage);

	if (0 <= nFXType && nFXType < DYN_ARRAY_SIZE && hObj)
	{
		CSpecialFX* pFX = FindSpecialFX(nFXType, hObj);

		if (pFX)
		{
			pFX->OnServerMessage(hMessage);
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

