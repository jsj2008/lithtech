// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleStreamFX.cpp
//
// PURPOSE : Special FX class for streams of particles
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#include "ParticleStreamFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "SoundTypes.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleStreamFX::Init
//
//	PURPOSE:	Init the stream
//
// ----------------------------------------------------------------------- //

DBOOL CParticleStreamFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	PSTREAMCREATESTRUCT* pSS = (PSTREAMCREATESTRUCT*)psfxCreateStruct;

	m_fRadius			= pSS->fRadius;
	m_fPosRadius		= pSS->fPosRadius;
	m_fMinVel			= pSS->fMinVel;
	m_fMaxVel			= pSS->fMaxVel;
	m_nNumParticles		= pSS->nNumParticles;
	m_fSpread			= pSS->fSpread;
	VEC_COPY(m_vColor1, pSS->vColor1);
	VEC_COPY(m_vColor2, pSS->vColor2);
	m_fAlpha			= pSS->fAlpha;
	m_fMinLife			= pSS->fMinLife;
	m_fMaxLife			= pSS->fMaxLife;
	m_fRampTime			= pSS->fRampTime;
	m_fDelay			= pSS->fDelay;
	m_bState			= pSS->bOn ? 2 : 0;
	m_bRampFlags		= pSS->bRampFlags;
	m_fGravity			= pSS->fGravity;

	m_hstrSound1		= pSS->hstrSound1;
	m_hstrSound2		= pSS->hstrSound2;
	m_hstrSound3		= pSS->hstrSound3;

	m_hstrTexture		= pSS->hstrTexture;
	m_pTextureName		= "SpriteTextures\\drop32_1.dtx";
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleStreamFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleStreamFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE ) return DFALSE;

	if(m_hstrTexture)
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	
	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(m_hServerObject)
	{
		DVector		vPos;
		DRotation	rRot;

		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		m_pClientDE->SetObjectPos(m_hObject, &vPos);

		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &m_vU, &m_vR, &m_vDir);
	}
	else
		return DFALSE;

	if(bRet)
	{
		DFLOAT		r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);
		if(m_bState)	AddParticles();
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleStreamFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CParticleStreamFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	// Update the position and direction of the particle stream based on the server object
	if(m_hServerObject)
	{
		DVector		vPos;
		DRotation	rRot;

		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		m_pClientDE->SetObjectPos(m_hObject, &vPos);

		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &m_vU, &m_vR, &m_vDir);
	}
	else
		return DFALSE;

	// Check to see if the server object is visible (determines on/off state of effect)
	DDWORD dwFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwFlags);

	if(dwFlags & USRFLG_VISIBLE)
	{
		if((m_bState == 0) || (m_bState == 3))
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			if(m_fRampTime)		m_bState = 1;
				else			m_bState = 2;
		}
	}
	else
	{
		if((m_bState == 1) || (m_bState == 2))
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			if(m_fRampTime)		m_bState = 3;
				else			m_bState = 0;
		}
	}

	// Play the correct sound according to the current state
	switch(m_bState)
	{
		case	1:
			if(m_hstrSound1)
			{
				if(m_hsSound)	m_pClientDE->KillSound(m_hsSound);
				m_hsSound = PlaySoundFromObject(m_hObject, m_pClientDE->GetStringData(m_hstrSound1),
							m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, DFALSE, DTRUE);
			}
			break;

		case	2:
			if(m_hstrSound2 && !m_hsSound)
			{
				m_hsSound = PlaySoundFromObject(m_hObject, m_pClientDE->GetStringData(m_hstrSound2),
							m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, DTRUE, DTRUE);
			}
			break;

		case	3:
			if(m_hstrSound3)
			{
				if(m_hsSound)	m_pClientDE->KillSound(m_hsSound);
				m_hsSound = PlaySoundFromObject(m_hObject, m_pClientDE->GetStringData(m_hstrSound3),
							m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, DFALSE, DTRUE);
			}
			break;
	}

	// See if it's time to add more particles... and do so
	if(m_pClientDE->GetTime() > m_fLastAddTime + m_fDelay)
		AddParticles();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleStreamFX::AddParticles
//
//	PURPOSE:	Make the liquid drops
//
// ----------------------------------------------------------------------- //

DBOOL CParticleStreamFX::AddParticles()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DVector		minOffset, maxOffset;
	DFLOAT		minVel, maxVel;
	DFLOAT		minLife, maxLife;
	DFLOAT		spread;
	DFLOAT		ratio, time = m_pClientDE->GetTime();
	DDWORD		num;

	if(!m_bState)	return	DTRUE;

	num = m_nNumParticles;
	VEC_SET(minOffset, -m_fPosRadius, -m_fPosRadius, -m_fPosRadius);
	VEC_SET(maxOffset, m_fPosRadius, m_fPosRadius, m_fPosRadius);
	minVel = m_fMinVel;
	maxVel = m_fMaxVel;
	minLife = m_fMinLife;
	maxLife = m_fMaxLife;
	spread = m_fSpread;

	if((m_bState == 1) || (m_bState == 3))
	{
		// Set the ratio to ramp the values with
		if(m_bState == 1)
		{
			ratio = (time - m_fTriggerTime) / m_fRampTime;
			if(ratio >= 1.0f)
				{ ratio = 1.0f; m_bState = 2; }
		}
		else
		{
			ratio = 1.0f - ((time - m_fTriggerTime) / m_fRampTime);
			if(ratio <= 0.0f)
				{ ratio = 0.0f; m_bState = 0; return DTRUE; }
		}

		// Check which elements of the stream we should ramp... and calculate new values
		if(m_bRampFlags & PSTREAM_RAMP_NUM)
			num = (DDWORD)(num * ratio);

		if(m_bRampFlags & PSTREAM_RAMP_OFFSET)
		{
			VEC_MULSCALAR(minOffset, minOffset, ratio);
			VEC_MULSCALAR(maxOffset, maxOffset, ratio);
		}

		if(m_bRampFlags & PSTREAM_RAMP_VEL)
		{
			minVel *= ratio;
			maxVel *= ratio;
			spread *= ratio;
		}

		if(m_bRampFlags & PSTREAM_RAMP_LIFE)
		{
			minLife *= ratio;
			maxLife *= ratio;
		}
	}

	// Add the particles one by one... (instead of AddParticles... cause I wanted the spread)
	for(DDWORD i = 0; i < num; i++)
	{
		DVector		pos, vel, vSpread, color;
		DFLOAT		life = GetRandom(minLife, maxLife);

		VEC_SET(pos, GetRandom(minOffset.x, maxOffset.x), GetRandom(minOffset.y, maxOffset.y), GetRandom(minOffset.z, maxOffset.z));
		VEC_SET(vSpread, GetRandom(-spread, spread), GetRandom(-spread, spread), GetRandom(-spread, spread));
		VEC_MULSCALAR(vel, m_vDir, maxVel);
		VEC_ADD(vel, vel, vSpread);
		VEC_NORM(vel);
		VEC_MULSCALAR(vel, vel, GetRandom(minVel, maxVel));

		GetRandomColorInRange(color);

		m_pClientDE->AddParticle(m_hObject, &pos, &vel, &color, life);
	}

	// Make sure we know when the last batch of particles were added
	m_fLastAddTime = time;
	return DTRUE;
}