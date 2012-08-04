// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailFX.cpp
//
// PURPOSE : ParticleTrail special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "ParticleTrailFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "ParticleTrailSegmentFX.h"
#include "RiotClientShell.h"
#include "RiotMsgIds.h"
#include "WeaponFXTypes.h"

extern CRiotClientShell* g_pRiotClientShell;

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
	m_bSmall = pST->bSmall;
	
	if (m_nType & PT_BLOOD)
	{
		VEC_SET(m_vDriftOffset, 60.0f, 60.0f, 60.0f);

		m_nNumPerPuff	= 1;
		VEC_SET(m_vColor1, 150.0f, 150.0f, 150.0f);
		VEC_SET(m_vColor2, 200.0f, 200.0f, 200.0f);

		m_fLifeTime		= 0.3f;
		m_fFadeTime		= 0.25f;

		m_fSegmentTime  = 0.1f;
	}
	else if (m_nType & PT_GIBSMOKE)
	{
		VEC_SET(m_vDriftOffset, 60.0f, 60.0f, 60.0f);

		m_nNumPerPuff	= 1;
		VEC_SET(m_vColor1, 100.0f, 100.0f, 100.0f);
		VEC_SET(m_vColor2, 125.0f, 125.0f, 125.0f);

		m_fLifeTime		= 0.75f;
		m_fFadeTime		= 0.25f;

		m_fSegmentTime  = 0.1f;
	}
	else if (m_nType & PT_SMOKE)
	{
		VEC_SET(m_vDriftOffset, 4.0f, 5.5f, 0.5f);

		m_nNumPerPuff	= 1;
		VEC_SET(m_vColor1, 150.0f, 150.0f, 150.0f);
		VEC_SET(m_vColor2, 230.0f, 230.0f, 230.0f);

		m_fLifeTime		= 1.5f;
		m_fFadeTime		= 0.25f;

		m_fSegmentTime  = 0.1f;
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
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
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

		pts.hServerObj = m_hServerObject;
		VEC_COPY(pts.vColor1, m_vColor1);
		VEC_COPY(pts.vColor2, m_vColor2);
		VEC_COPY(pts.vDriftOffset, m_vDriftOffset);
		pts.nType			= m_nType;
		pts.bSmall			= m_bSmall;
		pts.fLifeTime		= m_fLifeTime;
		pts.fFadeTime		= m_fFadeTime;
		pts.fOffsetTime		= m_fOffsetTime;
		pts.fRadius			= 2000.0f;
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
