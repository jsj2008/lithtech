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
#include "SmokeTrailFX.h"
#include "ParticleSystemFX.h"
#include "SFXMsgIds.h"
#include "MarkSFX.h"
#include "CastLineFX.h"
#include "TracerFX.h"
#include "RainFX.h"
#include "SparksFX.h"
//#include "BulletImpactSFX.h"
#include "SmokeTrailSegmentFX.h"
#include "VolumeBrushFX.h"
#include "WeaponFX.h"
#include "bloodtrailfx.h"
#include "bloodtrailsegmentfx.h"
#include "SmokeFX.h"
#include "SmokePuffFX.h"
#include "SmokeImpactFX.h"
#include "SplashFX.h"
#include "RippleFX.h"
#include "SurfaceFragmentFX.h"
#include "ExplosionFX.h"
#include "CameraFX.h"
#include "LightFX.h"
#include "WeaponPowerupFX.h"
#include "ParticleStreamFX.h"
#include "ClientServerShared.h"
#include "bloodsplatfx.h"
#include "ParticleExplosionFX.h"
#include "LightningFX.h"
#include "LightningSegmentFX.h"
#include "LaserBeamFX.h"
#include "WarpGateFX.h"
#include "gibfx.h"
#include "debugline.h"
#include "ObjectFX.h"
#include "ShellCasingFX.h"
#include "PickupObjectFX.h"
#include "FlashlightFX.h"
#include "IKChainFX.h"
#include "BloodclientShell.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::Init()
//
//	PURPOSE:	Init the CSFXMgr
//
// ----------------------------------------------------------------------- //

DBOOL CSFXMgr::Init(CClientDE *pClientDE)
{
	if (!pClientDE) return DFALSE;
	m_pClientDE = pClientDE;

	DBOOL bRet = DTRUE;

	for (int i=0; i < DYN_ARRAY_SIZE; i++)
	{
		bRet = m_dynSFXLists[i].Create(GetDynArrayMaxNum(i));
		if (!bRet) return DFALSE;
	}

	bRet = m_staticSFXList.Create(STATIC_LIST_SIZE);
	bRet = m_cameraSFXList.Create(CAMERA_LIST_SIZE);
	bRet = m_termSFXList.Create(STATIC_LIST_SIZE);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::HandleSFXMsg()
//
//	PURPOSE:	Handle a special fx message
//
// ----------------------------------------------------------------------- //

void CSFXMgr::HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage, DBYTE nId)
{
	if (!m_pClientDE) return;

	D_WORD wColor;

	switch(nId)
	{
		case SFX_POLYGRID_ID :
		{
			PGCREATESTRUCT pg;

			pg.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(pg.vDims));
			wColor = m_pClientDE->ReadFromMessageWord(hMessage);
			Color255WordToVector(wColor, &(pg.vColor1));
			wColor = m_pClientDE->ReadFromMessageWord(hMessage);
			Color255WordToVector(wColor, &(pg.vColor2));
			pg.fXScaleMin = m_pClientDE->ReadFromMessageFloat(hMessage); 
			pg.fXScaleMax = m_pClientDE->ReadFromMessageFloat(hMessage); 
			pg.fYScaleMin = m_pClientDE->ReadFromMessageFloat(hMessage); 
			pg.fYScaleMax = m_pClientDE->ReadFromMessageFloat(hMessage); 
			pg.fXScaleDuration = m_pClientDE->ReadFromMessageFloat(hMessage);
			pg.fYScaleDuration = m_pClientDE->ReadFromMessageFloat(hMessage);
			pg.fXPan = m_pClientDE->ReadFromMessageFloat(hMessage);
			pg.fYPan = m_pClientDE->ReadFromMessageFloat(hMessage);
			pg.fAlpha = m_pClientDE->ReadFromMessageFloat(hMessage);
			pg.hstrSurfaceSprite = m_pClientDE->ReadFromMessageHString(hMessage);
			pg.dwNumPolies = (DDWORD)m_pClientDE->ReadFromMessageWord(hMessage);
			pg.nPlasmaType = m_pClientDE->ReadFromMessageByte(hMessage);
			pg.nRingRate[0] = m_pClientDE->ReadFromMessageByte(hMessage);
			pg.nRingRate[1] = m_pClientDE->ReadFromMessageByte(hMessage);
			pg.nRingRate[2] = m_pClientDE->ReadFromMessageByte(hMessage);
			pg.nRingRate[3] = m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &pg);
		}
		break;


		case SFX_SMOKETRAIL_ID :
		{
			STCREATESTRUCT st;

			st.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(st.vVel));
			st.bSmall = (DBOOL)  m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &st);
		}
		break;

		//SCHLEGZ 5/21/98 1:42:10 PM: new blood trail effect
		case SFX_BLOODTRAIL_ID :
		{
			BTCREATESTRUCT bt;

			bt.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(bt.vVel));
			m_pClientDE->ReadFromMessageVector(hMessage, &(bt.vColor));			

			CreateSFX(nId, &bt);
		}
		break;

		case SFX_BLOODSPLAT_ID:
		{
			BSCREATESTRUCT splat;
			
			splat.hServerObj = hObj;
			m_pClientDE->ReadFromMessageCompPosition(hMessage, &splat.m_Pos);
			m_pClientDE->ReadFromMessageRotation(hMessage, &(splat.m_Rotation));
			splat.m_fScale = m_pClientDE->ReadFromMessageFloat(hMessage);
			splat.m_fGrowScale = m_pClientDE->ReadFromMessageFloat(hMessage);
			splat.m_hstrSprite = DNULL;

			CreateSFX(nId, &splat);
		}
		break;

		case SFX_GIB_ID:
		{
			GIBCREATESTRUCT gib;

			gib.hServerObj = hObj;
			m_pClientDE->ReadFromMessageCompPosition(hMessage, &(gib.m_Pos));
			m_pClientDE->ReadFromMessageCompVector(hMessage, &(gib.m_Dir));
			m_pClientDE->ReadFromMessageCompVector(hMessage, &(gib.m_Dims));
			gib.m_dwFlags = (DDWORD)m_pClientDE->ReadFromMessageWord( hMessage );
			gib.m_fScale = m_pClientDE->ReadFromMessageFloat(hMessage);
			DBYTE nCount = m_pClientDE->ReadFromMessageByte( hMessage );

			if (gib.m_dwFlags & TYPEFLAG_CUSTOM) // Read extra stuff if it's custom.
			{
				gib.m_hstrModel = m_pClientDE->ReadFromMessageHString( hMessage );
				gib.m_hstrSkin = m_pClientDE->ReadFromMessageHString( hMessage );
				gib.m_hstrSound = m_pClientDE->ReadFromMessageHString( hMessage );
			}

			// Adjust count based on detail levels
			DBYTE nVal = g_pBloodClientShell->GetGlobalDetail();

			DDWORD nType = (gib.m_dwFlags & TYPE_MASK) * 10;
			if (nVal == DETAIL_LOW)
			{
				nCount /= 2;		// Smallest possible
			}
			else if (nVal == DETAIL_HIGH && nType == SURFTYPE_FLESH)
			{
				if (IsRandomChance(20))			// Occasionally make more.
					nCount *= 2;
				else
					nCount *= (DBYTE)1.5;		// a little extra..
			}

			for (int i=0; i < nCount; i++)
				CreateSFX(nId, &gib);
		}
		break;

/*
		case SFX_ROCKETFLARE_ID :
		{
			RFCREATESTRUCT rf;

			rf.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(rf.vVel));
			rf.bSmall = (DBOOL)  m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &rf);
		}
		break;
*/
		case SFX_PARTICLESYSTEM_ID :
		{
			PSCREATESTRUCT ps;

			ps.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor2));
			ps.dwFlags			   = (DDWORD) m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fBurstWait		   = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fParticlesPerSecond = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fEmissionRadius	   = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMinimumVelocity	   = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMaximumVelocity    = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fVelocityOffset	   = m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fParticleLifetime   = m_pClientDE->ReadFromMessageFloat(hMessage);

			CreateSFX(nId, &ps);
		}
		break;

		case SFX_MARK_ID :
		{
			MARKCREATESTRUCT mark;

			mark.hServerObj = hObj;
//			m_pClientDE->ReadFromMessageVector(hMessage, &(mark.m_Pos));
			m_pClientDE->ReadFromMessageRotation(hMessage, &(mark.m_Rotation));
			mark.m_fScale = m_pClientDE->ReadFromMessageFloat(hMessage);
			mark.m_hstrSprite = m_pClientDE->ReadFromMessageHString( hMessage );
			mark.m_bServerObj = m_pClientDE->ReadFromMessageByte( hMessage );

			CreateSFX(nId, &mark);
			g_pClientDE->FreeString( mark.m_hstrSprite );
		}
		break;

		case SFX_WEAPON_ID :
		{
			WFXCREATESTRUCT w;

			w.hServerObj	= DNULL;

			m_pClientDE->ReadFromMessageVector(hMessage, &(w.vSourcePos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(w.vDestPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(w.vForward));
			m_pClientDE->ReadFromMessageVector(hMessage, &(w.vNormal));

			w.nFXFlags		= m_pClientDE->ReadFromMessageDWord(hMessage);
			w.nExtraData	= m_pClientDE->ReadFromMessageDWord(hMessage);

			// Check to see if we should read any extras...
			if(w.nExtraData & WFX_EXTRA_AMMOTYPE)
				w.nAmmoType			= m_pClientDE->ReadFromMessageByte(hMessage);
			if(w.nExtraData & WFX_EXTRA_SURFACETYPE)
				w.nSurfaceType		= m_pClientDE->ReadFromMessageByte(hMessage);
			if(w.nExtraData & WFX_EXTRA_EXPTYPE)
				w.nExplosionType	= m_pClientDE->ReadFromMessageByte(hMessage);

			if(w.nExtraData & WFX_EXTRA_DAMAGE)
				w.fDamage			= m_pClientDE->ReadFromMessageFloat(hMessage);
			if(w.nExtraData & WFX_EXTRA_DENSITY)
				w.fDensity			= m_pClientDE->ReadFromMessageFloat(hMessage);

			if(w.nExtraData & WFX_EXTRA_COLOR1)
				m_pClientDE->ReadFromMessageVector(hMessage, &(w.vColor1));
			if(w.nExtraData & WFX_EXTRA_COLOR2)
				m_pClientDE->ReadFromMessageVector(hMessage, &(w.vColor2));
			if(w.nExtraData & WFX_EXTRA_LIGHTCOLOR1)
				m_pClientDE->ReadFromMessageVector(hMessage, &(w.vLightColor1));
			if(w.nExtraData & WFX_EXTRA_LIGHTCOLOR2)
				m_pClientDE->ReadFromMessageVector(hMessage, &(w.vLightColor2));

			CreateSFX(nId, &w);
		}
		break;

		case SFX_TRACER_ID :
		{
			TRACERCREATESTRUCT tracer;

			TRACERCREATESTRUCT tr;

			tracer.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(tracer.vVel));
			m_pClientDE->ReadFromMessageVector(hMessage, &(tracer.vStartColor));
			m_pClientDE->ReadFromMessageVector(hMessage, &(tracer.vEndColor));
			m_pClientDE->ReadFromMessageVector(hMessage, &(tracer.vStartPos));
			tracer.fStartAlpha	= m_pClientDE->ReadFromMessageFloat(hMessage);
			tracer.fEndAlpha	= m_pClientDE->ReadFromMessageFloat(hMessage);

//			tracer.hServerObj = hObj;
//			m_pClientDE->ReadFromMessageVector(hMessage, &(tracer.vToPos));
//			tracer.hGun = m_pClientDE->ReadFromMessageObject(hMessage);

			CreateSFX(nId, &tracer);
		}
		break;

		case SFX_DEBUGLINE_ID:
		{
			DEBUGLINECREATESTRUCT db;

			db.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(db.vFromPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(db.vToPos));

			CreateSFX(nId, &db, DTRUE);
		}
		break;

		case SFX_RAIN_ID :
		{
			RAINCREATESTRUCT rain;

			rain.hServerObj = hObj;
			rain.dwFlags = (DDWORD) m_pClientDE->ReadFromMessageFloat(hMessage);
			rain.fDensity = m_pClientDE->ReadFromMessageFloat(hMessage);
			rain.fLifetime = m_pClientDE->ReadFromMessageFloat(hMessage);
			rain.bGravity = (DBOOL) m_pClientDE->ReadFromMessageByte(hMessage);
			rain.fParticleScale = m_pClientDE->ReadFromMessageFloat(hMessage);
			rain.fSpread = m_pClientDE->ReadFromMessageFloat(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &(rain.vDims));
			m_pClientDE->ReadFromMessageVector(hMessage, &(rain.vDirection));
			m_pClientDE->ReadFromMessageVector(hMessage, &(rain.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(rain.vColor2));
			rain.fTimeLimit = m_pClientDE->ReadFromMessageFloat(hMessage);
			rain.fPulse = m_pClientDE->ReadFromMessageFloat(hMessage);

			CreateSFX(nId, &rain);
		}
		break;

		case SFX_SPARKS_ID :
		{
			SCREATESTRUCT sp;

			sp.hServerObj = hObj;
			m_pClientDE->ReadFromMessageRotation(hMessage, &(sp.rRot));
			m_pClientDE->ReadFromMessageVector(hMessage, &(sp.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(sp.vDir));
			m_pClientDE->ReadFromMessageVector(hMessage, &(sp.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(sp.vColor2));
			sp.hstrTexture		= m_pClientDE->ReadFromMessageHString(hMessage);
			sp.nSparks			= m_pClientDE->ReadFromMessageByte(hMessage);
			sp.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			sp.fEmissionRadius	= m_pClientDE->ReadFromMessageFloat(hMessage);
			sp.fRadius			= m_pClientDE->ReadFromMessageFloat(hMessage);
			sp.fGravity			= m_pClientDE->ReadFromMessageFloat(hMessage);

			CreateSFX(nId, &sp);
			g_pClientDE->FreeString( sp.hstrTexture );
		}
		break;

		case SFX_CASTLINE_ID :
		{
			CLCREATESTRUCT cl;

			cl.hServerObj = hObj;
			m_pClientDE->ReadFromMessageVector(hMessage, &(cl.vStartColor));
			m_pClientDE->ReadFromMessageVector(hMessage, &(cl.vEndColor));
			cl.fStartAlpha	= m_pClientDE->ReadFromMessageFloat(hMessage);
			cl.fEndAlpha	= m_pClientDE->ReadFromMessageFloat(hMessage);
			cl.hCastTo		= m_pClientDE->ReadFromMessageObject(hMessage);

			CreateSFX(nId, &cl);
		}
		break;

		case SFX_VOLUMEBRUSH_ID :
		{
			VBCREATESTRUCT vb;

			vb.hServerObj		= hObj;
			vb.bFogEnable		= m_pClientDE->ReadFromMessageByte(hMessage);
			vb.fFogFarZ			= m_pClientDE->ReadFromMessageFloat(hMessage);
			vb.fFogNearZ		= m_pClientDE->ReadFromMessageFloat(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &vb.vFogColor);

			CreateSFX(nId, &vb);
		}
		break;

		case SFX_SPLASH_ID :
		{
			SPLASHCREATESTRUCT		splcs;
			RIPPLECREATESTRUCT		ripcs;
			DBOOL		bRipple;

			splcs.hServerObj = hObj;
			ripcs.hServerObj = hObj;

			m_pClientDE->ReadFromMessageVector(hMessage, &(splcs.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(splcs.vDir));
			splcs.fRadius		= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fPosRadius	= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fHeight		= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fDensity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fSpread		= m_pClientDE->ReadFromMessageFloat(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &(splcs.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(splcs.vColor2));

			/***** RIPPLE INFORMATION *****/
			m_pClientDE->ReadFromMessageVector(hMessage, &(ripcs.vMinScale));
			bRipple				= m_pClientDE->ReadFromMessageByte(hMessage);
			/******************************/

			splcs.fSprayTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.fGravity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			splcs.hstrTexture	= m_pClientDE->ReadFromMessageHString(hMessage);

			if(bRipple)
			{
				VEC_COPY(ripcs.vNormal, splcs.vDir);
				VEC_COPY(ripcs.vPos, splcs.vPos);
				ripcs.fDuration = 1.0f;
				VEC_COPY(ripcs.vMaxScale, ripcs.vMinScale);
				ripcs.fInitAlpha = 0.25f;
				ripcs.bFade = 0;
				CreateSFX(SFX_RIPPLE_ID, &ripcs);
			}

			CreateSFX(nId, &splcs);
		}
		break;

		case SFX_EXPLOSION_ID :
		{
			EXPLOSIONMODELCS	expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale2));
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.bWaveForm		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bFadeType		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bRandomRot	= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.szModel		= m_pClientDE->ReadFromMessageHString(hMessage);
			expCS.szSkin		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(SFX_EXPLOSION_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONSPRITE_ID :
		{
			EXPLOSIONSPRITECS	expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale2));
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.bWaveForm		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bFadeType		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bAlign		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.szSprite		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(SFX_EXPLOSIONSPRITE_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONRING_ID :
		{
			EXPLOSIONRINGCS	expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vColor));
			expCS.fRadius		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fPosRadius	= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fVelocity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fGravity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.nParticles	= m_pClientDE->ReadFromMessageDWord(hMessage);
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fDelay		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.bFadeType		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bAlign		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.szParticle	= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(SFX_EXPLOSIONRING_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONWAVE_ID :
		{
			EXPLOSIONWAVECS	expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale2));
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fDelay		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.bWaveForm		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bFadeType		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.bAlign		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.szWave		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(SFX_EXPLOSIONWAVE_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONLIGHT_ID :
		{
			EXPLOSIONLIGHTCS	expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vColor2));
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fDelay		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fRadius1		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fRadius2		= m_pClientDE->ReadFromMessageFloat(hMessage);

			CreateSFX(SFX_EXPLOSIONLIGHT_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONFRAG_ID :
		{
			EXPLOSIONFRAGCS		expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vScale));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vRotateMax));
			expCS.fSpread		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fDuration		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fVelocity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fBounceMod	= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fGravity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.fFadeTime		= m_pClientDE->ReadFromMessageFloat(hMessage);
			expCS.bRandDir		= m_pClientDE->ReadFromMessageByte(hMessage);
			expCS.szModel		= m_pClientDE->ReadFromMessageHString(hMessage);
			expCS.szSkin		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(SFX_EXPLOSIONFRAG_ID, &expCS);
		}
		break;

		case SFX_EXPLOSIONFX_ID :
		{
			EXPLOSIONFXCS		expCS;
			expCS.hServerObj = 0;
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vPos));
			m_pClientDE->ReadFromMessageVector(hMessage, &(expCS.vNormal));
			expCS.nType			= m_pClientDE->ReadFromMessageDWord(hMessage);

			CreateSFX(SFX_EXPLOSIONFX_ID, &expCS);
		}
		break;

		case SFX_CAMERA_ID :
		{
			CAMERACREATESTRUCT camera;
			DBYTE nFlags;

			camera.hServerObj	= hObj;
			camera.nType		= m_pClientDE->ReadFromMessageByte(hMessage);
			nFlags = (DBOOL)m_pClientDE->ReadFromMessageByte(hMessage);
			camera.bPlayerMovement = ( nFlags & CAMERASFXFLAG_ALLOWPLAYERMOVEMENT ) ? DTRUE : DFALSE;
			camera.bHidePlayer = ( nFlags & CAMERASFXFLAG_HIDEPLAYER ) ? DTRUE : DFALSE;
			camera.bIsListener = ( nFlags & CAMERASFXFLAG_ISLISTENER ) ? DTRUE : DFALSE;

//			m_pClientDE->CPrint("Camera client object: %d", hObj);

			CreateSFX(nId, &camera);
		}
		break;

		case SFX_LIGHT_ID :
		{
			LIGHTCREATESTRUCT light;

			light.hServerObj	= hObj;

			m_pClientDE->ReadFromMessageVector(hMessage, &light.vColor);
			light.dwLightFlags = m_pClientDE->ReadFromMessageDWord(hMessage);
			light.fIntensityMin = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.fIntensityMax = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.nIntensityWaveform = m_pClientDE->ReadFromMessageByte(hMessage);
			light.fIntensityFreq = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.fIntensityPhase = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.fRadiusMin = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.fRadiusMax = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.nRadiusWaveform = m_pClientDE->ReadFromMessageByte(hMessage);
			light.fRadiusFreq = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.fRadiusPhase = m_pClientDE->ReadFromMessageFloat(hMessage);
			light.hstrRampUpSound = m_pClientDE->ReadFromMessageHString(hMessage);
			light.hstrRampDownSound = m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(nId, &light);
		}
		break;

		case SFX_WEAPONPOWERUP_ID :
		{
			WEAPPOWERCREATESTRUCT weap;

			weap.hServerObj = hObj;
			weap.hGun = m_pClientDE->ReadFromMessageObject(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &(weap.vPosOffset));
			m_pClientDE->ReadFromMessageVector(hMessage, &(weap.vScale));
			weap.fLifeTime = m_pClientDE->ReadFromMessageFloat(hMessage);
			weap.fInitAlpha = m_pClientDE->ReadFromMessageFloat(hMessage);
			weap.bFade = m_pClientDE->ReadFromMessageByte(hMessage);
			weap.pSpriteFile = m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(nId, &weap);
		}
		break;

		case SFX_PARTICLESTREAM_ID :
		{
			PSTREAMCREATESTRUCT ps;

			ps.hServerObj = hObj;
			ps.fRadius		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fPosRadius	= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMinVel		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMaxVel		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.nNumParticles= m_pClientDE->ReadFromMessageDWord(hMessage);
			ps.fSpread		= m_pClientDE->ReadFromMessageFloat(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor1));
			m_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor2));
			ps.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMinLife		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fMaxLife		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fRampTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fDelay		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.fGravity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			ps.bRampFlags	= m_pClientDE->ReadFromMessageByte(hMessage);
			ps.bOn			= m_pClientDE->ReadFromMessageByte(hMessage);
			ps.hstrTexture	= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(nId, &ps);
		}
		break;

		case SFX_LIGHTNING_ID :
		{
			LIGHTNINGCREATESTRUCT	lcs;

			lcs.hServerObj	= hObj;

			m_pClientDE->ReadFromMessageVector(hMessage, &(lcs.vSource));
			m_pClientDE->ReadFromMessageVector(hMessage, &(lcs.vDest));
			lcs.nShape		= m_pClientDE->ReadFromMessageByte(hMessage);
			lcs.nForm		= m_pClientDE->ReadFromMessageByte(hMessage);
			lcs.nType		= m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &lcs);
		}
		break;

		case SFX_LASERBEAM_ID :
		{
			LASERBEAMCREATESTRUCT	lbcs;

			lbcs.hServerObj	= hObj;

			m_pClientDE->ReadFromMessageVector(hMessage, &(lbcs.vSource));
			m_pClientDE->ReadFromMessageVector(hMessage, &(lbcs.vDest));
			lbcs.nType		= m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &lbcs);
		}
		break;

		case SFX_WARPGATESPRITE_ID :
		{
			WARPGATESPRITECS	wgSpr;

			wgSpr.hServerObj	= hObj;

			wgSpr.hObj			= m_pClientDE->ReadFromMessageObject(hMessage);
			wgSpr.fRampUpTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgSpr.fRampDownTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgSpr.fMinScale		= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgSpr.fMaxScale		= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgSpr.fAlpha		= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgSpr.nRampUpType	= m_pClientDE->ReadFromMessageDWord(hMessage);
			wgSpr.nRampDownType	= m_pClientDE->ReadFromMessageDWord(hMessage);
			wgSpr.bAlign		= m_pClientDE->ReadFromMessageByte(hMessage);
			wgSpr.szSprite		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(nId, &wgSpr);
		}
		break;

		case SFX_WARPGATEPARTICLE_ID :
		{
			WARPGATEPARTICLECS	wgPS;

			wgPS.hServerObj		= hObj;

			wgPS.hObj			= m_pClientDE->ReadFromMessageObject(hMessage);
			wgPS.fRampUpTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fRampDownTime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fSystemRadius	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fPosRadius		= m_pClientDE->ReadFromMessageFloat(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &wgPS.vOffset);
			m_pClientDE->ReadFromMessageVector(hMessage, &wgPS.vRotations);
			wgPS.fMinVelocity	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fMaxVelocity	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.nNumParticles	= m_pClientDE->ReadFromMessageDWord(hMessage);
			wgPS.nEmitType		= m_pClientDE->ReadFromMessageDWord(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &wgPS.vMinColor);
			m_pClientDE->ReadFromMessageVector(hMessage, &wgPS.vMaxColor);
			wgPS.fAlpha			= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fMinLifetime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fMaxLifetime	= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fAddDelay		= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.fGravity		= m_pClientDE->ReadFromMessageFloat(hMessage);
			wgPS.nRampUpType	= m_pClientDE->ReadFromMessageDWord(hMessage);
			wgPS.nRampDownType	= m_pClientDE->ReadFromMessageDWord(hMessage);
			wgPS.bAlign			= m_pClientDE->ReadFromMessageByte(hMessage);
			wgPS.szParticle		= m_pClientDE->ReadFromMessageHString(hMessage);

			CreateSFX(nId, &wgPS);
		}
		break;

		case SFX_OBJECTFX_ID :
		{
			OBJECTFXCS	ofx;

			ofx.hServerObj		= DNULL;

			ofx.hObj			= m_pClientDE->ReadFromMessageObject(hMessage);
			m_pClientDE->ReadFromMessageVector(hMessage, &ofx.vOffset);
			ofx.fScale			= m_pClientDE->ReadFromMessageFloat(hMessage);
			ofx.nScaleFlags		= m_pClientDE->ReadFromMessageDWord(hMessage);
			ofx.nFXType			= m_pClientDE->ReadFromMessageDWord(hMessage);
			ofx.nFXFlags		= m_pClientDE->ReadFromMessageDWord(hMessage);

			if(hObj && !ofx.hObj)
				ofx.hObj = hObj;

			CreateSFX(nId, &ofx);
		}
		break;

		case SFX_POWERUP_ID :
		{
			PICKUPOBJCREATESTRUCT pobj;
			pobj.hServerObj = hObj;
			CreateSFX(nId, &pobj);
		}
		break;

		case SFX_FLASHLIGHT_ID :
		{
			FLASHLIGHTCREATESTRUCT fobj;
			fobj.hServerObj = hObj;
			CreateSFX(nId, &fobj);
		}
		break;

		case SFX_PARTICLEEXPLOSION_ID:
		{

			char* szBlood[4] = { "spritetextures\\particles\\blooddrop_1.dtx", 
								 "spritetextures\\particles\\blooddrop_2.dtx",
								 "spritetextures\\particles\\blooddrop_3.dtx",
								 "spritetextures\\particles\\blooddrop_4.dtx"};

			PESCREATESTRUCT pe;

			DVector vDir;

			m_pClientDE->ReadFromMessageVector(hMessage, &(pe.vPos));
			m_pClientDE->ReadFromMessageCompVector(hMessage, &vDir);


			VEC_MULSCALAR(pe.vMinVel, vDir, 0.6f);
			VEC_MULSCALAR(pe.vMaxVel, vDir, 0.6f);

			VEC_SET(pe.vColor1, 128.0f, 128.0f, 128.0f);
			VEC_SET(pe.vColor2, 128.0f, 128.0f, 128.0f);
			VEC_SET(pe.vMinDriftOffset, 0.0f, -10.0f, 0.0f);
			VEC_SET(pe.vMaxDriftOffset, 0.0f, -5.0f, 0.0f);
			pe.bSmall			= DFALSE;
			pe.fLifeTime		= 2.0f;
			pe.fFadeTime		= 0.5f;
			pe.fOffsetTime		= 0.0f;
			pe.fRadius			= 200.0f;
			pe.fGravity			= -100.0f;
			pe.nNumPerPuff		= 2;
			pe.nNumEmitters		= 1; //GetRandom(1,4);
			pe.nEmitterFlags	= MO_HALFGRAVITY;
			pe.bIgnoreWind		= DTRUE;
			pe.pFilename		= szBlood[GetRandom(0,2)];
			pe.nSurfaceType		= SURFTYPE_FLESH;
			pe.nNumSteps		= 6;
			pe.bBounce			= DFALSE;
			CreateSFX(nId, &pe);
		}
		break;

#ifdef _ADD_ON
		case SFX_FLAYER_CHAIN_ID :
		{
			IKCHAINCS	ccs;

			ccs.hServerObj		= hObj;
			ccs.byNumLinks		= m_pClientDE->ReadFromMessageByte(hMessage);
			ccs.fScale			= m_pClientDE->ReadFromMessageFloat(hMessage);
			ccs.fTime			= m_pClientDE->ReadFromMessageFloat(hMessage);
			ccs.byFXType		= m_pClientDE->ReadFromMessageByte(hMessage);
			ccs.byFXFlags		= m_pClientDE->ReadFromMessageByte(hMessage);

			CreateSFX(nId, &ccs);
		}
		break;
#endif

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::CreateSFX()
//
//	PURPOSE:	Create the special fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::CreateSFX(DBYTE nId, SFXCREATESTRUCT *psfxCreateStruct, DBOOL bStatic, CSpecialFX* pParentFX)
{
	if (!m_pClientDE) return DNULL;

	CSpecialFX* pSFX = DNULL;

	switch(nId)
	{
		case SFX_POLYGRID_ID :
		{
			pSFX = new CPolyGridFX();
		}
		break;
		
		case SFX_SMOKETRAIL_ID :
		{
			pSFX = new CSmokeTrailFX();
		}
		break;

		case SFX_SMOKETRAILSEG_ID :
		{
			pSFX = new CSmokeTrailSegmentFX();
		}
		break;

		case SFX_PARTICLESYSTEM_ID :
		{
			pSFX = new CParticleSystemFX();
		}
		break;

		case SFX_MARK_ID :
		{
			pSFX = new CMarkSFX();
		}
		break;
		
		case SFX_WEAPON_ID :
		{
			pSFX = new CWeaponFX();
		}
		break;
		
		case SFX_TRACER_ID :
		{
			pSFX = new CTracerFX();
		}
		break;

		case SFX_DEBUGLINE_ID:
		{
			pSFX = new CDebugLine();
		}
		break;

		case SFX_RAIN_ID :
		{
			pSFX = new CRainFX();
		}
		break;

		case SFX_SPARKS_ID :
		{
			pSFX = new CSparksFX();
		}
		break;
        
		case SFX_CASTLINE_ID :
		{
			pSFX = new CCastLineFX();
		}
		break;

		case SFX_VOLUMEBRUSH_ID:
		{
			pSFX = new CVolumeBrushFX();
		}
		break;

		//SCHLEGZ 5/21/98 1:42:00 PM: new blood trail effect
		case SFX_BLOODTRAIL_ID :
		{
			pSFX = new CBloodTrailFX();
		}
		break;

		case SFX_BLOODTRAILSEG_ID :
		{
			pSFX = new CBloodTrailSegmentFX();
		}
		break;

		case SFX_BLOODSPLAT_ID :
		{
			pSFX = new CBloodSplatFX();
		}
		break;

		case SFX_GIB_ID:
		{
			pSFX = new CGibFX();
		}
		break;

		case SFX_PARTICLEEXPLOSION_ID:
		{
			pSFX = new CParticleExplosionFX();
		}
		break;

		case SFX_SMOKEPUFF_ID :
		{
			pSFX = new CSmokePuffFX();
		}
		break;

		// ANDY 6/17/98 12:42pm Just made a new one as to not change the old one
		case SFX_SMOKEIMPACT_ID :
		{
			pSFX = new CSmokeImpactFX();
		}
		break;

		case SFX_FRAGMENTS_ID :
		{
			pSFX = new CSurfaceFragmentFX();
		}
		break;

		case SFX_SPLASH_ID :
		{
			pSFX = new CSplashFX();
		}
		break;

		case SFX_RIPPLE_ID :
		{
			pSFX = new CRippleFX();
		}
		break;

		case SFX_EXPLOSION_ID :
		{
			pSFX = new CExplosionModelFX();
		}
		break;

		case SFX_EXPLOSIONSPRITE_ID :
		{
			pSFX = new CExplosionSpriteFX();
		}
		break;

		case SFX_EXPLOSIONRING_ID :
		{
			pSFX = new CExplosionRingFX();
		}
		break;

		case SFX_EXPLOSIONFLAME_ID :
		{
			pSFX = new CExplosionFlameFX();
		}
		break;

		case SFX_EXPLOSIONWAVE_ID :
		{
			pSFX = new CExplosionWaveFX();
		}
		break;

		case SFX_EXPLOSIONLIGHT_ID :
		{
			pSFX = new CExplosionLightFX();
		}
		break;

		case SFX_EXPLOSIONFRAG_ID :
		{
			pSFX = new CExplosionFragFX();
		}
		break;

		case SFX_EXPLOSIONFX_ID :
		{
			pSFX = new CExplosionFX();
		}
		break;

		case SFX_CAMERA_ID :
		{
			pSFX = new CCameraFX();

			// Put the camera in a special list
			if (pSFX)
			{
				if (pSFX->Init(psfxCreateStruct))
				{
					if (m_pClientDE->IsConnected())
					{
						pSFX->CreateObject(m_pClientDE);
						m_cameraSFXList.Add(pSFX);
					}
					else
					{
						delete pSFX;
						pSFX = DNULL;
					}
				}
			}

			return pSFX;
		}
		break;

		case SFX_LIGHT_ID :
		{
			pSFX = new CLightFX();
		}
		break;

		case SFX_WEAPONPOWERUP_ID :
		{
			pSFX = new CWeaponPowerupFX();
		}
		break;

		case SFX_PARTICLESTREAM_ID :
		{
			pSFX = new CParticleStreamFX();
		}
		break;

		case SFX_LIGHTNING_ID :
		{
			pSFX = new CLightningBoltFX();
		}
		break;

		case SFX_LIGHTNINGSEG_ID :
		{
			pSFX = new CLightningSegmentFX();
		}
		break;

		case SFX_LASERBEAM_ID :
		{
			pSFX = new CLaserBeamFX();
		}
		break;

		case SFX_WARPGATESPRITE_ID :
		{
			pSFX = new CWarpGateSpriteFX();
		}
		break;

		case SFX_WARPGATEPARTICLE_ID :
		{
			pSFX = new CWarpGateParticleFX();
		}
		break;

		case SFX_OBJECTFX_ID :
		{
			pSFX = new CObjectGeneralFX();
		}
		break;

		case SFX_OBJECTPARTICLES_ID :
		{
			pSFX = new CObjectParticleFX();
		}
		break;

		case SFX_SHELLCASING_ID :
		{
			pSFX = new CShellCasingFX();
		}
		break;

		case SFX_POWERUP_ID :
		{
			pSFX = new CPickupObjectFX();
		}
		break;

		case SFX_FLASHLIGHT_ID :
		{
			pSFX = new CFlashlightFX();
		}
		break;

#ifdef _ADD_ON
		case SFX_FLAYER_CHAIN_ID :
		{
			pSFX = new CIKChainFX();
		}
		break;
#endif

		default : return DNULL;
	}

	// Initialize the sfx, and add it to the appropriate array...
	if (!pSFX) return DNULL;


	if (!pSFX->Init(psfxCreateStruct))
	{
		delete pSFX;
		return DNULL;
	}

	// Only create the sfx if we are connected to the server...

	if (!m_pClientDE->IsConnected())
	{
		delete pSFX;
		return DNULL;
	}

	if (!pSFX->CreateObject(m_pClientDE))
	{
		delete pSFX;
		return DNULL;
	}

	if (bStatic) 
	{
		CSpecialFX* pFX = m_staticSFXList.Add(pSFX);

		if(pFX)
		{
			if(pFX->Term())
				delete pFX;
			else
			{
				CSpecialFX* pDelFX = m_termSFXList.Add(pFX);
				if(pDelFX)
					delete pDelFX;
			}
		}
	}
	else 
	{
		AddDynamicSpecialFX(pSFX, nId, pParentFX);
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
	if (!m_pClientDE) return;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Update static sfx...

	int nNumSFX = m_staticSFXList.GetSize();
	
	for (int i=0; i < nNumSFX; i++)
	{
		if (m_staticSFXList[i]) 
		{
			if (fTime >= m_staticSFXList[i]->m_fNextUpdateTime)
			{
				if(!m_staticSFXList[i]->Update())
					m_staticSFXList.Remove(m_staticSFXList[i]);
				else
					m_staticSFXList[i]->m_fNextUpdateTime = fTime + m_staticSFXList[i]->GetUpdateDelta();
			}
		}
	}

	// Update dynamic sfx...

	UpdateDynamicSpecialFX();

	// Update camera sfx...

	nNumSFX = m_cameraSFXList.GetSize();
	
	for (i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i]) 
		{
			if (!m_cameraSFXList[i]->Update())
			{
				m_cameraSFXList.Remove(m_cameraSFXList[i]);
			}
		}
	}

	//slow terminating FX update
	nNumSFX = m_termSFXList.GetSize();
	
	for (i=0; i < nNumSFX; i++)
	{
		if (m_termSFXList[i]) 
		{
			if (fTime >= m_termSFXList[i]->m_fNextUpdateTime)
			{
				if(!m_termSFXList[i]->Update())
					m_termSFXList.Remove(m_termSFXList[i]);
				else
					m_termSFXList[i]->m_fNextUpdateTime = fTime + m_termSFXList[i]->GetUpdateDelta();
			}
		}
	}

	return;
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

	int nNumSFX = m_staticSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_staticSFXList[i] && m_staticSFXList[i]->GetServerObj() == hObj)
		{
			m_staticSFXList[i]->WantRemove();
		}
	}

	// Check camera list...

	nNumSFX = m_cameraSFXList.GetSize();

	for (i=0; i < nNumSFX; i++)
	{
		if (m_cameraSFXList[i] && m_cameraSFXList[i]->GetServerObj() == hObj)
		{
			m_cameraSFXList[i]->WantRemove();
		}
	}

	//slow terminating FX
	nNumSFX = m_termSFXList.GetSize();

	for (i=0; i < nNumSFX; i++)
	{
		if (m_termSFXList[i] && m_termSFXList[i]->GetServerObj() == hObj)
		{
			m_termSFXList[i]->WantRemove();
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

	int nNumSFX = m_staticSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		m_staticSFXList.Remove(m_staticSFXList[i]);
	}

	nNumSFX = m_cameraSFXList.GetSize();

	for (i=0; i < nNumSFX; i++)
	{
		m_cameraSFXList.Remove(m_cameraSFXList[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::FindStaticSpecialFX()
//
//	PURPOSE:	Find a static special fx associated with an object
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::FindStaticSpecialFX(HLOCALOBJ hObj)
{
	CSpecialFX* pFX = DNULL;
	int nNumSFX = m_staticSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		if (m_staticSFXList[i] && m_staticSFXList[i]->GetServerObj() == hObj)
		{
			pFX = m_staticSFXList[i];
			break;
		}
	}

	return pFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::AddDynamicSpecialFX()
//
//	PURPOSE:	Add a dyanamic special fx to our lists
//
// ----------------------------------------------------------------------- //

void CSFXMgr::AddDynamicSpecialFX(CSpecialFX* pSFX, DBYTE nId, CSpecialFX* pParentFX)
{
	int nIndex = GetDynArrayIndex(nId);

	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		CSpecialFX* pFX = m_dynSFXLists[nIndex].Add(pSFX, pParentFX);

		if(pFX)
		{
			if(pFX->Term())
				delete pFX;
			else
			{
				CSpecialFX* pDelFX = m_termSFXList.Add(pFX);
				if(pDelFX)
					delete pDelFX;
			}
		}
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
	if (!m_pClientDE) return;

	DFLOAT fTime = m_pClientDE->GetTime();

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

	int nNumSFX = m_termSFXList.GetSize();

	for (int i=0; i < nNumSFX; i++)
	{
		m_termSFXList.Remove(m_termSFXList[i]);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::FindDynamicSpecialFX()
//
//	PURPOSE:	Find a dyanamic special fx associated with an object
//
// ----------------------------------------------------------------------- //

CSpecialFX* CSFXMgr::FindDynamicSpecialFX(HLOCALOBJ hObj)
{
	for (int j=0; j < DYN_ARRAY_SIZE; j++)
	{
		int nNumSFX  = m_dynSFXLists[j].GetSize();

		for (int i=0; i < nNumSFX; i++)
		{
			if (m_dynSFXLists[j][i] && m_dynSFXLists[j][i]->GetServerObj() == hObj)
			{
				return m_dynSFXLists[j][i];
			}
		}
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayIndex()
//
//	PURPOSE:	Get the array index associated with a particular type of
//				dynamic special fx
//
// ----------------------------------------------------------------------- //

int	CSFXMgr::GetDynArrayIndex(DBYTE nFXId)
{
	int nIndex = 0;

	switch (nFXId)
	{
		case SFX_POLYGRID_ID :
		case SFX_PARTICLESYSTEM_ID :
		case SFX_RAIN_ID :
		case SFX_VOLUMEBRUSH_ID :
		case SFX_CAMERA_ID :
		case SFX_LIGHT_ID :
		case SFX_WEAPONPOWERUP_ID :
		case SFX_PARTICLESTREAM_ID :
		case SFX_POWERUP_ID :
			nIndex = 0;
			break;
		
		case SFX_MARK_ID :
			nIndex = 1;
			break;
		
		case SFX_WEAPON_ID :
		case SFX_ROCKETFLARE_ID :
		case SFX_TRACER_ID :
		case SFX_LASER_ID :
		case SFX_LIGHTNING_ID :
		case SFX_LIGHTNINGSEG_ID :
		case SFX_LASERBEAM_ID :
		case SFX_WARPGATESPRITE_ID :
		case SFX_WARPGATEPARTICLE_ID :
		case SFX_FLASHLIGHT_ID :
		case SFX_WEAPONSOUND_ID :
			nIndex = 2;
			break;

		case SFX_SMOKETRAIL_ID :
		case SFX_SMOKETRAILSEG_ID :
		case SFX_SPARKS_ID :
		case SFX_SMOKEPUFF_ID :
		case SFX_SMOKEIMPACT_ID :
		case SFX_SPLASH_ID :
		case SFX_RIPPLE_ID :
		case SFX_FRAGMENTS_ID :
			nIndex = 3;
			break;

		case SFX_EXPLOSION_ID :
		case SFX_EXPLOSIONSPRITE_ID :
		case SFX_EXPLOSIONRING_ID :
		case SFX_EXPLOSIONWAVE_ID :
		case SFX_EXPLOSIONLIGHT_ID :
		case SFX_EXPLOSIONFRAG_ID :
		case SFX_EXPLOSIONFX_ID :
		case SFX_EXPLOSIONFLAME_ID :
			nIndex = 4;
			break;

		case SFX_GIB_ID:
		case SFX_BLOODTRAIL_ID :
		case SFX_BLOODTRAILSEG_ID :
		case SFX_BLOODSPLAT_ID:
		case SFX_PARTICLEEXPLOSION_ID:
			nIndex = 5;
			break;
		
		case SFX_SHELLCASING_ID:
			nIndex = 6;
			break;
		
		case SFX_OBJECTFX_ID :
		case SFX_OBJECTPARTICLES_ID :
			nIndex = 7;
			break;
		
	}

	return nIndex;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSFXMgr::GetDynArrayMaxNum()
//
//	PURPOSE:	Find a dyanamic special fx associated with an object
//
// ----------------------------------------------------------------------- //

static unsigned int s_nDynArrayMaxNums[DYN_ARRAY_SIZE] =
{
	300,	// General FX
	300,	// Marks - bullet holes
	80,		// Weapon, Tracers, Lasers
	80,		// Smoke, Smoke trail seg, Sparks, Smokepuff, Smokeimpact, Splashes, ripples, Fragments
	80,		// Explosions, Explosion sprites, Explosion rings, Explosion wave, Explosion light, Explosion frag
	80,		// gibs, Blood trail, Blood trail seg, blood splat
	100,	// shell casings
	80		// Object
};

unsigned int CSFXMgr::GetDynArrayMaxNum(DBYTE nIndex)
{
	if (0 <= nIndex && nIndex < DYN_ARRAY_SIZE)
	{
		return s_nDynArrayMaxNums[nIndex];
	}

	return 0;
}


