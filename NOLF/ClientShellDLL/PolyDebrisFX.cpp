// ----------------------------------------------------------------------- //
//
// MODULE  : PolyDebrisFX.cpp
//
// PURPOSE : Polygon Debris - Implementation
//
// CREATED : 7/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyDebrisFX.h"
#include "GameClientShell.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarPolyDebrisTrailTime;
VarTrack	g_cvarPolyDebrisScaleDist;
VarTrack	g_cvarPolyDebrisMinDistScale;
VarTrack	g_cvarPolyDebrisMaxDistScale;

CPolygonDebrisFX::~CPolygonDebrisFX()
{
	// Clear out the PolyFX list
	for (uint32 nClearLoop = 0; nClearLoop < MAX_DEBRIS; ++nClearLoop)
	{
		if (m_Polies[nClearLoop])
		{
			GetPolyLineFXBank()->Delete(m_Polies[nClearLoop]);
			m_Polies[nClearLoop] = LTNULL;
		}
	}
}

CBankedList<CPolyLineFX> *CPolygonDebrisFX::GetPolyLineFXBank()
{
	static CBankedList<CPolyLineFX> theBank;

	return &theBank;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::Init
//
//	PURPOSE:	Init the polygon debris
//
// ----------------------------------------------------------------------- //

LTBOOL CPolygonDebrisFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return LTFALSE;

	m_cs = *(POLYDEBRISCREATESTRUCT*)psfxCreateStruct;

	DEBRISCREATESTRUCT debris;

    g_pLTClient->AlignRotation(&(debris.rRot), &m_cs.vNormal, LTNULL);
	debris.vPos				= m_cs.vPos + (m_cs.vDir * m_cs.PolyDebrisFX.fDirOffset);
	debris.vMinVel			= m_cs.PolyDebrisFX.vMinVel;
	debris.vMaxVel			= m_cs.PolyDebrisFX.vMaxVel;
	debris.fMinLifeTime		= m_cs.PolyDebrisFX.fMinDuration;
	debris.fMaxLifeTime		= m_cs.PolyDebrisFX.fMaxDuration;
	debris.nNumDebris		= GetRandom(m_cs.PolyDebrisFX.nMinDebris, m_cs.PolyDebrisFX.nMaxDebris);
	debris.nMinBounce		= m_cs.PolyDebrisFX.nMinBounce;
	debris.nMaxBounce		= m_cs.PolyDebrisFX.nMaxBounce;
	debris.fGravityScale	= m_cs.PolyDebrisFX.fGravityScale;
	debris.vMinDOffset		= m_cs.PolyDebrisFX.vMinDOffset;
	debris.vMaxDOffset		= m_cs.PolyDebrisFX.vMaxDOffset;
	debris.bDirOffsetOnly	= m_cs.PolyDebrisFX.bDirOffsetOnly;

    if (!CDebrisFX::Init(&debris)) return LTFALSE;

	// Too expensive to have poly debris bouncing...

	m_ds.bBounce = LTFALSE;

	if (!g_cvarPolyDebrisTrailTime.IsInitted())
	{
		g_cvarPolyDebrisTrailTime.Init(g_pLTClient, "PolyDebrisTrailTime", LTNULL, 0.25f);
	}

	if (!g_cvarPolyDebrisScaleDist.IsInitted())
	{
		g_cvarPolyDebrisScaleDist.Init(g_pLTClient, "PolyDebrisScaleDist", LTNULL, 150.0f);
	}

	if (!g_cvarPolyDebrisMinDistScale.IsInitted())
	{
		g_cvarPolyDebrisMinDistScale.Init(g_pLTClient, "PolyDebrisMinDistScale", LTNULL, 1.0f);
	}

	if (!g_cvarPolyDebrisMaxDistScale.IsInitted())
	{
		g_cvarPolyDebrisMaxDistScale.Init(g_pLTClient, "PolyDebrisMaxDistScale", LTNULL, 1.0f);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::IsValidDebris
//
//	PURPOSE:	Is this debris object valid
//
// ----------------------------------------------------------------------- //

LTBOOL CPolygonDebrisFX::IsValidDebris(int i)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;

	return (i < m_nNumPolies && (m_Polies[i] != LTNULL));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::CreateDebris
//
//	PURPOSE:	Create a debris poly
//
// ----------------------------------------------------------------------- //

void CPolygonDebrisFX::CreateDebris(int i, LTVector vPos)
{
	if (i < 0 || i >= m_ds.nNumDebris || i != m_nNumPolies || (m_Polies[i] != LTNULL)) return;

	// Create a poly debris object...

	PLFXCREATESTRUCT pls;

    LTVector vLength = (m_cs.vDir * GetRandom(m_cs.PolyDebrisFX.fMinLength, m_cs.PolyDebrisFX.fMaxLength)) / 2.0f;

    LTVector vMinC1 = m_cs.PolyDebrisFX.vMinColor1;
    LTVector vMaxC1 = m_cs.PolyDebrisFX.vMaxColor1;
    LTVector vMinC2 = m_cs.PolyDebrisFX.vMinColor2;
    LTVector vMaxC2 = m_cs.PolyDebrisFX.vMaxColor2;

	pls.pTexture			= m_cs.PolyDebrisFX.szTexture[0] ? m_cs.PolyDebrisFX.szTexture : LTNULL;
	pls.vStartPos			= vPos - vLength;
	pls.vEndPos				= vPos + vLength;
    pls.vInnerColorStart    = LTVector(GetRandom(vMinC1.x, vMaxC1.x), GetRandom(vMinC1.y, vMaxC1.y), GetRandom(vMinC1.z, vMaxC1.z));
    pls.vInnerColorEnd      = LTVector(GetRandom(vMinC2.x, vMaxC2.x), GetRandom(vMinC2.y, vMaxC2.y), GetRandom(vMinC2.z, vMaxC2.z));
	pls.vOuterColorStart	= pls.vInnerColorStart;
	pls.vOuterColorEnd		= pls.vInnerColorEnd;
	pls.fAlphaStart			= m_cs.PolyDebrisFX.fInitialAlpha;
	pls.fAlphaEnd			= m_cs.PolyDebrisFX.fFinalAlpha;
	pls.fMinWidth			= 0.0f;
	pls.fMaxWidth			= GetRandom(m_cs.PolyDebrisFX.fMinWidth, m_cs.PolyDebrisFX.fMaxWidth);
	pls.fLifeTime			= GetDebrisLife(i);
	pls.fAlphaLifeTime		= GetDebrisLife(i);
	pls.bAdditive			= m_cs.PolyDebrisFX.bAdditive;
	pls.bMultiply			= m_cs.PolyDebrisFX.bMultiply;
	pls.bDontFadeAlphaAtEdge= !m_cs.PolyDebrisFX.bAdditive;
	pls.nWidthStyle			= m_cs.PolyDebrisFX.nStyle > PLWS_CONSTANT ? GetRandom(PLWS_BIG_TO_SMALL, PLWS_CONSTANT) : m_cs.PolyDebrisFX.nStyle;
	pls.bUseObjectRotation	= !m_cs.PolyDebrisFX.bShowTrail;
	pls.bNoZ				= m_cs.PolyDebrisFX.bShowTrail;
	pls.nNumSegments		= 1;

	pls.fMinDistMult		= 1.0f;
	pls.fMaxDistMult		= 1.0f;
	pls.fPerturb			= 0.0f;

	// Scale the width based on the distance the camera is away from the
	// origin of the debris...
	
	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    if (hCamera)
	{
		LTVector vCamPos;
		g_pLTClient->GetObjectPos(hCamera, &vCamPos);

		vCamPos -= vPos;
		LTFLOAT fScaleVal = vCamPos.Mag() / g_cvarPolyDebrisScaleDist.GetFloat();

		fScaleVal = (fScaleVal < g_cvarPolyDebrisMinDistScale.GetFloat() ? g_cvarPolyDebrisMinDistScale.GetFloat() 
			: (fScaleVal > g_cvarPolyDebrisMaxDistScale.GetFloat() ? g_cvarPolyDebrisMaxDistScale.GetFloat() : fScaleVal));

		pls.fMaxWidth *= fScaleVal;
	}

	CPolyLineFX *pNewPoly = GetPolyLineFXBank()->New();

	if (!pNewPoly->Init(&pls) ||
		!pNewPoly->CreateObject(m_pClientDE))
	{
		GetPolyLineFXBank()->Delete(pNewPoly);
		return;
	}
	else
	{
		m_Polies[m_nNumPolies] = pNewPoly;
	}

	m_nNumPolies++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::OkToRemoveDebris
//
//	PURPOSE:	See if this particular debris can be removed.
//
// ----------------------------------------------------------------------- //

LTBOOL CPolygonDebrisFX::OkToRemoveDebris(int i)
{
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::RemoveDebris
//
//	PURPOSE:	Remove the specified debris object
//
// ----------------------------------------------------------------------- //

void CPolygonDebrisFX::RemoveDebris(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;
	if (i >= m_nNumPolies) return;
	if (m_Polies[i] == LTNULL) return;

	GetPolyLineFXBank()->Delete(m_Polies[i]);
	m_Polies[i] = LTNULL;
	
	CDebrisFX::RemoveDebris(i);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::RotateDebrisToRest
//
//	PURPOSE:	Rotate the debris to the rest position
//
// ----------------------------------------------------------------------- //

void CPolygonDebrisFX::RotateDebrisToRest(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;
	if (i >= m_nNumPolies) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::SetDebrisPos
//
//	PURPOSE:	Set the debris position
//
// ----------------------------------------------------------------------- //

void CPolygonDebrisFX::SetDebrisPos(int i, LTVector vPos)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;
	if (i >= m_nNumPolies) return;
	if (m_Polies[i] == LTNULL) return;

	// Instead of moving the current poly, add another one at the
	// new position if we're showing a trail..

	if (m_cs.PolyDebrisFX.bShowTrail)
	{
		PLFXLINESTRUCT ls;

        LTVector vLength = (m_cs.vDir * GetRandom(m_cs.PolyDebrisFX.fMinLength, m_cs.PolyDebrisFX.fMaxLength)) / 2.0f;

		ls.vStartPos = vPos - vLength;

		// Get the last vert position...

		PolyLineList* pLines = m_Polies[i]->GetLines();
		if (pLines->GetLength() > 0)
		{
			PolyLine** pLine = pLines->GetItem(TLIT_LAST);
			if (pLine && *pLine)
			{
				PolyVertStruct** pVert = (*pLine)->list.GetItem(TLIT_LAST);
				if (pVert && *pVert)
				{
					ls.vStartPos = m_Polies[i]->GetVertPos((*pVert));
				}
			}
		}

        LTVector vMinC1 = m_cs.PolyDebrisFX.vMinColor1;
        LTVector vMaxC1 = m_cs.PolyDebrisFX.vMaxColor1;
        LTVector vMinC2 = m_cs.PolyDebrisFX.vMinColor2;
        LTVector vMaxC2 = m_cs.PolyDebrisFX.vMaxColor2;

		ls.vEndPos				= vPos;
        ls.vInnerColorStart     = LTVector(GetRandom(vMinC1.x, vMaxC1.x), GetRandom(vMinC1.y, vMaxC1.y), GetRandom(vMinC1.z, vMaxC1.z));
        ls.vInnerColorEnd       = LTVector(GetRandom(vMinC2.x, vMaxC2.x), GetRandom(vMinC2.y, vMaxC2.y), GetRandom(vMinC2.z, vMaxC2.z));
		ls.fAlphaStart			= m_cs.PolyDebrisFX.fInitialAlpha;
		ls.fAlphaEnd			= m_cs.PolyDebrisFX.fFinalAlpha;
        //ls.fLifeTime            = m_fDebrisLife[i] - (g_pLTClient->GetTime() - m_fStartTime);
		ls.fLifeTime			= g_cvarPolyDebrisTrailTime.GetFloat();
		ls.fLifeTime			= ls.fLifeTime < 0.0f ? 0.0f : ls.fLifeTime;
		ls.fAlphaLifeTime		= ls.fLifeTime;

		m_Polies[i]->AddLine(ls);
	}
	else
	{
		m_Polies[i]->SetPos(vPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::GetDebrisPos
//
//	PURPOSE:	Get the debris position
//
// ----------------------------------------------------------------------- //

LTBOOL CPolygonDebrisFX::GetDebrisPos(int i, LTVector & vPos)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;
    if (i >= m_nNumPolies) return LTFALSE;
	if (m_Polies[i] == LTNULL) return LTFALSE;

	vPos = m_Polies[i]->GetPos();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::SetDebrisRot
//
//	PURPOSE:	Set the debris rotation
//
// ----------------------------------------------------------------------- //

void CPolygonDebrisFX::SetDebrisRot(int i, LTRotation rRot)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;
	if (i >= m_nNumPolies) return;
	if (m_Polies[i] == LTNULL) return;

	if (!m_cs.PolyDebrisFX.bShowTrail)
	{
		m_Polies[i]->SetRot(rRot);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolygonDebrisFX::Update
//
//	PURPOSE:	Update the debris
//
// ----------------------------------------------------------------------- //

LTBOOL CPolygonDebrisFX::Update()
{
	if (!CDebrisFX::Update())
		return LTFALSE;

	for (int i=0; i < m_nNumPolies; i++)
	{
		if (m_Polies[i] == LTNULL) continue;

		// Align poly forward vector to movement direction...

		if (!m_cs.PolyDebrisFX.bShowTrail)
		{
            LTVector vDir = GetEmitter(i)->m_vLastPos - GetEmitter(i)->m_vPos;

            LTRotation rRot;
			rRot = m_Polies[i]->GetRot();
            g_pLTClient->AlignRotation(&rRot, &vDir, LTNULL);
			m_Polies[i]->SetRot(rRot);
		}

		m_Polies[i]->Update();
	}

    return LTTRUE;
}