// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.cpp
//
// PURPOSE : FireFX special FX - Implementation
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FireFX.h"
#include "RandomSparksFX.h"
#include "GameClientShell.h"
#include "iltcustomdraw.h"

#define FFX_DEFAULT_RADIUS					100.0f
#define	FFX_MIN_RADIUS						20.0f
#define	FFX_MAX_RADIUS						500.0f
#define	FFX_INNER_CAM_RADIUS				300.0f
#define	FFX_CAM_FALLOFF_RANGE				300.0f
#define FFX_DEFAULT_SMOKE_PARTICLE_RADIUS	7000.0f
#define FFX_DEFAULT_FIRE_PARTICLE_RADIUS	4000.0f
#define FFX_MAX_SMOKE_PARTICLE_RADIUS		(FFX_DEFAULT_SMOKE_PARTICLE_RADIUS * 1.3f)
#define FFX_MAX_FIRE_PARTICLE_RADIUS		(FFX_DEFAULT_FIRE_PARTICLE_RADIUS)
#define FFX_MIN_FIRE_PARTICLE_LIFETIME		0.25f
#define FFX_MAX_FIRE_PARTICLE_LIFETIME		2.0f
#define FFX_MIN_SMOKE_PARTICLE_LIFETIME		0.5f
#define FFX_MAX_SMOKE_PARTICLE_LIFETIME		6.0f
#define FFX_MAX_LIGHT_RADIUS				300.0f


extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	FIRECREATESTRUCT fire;
	fire.hServerObj		= hServObj;
    fire.fRadius        = g_pLTClient->ReadFromMessageFloat(hMessage);
    fire.fSoundRadius   = g_pLTClient->ReadFromMessageFloat(hMessage);
    fire.fLightRadius   = g_pLTClient->ReadFromMessageFloat(hMessage);
    fire.fLightPhase    = g_pLTClient->ReadFromMessageFloat(hMessage);
    fire.fLightFreq     = g_pLTClient->ReadFromMessageFloat(hMessage);
    g_pLTClient->ReadFromMessageVector(hMessage, &(fire.vLightOffset));
    g_pLTClient->ReadFromMessageVector(hMessage, &(fire.vLightColor));
    fire.bCreateSmoke   = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
    fire.bCreateLight   = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
    fire.bCreateSparks  = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
    fire.bCreateSound   = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
    fire.bBlackSmoke    = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
    fire.bSmokeOnly     = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);

	return Init(&fire);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the Fire fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((FIRECREATESTRUCT*)psfxCreateStruct);
	m_cs.fRadius = m_cs.fRadius < FFX_MIN_RADIUS ? FFX_MIN_RADIUS :
		(m_cs.fRadius > FFX_MAX_RADIUS ? FFX_MAX_RADIUS : m_cs.fRadius);

	m_fSizeAdjust = m_cs.fRadius / FFX_DEFAULT_RADIUS;

	// If we're creating smoke, don't do light, sound or sparks...

	if (m_cs.bSmokeOnly)
	{
        m_cs.bCreateLight = m_cs.bCreateSound = m_cs.bCreateSparks = LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;

    LTVector vZero(0, 0, 0), vOffset(0, 0, 0);
	CString str;
    uint32 dwFlags;

	// Get our initial pos...

	if (m_cs.vPos.Equals(vZero) && m_hServerObject)
    {
        g_pLTClient->GetObjectPos(m_hServerObject, &(m_cs.vPos));
	}


	SMCREATESTRUCT sm;
	sm.hServerObj		= m_hServerObject;
    sm.bRelToCameraPos  = LTTRUE;
	sm.fInnerCamRadius	= FFX_INNER_CAM_RADIUS;
	sm.fOuterCamRadius	= FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);

	// Create the smoke particles...

	if (m_cs.bCreateSmoke)
	{
		vOffset.Init(0, 20.0f * m_fSizeAdjust, 0);
		vOffset.y = vOffset.y > 50.0f ? 50.0f : vOffset.y;

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 15.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 50.0f, 2.0f);
		sm.fVolumeRadius		= 5.0f * (1.5f * m_fSizeAdjust);
		sm.fLifeTime			= 100000.0f;
		sm.fRadius				= FFX_DEFAULT_SMOKE_PARTICLE_RADIUS * m_fSizeAdjust;
		sm.fParticleCreateDelta	= 0.25f;
		sm.fMinParticleLife		= 2.0f * m_fSizeAdjust;
		sm.fMaxParticleLife		= 4.0f * m_fSizeAdjust;
		sm.nNumParticles		= 2;
		//sm.bMultiply			= m_cs.bBlackSmoke;
        sm.bIgnoreWind          = LTFALSE;
		sm.vPos					= m_cs.vPos + vOffset;

        sm.bAdjustParticleScale = LTTRUE;
		sm.fStartParticleScale	= 1.0f;
		sm.fEndParticleScale	= 0.5f;
        sm.bAdjustParticleAlpha = LTTRUE;
		sm.fStartParticleAlpha	= 1.0f;
		sm.fEndParticleAlpha	= 0.0f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_SMOKE_PARTICLE_RADIUS ? FFX_MAX_SMOKE_PARTICLE_RADIUS : sm.fRadius;

		str = g_pClientButeMgr->GetSpecialFXAttributeString("FireSmokeTex");
        if (str.IsEmpty()) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString((char *)(LPCSTR)str);

		if (!m_Smoke1.Init(&sm) || !m_Smoke1.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}
        g_pLTClient->FreeString(sm.hstrTexture);

        dwFlags = g_pLTClient->GetObjectFlags(m_Smoke1.GetObject());
        g_pLTClient->SetObjectFlags(m_Smoke1.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

		m_Smoke1.Update();
	}

	// Create the fire particles...

	if (!m_cs.bSmokeOnly)
	{
        LTFLOAT fVolumeAdjust = m_fSizeAdjust < 1.0 ? m_fSizeAdjust / 1.5f : m_fSizeAdjust * 1.5f;

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 8.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 15.0f, 2.0f);
		sm.fVolumeRadius		= 10.0f * fVolumeAdjust;
		sm.fLifeTime			= 100000.0f;
		sm.fRadius				= FFX_DEFAULT_FIRE_PARTICLE_RADIUS * m_fSizeAdjust;
		sm.fParticleCreateDelta	= 0.1f;
		sm.fMinParticleLife		= 1.0f * m_fSizeAdjust;
		sm.fMaxParticleLife		= 2.0f * m_fSizeAdjust;
		sm.nNumParticles		= 3;
        sm.bAdditive            = LTTRUE;
		sm.vPos					= m_cs.vPos;

		//sm.fStartParticleScale	= 1.0f;
		//sm.fEndParticleScale	= 0.0f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;

		str = g_pClientButeMgr->GetSpecialFXAttributeString("FireTex");
        if (str.IsEmpty()) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString((char *)(LPCSTR)str);

		if (!m_Fire1.Init(&sm) || !m_Fire1.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}

        dwFlags = g_pLTClient->GetObjectFlags(m_Fire1.GetObject());
        g_pLTClient->SetObjectFlags(m_Fire1.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

		m_Fire1.Update();
        g_pLTClient->FreeString(sm.hstrTexture);


		// Create inner fire particles...

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 25.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 35.0f, 2.0f);
		sm.fRadius			= FFX_DEFAULT_FIRE_PARTICLE_RADIUS * 0.75f * m_fSizeAdjust;
		sm.nNumParticles	= 5;
		sm.fVolumeRadius	= 5.0f * fVolumeAdjust;
		sm.fLifeTime		= 100000.0f;
		sm.fMinParticleLife	= 0.5f * m_fSizeAdjust;
		sm.fMaxParticleLife	= 1.25f * m_fSizeAdjust;

		//sm.fStartParticleScale	= 1.0f;
		//sm.fEndParticleScale	= 0.5f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;

		str = g_pClientButeMgr->GetSpecialFXAttributeString("FireTex2");
        if (str.IsEmpty()) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString((char *)(LPCSTR)str);

		if (!m_Fire2.Init(&sm) || !m_Fire2.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}

        dwFlags = g_pLTClient->GetObjectFlags(m_Fire2.GetObject());
        g_pLTClient->SetObjectFlags(m_Fire2.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

		m_Fire2.Update();
        g_pLTClient->FreeString(sm.hstrTexture);


		// Create the sound...

		if (m_cs.bCreateSound)
		{
			str = g_pClientButeMgr->GetSpecialFXAttributeString("FireSnd");

			m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, (char*)(LPCSTR)str,
				m_cs.fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
		}


		// Create the dynamic light...

		if (m_cs.bCreateLight)
		{
			LIGHTCREATESTRUCT light;

            LTFLOAT fRadiusMin = m_cs.fLightRadius;
			fRadiusMin = fRadiusMin < 20.0f ? 20.0f : (fRadiusMin > FFX_MAX_LIGHT_RADIUS ? FFX_MAX_LIGHT_RADIUS : fRadiusMin);

			light.vColor				= m_cs.vLightColor;
			light.hServerObj			= m_hServerObject;
			light.vOffset				= m_cs.vLightOffset;
			light.dwLightFlags			= FLAG_DONTLIGHTBACKFACING;
			light.fIntensityMin			= 1.0f;
			light.fIntensityMax			= 1.0f;
			light.nIntensityWaveform	= WAVE_NONE;
			light.fIntensityFreq		= 1.0f;
			light.fIntensityPhase		= 0.0f;
			light.fRadiusMin			= fRadiusMin;
			light.fRadiusMax			= fRadiusMin * 1.1f;
			light.nRadiusWaveform		= WAVE_FLICKER2;
			light.fRadiusFreq			= m_cs.fLightFreq;
			light.fRadiusPhase			= m_cs.fLightPhase;
			light.m_hLightAnim			= INVALID_LIGHT_ANIM;

			if (!m_Light.Init(&light) || !m_Light.CreateObject(m_pClientDE))
			{
                return LTFALSE;
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Update
//
//	PURPOSE:	Update the Fire Fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr || !m_pClientDE || !m_hServerObject) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
        return LTFALSE;
	}


	// Update FX...

	if (m_cs.bCreateSmoke)
	{
		m_Smoke1.Update();
	}

	if (m_cs.bCreateLight)
	{
		m_Light.Update();
	}

	m_Fire1.Update();
	m_Fire2.Update();


	// Hide/show the fire if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if (m_hSound)
			{
                g_pLTClient->KillSound(m_hSound);
                m_hSound = LTNULL;
			}

            return LTTRUE;
		}
		else
		{
			if (m_cs.bCreateSound && !m_hSound)
			{
				CString str = g_pClientButeMgr->GetSpecialFXAttributeString("FireSnd");

				m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, (char*)(LPCSTR)str,
					m_cs.fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
			}
		}
	}

	// Create the random spark particles...

	if (m_cs.bCreateSparks && GetRandom(1, 10) == 1)
	{
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
        if (!psfxMgr) return LTFALSE;

		RANDOMSPARKSCREATESTRUCT sparks;
		sparks.hServerObj = m_hServerObject;

        LTFLOAT fVel = m_fSizeAdjust * GetRandom(50.0f, 70.0f);
		fVel = (fVel < 30.0f ? 30.0f : (fVel > 100.0f ? 100.0f : fVel));

        LTVector vDir(0.0, 1.0, 0.0);
		sparks.vMinVelAdjust.Init(1, 3, 1);
		sparks.vMaxVelAdjust.Init(1, 6, 1);
		sparks.vDir	= vDir * fVel;
		sparks.nSparks = GetRandom(1, 5);
		sparks.fDuration = m_fSizeAdjust * GetRandom(1.0f, 2.0f);
        sparks.bRelToCameraPos = LTTRUE;
		sparks.fInnerCamRadius = FFX_INNER_CAM_RADIUS;
		sparks.fOuterCamRadius = FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);
		sparks.fRadius = 300.0f * m_fSizeAdjust;
		sparks.fRadius = sparks.fRadius < 100.0f ? 100.0f : (sparks.fRadius > 500.0f ? 500.0f : sparks.fRadius);

		psfxMgr->CreateSFX(SFX_RANDOMSPARKS_ID, &sparks);
	}

    return LTTRUE;
}