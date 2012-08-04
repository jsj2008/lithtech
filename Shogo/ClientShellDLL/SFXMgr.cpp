// ----------------------------------------------------------------------- //
//
// MODULE  : CSFXMgr.cpp
//
// PURPOSE : Special FX Mgr	- Implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "SFXMgr.h"
#include "SpecialFX.h"
#include "PolyGridFX.h"
#include "ParticleTrailFX.h"
#include "ParticleSystemFX.h"
#include "MarkSFX.h"
#include "CastLineFX.h"
#include "SparksFX.h"
#include "TracerFX.h"
#include "WeaponFX.h"
#include "DynamicLightFX.h"
#include "ParticleTrailSegmentFX.h"
#include "SmokeFX.h"
#include "BulletTrailFX.h"
#include "VolumeBrushFX.h"
#include "ShellCasingFX.h"
#include "RiotCommonUtilities.h"
#include "WeaponFXTypes.h"
#include "CameraFX.h"
#include "ParticleExplosionFX.h"
#include "SpriteFX.h"
#include "ExplosionFX.h"
#include "DebrisFX.h"
#include "DeathFX.h"
#include "GibFX.h"
#include "ProjectileFX.h"
#include "LightFX.h"
#include "PickupItemFX.h"
#include "CriticalHitFX.h"
#include "RiotClientShell.h"
#include "PlayerFX.h"
#include "LineBallFX.h"
#include "AnimeLineFX.h"
#include "WeaponSoundFX.h"
#include "SFXReg.h"


extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::Init()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSFXMgr::Init(ILTClient *pClientDE)
{
	if (!pClientDE) return LTFALSE;

	m_pClientDE = pClientDE;

	LTBOOL bRet = LTTRUE;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		bRet = m_dynSFXLists[i].Create(GetDynArrayMaxNum(i));
		if (!bRet) return LTFALSE;
	}

	bRet = m_cameraSFXList.Create(CAMERA_LIST_SIZE);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::HandleSFXMsg()
//
//	PURPOSE:	Handle a special fx message
//
// ----------------------------------------------------------------------- //

void CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read* hMessage)
{
	uint16 wColor;

	if (!m_pClientDE) return;

	uint8 nId = hMessage->Readuint8();

	switch(nId)
	{
		case SFX_WEAPON_ID :
		{
			WCREATESTRUCT w;

			w.hServerObj	= hObj;
			w.nWeaponId		= hMessage->Readuint8();
			w.nSurfaceType	= hMessage->Readuint8();
			w.nIgnoreFX 	= hMessage->Readuint8();
			w.nShooterId 	= hMessage->Readuint8();
			w.vFirePos = hMessage->ReadCompPos();
			w.vPos = hMessage->ReadCompPos();
			w.rRot = hMessage->ReadLTRotation();

			CreateSFX(nId, &w);
		}
		break;

		case SFX_WEAPONSOUND_ID :
		{
			WSOUNDCREATESTRUCT w;

			w.hServerObj	= hObj;
			w.nType			= hMessage->Readuint8();
			w.nWeaponId		= hMessage->Readuint8();
			w.vPos			= hMessage->ReadCompPos();
			w.hSound		= hMessage->ReadHString();
			w.nClientId		= hMessage->Readuint8();

			CreateSFX(nId, &w);
		}
		break;

		case SFX_PROJECTILE_ID :
		{
			PROJECTILECREATESTRUCT projectile;

			projectile.hServerObj = hObj;
			projectile.nWeaponId  = hMessage->Readuint8();
			projectile.nShooterId = hMessage->Readuint8();

			CreateSFX(nId, &projectile);
		}
		break;

		case SFX_DEBRIS_ID :
		{
			DEBRISCREATESTRUCT debris;

			debris.hServerObj	= hObj;
			debris.rRot			= hMessage->ReadLTRotation();
			debris.vPos			= hMessage->ReadCompPos();
			debris.vMinVel		= hMessage->ReadCompLTVector();
			debris.vMaxVel		= hMessage->ReadCompLTVector();
			debris.fLifeTime	= hMessage->Readfloat();
			debris.fFadeTime	= hMessage->Readfloat();
			debris.fMinScale	= hMessage->Readfloat();
			debris.fMaxScale	= hMessage->Readfloat();
			debris.nNumDebris	= hMessage->Readuint8();
			debris.nDebrisFlags = hMessage->Readuint8();
			debris.nDebrisType  = hMessage->Readuint8();
			debris.bRotate		= (LTBOOL)hMessage->Readuint8();
			debris.bPlayBounceSound = (LTBOOL)hMessage->Readuint8();
			debris.bPlayExplodeSound = (LTBOOL)hMessage->Readuint8();
			
			CreateSFX(nId, &debris);
		}
		break;

		case SFX_DEATH_ID :
		{
			DEATHCREATESTRUCT d;

			d.hServerObj		= hObj;
			d.nModelId			= hMessage->Readuint8();
			d.nSize				= hMessage->Readuint8();
			d.nDeathType		= hMessage->Readuint8();
			d.nCharacterClass	= hMessage->Readuint8();
			d.vPos				= hMessage->ReadLTVector();
			d.vDir				= hMessage->ReadLTVector();

			CreateSFX(nId, &d);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			VBCREATESTRUCT vb;

			vb.hServerObj	= hObj;
			vb.bFogEnable	= (LTBOOL)hMessage->Readuint8();
			vb.fFogFarZ		= hMessage->Readfloat();
			vb.fFogNearZ	= hMessage->Readfloat();
			wColor = hMessage->Readuint16();
			Color255WordToVector( wColor, &vb.vFogColor);

			CreateSFX(nId, &vb);
		}
		break;

		case SFX_CAMERA_ID :
		{
			CAMCREATESTRUCT cam;

			cam.hServerObj			 = hObj;
			cam.bAllowPlayerMovement = (LTBOOL) hMessage->Readuint8();
			cam.nCameraType			 = hMessage->Readuint8();
			cam.bIsListener			 = (LTBOOL) hMessage->Readuint8();

			CreateSFX(nId, &cam);
		}
		break;

		case SFX_TRACER_ID :
		{
			TRCREATESTRUCT tr;

			tr.hServerObj = hObj;
			tr.vVel			= hMessage->ReadLTVector();
			tr.vStartColor	= hMessage->ReadLTVector();
			tr.vEndColor	= hMessage->ReadLTVector();
			tr.vStartPos	= hMessage->ReadLTVector();
			tr.fStartAlpha	= hMessage->Readfloat();
			tr.fEndAlpha	= hMessage->Readfloat();

			CreateSFX(nId, &tr);
		}
		break;

		case SFX_SPARKS_ID :
		{
			SCREATESTRUCT sp;

			sp.hServerObj = hObj;
			sp.vPos				= hMessage->ReadCompPos();
			sp.vDir				= hMessage->ReadLTVector();
			wColor = hMessage->Readuint16();
			Color255WordToVector( wColor, &sp.vColor1 );
			wColor = hMessage->Readuint16();
			Color255WordToVector( wColor, &sp.vColor2 );
			sp.hstrTexture		= hMessage->ReadHString();
			sp.nSparks			= hMessage->Readuint8();
			sp.fDuration		= hMessage->Readuint16();
			sp.fEmissionRadius	= hMessage->Readfloat();
			sp.fRadius			= ( LTFLOAT )hMessage->Readuint16();

			CreateSFX(nId, &sp);
		}
		break;

		case SFX_CASTLINE_ID :
		{
			CLCREATESTRUCT cl;

			cl.hServerObj = hObj;
			cl.vStartColor	= hMessage->ReadLTVector();
			cl.vEndColor	= hMessage->ReadLTVector();
			cl.fStartAlpha	= hMessage->Readfloat();
			cl.fEndAlpha	= hMessage->Readfloat();

			CreateSFX(nId, &cl);
		}
		break;

		case SFX_POLYGRID_ID :
		{
			PGCREATESTRUCT pg;

			pg.hServerObj = hObj;
			pg.vDims = hMessage->ReadLTVector();
			wColor = hMessage->Readuint16();
			Color255WordToVector(wColor, &(pg.vColor1));
			wColor = hMessage->Readuint16();
			Color255WordToVector(wColor, &(pg.vColor2));
			pg.fXScaleMin = hMessage->Readfloat(); 
			pg.fXScaleMax = hMessage->Readfloat();  
			pg.fYScaleMin = hMessage->Readfloat(); 
			pg.fYScaleMax = hMessage->Readfloat();  
			pg.fXScaleDuration = hMessage->Readfloat(); 
			pg.fYScaleDuration = hMessage->Readfloat(); 
			pg.fXPan = hMessage->Readfloat(); 
			pg.fYPan = hMessage->Readfloat(); 
			pg.fAlpha = hMessage->Readfloat(); 
			pg.hstrSurfaceSprite = hMessage->ReadHString();
			pg.dwNumPolies = (uint32)hMessage->Readuint16();
			pg.nPlasmaType = hMessage->Readuint8();
			pg.nRingRate[0] = hMessage->Readuint8();
			pg.nRingRate[1] = hMessage->Readuint8();
			pg.nRingRate[2] = hMessage->Readuint8();
			pg.nRingRate[3] = hMessage->Readuint8();

			CreateSFX(nId, &pg);
		}
		break;

		case SFX_PARTICLETRAIL_ID :
		{
			PTCREATESTRUCT pt;

			pt.hServerObj = hObj;
			pt.nType  = hMessage->Readuint8();
			pt.bSmall = ( pt.nType & PT_SMALL ) ? LTTRUE : LTFALSE;
			pt.nType &= ~PT_SMALL;

			CreateSFX(nId, &pt);
		}
		break;
		
		case SFX_PARTICLESYSTEM_ID :
		{
			PSCREATESTRUCT ps;

			ps.hServerObj = hObj;
			ps.vColor1			   = hMessage->ReadLTVector();
			ps.vColor2			   = hMessage->ReadLTVector();
			ps.dwFlags			   = (uint32) hMessage->Readfloat();
			ps.fBurstWait		   = hMessage->Readfloat();
			ps.fParticlesPerSecond = hMessage->Readfloat();
			ps.fEmissionRadius	   = hMessage->Readfloat();
			ps.fMinimumVelocity	   = hMessage->Readfloat();
			ps.fMaximumVelocity    = hMessage->Readfloat();
			ps.fVelocityOffset	   = hMessage->Readfloat();
			ps.fParticleLifetime   = hMessage->Readfloat();
			ps.fParticleRadius	   = hMessage->Readfloat();
			ps.fGravity			   = hMessage->Readfloat();
			ps.fRotationVelocity   = hMessage->Readfloat();
			ps.hstrTextureName	   = hMessage->ReadHString();

			CreateSFX(nId, &ps);
		}
		break;

		case SFX_MARK_ID :
		{
			MARKCREATESTRUCT mark;

			mark.hServerObj = hObj;
			mark.m_Rotation = hMessage->ReadLTRotation();
			mark.m_fScale = hMessage->Readfloat();
			mark.m_hstrSprite = hMessage->ReadHString();

			CreateSFX(nId, &mark);
		}
		break;

		case SFX_CRITICALHIT_ID :
		{
			CHCREATESTRUCT ch;

			ch.vPos	= hMessage->ReadCompPos();
			ch.fClientIDHitter = hMessage->Readfloat();
			ch.fClientIDHittee = hMessage->Readfloat();

			CreateSFX(nId, &ch);
		}
		break;

		case SFX_LIGHT_ID :
		{
			LIGHTCREATESTRUCT light;

			light.hServerObj	= hObj;

			light.vColor = hMessage->ReadLTVector();
			light.dwLightFlags = hMessage->Readuint32();
			light.fIntensityMin = hMessage->Readfloat();
			light.fIntensityMax = hMessage->Readfloat();
			light.nIntensityWaveform = hMessage->Readuint8();
			light.fIntensityFreq = hMessage->Readfloat();
			light.fIntensityPhase = hMessage->Readfloat();
			light.fRadiusMin = hMessage->Readfloat();
			light.fRadiusMax = hMessage->Readfloat();
			light.nRadiusWaveform = hMessage->Readuint8();
			light.fRadiusFreq = hMessage->Readfloat();
			light.fRadiusPhase = hMessage->Readfloat();
			light.hstrRampUpSound = hMessage->ReadHString();
			light.hstrRampDownSound = hMessage->ReadHString();

			CreateSFX(nId, &light);
		}
		break;

		case SFX_PICKUPITEM_ID :
		{
			PICKUPITEMCREATESTRUCT pickupitem;

			pickupitem.hServerObj	= hObj;
			CreateSFX(nId, &pickupitem);
		}
		break;

		case SFX_PLAYER_ID :
		{
			PLAYERCREATESTRUCT player;

			player.hServerObj = hObj;
			CreateSFX(nId, &player);
		}
		break;

		case SFX_ANIMELINES_ID:
		{
			ALCREATESTRUCT cs;
			uint8 theAngle;
			LTFLOAT fAngle;
			LTRotation tempRot;
			LTVector up, right;

			cs.m_Pos = hMessage->ReadCompPos();
			
			theAngle = hMessage->Readuint8();
			fAngle = ((LTFLOAT)theAngle / 255.0f) * MATH_CIRCLE;
			m_pClientDE->Math()->SetupEuler(tempRot, 0.0f, fAngle, 0.0f);
			m_pClientDE->Math()->GetRotationVectors(tempRot, up, right, cs.m_DirVec);
			CreateSFX(nId, &cs);
		}
		break;

		case SFX_AUTO_ID:
		{
			CreateAutoSFX(hObj, hMessage);
		}
		break;

		default : break;
	}
}


CSpecialFX* CSFXMgr::CreateAutoSFX(HOBJECT hServerObj, ILTMessage_Read* hMessage)
{
	SFXReg *pReg;
	CAutoSpecialFX *pAuto;
	uint8 sfxID;


	sfxID = hMessage->Readuint8();

	if(pReg = FindSFXReg(sfxID))
	{
		if(pAuto = pReg->m_Fn())
		{
			if(pAuto->InitAuto(hServerObj, hMessage))
			{
				if (!m_pClientDE->IsConnected())
				{
					delete pAuto;
					return LTNULL;
				}

				if(AddDynamicSpecialFX(pAuto, SFX_AUTO_ID))
					return pAuto;
			}
		}
	}
	
	return NULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::CreateSFX()
//
//	PURPOSE:	Create the special fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct)
{
	if (!m_pClientDE) return LTNULL;

	CSpecialFX* pSFX = LTNULL;

	switch(nId)
	{
		case SFX_WEAPON_ID :
		{
			pSFX = new CWeaponFX();
		}
		break;

		case SFX_WEAPONSOUND_ID :
		{
			pSFX = new CWeaponSoundFX();
		}
		break;

		case SFX_PROJECTILE_ID :
		{
			pSFX = new CProjectileFX();
		}
		break;

		case SFX_SPRITE_ID :
		{
			pSFX = new CSpriteFX();
		}
		break;

		case SFX_TRACER_ID :
		{
			pSFX = new CTracerFX();
		}
		break;

		case SFX_EXPLOSION_ID :
		{
			pSFX = new CExplosionFX();
		}
		break;

		case SFX_ANIMELINES_ID:
		{
			pSFX = new CAnimeLineFX();
		}
		break;

		case SFX_LINEBALL_ID :
		{
			pSFX = new CLineBallFX();
		}
		break;

		case SFX_PARTICLETRAIL_ID :
		{
			pSFX = new CParticleTrailFX();
		}
		break;

		case SFX_SPARKS_ID :
		{
			pSFX = new CSparksFX();
		}
		break;

		case SFX_PARTICLETRAILSEG_ID :
		{
			pSFX = new CParticleTrailSegmentFX();
		}
		break;

		case SFX_BULLETTRAIL_ID :
		{
			pSFX = new CBulletTrailFX();
		}
		break;

		case SFX_MARK_ID :
		{
			pSFX = new CMarkSFX();
		}
		break;

		case SFX_SHELLCASING_ID :
		{
			pSFX = new CShellCasingFX();
		}
		break;

		case SFX_PARTICLEEXPLOSION_ID :
		{
			pSFX = new CParticleExplosionFX();
		}
		break;

		case SFX_DEBRIS_ID :
		{
			pSFX = new CDebrisFX();
		}
		break;

		case SFX_CRITICALHIT_ID :
		{
			pSFX = new CCriticalHitFX();
		}
		break;

		case SFX_GIB_ID :
		{
			pSFX = new CGibFX();
		}
		break;

		case SFX_DYNAMICLIGHT_ID :
		{
			pSFX = new CDynamicLightFX();
		}
		break;

		case SFX_SMOKE_ID :
		{
			pSFX = new CSmokeFX();
		}
		break;
		
		case SFX_DEATH_ID :
		{
			pSFX = new CDeathFX();
		}
		break;

		case SFX_CAMERA_ID :
		{
			pSFX = new CCameraFX();
			if (pSFX)
			{
				if (pSFX->Init(psfxCreateStruct))
				{
					if (m_pClientDE->IsConnected())
					{
						if (pSFX->CreateObject(m_pClientDE))
						{
							m_cameraSFXList.Add(pSFX);
						}
						else
						{
							delete pSFX;
							pSFX = LTNULL;
						}
					}
					else
					{
						delete pSFX;
						pSFX = LTNULL;
					}
				}
			}

			return pSFX;
		}
		break;

		case SFX_CASTLINE_ID :
		{
			pSFX = new CCastLineFX();
		}
		break;

		case SFX_POLYGRID_ID :
		{
			pSFX = new CPolyGridFX();
		}
		break;
		
		case SFX_VOLUMEBRUSH_ID :
		{
			pSFX = new CVolumeBrushFX();
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
			pSFX = new CParticleSystemFX();
		}
		break;

		case SFX_LIGHT_ID :
		{
			pSFX = new CLightFX();
		}
		break;

		default : return LTNULL;

		case SFX_PICKUPITEM_ID :
		{
			pSFX = new CPickupItemFX();
		}
		break;

		case SFX_PLAYER_ID :
		{
			pSFX = new CPlayerFX();
		}
		break;
	}


	// Initialize the sfx, and add it to the appropriate array...

	if (!pSFX) return LTNULL;


	if (!pSFX->Init(psfxCreateStruct))
	{
		delete pSFX;
		return LTNULL;
	}

	// Only create the sfx if we are connected to the server...

	if (!m_pClientDE->IsConnected())
	{
		delete pSFX;
		return LTNULL;
	}

	if (!pSFX->CreateObject(m_pClientDE))
	{
		delete pSFX;
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
	if (!m_pClientDE) return;

	LTFLOAT fTime = m_pClientDE->GetTime();

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
	if (!m_pClientDE) return;

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

LTBOOL CSFXMgr::AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId)
{
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		m_dynSFXLists[nIndex].Add(pSFX);
		return LTTRUE;
	}

	return LTFALSE;
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
	if (!m_pClientDE) return;

	LTFLOAT fTime = m_pClientDE->GetTime();

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

int	CSFXMgr::GetDynArrayIndex(uint8 nFXId)
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
		// NOTE:  The indexes should map EXACTLY to the SFX ids defined
		// in SFXMsgIds.h...

		unsigned int s_nDynArrayMaxNums[DYN_ARRAY_SIZE] =
		{
			100,	// General fx
			10,		// Polygrid
			20,		// Particle trails
			20,		// Particle systems
			5,		// Cast line
			20,		// Sparks
			10,		// Tracers
			20,		// Weapons
			10,		// Dynamic lights
			50,		// Particle trail segments
			20,		// Smoke
			20,		// Bullet trail
			15,		// Volume brush
			200,	// Shell casings
			1,		// Camera - Unused, it has its own list
			10,		// Particle explosions
			150,	// Sprites
			10,		// Explosions
			100,	// Debris
			20,		// Death
			50,		// Gibs
			20,		// Projectile
			300,	// Marks - bullet holes
			10,		// Light
			5,		// Critical hits
			1,		// Unused
			10,		// Pickup item
			10,		// Player
			5,		// Line ball
			5,		// Anime lines
			15,		// Weapon sounds
			5		// Auto
		};

		// Use detail setting for bullet holes...

		if (nIndex == SFX_MARK_ID)
		{
			CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
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

void CSFXMgr::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, LTFLOAT forceMag)
{
	if (!hMain) return;

	CSpecialFX* pFX = FindSpecialFX(SFX_PROJECTILE_ID, hMain);

	if (pFX)
	{
		pFX->HandleTouch(pInfo, forceMag);
	}
}
