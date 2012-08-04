// ----------------------------------------------------------------------- //
//
// MODULE  : BeamFX.cpp
//
// PURPOSE : Beam special FX - Implementation
//
// CREATED : 5/15/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BeamFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBeamFX::Init
//
//	PURPOSE:	Init the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBeamFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Don't support server-side versions of this fx...

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBeamFX::Init
//
//	PURPOSE:	Init the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBeamFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	BEAMCREATESTRUCT* pBeam = (BEAMCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pBeam;

    if (!m_cs.pBeamFX) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBeamFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

LTBOOL CBeamFX::Update()
{
    if (!m_cs.pBeamFX) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate  = LTFALSE;
		m_fStartTime	= fTime;

		PLFXCREATESTRUCT pls;

		pls.pTexture				= m_cs.pBeamFX->szTexture[0] ? m_cs.pBeamFX->szTexture : LTNULL;
		pls.vStartPos				= m_cs.vStartPos;
		pls.vEndPos					= m_cs.vEndPos;
		pls.vInnerColorStart		= m_cs.pBeamFX->vColor;
		pls.vInnerColorEnd			= m_cs.pBeamFX->vColor;
		pls.vOuterColorStart		= m_cs.pBeamFX->vColor;
		pls.vOuterColorEnd			= m_cs.pBeamFX->vColor;
        //pls.vOuterColorStart        = LTVector(0, 0, 0);
        //pls.vOuterColorEnd          = LTVector(0, 0, 0);
		pls.fAlphaStart				= m_cs.pBeamFX->fInitialAlpha;
		pls.fAlphaEnd				= m_cs.pBeamFX->fFinalAlpha;
		pls.fMinWidth				= 0;
		pls.fMaxWidth				= m_cs.pBeamFX->fWidth;
		pls.fMinDistMult			= 1.0f;
		pls.fMaxDistMult			= 1.0f;
		pls.fLifeTime				= m_cs.pBeamFX->fDuration;
		pls.fAlphaLifeTime			= m_cs.pBeamFX->fDuration;
		pls.fPerturb				= 0.0f;
		pls.bAdditive				= LTTRUE;
		pls.bNoZ					= LTTRUE;
		pls.nWidthStyle				= PLWS_CONSTANT;
		pls.nNumSegments			= 3;
		pls.bAlignUp				= m_cs.pBeamFX->bAlignUp;
		pls.bAlignFlat				= m_cs.pBeamFX->bAlignFlat;

		if (!m_Beam.Init(&pls) || !m_Beam.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}
	}
	else if (fTime > m_fStartTime + m_cs.pBeamFX->fDuration)
	{
        return LTFALSE;
	}


	// Draw the beam...

	m_Beam.Update();


	// Update the beam...

	if (m_Beam.HasBeenDrawn())
	{

	}

    return LTTRUE;
}