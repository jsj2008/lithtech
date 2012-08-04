// ----------------------------------------------------------------------- //
//
// MODULE  : SparksFX.cpp
//
// PURPOSE : Sparks special FX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "SparksFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::Init
//
//	PURPOSE:	Init the sparks
//
// ----------------------------------------------------------------------- //

LTBOOL CSparksFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	SCREATESTRUCT* pSS = (SCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vPos, pSS->vPos);
	VEC_COPY(m_vDir, pSS->vDir);
	VEC_COPY(m_vColor1, pSS->vColor1);
	VEC_COPY(m_vColor2, pSS->vColor2);
	m_nSparks			= pSS->nSparks;
	m_fDuration			= pSS->fDuration;
	m_fEmissionRadius	= pSS->fEmissionRadius;
	m_hstrTexture		= pSS->hstrTexture;
	m_fRadius			= pSS->fRadius;
	m_fGravity			= pSS->fGravity;

	m_pTextureName = "SpecialFX\\ParticleTextures\\Spark_yellow_1.dtx";

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CSparksFX::CreateObject(ILTClient *pClientDE)
{
	if (!pClientDE ) return LTFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = const_cast<char *>(pClientDE->GetStringData(m_hstrTexture));
	}

	LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet)
	{
		bRet = AddSparks();
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::Update
//
//	PURPOSE:	Update the smoke trail (add smoke)
//
// ----------------------------------------------------------------------- //

LTBOOL CSparksFX::Update()
{
	if(!m_hObject || !m_pClientDE) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (fTime > m_fStartTime + m_fDuration)
	{
		return LTFALSE;
	}


	// Fade sparks over duration...

	LTFLOAT fScale = (m_fDuration - (fTime - m_fStartTime)) / m_fDuration;

	// m_pClientDE->SetParticleSystemColorScale(m_hObject, fScale);

	LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::AddSparks
//
//	PURPOSE:	Make the sparks
//
// ----------------------------------------------------------------------- //```
LTBOOL CSparksFX::AddSparks()
{
	if(!m_hObject || !m_pClientDE) return LTFALSE;

	LTVector vMinOffset, vMaxOffset, vMinVel, vMaxVel;
	VEC_SET(vMinOffset, -m_fEmissionRadius, -m_fEmissionRadius, -m_fEmissionRadius);
	VEC_SET(vMaxOffset, m_fEmissionRadius, m_fEmissionRadius, m_fEmissionRadius);

	LTFLOAT fVelOffset = VEC_MAG(m_vDir);
	VEC_NORM(m_vDir);

	LTRotation rRot;
	m_pClientDE->Math()->AlignRotation(rRot, m_vDir, LTVector(0, 1, 0));

	LTVector vF, vU, vR;
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 1.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}

	LTVector vTemp;

	VEC_MULSCALAR(vMinVel, vF, fVelOffset * .025f); 
	VEC_MULSCALAR(vMaxVel, vF, fVelOffset); 

	VEC_MULSCALAR(vTemp, vR, -fVelOffset);
	VEC_ADD(vMinVel, vMinVel, vTemp);

	VEC_MULSCALAR(vTemp, vR, fVelOffset);
	VEC_ADD(vMaxVel, vMaxVel, vTemp);

	VEC_MULSCALAR(vTemp, vU, -fVelOffset);
	VEC_ADD(vMinVel, vMinVel, vTemp);

	VEC_MULSCALAR(vTemp, vU, fVelOffset);
	VEC_ADD(vMaxVel, vMaxVel, vTemp);

	m_pClientDE->AddParticles(m_hObject, m_nSparks,
							  &vMinOffset, &vMaxOffset,		// Position offset
							  &vMinVel, &vMaxVel,			// Velocity
							  &m_vColor1, &m_vColor2,		// Color
							  m_fDuration, m_fDuration);
	
	m_fStartTime = m_pClientDE->GetTime();

	return LTTRUE;
}