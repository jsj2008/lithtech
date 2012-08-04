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
#include "clientheaders.h"
#include "ClientUtilities.h"

extern LTVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	m_pTextureName	= "SpecialFX\\ParticleTextures\\smoke.dtx";

	SMCREATESTRUCT* pSM = (SMCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vPos, pSM->vPos);
	VEC_COPY(m_vColor1, pSM->vColor1);
	VEC_COPY(m_vColor2, pSM->vColor1);
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
	m_hstrTexture			= pSM->hstrTexture;

	m_fGravity		= 0.0f;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::CreateObject(ILTClient *pClientDE)
{
	if (!pClientDE ) return LTFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = const_cast<char *>(pClientDE->GetStringData(m_hstrTexture));
	}

	LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject)
	{
		pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Update()
{
	if(!m_hObject || !m_pClientDE) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fStartTime < 0)
	{
		m_fStartTime = m_fLastTime = fTime;
	}


	// Make sure we update our position relative to the server object (if the
	// server object is valid)...

	if (m_hServerObject)
	{
		LTVector vServPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vServPos);
		m_pClientDE->SetObjectPos(m_hObject, &vServPos);
	}


	// Check to see if we should just wait for last smoke puff to go away...

	if (fTime > m_fStartTime + m_fLifeTime)
	{
		if (fTime > m_fLastTime + m_fMaxParticleLife)
		{
			return LTFALSE;
		}
		
		return LTTRUE;
	}


	// See if it is time to add some more smoke...

	if (fTime > m_fLastTime + m_fParticleCreateDelta)
	{
		LTVector vDriftVel, vColor, vPos;

		// What is the range of colors?

		LTFLOAT fRange = m_vColor2.x - m_vColor1.x;


		// Build the individual smoke puffs...

		for (uint32 j=0; j < m_nNumParticles; j++)
		{
			VEC_SET(vPos,  GetRandom(-m_fVolumeRadius, m_fVolumeRadius),
					-2.0f, GetRandom(-m_fVolumeRadius, m_fVolumeRadius));

			VEC_SET(vDriftVel,	
					GetRandom(m_vMinDriftVel.x, m_vMaxDriftVel.x), 
					GetRandom(m_vMinDriftVel.y, m_vMaxDriftVel.y), 
					GetRandom(m_vMinDriftVel.z, m_vMaxDriftVel.z));

			if (!m_bIgnoreWind)
			{
				VEC_ADD(vDriftVel, vDriftVel, g_vWorldWindVel);
			}

			GetRandomColorInRange(vColor);

			LTFLOAT fLifeTime = GetRandom(m_fMinParticleLife, m_fMaxParticleLife);

			m_pClientDE->AddParticle(m_hObject, &vPos, &vDriftVel, &vColor, fLifeTime);
		}

		m_fLastTime = fTime;
	}

	return LTTRUE;
}
