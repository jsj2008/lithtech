// ----------------------------------------------------------------------- //
//
// MODULE  : PolyLineFX.cpp
//
// PURPOSE : Poly line special FX - Implementation
//
// CREATED : 01/25/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyLineFX.h"
#include "iltclient.h"
#include "SFXMgr.h"
#include "GameClientShell.h"
#include "DynamicLightFX.h"
#include "GameButes.h"
#include "VarTrack.h"

VarTrack g_vtTestingTexture;
VarTrack g_vtTextureAddr;
VarTrack g_vtTextureColor;
VarTrack g_vtTextureAlpha;
VarTrack g_vtTexture;

extern CGameClientShell* g_pGameClientShell;

// The global vertex structure bank..
CBankedList<PolyVertStruct> g_PolyVertStructBank;

// The global PolyLine bank
CBankedList<PolyLine> g_PolyLineBank;

static LTMatrix GetCameraTransform(HOBJECT hCamera)
{
    LTVector vPos, vRight, vUp, vForward;
    LTRotation dRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &dRot);
    g_pLTClient->GetRotationVectors(&dRot, &vUp, &vRight, &vForward);

	vPos.x = -vPos.x;
	vPos.y = -vPos.y;
	vPos.z = -vPos.z;

    LTMatrix mTran, mRot, mFull;

	Mat_SetBasisVectors(&mRot, &vRight, &vUp, &vForward);
	MatTranspose3x3(&mRot);

	Mat_Identity(&mTran);
	mTran.m[0][3] = vPos.x;
	mTran.m[1][3] = vPos.y;
	mTran.m[2][3] = vPos.z;

	MatMul(&mFull, &mRot, &mTran);

	return mFull;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::CPolyLineFX
//
//	PURPOSE:	CPolyLineFX ctor
//
// ----------------------------------------------------------------------- //

CPolyLineFX::CPolyLineFX() : CBasePolyDrawFX(), m_bHasBeenDrawn(LTFALSE)
{
	m_Lines.Init(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::~CPolyLineFX
//
//	PURPOSE:	CPolyLineFX dtor
//
// ----------------------------------------------------------------------- //

CPolyLineFX::~CPolyLineFX()
{
	DeleteLines();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::Init
//
//	PURPOSE:	Init the poly line fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	// Client-side only...

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::Init
//
//	PURPOSE:	Init the poly line fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBasePolyDrawFX::Init(psfxCreateStruct)) return LTFALSE;

	PLFXCREATESTRUCT* pplfx = (PLFXCREATESTRUCT*)psfxCreateStruct;

	m_cs   = *pplfx;
	m_vPos = m_cs.vStartPos;

	if (!g_vtTextureAddr.IsInitted())
	{
		g_vtTextureAddr.Init(g_pLTClient, "TexAddr", LTNULL, 0.0f);
	}

	if (!g_vtTextureColor.IsInitted())
	{
		g_vtTextureColor.Init(g_pLTClient, "TexColor", LTNULL, 0.0f);
	}

	if (!g_vtTextureAlpha.IsInitted())
	{
		g_vtTextureAlpha.Init(g_pLTClient, "TexAlpha", LTNULL, 0.0f);
	}

	if (!g_vtTexture.IsInitted())
	{
		g_vtTexture.Init(g_pLTClient, "Tex", "fxtest09", 0.0f);
	}

	if (!g_vtTestingTexture.IsInitted())
	{
		g_vtTestingTexture.Init(g_pLTClient, "TexTest", LTNULL, 0.0f);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::ReInit
//
//	PURPOSE:	ReInit the polyline fx (Assumes CreateObject() was already
//				called)
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::ReInit(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!m_pClientDE) return LTFALSE;

	PLFXCREATESTRUCT* pplfx = (PLFXCREATESTRUCT*)psfxCreateStruct;

	m_cs   = *pplfx;
	m_vPos = m_cs.vStartPos;

	Setup();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::CreateObject
//
//	PURPOSE:	Create object associated the object
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::CreateObject(ILTClient *pClientDE)
{
    if (!CBasePolyDrawFX::CreateObject(pClientDE)) return LTFALSE;

	// Validate our init info...

    if (m_cs.nNumSegments < 1) return LTFALSE;


	// Set up the poly line...

	return Setup();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::Setup
//
//	PURPOSE:	Setup the line used to draw the poly line
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::Setup()
{
	// Clear list if necessary...
	DeleteLines();

    m_bHasBeenDrawn = LTFALSE;


	// Add the first (only?) line...

	PLFXLINESTRUCT ls;

	ls.vStartPos		= m_cs.vStartPos;
	ls.vEndPos			= m_cs.vEndPos;
	ls.fAlphaStart		= m_cs.fAlphaStart;
	ls.fAlphaEnd		= m_cs.fAlphaEnd;
	ls.fLifeTime		= m_cs.fLifeTime;
	ls.fAlphaLifeTime	= m_cs.fAlphaLifeTime;
	ls.vInnerColorStart	= m_cs.vInnerColorStart;
	ls.vInnerColorEnd	= m_cs.vInnerColorEnd;
	ls.vOuterColorStart	= m_cs.vOuterColorStart;
	ls.vOuterColorEnd	= m_cs.vOuterColorEnd;

	return AddLine(ls);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::AddLine
//
//	PURPOSE:	Add another line segment...This allows lines to be added
//				dynamically :)
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::AddLine(PLFXLINESTRUCT ls)
{
	PolyLine* pLine = g_PolyLineBank.New();
    if (!pLine) return LTFALSE;

    pLine->list.Init(LTFALSE);

	pLine->fAlphaStart		= ls.fAlphaStart;
	pLine->fAlphaEnd		= ls.fAlphaEnd;
	pLine->fLifeTime		= ls.fLifeTime;
	pLine->fAlphaLifeTime	= ls.fAlphaLifeTime;
	pLine->vInnerColorStart	= ls.vInnerColorStart;
	pLine->vInnerColorEnd	= ls.vInnerColorEnd;
	pLine->vOuterColorStart	= ls.vOuterColorStart;
	pLine->vOuterColorEnd	= ls.vOuterColorEnd;
    pLine->fStartTime       = g_pLTClient->GetTime();

	if (SetupLine(pLine, ls.vStartPos, ls.vEndPos))
	{
		m_Lines.AddTail(pLine);
	}
	else
	{
		g_PolyLineBank.Delete(pLine);
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::DeleteLines
//
//	PURPOSE:	Clears out the line list
//
// ----------------------------------------------------------------------- //

void CPolyLineFX::DeleteLines()
{
	// Manually delete the PolyLine pointers since they're in a bank..
	PolyLine** pCurLine = m_Lines.GetItem(TLIT_FIRST);
	while (pCurLine)
	{
		if (*pCurLine)
		{
			g_PolyLineBank.Delete(*pCurLine);
			*pCurLine = LTNULL;
		}
		pCurLine = m_Lines.GetItem(TLIT_NEXT);
	}
	// Clear the list
	m_Lines.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::SetupLine
//
//	PURPOSE:	Setup the verts for a line segment
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::SetupLine(PolyLine* pLine, LTVector vStartPos, LTVector vEndPos)
{
    if (!pLine) return LTFALSE;

	// Make all the verticies relative to the object's position...

    LTVector vPos = vStartPos;
    LTVector vDir = (vEndPos - vStartPos);
	float fDist  = vDir.Mag();
	float fTotalDist = fDist;

	vDir.Norm();

    LTRotation rRot;
	if (m_cs.bUseObjectRotation)
	{
		rRot = m_rRot;
	}
	else
	{
        g_pLTClient->AlignRotation(&rRot, &vDir, LTNULL);
	}

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	float fMinDist = fDist / m_cs.nNumSegments;
	fMinDist *= m_cs.fMinDistMult;
	float fMaxDist = fMinDist * m_cs.fMaxDistMult;

	PolyVertStruct* pCurVert;

	for (int i=0; i <= m_cs.nNumSegments; i++)
	{
		// Get a new vert from the list
        pCurVert = g_PolyVertStructBank.New();
        if (!pCurVert) return LTFALSE;

		// Set up the vertex
		pCurVert->fPosOffset = fTotalDist - fDist;
		pCurVert->vPos		 = vPos - m_vPos;
		pCurVert->fOffset	 = CalcCurOffset(i);

		pLine->list.AddTail(pCurVert);

		float fNewDist = GetRandom(fMinDist, fMaxDist);
		fDist -= fNewDist;

		if (fDist > 0.0)
		{
			vPos += (vF * fNewDist);
			vPos += (vU * GetRandom(-m_cs.fPerturb, m_cs.fPerturb));
			vPos += (vR * GetRandom(-m_cs.fPerturb, m_cs.fPerturb));
		}

		// Calculate end point vert...

		if (i == m_cs.nNumSegments)
		{
			pCurVert->fPosOffset = fTotalDist;
			pCurVert->vPos = vEndPos - m_vPos;
		}
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::CalcCurOffset
//
//	PURPOSE:	Calculate the current offset for indicated vertex
//
// ----------------------------------------------------------------------- //

LTFLOAT CPolyLineFX::CalcCurOffset(int nVertexNum)
{
	// Determine the offset to use...

	float fMaxOffset = m_cs.fMaxWidth/2.0f;
	float fMinOffset = m_cs.fMinWidth/2.0f;
	float fWidth = fMaxOffset - fMinOffset;
	float fHalfNumVerts = float(m_cs.nNumSegments+1)/2.0f;

    LTFLOAT fOffset = 0.0f;

	switch (m_cs.nWidthStyle)
	{
		case PLWS_CONSTANT :
		{
			fOffset = fWidth / 2.0f;
		}
		break;

		case PLWS_SMALL_TO_BIG :
		{
			fOffset = fMinOffset + (fWidth * float(nVertexNum) / float(m_cs.nNumSegments));
		}
		break;

		case PLWS_SMALL_TO_SMALL :
		{
			if (nVertexNum < (int)fHalfNumVerts)
			{
				fOffset = fMinOffset + (fWidth * float(nVertexNum) / fHalfNumVerts);
			}
			else
			{
				fOffset = fMaxOffset - (fWidth * (float(nVertexNum+1) - fHalfNumVerts) / fHalfNumVerts);
			}
		}
		break;

		default:
		case PLWS_BIG_TO_SMALL :
		{
			fOffset = fMaxOffset - (fWidth * float(nVertexNum) / float(m_cs.nNumSegments));
		}
		break;
	}

	return fOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::Draw
//
//	PURPOSE:	Draw the poly line
//
// ----------------------------------------------------------------------- //
LTBOOL CPolyLineFX::Draw(ILTCustomDraw *pDraw)
{
    if (!pDraw || m_Lines.GetLength() < 1) return LTFALSE;

	// Save current state settings...

    uint32 dwAlphaBlend, dwSrcBlend, dwDestBlend, dwZRead, dwZWrite,
		dwAlphaOp, dwTexAddr, dwColorOp;
    pDraw->GetState(LTRSTATE_ALPHABLENDENABLE, dwAlphaBlend);
	pDraw->GetState(LTRSTATE_SRCBLEND, dwSrcBlend);
	pDraw->GetState(LTRSTATE_DESTBLEND, dwDestBlend);
	pDraw->GetState(LTRSTATE_ZREADENABLE, dwZRead);
	pDraw->GetState(LTRSTATE_ZWRITEENABLE, dwZWrite);
	pDraw->GetState(LTRSTATE_ALPHAOP, dwAlphaOp);
	pDraw->GetState(LTRSTATE_TEXADDR, dwTexAddr);
	pDraw->GetState(LTRSTATE_COLOROP, dwColorOp);

    pDraw->SetState(LTRSTATE_ALPHABLENDENABLE, 1);

	if (m_cs.bAdditive)
	{
        pDraw->SetState(LTRSTATE_SRCBLEND, LTBLEND_ONE);
		pDraw->SetState(LTRSTATE_DESTBLEND, LTBLEND_ONE);
	}
	else if (m_cs.bMultiply)
	{
		pDraw->SetState(LTRSTATE_SRCBLEND, LTBLEND_ZERO);
		pDraw->SetState(LTRSTATE_DESTBLEND, LTBLEND_SRCCOLOR);
	}

	if (m_cs.bNoZ)
	{
        // Do we need read enable set to 0????  Set to one, the z-buffer
		// clipping issues go away, but the lines are still z-buffered...
		pDraw->SetState(LTRSTATE_ZREADENABLE, 1);
		pDraw->SetState(LTRSTATE_ZWRITEENABLE, 0);
	}

	if (m_cs.pTexture)
	{
		pDraw->SetState(LTRSTATE_ALPHAOP, LTOP_SELECTTEXTURE);
	}
	else
	{
		pDraw->SetState(LTRSTATE_ALPHAOP, LTOP_SELECTDIFFUSE);
	}

	pDraw->SetState(LTRSTATE_TEXADDR, m_cs.dwTexAddr);
	pDraw->SetState(LTRSTATE_COLOROP, m_cs.dwColorOp);
	pDraw->SetTexture(m_cs.pTexture);


	// TESTING TEXTURED CANVASES
	if (g_vtTestingTexture.GetFloat() > 0.0f)
	{
		if (g_vtTextureAddr.GetFloat() == 1.0)
		{
			pDraw->SetState(LTRSTATE_TEXADDR, LTTEXADDR_CLAMP);
			g_pLTClient->CPrint("TEXADDR: CLAMP");
		}
		else
		{
			pDraw->SetState(LTRSTATE_TEXADDR, LTTEXADDR_WRAP);
			g_pLTClient->CPrint("TEXADDR: WRAP");
		}

		if (g_vtTextureColor.GetFloat() == 1.0)
		{
			pDraw->SetState(LTRSTATE_COLOROP, LTOP_ADD);
			g_pLTClient->CPrint("TEXCOLOR: ADD");
		}
		else if (g_vtTextureColor.GetFloat() == 2.0)
		{
			pDraw->SetState(LTRSTATE_COLOROP, LTOP_ADDSIGNED);
			g_pLTClient->CPrint("TEXCOLOR: ADD SIGNED");
		}
		else
		{
			pDraw->SetState(LTRSTATE_COLOROP, LTOP_MODULATE);
			g_pLTClient->CPrint("TEXCOLOR: MODULATE");
		}

		if (g_vtTextureAlpha.GetFloat() == 1.0)
		{
			pDraw->SetState(LTRSTATE_ALPHAOP, LTOP_SELECTDIFFUSE);
			g_pLTClient->CPrint("TEXALPHA: SELECT DIFFUSE");
		}
		else
		{
			pDraw->SetState(LTRSTATE_ALPHAOP, LTOP_SELECTTEXTURE);
			g_pLTClient->CPrint("TEXALPHA: SELECT TEXTURE");
		}

		if (g_vtTexture.GetStr())
		{
			char buf[256];
			sprintf(buf, "sfx\\test\\%s.dtx", g_vtTexture.GetStr());
			pDraw->SetTexture(buf);
			g_pLTClient->CPrint("USING TEXTURE: %s", buf);
		}
	}
	// TESTING TEXTURE CANVASES


	// Calculate the transform relative to the camera...

	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    if (!hCamera) return LTFALSE;

    LTRotation rRot;


	// Used to align the segments using our rotation (usually used with
	// m_cs.bAlignFlat or m_cs.bAlignUp...

	if (m_cs.bAlignUsingRot)
	{
		rRot = m_rRot;
	}
	else
	{
        g_pLTClient->GetObjectRotation(hCamera, &rRot);
	}


    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

    LTMatrix mCam, mInvCam;
	mCam = GetCameraTransform(hCamera);
	mCam.Normalize();

	mInvCam = mCam.MakeInverseTransform();
	mInvCam.Normalize();

    LTVertex verts[4];

	PolyLine** pLine = m_Lines.GetItem(TLIT_FIRST);

	// Need to pass this to draw primitive, because the flag has already been
	// checked on the object by this time (and it can't be set on s_hCanvasObj
	// because this is used for all BasePolyDrawFX objects)...

    uint32 dwFlags = (m_Flags & FLAG_REALLYCLOSE) ? FLAG_REALLYCLOSE : 0;

    LTBOOL bCalculatedNormal = LTFALSE;

    LTVector vPt1, vPt2, vPt1Trans, vPt2Trans, vDir;
    LTVector vPt2MinusNonTrans, vPt2PlusNonTrans;
    LTVector vLastPt2;
    LTVector vLastPt2MinusNonTrans;
    LTVector vLastPt2PlusNonTrans;

	float x, y;

	while (pLine && *pLine)
	{
		PolyVertStruct** pCurVert  = (*pLine)->list.GetItem(TLIT_FIRST);
		PolyVertStruct** pNextVert = (*pLine)->list.GetItem(TLIT_NEXT);
        if (!pCurVert || !*pCurVert || !pNextVert || !*pNextVert) return LTFALSE;

		vPt1 = GetVertPos(*pCurVert);
		vPt2 = GetVertPos(*pNextVert);

		MatVMul(&vPt1Trans, &mCam, &vPt1);
		MatVMul(&vPt2Trans, &mCam, &vPt2);

		if (m_cs.bLinesShareNormal || !bCalculatedNormal)
		{
			x = (vPt2Trans.y - vPt1Trans.y);
			y = -(vPt2Trans.x - vPt1Trans.x);

			vDir.Init(x, y, 0.0f);
			vDir.Norm();

            bCalculatedNormal = LTTRUE;
		}

		// Initialize last values...

		vLastPt2 = vPt1;
		vLastPt2MinusNonTrans = vPt1Trans - (vDir * (*pCurVert)->fOffset);
		vLastPt2PlusNonTrans  = vPt1Trans + (vDir * (*pCurVert)->fOffset);

		PolyVertStruct** pNextNextVert = (*pLine)->list.GetItem(TLIT_NEXT);

		while (pNextVert)
		{
			vPt1 = GetVertPos(*pCurVert);
			vPt2 = GetVertPos(*pNextVert);

			MatVMul(&vPt1Trans, &mCam, &vPt1);
			MatVMul(&vPt2Trans, &mCam, &vPt2);

			if (!m_cs.bLinesShareNormal)
			{
				x = (vPt2Trans.y - vPt1Trans.y);
				y = -(vPt2Trans.x - vPt1Trans.x);

				vDir.Init(x, y, 0.0f);
				vDir.Norm();

				if (pNextNextVert)
				{
					// Calculate normal for the next segment...

                    LTVector vPt3Trans;
                    LTVector vVertPos = GetVertPos(*pNextNextVert);
					MatVMul(&vPt3Trans, &mCam, &vVertPos);

					x = (vPt3Trans.y - vPt2Trans.y);
					y = -(vPt3Trans.x - vPt2Trans.x);

                    LTVector vDir2(x, y, 0.0f);
					vDir2.Norm();

                    LTVector vNewDir = (vDir + vDir2) / 2.0f;

					vPt2MinusNonTrans = vPt2Trans - (vNewDir * (*pNextVert)->fOffset);
					vPt2PlusNonTrans  = vPt2Trans + (vNewDir * (*pNextVert)->fOffset);
				}
				else
				{
					vPt2MinusNonTrans = vPt2Trans - (vDir * (*pNextVert)->fOffset);
					vPt2PlusNonTrans  = vPt2Trans + (vDir * (*pNextVert)->fOffset);
				}
			}
			else
			{
				vPt2MinusNonTrans = vPt2Trans - (vDir * (*pNextVert)->fOffset);
				vPt2PlusNonTrans  = vPt2Trans + (vDir * (*pNextVert)->fOffset);
			}


			// Set up lower left vertex...

			if (m_cs.bAlignUp)
			{
				verts[0].m_Vec = (vLastPt2 + (-vU * (*pNextVert)->fOffset));
			}
			else if (m_cs.bAlignFlat)
			{
				verts[0].m_Vec = (vLastPt2 + (-vR * (*pNextVert)->fOffset));
			}
			else
			{
				verts[0].m_Vec = vLastPt2MinusNonTrans;
				MatVMul_InPlace(&mInvCam, &(verts[0].m_Vec));
			}

			verts[0].m_Color.Init(
                (uint8)((*pCurVert)->vCurOuterColor.x),
                (uint8)((*pCurVert)->vCurOuterColor.y),
                (uint8)((*pCurVert)->vCurOuterColor.z),
                (uint8) (m_cs.bDontFadeAlphaAtEdge ? ((*pCurVert)->fCurAlpha) : 0));
			verts[0].m_TU = 0.0;
			verts[0].m_TV = 0.0;


			// Set up upper left vertex...

			// verts[1].m_Vec = vLastPt2;
			if (m_cs.bAlignUp)
			{
				verts[1].m_Vec = (vLastPt2 + (vU * (*pNextVert)->fOffset));
			}
			else if (m_cs.bAlignFlat)
			{
				verts[1].m_Vec = (vLastPt2 + (vR * (*pNextVert)->fOffset));
			}
			else
			{
				verts[1].m_Vec = vLastPt2PlusNonTrans;
				MatVMul_InPlace(&mInvCam, &(verts[1].m_Vec));
			}

			verts[1].m_Color.Init(
                (uint8)((*pCurVert)->vCurInnerColor.x),
                (uint8)((*pCurVert)->vCurInnerColor.y),
                (uint8)((*pCurVert)->vCurInnerColor.z),
                (uint8)((*pCurVert)->fCurAlpha));
			verts[1].m_TU = 0.0;
			verts[1].m_TV = 1.0;


			// Set up upper right vertex...

			// verts[2].m_Vec = vPt2;

			if (m_cs.bAlignUp)
			{
				verts[2].m_Vec = (vPt2 + (vU * (*pNextVert)->fOffset));
			}
			else if (m_cs.bAlignFlat)
			{
				verts[2].m_Vec = (vPt2 + (vR * (*pNextVert)->fOffset));
			}
			else
			{
				verts[2].m_Vec = vPt2PlusNonTrans;
				MatVMul_InPlace(&mInvCam, &(verts[2].m_Vec));
			}

			verts[2].m_Color.Init(
                (uint8)((*pNextVert)->vCurInnerColor.x),
                (uint8)((*pNextVert)->vCurInnerColor.y),
                (uint8)((*pNextVert)->vCurInnerColor.z),
                (uint8)((*pNextVert)->fCurAlpha));
			verts[2].m_TU = 1.0;
			verts[2].m_TV = 1.0;


			// Set up lower right vertex...

			if (m_cs.bAlignUp)
			{
				verts[3].m_Vec = (vPt2 + (-vU * (*pNextVert)->fOffset));
			}
			else if (m_cs.bAlignFlat)
			{
				verts[3].m_Vec = (vPt2 + (-vR * (*pNextVert)->fOffset));
			}
			else
			{
				verts[3].m_Vec = vPt2MinusNonTrans;
				MatVMul_InPlace(&mInvCam, &(verts[3].m_Vec));
			}

			verts[3].m_Color.Init(
                (uint8)((*pNextVert)->vCurOuterColor.x),
                (uint8)((*pNextVert)->vCurOuterColor.y),
                (uint8)((*pNextVert)->vCurOuterColor.z),
                (uint8)(m_cs.bDontFadeAlphaAtEdge ? ((*pNextVert)->fCurAlpha) : 0));
			verts[3].m_TU = 1.0;
			verts[3].m_TV = 0.0;

			// Draw the poly...

            pDraw->DrawPrimitive(verts, 4, dwFlags);
			// Save value of last points...

			vLastPt2 = vPt2;
			vLastPt2MinusNonTrans = vPt2MinusNonTrans;
			vLastPt2PlusNonTrans  = vPt2PlusNonTrans;

			// Get next verts...

			pCurVert	  = pNextVert;
			pNextVert	  = pNextNextVert;
			pNextNextVert = (*pLine)->list.GetItem(TLIT_NEXT);

            m_bHasBeenDrawn = LTTRUE;
		}

		pLine = m_Lines.GetItem(TLIT_NEXT);
	}

	// Reset old states...

	pDraw->SetState(LTRSTATE_ALPHABLENDENABLE, dwAlphaBlend);
	pDraw->SetState(LTRSTATE_SRCBLEND, dwSrcBlend);
	pDraw->SetState(LTRSTATE_DESTBLEND, dwDestBlend);
	pDraw->SetState(LTRSTATE_ZREADENABLE, dwZRead);
	pDraw->SetState(LTRSTATE_ZWRITEENABLE, dwZWrite);
	pDraw->SetState(LTRSTATE_ALPHAOP, dwAlphaOp);
	pDraw->SetState(LTRSTATE_TEXADDR, dwTexAddr);
	pDraw->SetState(LTRSTATE_COLOROP, dwColorOp);

	return m_bHasBeenDrawn;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::GetVertPos
//
//	PURPOSE:	Get the PolyVertStruct absolute position
//
// ----------------------------------------------------------------------- //

LTVector CPolyLineFX::GetVertPos(PolyVertStruct *pVert)
{
	if (m_cs.bUseObjectRotation)
	{
        LTVector vU, vR, vF;
        g_pLTClient->GetRotationVectors(&m_rRot, &vU, &vR, &vF);

		return m_vPos + (vF * pVert->fPosOffset);
	}

	return (pVert->vPos + m_vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyLineFX::Update
//
//	PURPOSE:	Update the poly line
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyLineFX::Update()
{
    if (m_bWantRemove) return LTFALSE;

	// Hide/show poly line if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			m_Flags &= ~FLAG_VISIBLE;
            return LTTRUE;
		}
		else
		{
			m_Flags |= FLAG_VISIBLE;
		}
	}


	// Update the color and alpha for all the lines...

    LTFLOAT fTime = g_pLTClient->GetTime();
	PolyLine** ppLine = m_Lines.GetItem(TLIT_FIRST);

	while (ppLine && *ppLine)
	{
		PolyLine* pLine = *ppLine;

		LTFLOAT fTimeDelta = fTime - pLine->fStartTime;

		pLine->UpdateColorAlpha(fTimeDelta, (m_cs.bAdditive || m_cs.bMultiply));

		if (fTimeDelta > pLine->fLifeTime)
		{
			m_Lines.Remove(pLine);
            g_PolyLineBank.Delete(pLine);

            ppLine = m_Lines.GetItem(TLIT_CURRENT);
		}
		else
		{
			ppLine = m_Lines.GetItem(TLIT_NEXT);
		}
	}


	return m_Lines.GetLength();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyLine::UpdateColorAlpha
//
//	PURPOSE:	Update the line color/alpha
//
// ----------------------------------------------------------------------- //

void PolyLine::UpdateColorAlpha(LTFLOAT fTimeDelta, LTBOOL bAdjustColors)
{
	PolyVertStruct** pCurVert  = list.GetItem(TLIT_FIRST);

	while (pCurVert && *pCurVert)
	{
		PolyVertStruct* pCur = (*pCurVert);

		// If using additive or multiply blending, colors needs to be
		// adjusted to change alpha...

		pCur->UpdateColorAlpha(this, fTimeDelta, bAdjustColors);

		pCurVert = list.GetItem(TLIT_NEXT);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyVertStruct::UpdateColorAlpha
//
//	PURPOSE:	Update the poly line color/alpha
//
// ----------------------------------------------------------------------- //

void PolyVertStruct::UpdateColorAlpha(PolyLine* pLine, LTFLOAT fTimeDelta,
                                      LTBOOL bAdjustColors)
{
	if (!pLine) return;

    LTFLOAT fAlphaPercent = 1.0f;

	if (pLine->fAlphaLifeTime > 0.0f)
	{
		fAlphaPercent = (pLine->fAlphaStart + (fTimeDelta * (pLine->fAlphaEnd - pLine->fAlphaStart) / pLine->fAlphaLifeTime));
		fAlphaPercent = fAlphaPercent < 0.0f ? 0.0f : (fAlphaPercent > 1.0f ? 1.0f : fAlphaPercent);
	}

	fCurAlpha = 255.0f * fAlphaPercent;

	vCurInnerColor.x = pLine->vInnerColorStart.x + (fTimeDelta * (pLine->vInnerColorEnd.x - pLine->vInnerColorStart.x) / pLine->fLifeTime);
	vCurInnerColor.x = vCurInnerColor.x < 0.0f ? 0.0f : (vCurInnerColor.x > 255.0f ? 255.0f : vCurInnerColor.x);
	vCurInnerColor.y = pLine->vInnerColorStart.y + (fTimeDelta * (pLine->vInnerColorEnd.y - pLine->vInnerColorStart.y) / pLine->fLifeTime);
	vCurInnerColor.y = vCurInnerColor.y < 0.0f ? 0.0f : (vCurInnerColor.y > 255.0f ? 255.0f : vCurInnerColor.y);
	vCurInnerColor.z = pLine->vInnerColorStart.z + (fTimeDelta * (pLine->vInnerColorEnd.z - pLine->vInnerColorStart.z) / pLine->fLifeTime);
	vCurInnerColor.z = vCurInnerColor.z < 0.0f ? 0.0f : (vCurInnerColor.z > 255.0f ? 255.0f : vCurInnerColor.z);

	vCurOuterColor.x = pLine->vOuterColorStart.x + (fTimeDelta * (pLine->vOuterColorEnd.x - pLine->vOuterColorStart.x) / pLine->fLifeTime);
	vCurOuterColor.x = vCurOuterColor.x < 0.0f ? 0.0f : (vCurOuterColor.x > 255.0f ? 255.0f : vCurOuterColor.x);
	vCurOuterColor.y = pLine->vOuterColorStart.y + (fTimeDelta * (pLine->vOuterColorEnd.y - pLine->vOuterColorStart.y) / pLine->fLifeTime);
	vCurOuterColor.y = vCurOuterColor.y < 0.0f ? 0.0f : (vCurOuterColor.y > 255.0f ? 255.0f : vCurOuterColor.y);
	vCurOuterColor.z = pLine->vOuterColorStart.z + (fTimeDelta * (pLine->vOuterColorEnd.z - pLine->vOuterColorStart.z) / pLine->fLifeTime);
	vCurOuterColor.z = vCurOuterColor.z < 0.0f ? 0.0f : (vCurOuterColor.z > 255.0f ? 255.0f : vCurOuterColor.z);

	if (bAdjustColors)
	{
		vCurInnerColor.x *= fAlphaPercent;
		vCurInnerColor.y *= fAlphaPercent;
		vCurInnerColor.z *= fAlphaPercent;

		vCurOuterColor.x *= fAlphaPercent;
		vCurOuterColor.y *= fAlphaPercent;
		vCurOuterColor.z *= fAlphaPercent;
	}
}