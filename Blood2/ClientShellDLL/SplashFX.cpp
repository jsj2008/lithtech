// ----------------------------------------------------------------------- //
//
// MODULE  : SplashFX.cpp
//
// PURPOSE : Splash special FX - Implementation
//
// CREATED : 6/23/98
//
// ----------------------------------------------------------------------- //

#include "SplashFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CSplashFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	SPLASHCREATESTRUCT* pSS = (SPLASHCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vPos, pSS->vPos);
	VEC_COPY(m_vDir, pSS->vDir);
	m_fRadius			= pSS->fRadius;
	m_fPosRadius		= pSS->fPosRadius;
	m_fHeight			= pSS->fHeight;
	m_fDensity			= pSS->fDensity;
	m_fSpread			= pSS->fSpread;
	VEC_COPY(m_vColor1, pSS->vColor1);
	VEC_COPY(m_vColor2, pSS->vColor2);
	m_fSprayTime		= pSS->fSprayTime;
	m_fDuration			= pSS->fDuration;
	m_fGravity			= pSS->fGravity;

	m_hstrTexture		= pSS->hstrTexture;
	m_pTextureName		= "SpriteTextures\\drop32_1.dtx";
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CSplashFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	}
	
	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet)
	{
		bRet = AddParticles();
		m_fStartTime = m_pClientDE->GetTime();
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CSplashFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if(m_bWantRemove)
	{
		if(fTime > m_fStartTime + m_fDuration + m_fSprayTime)
		{
			return DFALSE;
		}
	}

	if(fTime < m_fStartTime + m_fSprayTime)
		AddParticles();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::AddParticles
//
//	PURPOSE:	Make the liquid drops
//
// ----------------------------------------------------------------------- //

DBOOL CSplashFX::AddParticles()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DVector	start, vel, spread;//, tempColor;

	for(DFLOAT i = 0; i < m_fDensity; i += 1.0f)
	{
//		VEC_SET(tempColor, GetRandom(m_vColor1.x, m_vColor2.x), GetRandom(m_vColor1.y, m_vColor2.y), GetRandom(m_vColor1.z, m_vColor2.z));

		VEC_SET(start, GetRandom(-m_fPosRadius, m_fPosRadius), GetRandom(-m_fPosRadius, m_fPosRadius), GetRandom(-m_fPosRadius, m_fPosRadius));
		VEC_SET(spread, GetRandom(-m_fSpread, m_fSpread), GetRandom(-m_fSpread, m_fSpread), GetRandom(-m_fSpread, m_fSpread));
		VEC_MULSCALAR(vel, m_vDir, m_fHeight);
		VEC_ADD(vel, vel, spread);

		if(GetRandom(0,1))
			m_pClientDE->AddParticle(m_hObject, &start, &vel, &m_vColor1, m_fDuration);
		else
			m_pClientDE->AddParticle(m_hObject, &start, &vel, &m_vColor2, m_fDuration);
	}
	return DTRUE;
}