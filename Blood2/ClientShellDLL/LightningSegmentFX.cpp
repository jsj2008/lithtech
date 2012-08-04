// ----------------------------------------------------------------------- //
//
// MODULE  : LightningSegmentFX.cpp
//
// PURPOSE : Special FX class for lightning-like instant particle streams
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#include "LightningSegmentFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningSegmentFX::Init
//
//	PURPOSE:	Init the segment
//
// ----------------------------------------------------------------------- //

DBOOL CLightningSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	LSEGMENTCREATESTRUCT	*pLS = (LSEGMENTCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vOffset, pLS->vOffset);
	VEC_COPY(m_vNextOffset, pLS->vNextOffset);
	VEC_COPY(m_vColor1, pLS->vColor);
	VEC_COPY(m_vColor2, pLS->vColor);
	m_fIncrement	= pLS->fIncrement;
	m_fAlpha		= pLS->fAlpha;
	m_fRadius		= pLS->fRadius;
	m_fDuration		= pLS->fDuration;
	m_fFadeTime		= pLS->fFadeTime;
	m_nNumParticles	= pLS->nNumParticles;
	m_hstrTexture	= pLS->hstrTexture;

	m_fGravity		= 0;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningSegmentFX::CreateObject()
//
//	PURPOSE:	Create the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CLightningSegmentFX::CreateObject(CClientDE* pClientDE)
{
	if(!pClientDE) return DFALSE;

	if(m_hstrTexture)
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	else
		m_pTextureName	= "SpriteTextures\\Particles\\Particle1.dtx";

	int ret = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(ret)
	{
		DFLOAT		r, g, b, a;
		DRotation	rRot;
		DVector		vDir, vU;

		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
		VEC_SUB(vDir, m_vNextOffset, m_vOffset);
		m_pClientDE->AlignRotation(&rRot, &vDir, &vU);

		m_pClientDE->SetObjectPos(m_hObject, &m_vOffset);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);

		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);

		CreateParticles();
		m_fStartTime = m_pClientDE->GetTime();
	}

	return ret;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningSegmentFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CLightningSegmentFX::Update()
{
	if(!m_pClientDE) return DFALSE;

	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;

	// Check to see if we should start fading the particles out
	if(m_fFadeTime && (fTime >= (m_fDuration - m_fFadeTime)))
	{
		DFLOAT		r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);

		a = m_fAlpha * (1.0f - ((fTime - (m_fDuration - m_fFadeTime)) / m_fFadeTime));
		if(a < 0.0f)	a = 0.0f;

		m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
	}

	// Return TRUE to keep... FALSE to remove
	return	(fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningSegmentFX::CreateParticles
//
//	PURPOSE:	Create the particles
//
// ----------------------------------------------------------------------- //

DBOOL CLightningSegmentFX::CreateParticles()
{
	DVector		tempPos, tempPosOffset, vel;
	VEC_SET(vel, 0.0f, 0.0f, 0.0f);
	VEC_SET(tempPosOffset, 0.0f, 0.0f, m_fIncrement);

	for(DDWORD i = 0; i < m_nNumParticles; i++)
	{
		m_pClientDE->AddParticle(m_hObject, &tempPos, &vel, &m_vColor1, m_fDuration);
		VEC_ADD(tempPos, tempPos, tempPosOffset);
	}

	return	DTRUE;
}