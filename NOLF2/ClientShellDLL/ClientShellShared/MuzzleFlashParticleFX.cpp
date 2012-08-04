// ----------------------------------------------------------------------- //
//
// MODULE  : MuzzleFlashParticleFX.cpp
//
// PURPOSE : MuzzleFlash special FX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MuzzleFlashParticleFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "iltmodel.h"
#include "ilttransform.h"

#define MFPFX_INFINITE_DURATION 1000000.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Init
//
//	PURPOSE:	Init the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CBaseParticleSystemFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	// Don't support server-side versions of this fx...

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Init
//
//	PURPOSE:	Init the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	MFPCREATESTRUCT* pMF = (MFPCREATESTRUCT*)psfxCreateStruct;

	m_cs = *((MFPCREATESTRUCT*)pMF);

    if (!m_cs.pPMuzzleFX) return LTFALSE;

	// Make sure parent fx has correct values...

	m_basecs.bAdditive = m_cs.bAdditive	= m_cs.pPMuzzleFX->bAdditive;
	m_basecs.bMultiply = m_cs.bMultiply	= m_cs.pPMuzzleFX->bMultiply;

	m_vPos		= m_cs.vPos;
	m_rRot		= m_cs.rRot;
	m_fRadius	= m_cs.pPMuzzleFX->fRadius;
	m_fGravity	= 0.0f;

	// Set our server object to our fired from so we get notified when
	// the fired from goes away...

	if (!m_hServerObject)
	{
		m_hServerObject = m_cs.hFiredFrom;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (m_cs.pPMuzzleFX->szFile[0])
	{
		m_pTextureName = m_cs.pPMuzzleFX->szFile;
	}

    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet)
	{
		bRet = AddMuzzleFlash();
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Reset
//
//	PURPOSE:	Reset the muzzle flash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Reset(MFPCREATESTRUCT & mfcs)
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

	m_cs = mfcs;

    if (!m_cs.pPMuzzleFX) return LTFALSE;

	m_vPos			= m_cs.vPos;
	m_rRot			= m_cs.rRot;
	m_fRadius		= m_cs.pPMuzzleFX->fRadius;
	m_fGravity		= 0.0f;

	if (m_cs.pPMuzzleFX->szFile[0])
	{
		m_pTextureName = m_cs.pPMuzzleFX->szFile;
	}

	RemoveAllParticles();

	CBaseParticleSystemFX::SetupSystem();

	return AddMuzzleFlash();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Update()
{
    if (!m_hObject || !m_pClientDE || !m_cs.pPMuzzleFX) return LTFALSE;

	// If we have a fired from, it should be the same as our
	// server object.  If our server object has been removed, in
	// this case, we should go away...

    if (m_cs.hFiredFrom && !m_hServerObject) return LTFALSE;


    LTFLOAT fTime = m_pClientDE->GetTime();

    LTFLOAT fDuration = m_cs.bPlayerView ? MFPFX_INFINITE_DURATION : m_cs.pPMuzzleFX->fDuration;

	// Check to see if we should go away...

	if (fTime > m_fStartTime + fDuration)
	{
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::AddMuzzleFlash
//
//	PURPOSE:	Make the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::AddMuzzleFlash()
{
    if (!m_hObject || !m_pClientDE || !m_cs.pPMuzzleFX) return LTFALSE;

    LTParticle* pParticle = LTNULL;

    LTVector vF, vU, vR;
	vU = m_cs.rRot.Up();
	vR = m_cs.rRot.Right();
	vF = m_cs.rRot.Forward();

    LTFLOAT fLength = m_cs.pPMuzzleFX->fLength;

    LTVector vPos(0, 0, 0), vZero(0, 0, 0), vColor;
	int nNumParticles = GetNumParticles(m_cs.pPMuzzleFX->nNumParticles);
    LTFLOAT fFDelta = fLength / (LTFLOAT)nNumParticles;

    LTFLOAT fDuration = m_cs.bPlayerView ? MFPFX_INFINITE_DURATION : m_cs.pPMuzzleFX->fDuration;

	// Add all the particles...stepping along the forward vector...

    for (LTFLOAT fFOffset = 0.0f; fFOffset < fLength; fFOffset += fFDelta)
	{
		// Add particle at current position along the forward vector...

		GetRandomColorInRange(vColor);
		pParticle = m_pClientDE->AddParticle(m_hObject, &vPos, &vZero, &vColor, fDuration);

		// Adjust particle size...

		if (pParticle)
		{
            LTFLOAT fDistPercent = (fFOffset / fLength);
			fDistPercent = fDistPercent <= 0.33f ? GetRandom(0.1f, 0.33f) :
				(fDistPercent <= 0.5f ? GetRandom(0.5f, 1.0f) : GetRandom(0.5f, 1.0f));

            LTFLOAT fScale = (fDistPercent * m_cs.pPMuzzleFX->fMaxScale);
			pParticle->m_Size = m_fRadius * (1.0f + fScale);
		}

		vPos.z += fFDelta;
	}

	m_fStartTime = m_pClientDE->GetTime();

    return LTTRUE;
}