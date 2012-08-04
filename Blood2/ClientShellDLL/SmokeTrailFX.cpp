// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeTrailFX.cpp
//
// PURPOSE : SmokeTrail special FX - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "SmokeTrailFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "SmokeTrailSegmentFX.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeTrailFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeTrailFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	STCREATESTRUCT* pST = (STCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vVel, pST->vVel);
	m_bSmall = pST->bSmall;
	
	m_nNumPerPuff	= 10;

	VEC_SET(m_vColor1, 175.0f, 175.0f, 175.0f);
	VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);

	m_fLifeTime		= 0.75f;
	m_fFadeTime		= 0.2f;
	m_fOffsetTime	= 0.025f;

	m_fSegmentTime  = 0.1f;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeTrailFX::Update
//
//	PURPOSE:	Update the smoke trail (add smoke)
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeTrailFX::Update()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr || !m_pClientDE || !m_hServerObject) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
		return DFALSE;
	}


	// See if it is time to create a new trail segment...
	if ((m_fStartTime < 0) || (fTime > m_fStartTime + m_fSegmentTime))
	{
		STSCREATESTRUCT sts;

		sts.hServerObj = m_hServerObject;
		VEC_COPY(sts.vVel, m_vVel);
		VEC_COPY(sts.vColor1, m_vColor1);
		VEC_COPY(sts.vColor2, m_vColor2);
		sts.bSmall			= m_bSmall;
		sts.fLifeTime		= m_fLifeTime;
		sts.fFadeTime		= m_fFadeTime;
		sts.fOffsetTime		= m_fOffsetTime;
		sts.fRadius			= 1000.0f;
		sts.fGravity		= 15.0f;
		sts.nNumPerPuff		= m_nNumPerPuff;

		if(VEC_MAG(sts.vVel) < 1.0f)
		{
			sts.fGravity = 25.0f;
			sts.fOffsetTime = 0.1f;
			sts.fRadius = 500.0f;
			sts.nNumPerPuff = 1;
			sts.fLifeTime = 1.0f;
		}

 		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SMOKETRAILSEG_ID, &sts, DFALSE, this);

		// Let each smoke segment do its initial update...
		if (pFX) pFX->Update();

		m_fStartTime = fTime;
	}

	return DTRUE;
}
