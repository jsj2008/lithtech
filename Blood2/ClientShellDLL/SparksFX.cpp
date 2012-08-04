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
#include "cpp_client_de.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::Init
//
//	PURPOSE:	Init the sparks
//
// ----------------------------------------------------------------------- //

DBOOL CSparksFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	SCREATESTRUCT* pSS = (SCREATESTRUCT*)psfxCreateStruct;
	ROT_COPY(m_rRot, pSS->rRot);
	VEC_COPY(m_vPos, pSS->vPos);
	VEC_COPY(m_vDir, pSS->vDir);
	VEC_COPY(m_vColor1, pSS->vColor1);
	VEC_COPY(m_vColor2, pSS->vColor2);
	m_nSparks			= pSS->nSparks;
	m_fDuration			= pSS->fDuration;
	m_fEmissionRadius	= pSS->fEmissionRadius;
	m_fRadius			= pSS->fRadius;
	m_fGravity			= pSS->fGravity;
	m_bFadeColors		= pSS->bFadeColors;

	if( m_hstrTexture )
		g_pClientDE->FreeString( m_hstrTexture );
	m_hstrTexture		= g_pClientDE->CopyString( pSS->hstrTexture );
	m_pTextureName = "SpriteTextures\\Particles\\Particle1.dtx";

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CSparksFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	}

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

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

DBOOL CSparksFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
		if (fTime > m_fStartTime + m_fDuration)
		{
			return DFALSE;
		}
	}


	// Fade alpha over duration...

	DFLOAT fScale = (m_fDuration - (fTime - m_fStartTime)) / m_fDuration;

	DFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	if (m_bFadeColors)
	{
		DFLOAT fOffset = fScale * (m_vColor2.x - m_vColor1.x);
		r = m_vColor1.x + fOffset;

		fOffset = fScale * (m_vColor2.y - m_vColor1.y);
		g = m_vColor1.y + fOffset;

		fOffset = fScale * (m_vColor2.z - m_vColor1.z);
		b = m_vColor1.z + fOffset;
	}
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::AddSparks
//
//	PURPOSE:	Make the sparks
//
// ----------------------------------------------------------------------- //

DBOOL CSparksFX::AddSparks()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DVector vMinOffset, vMaxOffset, vMinVel, vMaxVel;
	VEC_SET(vMinOffset, -m_fEmissionRadius, -m_fEmissionRadius, -m_fEmissionRadius);
	VEC_SET(vMaxOffset, m_fEmissionRadius, m_fEmissionRadius, m_fEmissionRadius);

	DFLOAT fVelOffset = VEC_MAG( m_vDir );
	VEC_NORM( m_vDir );
	VEC_SET(vMinVel, fVelOffset, fVelOffset, fVelOffset);
	VEC_SET(vMaxVel, fVelOffset, fVelOffset, fVelOffset);

	VEC_SUB(vMinVel, m_vDir, vMinVel);
	VEC_ADD(vMaxVel, m_vDir, vMaxVel);

	// Don't allow sparks to go through walls/floors/ceiling..
	// Basically, for each component of the direction vector, if 
	// the value is 0, the above calculation was correct.  
	// However, if it is non-zero, don't allow the sparks to
	// move opposite the direction...

	if (!(-0.001 < m_vDir.x && m_vDir.x < 0.001))
	{
		if (m_vDir.x < 0) vMaxVel.x = 0;
		else vMinVel.x = 0;
	}

	if (!(-0.001 < m_vDir.y && m_vDir.y < 0.001))
	{
		if (m_vDir.y < 0) vMaxVel.y = 0;
		else vMinVel.y = 0;
	}

	if (!(-0.001 < m_vDir.z && m_vDir.z < 0.001))
	{
		if (m_vDir.z < 0) vMaxVel.z = 0;
		else vMinVel.z = 0;
	}

	m_pClientDE->AddParticles( m_hObject, m_nSparks,
		&vMinOffset, &vMaxOffset,		// Position offset
		&vMinVel, &vMaxVel,				// Velocity
		&m_vColor1, m_bFadeColors ? &m_vColor1 : &m_vColor2,		// Color
		m_fDuration, m_fDuration);
	
	m_fStartTime = m_pClientDE->GetTime();

	return DTRUE;
}