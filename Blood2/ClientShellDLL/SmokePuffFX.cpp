// ----------------------------------------------------------------------- //
//
// MODULE  : SmokePuffFX.cpp
//
// PURPOSE : Smoke special FX - Implementation
//
// CREATED : 3/2/98
//
// ----------------------------------------------------------------------- //

#include "SmokePuffFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"

extern DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokePuffFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

DBOOL CSmokePuffFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	m_pTextureName	= "SpriteTextures\\Particles\\ParticleSmoke2.dtx";

	SMPCREATESTRUCT* pSM = (SMPCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vColor1, pSM->vColor1);
	VEC_COPY(m_vColor2, pSM->vColor2);
	VEC_COPY(m_vMinDriftVel, pSM->vMinDriftVel);
	VEC_COPY(m_vMaxDriftVel, pSM->vMaxDriftVel);
	m_fDriftDeceleration	= pSM->fDriftDeceleration;
	VEC_COPY(m_vPos, pSM->vPos);
	m_fVolumeRadius			= pSM->fVolumeRadius;
	m_fLifeTime				= pSM->fLifeTime;
	m_fRadius				= pSM->fRadius;
	m_fParticleCreateDelta	= pSM->fParticleCreateDelta;
	m_fMinParticleLife		= pSM->fMinParticleLife;
	m_fMaxParticleLife		= pSM->fMaxParticleLife;
	m_fMaxAlpha				= pSM->fMaxAlpha;
	m_fCreateLifetime		= pSM->fCreateLifetime;
	m_nNumParticles			= pSM->nNumParticles;
	m_bIgnoreWind			= pSM->bIgnoreWind;
	m_pTextureName			= pSM->pTexture;

	m_fFadeTime				= m_fLifeTime*0.6f;
	m_fCreateLifetime = m_fLifeTime * m_fCreateLifetime;

	m_fGravity		= 0.0f;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokePuffFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CSmokePuffFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	return CBaseParticleSystemFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokePuffFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

DBOOL CSmokePuffFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_fStartTime < 0)
	{
		m_fStartTime = m_fLastTime = fTime;
	}


	// Make sure we update our position relative to the server object (if the
	// server object is valid)...

	if (m_hServerObject)
	{
		DVector vServPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vServPos);
		m_pClientDE->SetObjectPos(m_hObject, &vServPos);
	}


	// Check to see if we should just wait for last smoke puff to go away...

	DFLOAT fTimeDelta = fTime - m_fStartTime;

	if (fTimeDelta > m_fLifeTime)
	{
		return DFALSE;
	}

	if (m_fDriftDeceleration)
	{
		DFLOAT fDecel = 1 - m_pClientDE->GetFrameTime() * m_fDriftDeceleration;
		VEC_MULSCALAR(m_vMinDriftVel, m_vMinDriftVel, fDecel);
		VEC_MULSCALAR(m_vMaxDriftVel, m_vMaxDriftVel, fDecel);
	}


	DFLOAT fScale = ((m_fLifeTime - fTimeDelta) / m_fLifeTime) * m_fMaxAlpha;

	// Adjust the alpha
	DFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);



	// See if it is time to add some more smoke...

	if (fTime > m_fLastTime + m_fParticleCreateDelta && (fTimeDelta < m_fCreateLifetime) )
	{
		DVector vDriftVel, vColor, vPos;

		// What is the range of colors?

		DFLOAT fRange = m_vColor2.x - m_vColor1.x;


		// Build the individual smoke puffs...

		for (DDWORD j=0; j < m_nNumParticles; j++)
		{
			DFLOAT fX, fY, fAngle, fRadius;
			fAngle = GetRandom(-MATH_PI, MATH_PI);
			fRadius = GetRandom(0.0f, m_fVolumeRadius);
			fX = fRadius * (DFLOAT)cos(fAngle);
			fY = fRadius * (DFLOAT)sin(fAngle);
			VEC_SET(vPos,  fX, -2.0f, fY);

			VEC_SET(vDriftVel,	
					GetRandom(m_vMinDriftVel.x, m_vMaxDriftVel.x), 
					GetRandom(m_vMinDriftVel.y, m_vMaxDriftVel.y), 
					GetRandom(m_vMinDriftVel.z, m_vMaxDriftVel.z));

			if (!m_bIgnoreWind)
			{
				VEC_ADD(vDriftVel, vDriftVel, g_vWorldWindVel);
			}

			DFLOAT fOffset  = GetRandom(m_vColor1.x, m_vColor2.x);
			DFLOAT fPercent = 1.0f;

			if (fRange > 0.01)
			{
				fPercent = fOffset / fRange;
			}

			vColor.x = m_vColor1.x + fOffset;

			fOffset = fPercent * (m_vColor2.y - m_vColor1.y);
			vColor.y = m_vColor1.y + fOffset;

			fOffset = fPercent * (m_vColor2.z - m_vColor1.z);
			vColor.z = m_vColor1.z + fOffset;

			DFLOAT fLifeTime = GetRandom(m_fMinParticleLife, m_fMaxParticleLife);

			m_pClientDE->AddParticle(m_hObject, &vPos, &vDriftVel, &vColor, fLifeTime);
		}

		m_fLastTime = fTime;
	}

	return DTRUE;
}
