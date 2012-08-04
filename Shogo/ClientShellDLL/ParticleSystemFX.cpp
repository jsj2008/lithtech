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
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"


#define MAX_PS_TIME_DELTA 0.2f



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX() : CBaseParticleSystemFX()
{
	m_bFirstUpdate			= LTTRUE;
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

LTBOOL CParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	// Set up our creation struct...

	PSCREATESTRUCT* pPS = (PSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pPS;

	// Set our (parent's) flags...

	m_dwFlags  = m_cs.dwFlags;
	m_fRadius  = m_cs.fParticleRadius;
	m_fGravity = m_cs.fGravity;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleSystemFX::CreateObject(ILTClient *pClientDE)
{
	if (!pClientDE ) return LTFALSE;

	if (m_cs.hstrTextureName)
	{
		m_pTextureName = const_cast<char *>(pClientDE->GetStringData(m_cs.hstrTextureName));
	}

	LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
		LTRotation rRot;
		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		pClientDE->SetObjectRotation(m_hObject, &rRot);

		pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_User, m_dwLastFrameUserFlags);
		if (!(m_dwLastFrameUserFlags & USRFLG_VISIBLE))
		{
			pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);
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

LTBOOL CParticleSystemFX::Update()
{
	if (!m_hObject || !m_pClientDE || m_bWantRemove) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
		uint32 dwUserFlags;
		m_pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		LTBOOL bRet = LTFALSE;

		if (dwUserFlags != m_dwLastFrameUserFlags)
		{
			if (!(dwUserFlags & USRFLG_VISIBLE))
			{
				m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);
				bRet = LTTRUE;
			}
			else
			{
				m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}
		}

		m_dwLastFrameUserFlags = dwUserFlags;

		if (bRet) 
		{
			m_fLastTime = fTime;
			return LTTRUE;
		}
	}


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;

		VEC_SET(m_vMinOffset, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius);
		VEC_SET(m_vMaxOffset, m_cs.fEmissionRadius, m_cs.fEmissionRadius, m_cs.fEmissionRadius);

		VEC_SET(m_vMinVel, -m_cs.fVelocityOffset, m_cs.fMinimumVelocity, -m_cs.fVelocityOffset);
		VEC_SET(m_vMaxVel, m_cs.fVelocityOffset, m_cs.fMaximumVelocity, m_cs.fVelocityOffset);
						
		m_bFirstUpdate = LTFALSE;
	}


	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return LTTRUE;
	}


	// Ok, how many to add this frame....
	LTFLOAT timeDelta = LTMIN(fTime - m_fLastTime, MAX_PS_TIME_DELTA);
	int nToAdd = (int) floor(m_cs.fParticlesPerSecond * timeDelta);

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
		LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hObject, &rRot);
		m_pClientDE->Math()->EulerRotateY(rRot, m_pClientDE->GetFrameTime() * m_cs.fRotationVelocity);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	m_fLastTime = fTime;

	return LTTRUE;
}


