// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailFX.cpp
//
// PURPOSE : ParticleTrail special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleTrailFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ParticleTrailSegmentFX.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "WeaponFXTypes.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarSmokeTrailNumPerPuff;
VarTrack	g_cvarSmokeTrailColor1R;
VarTrack	g_cvarSmokeTrailColor1G;
VarTrack	g_cvarSmokeTrailColor1B;
VarTrack	g_cvarSmokeTrailColor2R;
VarTrack	g_cvarSmokeTrailColor2G;
VarTrack	g_cvarSmokeTrailColor2B;
VarTrack	g_cvarSmokeTrailLifetime;
VarTrack	g_cvarSmokeTrailLWFadetime;
VarTrack	g_cvarSmokeTrailSWFadetime;
VarTrack	g_cvarSmokeTrailSBFadetime;
VarTrack	g_cvarSmokeTrailSegtime;
VarTrack	g_cvarSmokeTrailDriftOffsetX;
VarTrack	g_cvarSmokeTrailDriftOffsetY;
VarTrack	g_cvarSmokeTrailDriftOffsetZ;
VarTrack	g_cvarSmokeTrailParticleRadius;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailFX::Init
//
//	PURPOSE:	Init the Particle trail
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PTCREATESTRUCT* pST = (PTCREATESTRUCT*)psfxCreateStruct;
	m_nType  = pST->nType;

    g_cvarSmokeTrailNumPerPuff.Init(g_pLTClient, "SmokeTrailNumPerPuff", LTNULL, 1.0f);
    g_cvarSmokeTrailColor1R.Init(g_pLTClient, "SmokeTrailColor1R", LTNULL, 150.0f);
    g_cvarSmokeTrailColor1G.Init(g_pLTClient, "SmokeTrailColor1G", LTNULL, 150.0f);
    g_cvarSmokeTrailColor1B.Init(g_pLTClient, "SmokeTrailColor1B", LTNULL, 150.0f);
    g_cvarSmokeTrailColor2R.Init(g_pLTClient, "SmokeTrailColor2R", LTNULL, 230.0f);
    g_cvarSmokeTrailColor2G.Init(g_pLTClient, "SmokeTrailColor2G", LTNULL, 230.0f);
    g_cvarSmokeTrailColor2B.Init(g_pLTClient, "SmokeTrailColor2B", LTNULL, 230.0f);
    g_cvarSmokeTrailLifetime.Init(g_pLTClient, "SmokeTrailLifetime", LTNULL, 1.5f);
    g_cvarSmokeTrailLWFadetime.Init(g_pLTClient, "SmokeTrailLWFadetime", LTNULL, 0.2f);
    g_cvarSmokeTrailSWFadetime.Init(g_pLTClient, "SmokeTrailSWFadetime", LTNULL, 0.2f);
    g_cvarSmokeTrailSBFadetime.Init(g_pLTClient, "SmokeTrailSBFadetime", LTNULL, 0.3f);
    g_cvarSmokeTrailSegtime.Init(g_pLTClient, "SmokeTrailSegtime", LTNULL, 0.1f);
    g_cvarSmokeTrailDriftOffsetX.Init(g_pLTClient, "SmokeTrailDriftOffsetX", LTNULL, 0.0f);
    g_cvarSmokeTrailDriftOffsetY.Init(g_pLTClient, "SmokeTrailDriftOffsetY", LTNULL, 20.0f);
    g_cvarSmokeTrailDriftOffsetZ.Init(g_pLTClient, "SmokeTrailDriftOffsetZ", LTNULL, 5.0f);
    g_cvarSmokeTrailParticleRadius.Init(g_pLTClient, "SmokeTrailParticleRadius", LTNULL, 2000.0f);

	if (m_nType & PT_BLOOD)
	{
		m_vDriftOffset.Init(60.0f, 60.0f, 60.0f);

		m_nNumPerPuff	= 1;
		m_vColor1.Init(150.0f, 150.0f, 150.0f);
		m_vColor2.Init(200.0f, 200.0f, 200.0f);

		m_fLifeTime		= 0.3f;
		m_fFadeTime		= 0.25f;

		m_fSegmentTime  = 0.1f;
	}
	else if (m_nType & PT_GIBSMOKE)
	{
		m_vDriftOffset.Init(60.0f, 60.0f, 60.0f);

		m_nNumPerPuff	= 1;
		m_vColor1.Init(100.0f, 100.0f, 100.0f);
		m_vColor2.Init(125.0f, 125.0f, 125.0f);

		m_fLifeTime		= 0.75f;
		m_fFadeTime		= 0.25f;

		m_fSegmentTime  = 0.1f;
	}
	else if ((m_nType & PT_SMOKE) || (m_nType & PT_SMOKE_LONG) || (m_nType & PT_SMOKE_BLACK))
	{
		m_vDriftOffset.Init(
			g_cvarSmokeTrailDriftOffsetX.GetFloat(),
			g_cvarSmokeTrailDriftOffsetY.GetFloat(),
			g_cvarSmokeTrailDriftOffsetZ.GetFloat());

		m_nNumPerPuff = (int)g_cvarSmokeTrailNumPerPuff.GetFloat();

		m_vColor1.Init(
			g_cvarSmokeTrailColor1R.GetFloat(),
			g_cvarSmokeTrailColor1G.GetFloat(),
			g_cvarSmokeTrailColor1B.GetFloat());

		m_vColor2.Init(
			g_cvarSmokeTrailColor2R.GetFloat(),
			g_cvarSmokeTrailColor2G.GetFloat(),
			g_cvarSmokeTrailColor2B.GetFloat());

		m_fLifeTime	= g_cvarSmokeTrailLifetime.GetFloat();
		if ((m_nType & PT_SMOKE) || (m_nType & PT_SMOKE_BLACK))
		{
			m_fLifeTime /= 2.0f; 
		}
	
		if (m_nType & PT_SMOKE)
		{
			m_fFadeTime	= g_cvarSmokeTrailSWFadetime.GetFloat();
		}
		else if (m_nType & PT_SMOKE_LONG)
		{
			m_fFadeTime	= g_cvarSmokeTrailLWFadetime.GetFloat();
		}
		else // Short black
		{
			m_fFadeTime	= g_cvarSmokeTrailSBFadetime.GetFloat();
		}

		m_fSegmentTime = g_cvarSmokeTrailSegtime.GetFloat();
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr || !m_pClientDE || !m_hServerObject) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
        return LTFALSE;
	}


	// See if it is time to create a new trail segment...

	if ((m_fStartTime < 0) || (fTime > m_fStartTime + m_fSegmentTime))
	{
		PTSCREATESTRUCT pts;

		pts.hServerObj		= m_hServerObject;
		pts.vColor1			= m_vColor1;
		pts.vColor2			= m_vColor2;
		pts.vDriftOffset	= m_vDriftOffset;
		pts.nType			= m_nType;
		pts.fLifeTime		= m_fLifeTime;
		pts.fFadeTime		= m_fFadeTime;
		pts.fOffsetTime		= m_fOffsetTime;
		pts.fRadius			= g_cvarSmokeTrailParticleRadius.GetFloat();
		pts.fGravity		= 0.0f;
		pts.nNumPerPuff		= m_nNumPerPuff;

		if (m_nType & PT_BLOOD)
		{
			pts.fRadius	 = 600.0f;
		}
		else if (m_nType & PT_GIBSMOKE)
		{
			pts.fRadius	 = 1250.0f;
		}

		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLETRAILSEG_ID, &pts);

		// Let each Particle segment do its initial update...

		if (pFX) pFX->Update();

		m_fStartTime = fTime;
	}

    return LTTRUE;
}