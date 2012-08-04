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

#ifdef __PSX2
class LTVertex
{
public:
    LTVector        m_Vec;
    float           m_RHW;      // Don't touch.
    LT_VERTRGBA     m_Color;
    LT_VERTRGBA     m_Specular; // Don't touch.
    float           m_TU, m_TV;
};
#endif

VarTrack g_vtTestingTexture;
VarTrack g_vtTextureAddr;
VarTrack g_vtTextureColor;
VarTrack g_vtTextureAlpha;
VarTrack g_vtTexture;

extern CGameClientShell* g_pGameClientShell;

#ifndef __PSX2
// The global vertex structure bank..
CBankedList<PolyVertStruct> g_PolyVertStructBank;

// The global PolyLine bank
CBankedList<PolyLine> g_PolyLineBank;
#endif

static LTMatrix GetCameraTransform(HOBJECT hCamera)
{
    LTVector vPos, vRight, vUp, vForward;
    LTRotation rRot;

	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

	vPos.x = -vPos.x;
	vPos.y = -vPos.y;
	vPos.z = -vPos.z;

    LTMatrix mTran, mRot, mFull;

	mRot.SetBasisVectors((LTVector*)&rRot.Right(), (LTVector*)&rRot.Up(), (LTVector*)&rRot.Forward());
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

LTBOOL CPolyLineFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
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
#ifdef __PSX2
    PolyLine* pLine = new PolyLine;
#else
	PolyLine* pLine = g_PolyLineBank.New();
#endif
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
#ifdef __PSX2
        delete pLine;
#else
		g_PolyLineBank.Delete(pLine);
#endif
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
#ifndef __PSX2
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
#endif
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

	vDir.Normalize();

    LTRotation rRot;
	if (m_cs.bUseObjectRotation)
	{
		rRot = m_rRot;
	}
	else
	{
		rRot = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));
	}

    LTVector vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	float fMinDist = fDist / m_cs.nNumSegments;
	fMinDist *= m_cs.fMinDistMult;
	float fMaxDist = fMinDist * m_cs.fMaxDistMult;

	PolyVertStruct* pCurVert;

	for (int i=0; i <= m_cs.nNumSegments; i++)
	{
		// Get a new vert from the list
#ifdef __PSX2
        pCurVert = new PolyVertStruct;
#else
        pCurVert = g_PolyVertStructBank.New();
#endif
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
LTBOOL CPolyLineFX::Draw(ILTDrawPrim   *pDraw)
{
    if (!pDraw || m_Lines.GetLength() < 1) return LTFALSE;

	// Save current state settings...

	if (m_cs.bAdditive)
	{
        pDraw->SetAlphaBlendMode(DRAWPRIM_BLEND_ADD);
	}
	else if (m_cs.bMultiply)
	{
        pDraw->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCCOLOR);
	}

	if (m_cs.bNoZ)
	{
        pDraw->SetZBufferMode(DRAWPRIM_ZRO);
	}

    pDraw->SetColorOp((ELTColorOp)m_cs.dwColorOp);


	// Calculate the transform relative to the camera...

	HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
    if (!hCamera) return LTFALSE;

	pDraw->SetCamera( hCamera );
	pDraw->SetTransformType( DRAWPRIM_TRANSFORM_WORLD );
	
	HTEXTURE	hTex = LTNULL;
	if( m_cs.pTexture )
	{
		g_pLTClient->GetTexInterface()->CreateTextureFromName( hTex, m_cs.pTexture );
		pDraw->SetTexture( hTex );
	}

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


    LTVector vU, vR;
	vU = rRot.Up();
	vR = rRot.Right();

    LTMatrix mCam, mInvCam;
	mCam = GetCameraTransform(hCamera);
	mCam.Normalize();

	mInvCam = mCam.MakeInverseTransform();
	mInvCam.Normalize();

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

	LTPoly_GT4 dpPoly;

	while (pLine && *pLine)
	{
		PolyVertStruct** pCurVert  = (*pLine)->list.GetItem(TLIT_FIRST);
		PolyVertStruct** pNextVert = (*pLine)->list.GetItem(TLIT_NEXT);
        if (!pCurVert || !*pCurVert || !pNextVert || !*pNextVert) return LTFALSE;

		vPt1 = GetVertPos(*pCurVert);
		vPt2 = GetVertPos(*pNextVert);

		MatVMul((LTVector*)&vPt1Trans, &mCam, (LTVector*)&vPt1);
		MatVMul((LTVector*)&vPt2Trans, &mCam, (LTVector*)&vPt2);

		if (m_cs.bLinesShareNormal || !bCalculatedNormal)
		{
			x = (vPt2Trans.y - vPt1Trans.y);
			y = -(vPt2Trans.x - vPt1Trans.x);

			vDir.Init(x, y, 0.0f);
			vDir.Normalize();

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

			MatVMul((LTVector*)&vPt1Trans, &mCam, (LTVector*)&vPt1);
			MatVMul((LTVector*)&vPt2Trans, &mCam, (LTVector*)&vPt2);

			if (!m_cs.bLinesShareNormal)
			{
				x = (vPt2Trans.y - vPt1Trans.y);
				y = -(vPt2Trans.x - vPt1Trans.x);

				vDir.Init(x, y, 0.0f);
				vDir.Normalize();

				if (pNextNextVert)
				{
					// Calculate normal for the next segment...

                    LTVector vPt3Trans;
                    LTVector vVertPos = GetVertPos(*pNextNextVert);
					MatVMul((LTVector*)&vPt3Trans, &mCam, (LTVector*)&vVertPos);

					x = (vPt3Trans.y - vPt2Trans.y);
					y = -(vPt3Trans.x - vPt2Trans.x);

                    LTVector vDir2(x, y, 0.0f);
					vDir2.Normalize();

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
				LTVector vTemp = (vLastPt2 + (-vU * (*pNextVert)->fOffset));
				dpPoly.verts[0].x = vTemp.x;
				dpPoly.verts[0].y = vTemp.y;
				dpPoly.verts[0].z = vTemp.z;
			}
			else if (m_cs.bAlignFlat)
			{
				LTVector vTemp = (vLastPt2 + (-vR * (*pNextVert)->fOffset));
				dpPoly.verts[0].x = vTemp.x;
				dpPoly.verts[0].y = vTemp.y;
				dpPoly.verts[0].z = vTemp.z;
			}
			else
			{
				LTVector vTemp = vLastPt2MinusNonTrans;
				MatVMul_InPlace(&mInvCam, (LTVector*)&vTemp);
				dpPoly.verts[0].x = vTemp.x;
				dpPoly.verts[0].y = vTemp.y;
				dpPoly.verts[0].z = vTemp.z;
			}

            dpPoly.verts[0].rgba.r = (uint8)((*pCurVert)->vCurOuterColor.x);
            dpPoly.verts[0].rgba.g = (uint8)((*pCurVert)->vCurOuterColor.y);
            dpPoly.verts[0].rgba.b = (uint8)((*pCurVert)->vCurOuterColor.z);
            dpPoly.verts[0].rgba.a = (uint8) (m_cs.bDontFadeAlphaAtEdge ? ((*pCurVert)->fCurAlpha) : 0);
			dpPoly.verts[0].u = 0.0;
			dpPoly.verts[0].v = 0.0;


			// Set up upper left vertex...

			// verts[1].m_Vec = vLastPt2;
			if (m_cs.bAlignUp)
			{
				LTVector vTemp = (vLastPt2 + (vU * (*pNextVert)->fOffset));
				dpPoly.verts[1].x = vTemp.x;
				dpPoly.verts[1].y = vTemp.y;
				dpPoly.verts[1].z = vTemp.z;
			}
			else if (m_cs.bAlignFlat)
			{
				LTVector vTemp = (vLastPt2 + (vR * (*pNextVert)->fOffset));
				dpPoly.verts[1].x = vTemp.x;
				dpPoly.verts[1].y = vTemp.y;
				dpPoly.verts[1].z = vTemp.z;
			}
			else
			{
				LTVector vTemp = vLastPt2PlusNonTrans;
				MatVMul_InPlace(&mInvCam, (LTVector*)&vTemp);
				dpPoly.verts[1].x = vTemp.x;
				dpPoly.verts[1].y = vTemp.y;
				dpPoly.verts[1].z = vTemp.z;
			}

            dpPoly.verts[1].rgba.r = (uint8)((*pCurVert)->vCurInnerColor.x);
            dpPoly.verts[1].rgba.g = (uint8)((*pCurVert)->vCurInnerColor.y);
            dpPoly.verts[1].rgba.b = (uint8)((*pCurVert)->vCurInnerColor.z);
            dpPoly.verts[1].rgba.a = (uint8)((*pCurVert)->fCurAlpha);
			dpPoly.verts[1].u = 0.0;
			dpPoly.verts[1].v = 1.0;


			// Set up upper right vertex...

			// verts[2].m_Vec = vPt2;

			if (m_cs.bAlignUp)
			{
				LTVector vTemp = (vPt2 + (vU * (*pNextVert)->fOffset));
				dpPoly.verts[2].x = vTemp.x;
				dpPoly.verts[2].y = vTemp.y;
				dpPoly.verts[2].z = vTemp.z;
			}
			else if (m_cs.bAlignFlat)
			{
				LTVector vTemp = (vPt2 + (vR * (*pNextVert)->fOffset));
				dpPoly.verts[2].x = vTemp.x;
				dpPoly.verts[2].y = vTemp.y;
				dpPoly.verts[2].z = vTemp.z;
			}
			else
			{
				LTVector vTemp = vPt2PlusNonTrans;
				MatVMul_InPlace(&mInvCam, (LTVector*)&vTemp);
				dpPoly.verts[2].x = vTemp.x;
				dpPoly.verts[2].y = vTemp.y;
				dpPoly.verts[2].z = vTemp.z;
			}

            dpPoly.verts[2].rgba.r = (uint8)((*pNextVert)->vCurInnerColor.x);
            dpPoly.verts[2].rgba.g = (uint8)((*pNextVert)->vCurInnerColor.y);
            dpPoly.verts[2].rgba.b = (uint8)((*pNextVert)->vCurInnerColor.z);
            dpPoly.verts[2].rgba.a = (uint8)((*pNextVert)->fCurAlpha);
			dpPoly.verts[2].u = 1.0;
			dpPoly.verts[2].v = 1.0;


			// Set up lower right vertex...

			if (m_cs.bAlignUp)
			{
				LTVector vTemp = (vPt2 + (-vU * (*pNextVert)->fOffset));
				dpPoly.verts[3].x = vTemp.x;
				dpPoly.verts[3].y = vTemp.y;
				dpPoly.verts[3].z = vTemp.z;
			}
			else if (m_cs.bAlignFlat)
			{
				LTVector vTemp = (vPt2 + (-vR * (*pNextVert)->fOffset));
				dpPoly.verts[3].x = vTemp.x;
				dpPoly.verts[3].y = vTemp.y;
				dpPoly.verts[3].z = vTemp.z;
			}
			else
			{
				LTVector vTemp = vPt2MinusNonTrans;
				MatVMul_InPlace(&mInvCam, (LTVector*)&vTemp);
				dpPoly.verts[3].x = vTemp.x;
				dpPoly.verts[3].y = vTemp.y;
				dpPoly.verts[3].z = vTemp.z;
			}

            dpPoly.verts[3].rgba.r = (uint8)((*pNextVert)->vCurOuterColor.x);
            dpPoly.verts[3].rgba.g = (uint8)((*pNextVert)->vCurOuterColor.y);
            dpPoly.verts[3].rgba.b = (uint8)((*pNextVert)->vCurOuterColor.z);
            dpPoly.verts[3].rgba.a = (uint8)(m_cs.bDontFadeAlphaAtEdge ? ((*pNextVert)->fCurAlpha) : 0);
			dpPoly.verts[3].u = 1.0;
			dpPoly.verts[3].v = 0.0;

			// Draw the poly...

            pDraw->DrawPrim(&dpPoly, 1);
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

	if( hTex )
		g_pLTClient->GetTexInterface()->ReleaseTextureHandle( hTex );

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
		return m_vPos + (m_rRot.Forward() * pVert->fPosOffset);
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
        g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

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
	PolyLine** pLine = m_Lines.GetItem(TLIT_FIRST);

	while (pLine && *pLine)
	{
        LTFLOAT fTimeDelta = fTime - (*pLine)->fStartTime;

		(*pLine)->UpdateColorAlpha(fTimeDelta, (m_cs.bAdditive || m_cs.bMultiply));

		if (fTimeDelta > (*pLine)->fLifeTime)
		{
			m_Lines.Remove(*pLine);
#ifndef __PSX2
            g_PolyLineBank.Delete(*pLine);
#endif

            pLine = m_Lines.GetItem(TLIT_CURRENT);
		}
		else
		{
			pLine = m_Lines.GetItem(TLIT_NEXT);
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