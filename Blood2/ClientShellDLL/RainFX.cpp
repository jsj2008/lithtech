// ----------------------------------------------------------------------- //
//
// MODULE  : RainFX.cpp
//
// PURPOSE : Rain special FX - Implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "RainFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRainFX::CRainFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CRainFX::CRainFX() : CBaseParticleSystemFX()
{
	m_bFirstUpdate			= DTRUE;
	m_nParticlesAdded		= 0;
	m_fTimeLen				= 0.0f;
	m_fLastTime				= 0.0f;
	m_fNextUpdate			= 0.01f;
	m_dwFlags				= 0;
	m_fLifetime				= 1.0f;
	m_fAreaDensity			= 0.0f;
	m_bGravity				= DTRUE;
	m_fSpread				= 0.0f;
	m_fTimeLimit			= 0.0f;

	VEC_SET(m_vColor1, 200, 255, 255);
	VEC_SET(m_vColor2, 40, 50, 50);
/*
	VEC_SET(m_vColor[0], 255, 255, 255);
	VEC_SET(m_vColor[1], 200, 200, 200);
	VEC_SET(m_vColor[2], 150, 150, 150);
	VEC_SET(m_vColor[3], 100, 100, 100);
	VEC_SET(m_vColor[4], 50, 50, 50);
	VEC_SET(m_vColor[5], 50, 50, 50);
*/
	VEC_INIT(m_vMinOffset);
	VEC_INIT(m_vMaxOffset);
	VEC_INIT(m_vMinVel);
	VEC_INIT(m_vMaxVel);
	VEC_INIT(m_vDims);
	VEC_INIT(m_vDirection);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRainFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CRainFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	// Set up our creation struct...

	RAINCREATESTRUCT* pPS = (RAINCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pPS;

	m_bGravity = pPS->bGravity;

	m_fRadius = 200.0f * pPS->fParticleScale;

	VEC_COPY(m_vDims, pPS->vDims)
	VEC_COPY(m_vDirection, pPS->vDirection)
	m_fLifetime = pPS->fLifetime;
	m_fSpread = pPS->fSpread;
	m_fTimeLimit = pPS->fTimeLimit;
	m_fPulse = pPS->fPulse;

	m_fAreaDensity = ((m_vDims.x * m_vDims.z) / 250) * pPS->fDensity;
	
	// Set our (parent's) flags...

	m_dwFlags = m_cs.dwFlags;

//	m_pTextureName	= "spritetextures\\Particles\\particle2.dtx";
	m_pTextureName	= "spritetextures\\blooddrop2.dtx";		//SCHLEGZ 5/3/98 10:19:28 PM: a better liquid particle

	VEC_COPY(m_vColor1, pPS->vColor1);
	VEC_COPY(m_vColor2, pPS->vColor2);
/*
	if (pPS->bBloodRain)	// Blood rain is red :)
	{
		VEC_COPY(m_vColor[0], 255, 30, 30);
		VEC_SET(m_vColor[1], 255, 30, 30);
		VEC_SET(m_vColor[2], 200, 24, 24);
		VEC_SET(m_vColor[3], 150, 18, 18);
		VEC_SET(m_vColor[4], 100, 12, 12);
		VEC_SET(m_vColor[5], 50, 6, 6);
	}
	else
	{
		VEC_SET(m_vColor[0], 200, 255, 255);
		VEC_SET(m_vColor[1], 160, 200, 200);
		VEC_SET(m_vColor[2], 120, 150, 150);
		VEC_SET(m_vColor[3], 80, 100, 100);
		VEC_SET(m_vColor[4], 40, 50, 50);
		VEC_SET(m_vColor[5], 40, 50, 50);
	}
*/
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRainFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CRainFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (!m_bGravity)
		m_fGravity = 0.0f;

	return CBaseParticleSystemFX::CreateObject(pClientDE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRainFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CRainFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		m_fLastTime = m_pClientDE->GetTime();

		m_bFirstUpdate = DFALSE;
		return DTRUE;
	}


	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return DTRUE;
	}

	//set our new position based on the server object
	if(m_hServerObject)
	{
		DVector vPos,vUp,vRight;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		m_pClientDE->SetObjectPos(m_hObject,&vPos);

		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->SetObjectRotation(m_hObject,&rRot);
		m_pClientDE->GetRotationVectors(&rRot,&vUp,&vRight,&m_vDirection);
	}
	else
		return DFALSE;

	// Total time it's been active...

	m_fTimeLen += fTime - m_fLastTime;
	m_fLastTime = fTime;

	//SCHLEGZ 5/1/98 7:07:33 PM: added time limit to the rain effect itself
	if(m_fTimeLimit && m_fTimeLen > m_fTimeLimit)
	{
		WantRemove();
		return DFALSE;
	}


	// How much time it's accounted for by adding particles...

	DFLOAT fAddedTime = m_nParticlesAdded / m_fAreaDensity;


	// Ok, how many to add this frame....

	int nToAdd = (int) floor( m_fAreaDensity * (m_fTimeLen - fAddedTime) );
	m_nParticlesAdded += nToAdd;

	DVector vel, vOffset;
	DBOOL bAddTrails = DFALSE;

	VEC_COPY(vel, m_vDirection);

	if (vel.x == 0.0f && vel.y == 0.0f && vel.z == 0.0f)
		vel.y = -1.0f;
	VEC_COPY(vOffset, vel);
	VEC_NORM(vOffset);
	bAddTrails = (!m_bGravity || (vel.x == 0 && vel.z == 0 && m_fSpread == 0));

	if (!bAddTrails) nToAdd *= 6;

	for(int i=0; i < nToAdd; i++)
	{
		DVector vPos, vTmp, vTmpVel;

		vPos.x = GetRandom(-m_vDims.x, m_vDims.x);
		vPos.y = GetRandom(-m_vDims.y, m_vDims.y);
		vPos.z = GetRandom(-m_vDims.z, m_vDims.z);
		VEC_MULSCALAR(vTmpVel, vel, GetRandom(0.8f, 1.2f));

		if (m_fSpread)
		{
			DFLOAT fOffset = GetRandom(-m_fSpread, m_fSpread);

			vTmpVel.x += fOffset;

			fOffset = GetRandom(-m_fSpread, m_fSpread);
			vTmpVel.z += fOffset;
		}

		// Add the particles
		if (bAddTrails)
		{
			// Set up colors for particle stream
			DVector vPartColors[6];
			vPartColors[0].x  = GetRandom(m_vColor1.x, m_vColor2.x);
			vPartColors[0].y  = GetRandom(m_vColor1.y, m_vColor2.y);
			vPartColors[0].z  = GetRandom(m_vColor1.z, m_vColor2.z);
			VEC_MULSCALAR(vPartColors[1], vPartColors[0], 0.8f);
			VEC_MULSCALAR(vPartColors[2], vPartColors[0], 0.6f);
			VEC_MULSCALAR(vPartColors[3], vPartColors[0], 0.4f);
			VEC_MULSCALAR(vPartColors[4], vPartColors[0], 0.2f);
			VEC_MULSCALAR(vPartColors[5], vPartColors[0], 0.2f);
			
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[0], m_fLifetime);
			VEC_MULSCALAR(vTmp, vOffset, -3.0f);
			VEC_ADD(vPos, vPos, vTmp);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[1], m_fLifetime);
			VEC_MULSCALAR(vTmp, vOffset, -4.0f);
			VEC_ADD(vPos, vPos, vTmp);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[2], m_fLifetime);
			VEC_MULSCALAR(vTmp, vOffset, -5.0f);
			VEC_ADD(vPos, vPos, vTmp);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[3], m_fLifetime);
			VEC_MULSCALAR(vTmp, vOffset, -6.0f);
			VEC_ADD(vPos, vPos, vTmp);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[4], m_fLifetime);
			VEC_MULSCALAR(vTmp, vOffset, -7.0f);
			VEC_ADD(vPos, vPos, vTmp);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColors[5], m_fLifetime);
		}
		else
		{
			DVector vPartColor;
			vPartColor.x  = GetRandom(m_vColor1.x, m_vColor2.x);
			vPartColor.y  = GetRandom(m_vColor1.y, m_vColor2.y);
			vPartColor.z  = GetRandom(m_vColor1.z, m_vColor2.z);
			m_pClientDE->AddParticle(m_hObject, &vPos, &vTmpVel, &vPartColor, m_fLifetime);
		}
	}

	// Determine when next update should occur...

	if (m_cs.fPulse > 0.001f) 
	{
		m_fNextUpdate = m_cs.fPulse * GetRandom(0.01f, 1.0f);
	}
	else 
	{
		m_fNextUpdate = 0.01f;
	}

	return DTRUE;
}


