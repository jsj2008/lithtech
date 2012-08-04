//------------------------------------------------------------------
//
//   MODULE    : SCREENTRAIL.CPP
//
//   PURPOSE   : Implements screen poly draw trail stuff
//
//   CREATED   : On 7/19/99 At 5:56:00 PM
//
//   COPYRIGHT : (C) 1999 Monolith Productions Inc
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "ScreenTrail.h"

void RenderPolyTrail(ILTClient *pClientDE, 
					 CLinkList<TRAIL_SECTION> *pList, 
					 HOBJECT hCamera, 
					 float fTrailWidth,
					 uint8 r,
					 uint8 g,
					 uint8 b,
					 uint8 a,
					 HTEXTURE hTexture,
					 uint32 dwExtraFlags)
{
	CLinkListNode<TRAIL_SECTION> *pNode = pList->GetHead();

	// Transform the path

	LTMatrix mCam = GetCamTransform(pClientDE, hCamera);

	while (pNode)
	{
		MatVMul(&pNode->m_Data.m_vTran, &mCam, &pNode->m_Data.m_vPos);	
		
		pNode = pNode->m_pNext;
	}

	// Do some precalculations

	pNode = pList->GetHead();

	float fCurU = 0.0f;
	
	while (pNode)
	{	
		LTVector vBisector;
		vBisector.z = 0.0f;

		// Compute the midpoint vectors

		if (pNode == pList->GetHead())
		{
			LTVector vStart = pNode->m_Data.m_vTran;
			LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;
			
			vBisector.x = vEnd.y - vStart.y;
			vBisector.y = -(vEnd.x - vStart.x);
		}
		else if (pNode == pList->GetTail())
		{
			LTVector vEnd   = pNode->m_Data.m_vTran;
			LTVector vStart = pNode->m_pPrev->m_Data.m_vTran;
			
			vBisector.x = vEnd.y - vStart.y;
			vBisector.y = -(vEnd.x - vStart.x);
		}
		else
		{
			LTVector vPrev  = pNode->m_pPrev->m_Data.m_vTran;
			LTVector vStart = pNode->m_Data.m_vTran;
			LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;

			float x1 = vEnd.y - vStart.y;
			float y1 = -(vEnd.x - vStart.x);

			float x2 = vStart.y - vPrev.y;
			float y2 = -(vStart.x - vPrev.x);
			
			vBisector.x = (x1 + x2) / 2.0f;
			vBisector.y = (y1 + y2) / 2.0f;
		}

		vBisector.Norm(fTrailWidth);
		pNode->m_Data.m_vBisector = vBisector;

		pNode->m_Data.m_red   = r;
		pNode->m_Data.m_green = g;
		pNode->m_Data.m_blue  = b;
		pNode->m_Data.m_alpha = a;

		pNode = pNode->m_pNext;
	}

	pNode = pList->GetHead();

	if (pList->GetSize() < 2) return;

	pNode = pList->GetHead();

	ILTDrawPrim *pDrawPrimLT;
	pDrawPrimLT = pClientDE->GetDrawPrim();

	pDrawPrimLT->SetTexture(hTexture);
	pDrawPrimLT->SetTransformType(DRAWPRIM_TRANSFORM_CAMERA);
	pDrawPrimLT->BeginDrawPrim();


	if (g_bAppFocus)
	{
		uint32 nTris = 0;
		uint32 nVerts = 0;

		LT_POLYGT3 *pTri = g_pTris;
		LTVector *pVerts = g_pVerts;

		while (pNode->m_pNext)
		{
			LTVector vStart = pNode->m_Data.m_vTran;
			LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;
		
			LTVector vBisector1 = pNode->m_Data.m_vBisector;
			LTVector vBisector2 = pNode->m_pNext->m_Data.m_vBisector;

			*pVerts ++ = vStart + vBisector1;
			*pVerts ++ = vEnd + vBisector2;
			*pVerts ++ = vEnd - vBisector2;
			*pVerts ++ = vStart - vBisector1;

			uint8 r1 = pNode->m_Data.m_red;
			uint8 g1 = pNode->m_Data.m_green;
			uint8 b1 = pNode->m_Data.m_blue;
			uint8 a1 = pNode->m_Data.m_alpha;
			float u1 = pNode->m_Data.m_uVal;

			uint8 r2 = pNode->m_pNext->m_Data.m_red;
			uint8 g2 = pNode->m_pNext->m_Data.m_green;
			uint8 b2 = pNode->m_pNext->m_Data.m_blue;
			uint8 a2 = pNode->m_pNext->m_Data.m_alpha;
			float u2 = pNode->m_pNext->m_Data.m_uVal;
			
			SetupVert(pTri, 0, g_pVerts[nVerts].x, g_pVerts[nVerts].y, g_pVerts[nVerts].z, r1, g1, b1, a1, u1, 0.0f);
			SetupVert(pTri, 1, g_pVerts[nVerts + 1].x, g_pVerts[nVerts + 1].y, g_pVerts[nVerts + 1].z, r2, g2, b2, a2, u2, 1.0f);
			SetupVert(pTri, 2, g_pVerts[nVerts + 2].x, g_pVerts[nVerts + 2].y, g_pVerts[nVerts + 2].z, r2, g2, b2, a2, u2, 1.0f);

			pTri ++;
			nTris ++;

			SetupVert(pTri, 0, g_pVerts[nVerts].x, g_pVerts[nVerts].y, g_pVerts[nVerts].z, r1, g1, b1, a1, u1, 0.0f);
			SetupVert(pTri, 1, g_pVerts[nVerts + 2].x, g_pVerts[nVerts + 2].y, g_pVerts[nVerts + 2].z, r2, g2, b2, a2, u2, 1.0f);
			SetupVert(pTri, 2, g_pVerts[nVerts + 3].x, g_pVerts[nVerts + 3].y, g_pVerts[nVerts + 3].z, r1, g1, b1, a1, u1, 0.0f);

			pTri ++;
			nTris ++;

			nVerts += 4;

			pNode = pNode->m_pNext;

			//see if we need to flush our buffer
			if(nTris >= MAX_BUFFER_TRIS - 2)
			{
				pDrawPrimLT->DrawPrim(g_pTris, nTris);
				nTris = 0;
			}
		}

		// Draw the polylist
		if(nTris > 0)
		{
			pDrawPrimLT->DrawPrim(g_pTris, nTris);
			nTris = 0;
		}
	}

	pDrawPrimLT->BeginDrawPrim();
}