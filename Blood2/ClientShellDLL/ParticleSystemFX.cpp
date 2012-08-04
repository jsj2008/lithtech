// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.cpp
//
// PURPOSE : ParticleSystem special FX - Implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "ParticleSystemFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX() : CBaseParticleSystemFX()
{
	m_bFirstUpdate			= DTRUE;
	m_fLastTime				= 0.0f;
	m_fNextUpdate			= 0.01f;

	VEC_INIT(m_vMinOffset);
	VEC_INIT(m_vMaxOffset);
	VEC_INIT(m_vMinVel);
	VEC_INIT(m_vMaxVel);

	m_dwLastFrameUserFlags = USRFLG_VISIBLE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	// Set up our creation struct...

	PSCREATESTRUCT* pPS = (PSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pPS;

	// Set our (parent's) flags...

	m_dwFlags  = m_cs.dwFlags;
	m_fRadius  = m_cs.fParticleRadius;
	m_fGravity = m_cs.fGravity;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (m_cs.hstrTextureName)
	{
		m_pTextureName = pClientDE->GetStringData(m_cs.hstrTextureName);
	}

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
		DRotation rRot;
		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		pClientDE->SetObjectRotation(m_hObject, &rRot);

		pClientDE->GetObjectUserFlags(m_hServerObject, &m_dwLastFrameUserFlags);
		if (!(m_dwLastFrameUserFlags & USRFLG_VISIBLE))
		{
			pClientDE->SetObjectFlags(m_hObject, 0);
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::Update()
{
	if (!m_hObject || !m_pClientDE || m_bWantRemove) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		DBOOL bRet = DFALSE;

		if (dwUserFlags != m_dwLastFrameUserFlags)
		{
			if (!(dwUserFlags & USRFLG_VISIBLE))
			{
				m_pClientDE->SetObjectFlags(m_hObject, 0);
				bRet = DTRUE;
			}
			else
			{
				m_pClientDE->SetObjectFlags(m_hObject, FLAG_VISIBLE);
			}
		}

		m_dwLastFrameUserFlags = dwUserFlags;

		if (bRet) 
		{
			m_fLastTime = fTime;
			return DTRUE;
		}
	}


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;

		VEC_SET(m_vMinOffset, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius);
		VEC_SET(m_vMaxOffset, m_cs.fEmissionRadius, m_cs.fEmissionRadius, m_cs.fEmissionRadius);

		VEC_SET(m_vMinVel, -m_cs.fVelocityOffset, m_cs.fMinimumVelocity, -m_cs.fVelocityOffset);
		VEC_SET(m_vMaxVel, m_cs.fVelocityOffset, m_cs.fMaximumVelocity, m_cs.fVelocityOffset);
						
		m_bFirstUpdate = DFALSE;
	}


	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return DTRUE;
	}


	// Ok, how many to add this frame....

	int nToAdd = (int) floor(m_cs.fParticlesPerSecond * (fTime - m_fLastTime));

	m_pClientDE->AddParticles(m_hObject, nToAdd,
		&m_vMinOffset, &m_vMaxOffset,			// Position offset
		&m_vMinVel, &m_vMaxVel,					// Velocity
		&(m_cs.vColor1), &(m_cs.vColor2),		// Color
		m_cs.fParticleLifetime, m_cs.fParticleLifetime);


	// Determine when next update should occur...

	if (m_cs.fBurstWait > 0.001f) 
	{
		m_fNextUpdate = m_cs.fBurstWait * GetRandom(0.01f, 1.0f);
	}
	else 
	{
		m_fNextUpdate = 0.01f;
	}

	
	// Rotate the particle system...

	if (m_cs.fRotationVelocity != 0.0f)
	{
		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hObject, &rRot);
		m_pClientDE->EulerRotateY(&rRot, m_pClientDE->GetFrameTime() * m_cs.fRotationVelocity);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	m_fLastTime = fTime;

	return DTRUE;
}


