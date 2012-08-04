// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailSegmentFX.cpp
//
// PURPOSE : ParticleTrail segment special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleTrailSegmentFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "GameSettings.h"
#include "GameClientShell.h"
#include "iltcustomdraw.h"
#include "VarTrack.h"

extern LTVector g_vWorldWindVel;
extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarSmokeTrailStartScale;
VarTrack	g_cvarSmokeTrailLWEndScale;
VarTrack	g_cvarSmokeTrailSWEndScale;
VarTrack	g_cvarSmokeTrailSBEndScale;
VarTrack	g_cvarSmokeTrailStartAlpha;
VarTrack	g_cvarSmokeTrailEndAlpha;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

    g_cvarSmokeTrailStartScale.Init(g_pLTClient, "SmokeTrailStartScale", LTNULL, 1.0f);
    g_cvarSmokeTrailLWEndScale.Init(g_pLTClient, "SmokeTrailLWEndScale", LTNULL, 15.0f);
    g_cvarSmokeTrailSWEndScale.Init(g_pLTClient, "SmokeTrailSWEndScale", LTNULL, 15.0f);
    g_cvarSmokeTrailSBEndScale.Init(g_pLTClient, "SmokeTrailSBEndScale", LTNULL, 15.0f);
    g_cvarSmokeTrailStartAlpha.Init(g_pLTClient, "SmokeTrailStartAlpha", LTNULL, 0.9f);
    g_cvarSmokeTrailEndAlpha.Init(g_pLTClient, "SmokeTrailEndAlpha", LTNULL, 0.0f);

	PTSCREATESTRUCT* pPTS = (PTSCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vColor1, pPTS->vColor1);
	VEC_COPY(m_vColor2, pPTS->vColor2);
	VEC_COPY(m_vDriftOffset, pPTS->vDriftOffset);
	m_nType			= pPTS->nType;
	m_fLifeTime		= pPTS->fLifeTime;
	m_fFadeTime		= pPTS->fFadeTime;
	m_fOffsetTime	= pPTS->fOffsetTime;
	m_fRadius		= pPTS->fRadius;
	m_fGravity		= pPTS->fGravity;
	m_nNumPerPuff	= pPTS->nNumPerPuff;

    m_bIgnoreWind   = LTFALSE;

	// We'll control the particle system's position...

    m_basecs.bClientControlsPos = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE || !m_hServerObject) return LTFALSE;

	if (m_nType & PT_SMOKE_LONG)
	{
		m_pTextureName = "SFX\\Smoke\\Spr\\WhiteTrailLong.spr";

        m_basecs.bAdjustParticleScale = LTTRUE;
		m_basecs.fStartParticleScale  = g_cvarSmokeTrailStartScale.GetFloat();
		m_basecs.fEndParticleScale	  = g_cvarSmokeTrailLWEndScale.GetFloat();

        m_basecs.bAdjustParticleAlpha = LTTRUE;
		m_basecs.fStartParticleAlpha  = g_cvarSmokeTrailStartAlpha.GetFloat();
		m_basecs.fEndParticleAlpha	  = g_cvarSmokeTrailEndAlpha.GetFloat();
	}
	else if (m_nType & PT_SMOKE)
	{
		m_pTextureName = "SFX\\Smoke\\Spr\\WhiteTrailShort.spr";

        m_basecs.bAdjustParticleScale = LTTRUE;
		m_basecs.fStartParticleScale  = g_cvarSmokeTrailStartScale.GetFloat();
		m_basecs.fEndParticleScale	  = g_cvarSmokeTrailSWEndScale.GetFloat();

        m_basecs.bAdjustParticleAlpha = LTTRUE;
		m_basecs.fStartParticleAlpha  = g_cvarSmokeTrailStartAlpha.GetFloat();
		m_basecs.fEndParticleAlpha	  = g_cvarSmokeTrailEndAlpha.GetFloat();
	}
	else if (m_nType & PT_SMOKE_BLACK)
	{
		m_pTextureName = "SFX\\Smoke\\Spr\\BlackTrailShort.spr";

        m_basecs.bAdjustParticleScale = LTTRUE;
		m_basecs.fStartParticleScale  = g_cvarSmokeTrailStartScale.GetFloat();
		m_basecs.fEndParticleScale	  = g_cvarSmokeTrailSBEndScale.GetFloat();

        m_basecs.bAdjustParticleAlpha = LTTRUE;
		m_basecs.fStartParticleAlpha  = g_cvarSmokeTrailStartAlpha.GetFloat();
		m_basecs.fEndParticleAlpha	  = g_cvarSmokeTrailEndAlpha.GetFloat();
	}
	else if ((m_nType & PT_GIBSMOKE))
	{
		m_pTextureName = "SFX\\Impact\\Spr\\Smoke.spr";

        m_basecs.bAdjustParticleScale = LTTRUE;
		m_basecs.fStartParticleScale  = g_cvarSmokeTrailStartScale.GetFloat();
		m_basecs.fEndParticleScale	  = g_cvarSmokeTrailSWEndScale.GetFloat();

        m_basecs.bAdjustParticleAlpha = LTTRUE;
		m_basecs.fStartParticleAlpha  = g_cvarSmokeTrailStartAlpha.GetFloat();
		m_basecs.fEndParticleAlpha	  = g_cvarSmokeTrailEndAlpha.GetFloat();
	}
	else if (m_nType & PT_BLOOD)
	{
		if (GetRandom(0, 1) == 0)
		{
			m_pTextureName = "SFX\\Particle\\Blood_1.dtx";
		}
		else
		{
			m_pTextureName = "SFX\\Particle\\Blood_2.dtx";
		}
	}

	// Determine if we are in a liquid...

    LTVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	HLOCALOBJ objList[1];
    uint32 dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
		pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
			if (pClientDE->GetContainerCode(objList[0], &dwCode))
			{
				if (IsLiquid((ContainerCode)dwCode))
				{
					// Don't create blood under water...

					if (m_nType & PT_BLOOD)
					{
                        m_bWantRemove = LTTRUE;
						m_fLifeTime = 0.0f;
						m_fFadeTime = 0.0f;
                        return LTFALSE;
					}

					m_fRadius = 500.0f;
					m_fGravity = 5.0f;
					m_nNumPerPuff *= 3;
                    m_bIgnoreWind = LTTRUE;
					m_pTextureName = DEFAULT_BUBBLE_TEXTURE;
					GetLiquidColorRange((ContainerCode)dwCode, &m_vColor1, &m_vColor2);
				}
			}
		}
	}

	return CBaseParticleSystemFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailSegmentFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailSegmentFX::Update()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    if (!CBaseParticleSystemFX::Update()) return LTFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTFALSE;

    uint8 nDetailLevel = pSettings->SpecialFXSetting();


    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        if (!m_hServerObject) return LTFALSE;

        m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;

		// Where is the server (moving) object...

        LTVector vPos, vTemp;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

		// Current position is relative to the particle system's postion (i.e.,
		// each puff of Particle is some distance away from the particle system's
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vPos, vPos, vTemp);

		VEC_COPY(m_vLastPos, vPos);
	}


	// Check to see if we should just wait for last Particle puff to go away...

	if (m_bWantRemove || (fTime > m_fStartTime + m_fFadeTime))
	{
		if (fTime > m_fLastTime + m_fLifeTime)
		{
            return LTFALSE;
		}

        LTFLOAT fScale = (m_fLifeTime - (fTime - m_fLastTime)) / m_fLifeTime;

        LTFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

        return LTTRUE;
	}


	// See if it is time to create a new Particle puff...

	if ((fTime > m_fLastTime + m_fOffsetTime) && m_hServerObject)
	{
        LTVector vCurPos, vPos, vDelta, vTemp, vDriftVel, vColor;

		// Calculate Particle puff position...

		// Where is the server (moving) object...

		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);


		// Current position is relative to the particle system's postion (i.e.,
		// each puff of Particle is some distance away from the particle system's
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vCurPos, vCurPos, vTemp);


		// How long has it been since the last Particle puff?

        LTFLOAT fTimeOffset = fTime - m_fLastTime;


		// What is the range of colors?

        LTFLOAT fRange = m_vColor2.x - m_vColor1.x;


		// Fill the distance between the last projectile position, and it's
		// current position with Particle puffs...

		int nNumSteps = (m_fLastTime > 0) ? (((m_nType & PT_BLOOD) || (m_nType & PT_GIBSMOKE)) ? 20 : 5): 1;

		if (nDetailLevel != RS_HIGH)
		{
			nNumSteps /= 2;
		}

		VEC_SUB(vTemp, vCurPos, m_vLastPos);
		VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(nNumSteps));

		VEC_COPY(vPos, m_vLastPos);

        LTFLOAT fCurLifeTime = 10.0f;
		if (nDetailLevel == RS_HIGH)
		{
			fCurLifeTime /= 2;
		}

        LTFLOAT fLifeTimeOffset = fTimeOffset / float(nNumSteps);

        LTFLOAT fOffset = 0.5f;

		int nNumPerPuff = GetNumParticles(m_nNumPerPuff);

		for (int i=0; i < nNumSteps; i++)
		{
			// Build the individual Particle puffs...

			for (int j=0; j < nNumPerPuff; j++)
			{
				VEC_COPY(vTemp, vPos);

				if (m_bIgnoreWind)
				{
					VEC_SET(vDriftVel, GetRandom(-m_vDriftOffset.x*2.0f, -m_vDriftOffset.x),
									   GetRandom(5.0f, 6.0f),
									   GetRandom(-m_vDriftOffset.z, m_vDriftOffset.z));
				}
				else
				{
					VEC_SET(vDriftVel, g_vWorldWindVel.x + GetRandom(-m_vDriftOffset.x*2.0f, -m_vDriftOffset.x),
									   g_vWorldWindVel.y + GetRandom(5.0f, 6.0f),
									   g_vWorldWindVel.z + GetRandom(-m_vDriftOffset.z, m_vDriftOffset.z));
				}

				vTemp.x += GetRandom(-fOffset, fOffset);
				vTemp.y += GetRandom(-fOffset, fOffset);
				vTemp.z += GetRandom(-fOffset, fOffset);

				GetRandomColorInRange(vColor);

				m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fCurLifeTime);
			}

			VEC_ADD(vPos, vPos, vDelta);
			fCurLifeTime += fLifeTimeOffset;
		}

		m_fLastTime = fTime;

		VEC_COPY(m_vLastPos, vCurPos);
	}

    return LTTRUE;
}