// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleShower.cpp
//
// PURPOSE : ParticleShower special FX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleShowerFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::Init
//
//	PURPOSE:	Init the sparks
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *(PARTICLESHOWERCREATESTRUCT*)psfxCreateStruct;

	m_vColor1		= m_cs.vColor1;
	m_vColor2		= m_cs.vColor2;
	m_fGravity		= m_cs.fGravity;
	m_fRadius		= m_cs.fRadius;
	m_vPos			= m_cs.vPos;
	m_pTextureName	= m_cs.pTexture;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (CBaseParticleSystemFX::CreateObject(pClientDE))
	{
		return AddParticles();
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::Update
//
//	PURPOSE:	Update the particle shower
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::Update()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (fTime > m_fStartTime + m_cs.fDuration)
	{
        return LTFALSE;
	}


	// Fade particles over duration...

    LTFLOAT fScale = (m_cs.fDuration - (fTime - m_fStartTime)) / m_cs.fDuration;

    LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::AddParticles
//
//	PURPOSE:	Make the particles
//
// ----------------------------------------------------------------------- //```

LTBOOL CParticleShowerFX::AddParticles()
{
    if(!m_hObject || !m_pClientDE) return LTFALSE;

    LTVector vMinOffset, vMaxOffset, vMinVel, vMaxVel;
	VEC_SET(vMinOffset, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius);
	VEC_SET(vMaxOffset, m_cs.fEmissionRadius, m_cs.fEmissionRadius, m_cs.fEmissionRadius);

    LTFLOAT fVelOffset = m_cs.vDir.Mag();
	m_cs.vDir.Normalize();

    LTRotation rRot(m_cs.vDir, LTVector(0.0f, 1.0f, 0.0f));

    LTVector vF, vU, vR;
	vF = rRot.Forward();
	vU = rRot.Up();
	vR = rRot.Right();

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

	vMinVel = vF * (fVelOffset * .025f);
	vMaxVel = vF * fVelOffset;

	vMinVel += vR * -fVelOffset;
	vMaxVel += vR * fVelOffset;
	vMinVel += vU * -fVelOffset;
	vMaxVel += vU * fVelOffset;

	int nParticles = GetNumParticles(m_cs.nParticles);

	m_pClientDE->AddParticles(m_hObject, nParticles,
							  &vMinOffset, &vMaxOffset,		// Position offset
							  &vMinVel, &vMaxVel,			// Velocity
							  &m_vColor1, &m_vColor2,		// Color
							  m_cs.fDuration, m_cs.fDuration);

	m_fStartTime = m_pClientDE->GetTime();

    return LTTRUE;
}