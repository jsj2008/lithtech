// ----------------------------------------------------------------------- //
//
// MODULE  : TracerFX.cpp
//
// PURPOSE : Tracer special FX - Implementation
//
// CREATED : 1/21/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TracerFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarTracerSegments;
VarTrack	g_cvarTracerLifetime;
VarTrack	g_cvarTracerVelocity;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTracerFX::Init
//
//	PURPOSE:	Init the tracer fx
//
// ----------------------------------------------------------------------- //

LTBOOL CTracerFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Don't support server-side versions of this fx...

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTracerFX::Init
//
//	PURPOSE:	Init the tracer fx
//
// ----------------------------------------------------------------------- //

LTBOOL CTracerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	TRCREATESTRUCT* pTR = (TRCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pTR;

    if (!m_cs.pTracerFX) return LTFALSE;

	m_vDir  = m_cs.vEndPos - m_cs.vStartPos;
	m_fDist = m_vDir.Mag();
	m_vDir.Norm();

	if (!g_cvarTracerSegments.IsInitted())
	{
        g_cvarTracerSegments.Init(g_pLTClient, "TracerSegments", NULL, 1.0f);
	}

	if (!g_cvarTracerLifetime.IsInitted())
	{
        g_cvarTracerLifetime.Init(g_pLTClient, "TracerLifetime", NULL, 0.2f);
	}

	if (!g_cvarTracerVelocity.IsInitted())
	{
        g_cvarTracerVelocity.Init(g_pLTClient, "TracerVeloctity", NULL, -1.0f);
	}

	if (g_cvarTracerVelocity.GetFloat() > 0.0f)
	{
		m_fVelocity = g_cvarTracerVelocity.GetFloat();
	}
	else
	{
		m_fVelocity = m_cs.pTracerFX->fVelocity;
	}


	// Calculate life time...

	if (m_fVelocity > 0.0f)
	{
		m_fLifetime = m_fDist / m_fVelocity;
	}
	else
	{
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTracerFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

LTBOOL CTracerFX::Update()
{
    if (!m_pClientDE || !m_cs.pTracerFX) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate  = LTFALSE;
		m_fStartTime	= fTime;

		PLFXCREATESTRUCT pls;

		m_vTracerPos = m_cs.vStartPos + (m_vDir * g_pGameClientShell->GetFrameTime() * m_fVelocity);

		pls.pTexture				= m_cs.pTracerFX->szTexture[0] ? m_cs.pTracerFX->szTexture : LTNULL;
		pls.vStartPos				= m_cs.vStartPos;
		pls.vEndPos					= m_vTracerPos;
		pls.vInnerColorStart		= m_cs.pTracerFX->vColor;
		pls.vInnerColorEnd			= m_cs.pTracerFX->vColor;
        pls.vOuterColorStart        = m_cs.pTracerFX->vColor;
        pls.vOuterColorEnd          = m_cs.pTracerFX->vColor;
		pls.fAlphaStart				= m_cs.pTracerFX->fInitialAlpha;
		pls.fAlphaEnd				= m_cs.pTracerFX->fFinalAlpha;
		pls.fMinWidth				= 0;
		pls.fMaxWidth				= m_cs.pTracerFX->fWidth;
		pls.fMinDistMult			= 1.0f;
		pls.fMaxDistMult			= 1.0f;
		pls.fLifeTime				= g_cvarTracerLifetime.GetFloat();
		//pls.fLifeTime				= m_fLifetime;
		pls.fAlphaLifeTime			= pls.fLifeTime;
		pls.fPerturb				= 0.0f;
        pls.bAdditive               = LTTRUE;
        pls.bNoZ                    = LTTRUE;
		pls.nWidthStyle				= PLWS_CONSTANT;
		pls.nNumSegments			= (int)g_cvarTracerSegments.GetFloat();
        pls.bAlignUp                = LTTRUE;

		if (!m_Tracer.Init(&pls) || !m_Tracer.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}
	}
	else if (fTime > m_fStartTime + m_fLifetime)
	{
        return LTFALSE;
	}


	// Draw the tracer...

	m_Tracer.Update();


	// Update the tracer positions...

	if (m_Tracer.HasBeenDrawn())
	{
 		// Use trail technology, instead of moving the tracer...

		PLFXLINESTRUCT ls;

		ls.vStartPos = m_vTracerPos;
		ls.vEndPos	 = m_vTracerPos + (m_vDir * g_pGameClientShell->GetFrameTime() * m_fVelocity);
		
		m_vTracerPos = ls.vEndPos;

		// Get the last vert position...

		PolyLineList* pLines = m_Tracer.GetLines();
		if (pLines->GetLength() > 0)
		{
			PolyLine** pLine = pLines->GetItem(TLIT_LAST);
			if (pLine && *pLine)
			{
				PolyVertStruct** pVert = (*pLine)->list.GetItem(TLIT_LAST);
				if (pVert && *pVert)
				{
					ls.vStartPos = m_Tracer.GetVertPos((*pVert));
				}
			}
		}

		ls.vInnerColorStart		= m_cs.pTracerFX->vColor;
		ls.vInnerColorEnd		= m_cs.pTracerFX->vColor;
		ls.vOuterColorStart		= m_cs.pTracerFX->vColor;
		ls.vOuterColorEnd		= m_cs.pTracerFX->vColor;
		ls.fAlphaStart			= m_cs.pTracerFX->fInitialAlpha;
		ls.fAlphaEnd			= m_cs.pTracerFX->fFinalAlpha;
		ls.fLifeTime			= g_cvarTracerLifetime.GetFloat();
        //ls.fLifeTime            = m_fLifetime - (g_pLTClient->GetTime() - m_fStartTime);
		ls.fLifeTime			= ls.fLifeTime < 0.0f ? 0.0f : ls.fLifeTime;
		ls.fAlphaLifeTime		= ls.fLifeTime;

		m_Tracer.AddLine(ls);
	}

    return LTTRUE;
}