// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeFX.cpp
//
// PURPOSE : Smoke special FX - Implementation
//
// CREATED : 3/2/98
//
// ----------------------------------------------------------------------- //

#include "SmokeFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"

extern DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	m_pTextureName	= "SpriteTextures\\Particles\\ParticleSmoke2.dtx";

	SMCREATESTRUCT* pSM = (SMCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vColor1, pSM->vColor1);
	VEC_COPY(m_vColor2, pSM->vColor2);
	VEC_COPY(m_vMinDriftVel, pSM->vMinDriftVel);
	VEC_COPY(m_vMaxDriftVel, pSM->vMaxDriftVel);
	m_fVolumeRadius			= pSM->fVolumeRadius;
	m_fLifeTime				= pSM->fLifeTime;
	m_fRadius				= pSM->fRadius;
	m_fParticleCreateDelta	= pSM->fParticleCreateDelta;
	m_fMinParticleLife		= pSM->fMinParticleLife;
	m_fMaxParticleLife		= pSM->fMaxParticleLife;
	m_nNumParticles			= pSM->nNumParticles;
	m_bIgnoreWind			= pSM->bIgnoreWind;
	m_fSegmentTime			= pSM->fSegmentTime;
	m_hstrTexture			= pSM->hstrTexture;

	m_fGravity		= 0.0f;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	}
	pClientDE->GetObjectPos(m_hServerObject, &m_vPos);

	return CSpecialFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeFX::Update()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...
	m_fLifeTime -= m_pClientDE->GetFrameTime();
	if (m_fLifeTime <= 0)
	{
		if (m_hstrTexture)
		{
			m_pClientDE->FreeString(m_hstrTexture);
			m_hstrTexture = DNULL;
		}
		return DFALSE;
	}

	// See if it is time to create a new puff...

	if ((m_fStartTime < 0) || (fTime > m_fStartTime + m_fSegmentTime))
	{
		SMPCREATESTRUCT sps;
		DFLOAT fParticleLife = GetRandom(m_fMinParticleLife, m_fMaxParticleLife);

		sps.hServerObj			= DNULL;
		VEC_COPY(sps.vColor1, m_vColor1);
		VEC_COPY(sps.vColor2, m_vColor2);
		VEC_COPY(sps.vMinDriftVel, m_vMinDriftVel);
		VEC_COPY(sps.vMaxDriftVel, m_vMaxDriftVel);
		VEC_COPY(sps.vPos, m_vPos);
		sps.fVolumeRadius		= m_fVolumeRadius;
		sps.fLifeTime			= fParticleLife;
		sps.fRadius				= m_fRadius;
		sps.fParticleCreateDelta = m_fParticleCreateDelta;
		sps.fMinParticleLife	= fParticleLife;
		sps.fMaxParticleLife	= fParticleLife;
		sps.fMaxAlpha			= 0.9f;
		sps.fCreateLifetime		= 0.25f;
		sps.fDriftDeceleration	= 0.0f;
		sps.nNumParticles		= m_nNumParticles;
		sps.bIgnoreWind			= m_bIgnoreWind;
		sps.pTexture			= m_pTextureName;

 		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SMOKEPUFF_ID, &sps, DFALSE, this);

		// Let each smoke segment do its initial update...

		if (pFX) pFX->Update();

		m_fStartTime = fTime;
	}

	return DTRUE;
}
