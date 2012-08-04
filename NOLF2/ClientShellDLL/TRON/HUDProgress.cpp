// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProgress.cpp
//
// PURPOSE : HUDItem to display objectives and working procedurals
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientRes.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "SubroutineMgr.h"

// TODO mark objectives that are complete or just remove them?

namespace
{
	char * szUVNames[] = 
	{
		"UVFrameTopLeft",
		"UVFrameTop",
		"UVFrameTopRight",
		"UVFrameLeft",
		"UVFrameBottomLeft",
		"UVFrameBottom",
		"UVBoxTopLeft",
		"UVBoxTop",
		"UVBoxTopRight",
		"UVBoxLeft",
//		"UVBoxLeftInset",
		"UVBoxCenter",
		"UVBoxRight",
		"UVBoxBottomLeft",
		"UVBoxBottom",
		"UVBoxBottomRight",
	};
};

//******************************************************************************************
//**
//** HUD Progress
//**
//******************************************************************************************

CHUDProgress::CHUDProgress()
{
	// Make this something new
	m_UpdateFlags = kHUDObjectives | kHUDProcedurals;
	m_fScale = 0.0f;
	m_bShow = false;
	m_nNumObjectives = 0;
	m_nNumProcs = 0;
	m_pTitleStr = LTNULL;
	int i;
	for (i = 0; i < 10; i++)
	{
		m_pObjectiveStr[i] = m_pCompletedObjectiveStr[i] = m_pSecondaryObjectiveStr[i] = LTNULL;
	}
	for (i = 0; i < 5; i++)
	{
		m_pProcStr[i] = m_pSubroutineStr[i] = LTNULL;
		m_hProcTex[i] = m_hSubTex[i] = LTNULL;
	}
}

LTBOOL CHUDProgress::Init()
{
	UpdateLayout();
	m_hProgressTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hud.dtx");

	int i;
	for (i = 0; i < LAST_PRIM; i++)
	{
		// set the color
		g_pDrawPrim->SetRGBA(&m_PrimArray[i], argbWhite);

		// set the texture coordinates
		m_rcPrimDims[i] = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect(szUVNames[i]);
		SetUVWH(&m_PrimArray[i], m_rcPrimDims[i]);

		// IF we're on the box, then copy the UV info to all 5 proc boxes.
		if (i >= BOX_TOP_LEFT && i <= BOX_BOTTOM_RIGHT)
		{
			for (int j = 0; j < 5; j++)
			{
				SetUVWH(&m_ProcPrimArray[j][i - BOX_TOP_LEFT + PROC_TOP_LEFT], m_rcPrimDims[i]);
				g_pDrawPrim->SetUVWH(&m_ProcPrim[j], 0.0f, 0.0f, 1.0f, 1.0f);
				g_pDrawPrim->SetUVWH(&m_SubPrim[j], 0.0f, 0.0f, 1.0f, 1.0f);

				g_pDrawPrim->SetRGBA(&m_ProcPrim[j], argbWhite);
				g_pDrawPrim->SetRGBA(&m_SubPrim[j], argbWhite);
				g_pDrawPrim->SetRGBA(&m_ProcPrimArray[j][i-BOX_TOP_LEFT+PROC_TOP_LEFT],argbWhite);
			}
		}
	}

	// Extracted the inset from the list of prims, since we have to draw multiple instances
	g_pDrawPrim->SetRGBA(&m_InsetPrim, argbWhite);
	m_InsetPrimDims = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVBoxLeftInset");
	SetUVWH(&m_InsetPrim, m_InsetPrimDims);

	g_pDrawPrim->SetRGBA(&m_InsetFullPrim, argbWhite);
	m_InsetFullPrimDims = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVBoxLeftFullInset");
	SetUVWH(&m_InsetFullPrim, m_InsetFullPrimDims);

	// Separator between mandatory and optional
	g_pDrawPrim->SetRGBA(&m_SeparatorPrim, argbWhite);
	m_rcSeparator = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVSeparator");
	SetUVWH(&m_SeparatorPrim, m_rcSeparator);

	// Build the "Progress" string
	char * szStr = LoadTempString(IDS_PROGRESS);
	if (!szStr || !szStr[0])
	{
		szStr = "Progress";
	}
	CUIFont * pFont = g_pInterfaceResMgr->GetFont(m_iTitleFont);
	m_pTitleStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont,szStr,0.0f, 0.0f);
	m_pTitleStr->SetCharScreenHeight(m_iTitleFontSize);
	m_pTitleStr->SetColor(m_iTitleFontColor);

	// Build the "Optional" string
	if (!szStr || !szStr[0])
	{
		szStr = "Optional";
	}
	szStr = LoadTempString(IDS_OPTIONAL_OBJECTIVES);
	m_pOptionalStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont,szStr,0.0f, 0.0f);
	m_pOptionalStr->SetCharScreenHeight(m_iObjectiveFontSize);
	m_pOptionalStr->SetColor(m_iTitleFontColor);

	// Base bar and progress bar
	LTRect rcBase;//(303, 441, 346, 449);
	LTRect rcProgress;//(303,454,346,462);
	rcBase = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVProgressEmpty");
	rcProgress = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("UVProgressFull");

	for (i = 0; i < 5; i++)
	{
		g_pDrawPrim->SetRGBA(&m_BaseBar[i], argbWhite);
		g_pDrawPrim->SetRGBA(&m_ProgressBar[i], argbWhite);
		SetUVWH(&m_BaseBar[i], rcBase);
		SetUVWH(&m_ProgressBar[i], rcProgress);
	}
	return LTTRUE;
}


void CHUDProgress::Term()
{
	// String cleanup
	if (m_pTitleStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pTitleStr);
		m_pTitleStr = LTNULL;
	}
}

void CHUDProgress::Render()
{
	m_bShow = ((CTronInterfaceMgr *)g_pInterfaceMgr)->IsDisplayingProgress();

	if (!m_bShow)
		return;

	SetRenderState();

	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	{
		g_pDrawPrim->SetTexture(m_hProgressTex);
		g_pDrawPrim->DrawPrim(m_PrimArray, LAST_PRIM);
	}

	// Place insets by each objective
	if (m_nNumObjectives || m_nNumCompletedObjectives || m_nNumProcs)
	{
		float w = (float)(m_InsetPrimDims.right - m_InsetPrimDims.left + 1);
		float h = (float)(m_InsetPrimDims.bottom - m_InsetPrimDims.top + 1);
		float x = (float)m_PrimArray[BOX_LEFT].verts[0].x;
		float strX, strY, y;

		// Render m_BoxLeftInset at each desired location.
		if (m_nNumObjectives)
		{
			for (int i = 0; i < m_nNumObjectives; i++)
			{
				m_pObjectiveStr[i]->GetPosition(&strX, &strY);
				y = strY;
				g_pDrawPrim->SetXYWH(&m_InsetPrim, x,y,w,h);
				g_pDrawPrim->DrawPrim(&m_InsetPrim);
			}
		}

		// Render m_BoxLeftInset at each desired location.
		if (m_nNumSecondaryObjectives)
		{
			for (int i = 0; i < m_nNumSecondaryObjectives; i++)
			{
				m_pSecondaryObjectiveStr[i]->GetPosition(&strX, &strY);
				y = strY;
				g_pDrawPrim->SetXYWH(&m_InsetPrim, x,y,w,h);
				g_pDrawPrim->DrawPrim(&m_InsetPrim);
			}
			// While placing insets, also place a separator for the secondary objectives
			g_pDrawPrim->DrawPrim(&m_SeparatorPrim);
		}

		// also render the empty for each procedural
		if (m_nNumProcs)
		{
			for (int i = 0; i < m_nNumProcs; i++)
			{
				y = m_ProcPrimArray[i][3].verts[0].y;
				g_pDrawPrim->SetXYWH(&m_InsetPrim, x,y,w,h);
				g_pDrawPrim->DrawPrim(&m_InsetPrim);
			}
		}

		if (m_nNumCompletedObjectives)
		{
			for (int i = 0; i < m_nNumCompletedObjectives; i++)
			{
				m_pCompletedObjectiveStr[i]->GetPosition(&strX, &strY);
				y = strY;
				g_pDrawPrim->SetXYWH(&m_InsetFullPrim, x,y,w,h);
				g_pDrawPrim->DrawPrim(&m_InsetFullPrim);
			}
		}
	}

	// if there are secondary objectives
		// draw a frame
		// render the icon (done/not done)
		// render the strings

	if (m_pTitleStr)
	{
		m_pTitleStr->Render();
	}

	if (m_nNumObjectives)
	{
		for (int i = 0; i < m_nNumObjectives; i++)
		{
			m_pObjectiveStr[i]->Render();
		}
	}
	if (m_nNumCompletedObjectives)
	{
		for (int i = 0; i < m_nNumCompletedObjectives; i++)
		{
			m_pCompletedObjectiveStr[i]->Render();
		}
	}
	if (m_nNumSecondaryObjectives)
	{
		m_pOptionalStr->Render();
		for (int i = 0; i < m_nNumSecondaryObjectives; i++)
		{
			m_pSecondaryObjectiveStr[i]->Render();
		}
	}
	if (m_nNumProcs)
	{
		int iTop = g_pInterfaceResMgr->GetScreenHeight() - 64;
		int i;

		// draw the individual frame
		for (i = 0; i < m_nNumProcs; i++)
		{
			g_pDrawPrim->DrawPrim(&m_ProcPrimArray[i][0], 9);
		}
		// draw the bitmaps
		for (i = 0; i < 5; i++)
		{
			if (m_bIsProcActive[i])
			{
				// Draw the bar
				g_pDrawPrim->SetTexture(m_hProgressTex);
				g_pDrawPrim->DrawPrim(&m_BaseBar[i]);
				g_pDrawPrim->DrawPrim(&m_ProgressBar[i]);

				// Draw the icons
				g_pDrawPrim->SetTexture(m_hProcTex[i]);
				g_pDrawPrim->DrawPrim(&m_ProcPrim[i]);
				g_pDrawPrim->SetTexture(m_hSubTex[i]);
				g_pDrawPrim->DrawPrim(&m_SubPrim[i]);
			}
		}
		for (i = 0; i < 5; i++)
		{
			if (m_bIsProcActive[i])
			{
				m_pProcStr[i]->Render();
				m_pSubroutineStr[i]->Render();
			}
		}
	}
}

void CHUDProgress::Update()
{
	uint32 dwUpdateFlags = g_pHUDMgr->QueryUpdateFlags();

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
	
	// Determine if the change was to the procedurals (i.e. g_pSubroutineMgr->IsProceduralActive())
	if (dwUpdateFlags && kHUDProcedurals)
	{
		if (g_pSubroutineMgr->IsProceduralActive())
		{
			CUIFont * pFont = g_pInterfaceResMgr->GetFont(m_iProceduralFont);
//			int iTextX = m_rcProgress.left + GetPieceWidth(FRAME_LEFT) + GetPieceWidth(BOX_LEFT) + 5;

			m_nNumProcs = 0;
			for (int i = 0; i < 5; i++)
			{
				Procedural * pProc = g_pSubroutineMgr->GetProcedural(i);
				if (pProc->pSub)
				{
					m_fProcPercent[i] = pProc->pSub->fPercentDone;
					m_nNumProcs++;
					// just went active?
					if (!m_bIsProcActive[i])
					{
						// Create and position proc name
						char * szStr = LoadTempString(pProc->nNameId);
						m_pProcStr[i] = g_pLTClient->GetFontManager()->CreatePolyString(pFont,szStr,0.0f,0.0f);
						m_pProcStr[i]->SetCharScreenHeight(m_iProceduralFontSize);
						m_pProcStr[i]->SetColor(m_iProceduralFontColor);
						m_hProcTex[i] = g_pInterfaceResMgr->GetTexture(pProc->szIdleSkin);

						// create and position sub name
						szStr = LoadTempString(pProc->pSub->pTronSubroutine->nNameId);
						m_pSubroutineStr[i] = g_pLTClient->GetFontManager()->CreatePolyString(pFont,szStr,0.0f,0.0f);
						m_pSubroutineStr[i]->SetCharScreenHeight(m_iProceduralFontSize);
						m_pSubroutineStr[i]->SetColor(m_iProceduralFontColor);
						m_hSubTex[i] = g_pInterfaceResMgr->GetTexture(pProc->pSub->pTronSubroutine->szSprite);
					}
					// position the subroutine build ring
				}
				else
				{
					// just went inactive?
					if (m_bIsProcActive[i])
					{
						// destroy proc name
						g_pLTClient->GetFontManager()->DestroyPolyString(m_pProcStr[i]);
						m_pProcStr[i] = LTNULL;
						m_hProcTex[i] = LTNULL;

						// destroy sub name
						g_pLTClient->GetFontManager()->DestroyPolyString(m_pSubroutineStr[i]);
						m_pSubroutineStr[i] = LTNULL;
						m_hSubTex[i] = LTNULL;
					}
				}
				m_bIsProcActive[i] = (pProc->pSub != LTNULL) ? true : false;
			}
		}
	}

//	else // the change was to the objectives.
	if (dwUpdateFlags && kHUDObjectives)
	{
		// clear the local list of strings
		for (int i = 0; i < 10; i++)
		{
			if (m_pObjectiveStr[i])
			{
				g_pLTClient->GetFontManager()->DestroyPolyString(m_pObjectiveStr[i]);
				m_pObjectiveStr[i] = LTNULL;
			}
		}
		m_nNumObjectives = 0;
		m_nNumCompletedObjectives = 0;
		m_nNumSecondaryObjectives = 0;

		// query TronPlayerStats for the objectives
		// add required objectives
		CUIFont * pFont = g_pInterfaceResMgr->GetFont(m_iObjectiveFont);
		IDList* pObj = g_pPlayerStats->GetObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumObjectives < 10)
			{
				uint32 objID = pObj->m_IDArray[i];
				char * szStr = LoadTempString(objID);
				m_pObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, szStr, 0.0f, 0.0f);
				m_pObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pObjectiveStr[i]->SetColor(m_iObjectiveFontColor);
				m_nNumObjectives++;
			}
		}
		// completed objectives
		pObj = g_pPlayerStats->GetCompletedObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumCompletedObjectives < 10)
			{
				uint32 objID = pObj->m_IDArray[i];
				char * szStr = LoadTempString(objID);
				m_pCompletedObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, szStr, 0.0f, 0.0f);
				m_pCompletedObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pCompletedObjectiveStr[i]->SetColor(m_iCompletedObjectiveFontColor);
				m_nNumCompletedObjectives++;
			}
		}
		// Secondary objectives
		pObj = g_pPlayerStats->GetOptionalObjectives();
		for (i = pObj->m_IDArray.size()-1; i >= 0; i--)
		{
			if (m_nNumSecondaryObjectives < 10)
			{
				uint32 objID = pObj->m_IDArray[i];
				char * szStr = LoadTempString(objID);
//				char buf[1024];
//				sprintf(buf, "(optional) %s", LoadTempString(objID));
				m_pSecondaryObjectiveStr[i] = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, szStr, 0.0f, 0.0f);
				m_pSecondaryObjectiveStr[i]->SetCharScreenHeight(m_iObjectiveFontSize);
				m_pSecondaryObjectiveStr[i]->SetColor(m_iSecondaryObjectiveFontColor);
				m_nNumSecondaryObjectives++;
			}
		}
	}
	UpdateGraphics();
}

int CHUDProgress::GetPieceWidth(ePrimType eType)
{
	return (m_rcPrimDims[eType].right - m_rcPrimDims[eType].left + 1);

}

int CHUDProgress::GetPieceHeight(ePrimType eType)
{
	return (m_rcPrimDims[eType].bottom - m_rcPrimDims[eType].top + 1);

}

void CHUDProgress::UpdateGraphics()
{
	// compute the height of the box here.
	int iBoxTop = g_pInterfaceResMgr->GetScreenHeight() - 64;
	int iBottom = iBoxTop;
	int i;

	// Compute the width of the text area
	// then use it to constrain all of the polystrings
	int iTextX = m_rcProgress.left + GetPieceWidth(FRAME_LEFT) + GetPieceWidth(BOX_LEFT) + 8;

	int iTextWidth = m_rcProgress.right - GetPieceWidth(BOX_RIGHT) - iTextX;

	// Position the text and elements of the procedural display
	// Subtract m_iProceduralHeight for every active procedural
	if (m_nNumProcs)
	{
		for (i = 0; i < 5; i++)
		{
			float fWidth = (float)iTextWidth - 100.0f;

			if (m_bIsProcActive[i])
			{
				iBoxTop -= m_iProceduralHeight + 2;

				m_pProcStr[i]->SetPosition((float)iTextX + 48.0f, (float)(iBoxTop));
				m_pSubroutineStr[i]->SetPosition((float)iTextX + 48.0f, (float)(iBoxTop + m_iProceduralHeight - 25));

				// position the procedural icon and the subroutine icon
				g_pDrawPrim->SetXYWH(&m_ProcPrim[i], (float)iTextX - 12, (float)iBoxTop - 6 ,64.0, 64.0);
				g_pDrawPrim->SetXYWH(&m_SubPrim[i], (float)(iTextX + iTextWidth - 64.0f), (float)iBoxTop -6 ,64.0, 64.0);

				// position the two drawprims that make up the progress bar
				g_pDrawPrim->SetXYWH(&m_ProgressBar[i],(float)iTextX + 50, (float)(iBoxTop + 20), fWidth * m_fProcPercent[i], 12.0f);
				g_pDrawPrim->SetXYWH(&m_BaseBar[i], (float)iTextX + 50, (float)(iBoxTop + 20), fWidth, 12.0f);
				m_BaseBar[i].verts[0].x = m_BaseBar[i].verts[3].x = m_ProgressBar[i].verts[0].x + 1;
			}
		}
	}

	// Subtract a margin for the bottom of the procedural box
	iBoxTop -= GetPieceHeight(FRAME_BOTTOM) + GetPieceHeight(BOX_BOTTOM);

	if (m_nNumSecondaryObjectives)
	{
		float fWidth, fHeight;
		for (i = m_nNumSecondaryObjectives - 1; i >= 0; i--)
		{
			m_pSecondaryObjectiveStr[i]->SetWrapWidth(iTextWidth);
			m_pSecondaryObjectiveStr[i]->GetDims(&fWidth, &fHeight);
			iBoxTop -= (int)fHeight + 8;
			m_pSecondaryObjectiveStr[i]->SetPosition((float)iTextX,(float)iBoxTop);
		}
		// Place the text
		m_pOptionalStr->GetDims(&fWidth, &fHeight);
		iBoxTop -= (int)fHeight + 8;
		float fCenterX = iTextX + ((iTextWidth - fWidth) / 2.0f);
		m_pOptionalStr->SetPosition(fCenterX, (float)iBoxTop);
		// Place the drawprim
		g_pDrawPrim->SetXYWH(&m_SeparatorPrim, (float)iTextX, (float)(iBoxTop+(fHeight /2)-1), (float)iTextWidth, 4.0f);
	}

	if (m_nNumObjectives)
	{
		for (i = m_nNumObjectives - 1; i >= 0; i--)
		{
			m_pObjectiveStr[i]->SetWrapWidth(iTextWidth);
			float fWidth, fHeight;
			m_pObjectiveStr[i]->GetDims(&fWidth, &fHeight);
			iBoxTop -= (int)fHeight + 8;
			m_pObjectiveStr[i]->SetPosition((float)iTextX,(float)iBoxTop);
		}
	}

	if (m_nNumCompletedObjectives)
	{
		for (i = m_nNumCompletedObjectives - 1; i >= 0; i--)
		{
			m_pCompletedObjectiveStr[i]->SetWrapWidth(iTextWidth);
			float fWidth, fHeight;
			m_pCompletedObjectiveStr[i]->GetDims(&fWidth, &fHeight);
			iBoxTop -= (int)fHeight + 8;
			m_pCompletedObjectiveStr[i]->SetPosition((float)iTextX,(float)iBoxTop);
		}
	}

	// Position the frames around each window
	int iLeft = m_rcProgress.left;
	int iRight = m_rcProgress.right;
	int iWidth = iRight - iLeft;

	int iTop = iBoxTop - GetPieceHeight(FRAME_TOP) - GetPieceHeight(BOX_TOP);
	// Go about the wonderful business of computing the locations for everything.
	float x,y,w,h;
	float w2, h2;

	// frame top left
	x = (float)iLeft;
	y = (float)iTop;
	w = (float)GetPieceWidth(FRAME_TOP_LEFT);
	h = (float)GetPieceHeight(FRAME_TOP_LEFT);
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_TOP_LEFT], x, y, w, h);

	// frame top (stretch)
	x += w;
	w2 = (float)GetPieceWidth(FRAME_TOP_RIGHT);
	w = iRight - x - w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_TOP], x, y, w, h);

	// Take a moment here to position the title...
	if (m_pTitleStr)
	{
		// center it over this graphic
		float fHeight, fWidth;
		m_pTitleStr->GetDims(&fWidth, &fHeight);
		float ty = y + (h * 0.5f) - (fHeight * 0.5f);
		ty -= 3;
		m_pTitleStr->SetPosition(x + 10,ty);
	}
	// frame top right
	x += w;
	w = w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_TOP_RIGHT], x, y, w, h);

	// frame left
	x = (float)iLeft;
	y += h;
	w = (float)GetPieceWidth(FRAME_LEFT);
	h2 = (float)GetPieceHeight(FRAME_BOTTOM);
	h = iBottom - y - h2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_LEFT], x, y, w, h);

	float fBoxTop = y;
	float fBoxLeft = x + w - 2;

	// frame bottom left
	y += h;
	h = h2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_BOTTOM_LEFT], x, y, w, h);

	// frame bottom
	x += w;
	w = iRight - x;
	g_pDrawPrim->SetXYWH(&m_PrimArray[FRAME_BOTTOM], x, y, w, h);

	float fBoxBottom = y;
	int spacer = m_rcPrimDims[FRAME_BOTTOM].top - m_rcPrimDims[BOX_BOTTOM].bottom;

	// box top left
	x = fBoxLeft;
	y = fBoxTop;
	w = (float)GetPieceWidth(BOX_TOP_LEFT);
	h = (float)GetPieceHeight(BOX_TOP_LEFT);
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_TOP_LEFT], x, y, w, h);

	// box top
	x += w;
	w2 = (float)GetPieceWidth(BOX_RIGHT);
	w = iRight - x - w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_TOP], x, y, w, h);

	// box top right
	x += w;
	w = w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_TOP_RIGHT], x, y, w, h);

	// box left
	x = fBoxLeft;
	y += h;
	w = (float)GetPieceWidth(BOX_LEFT);
	h2 = (float)GetPieceHeight(BOX_BOTTOM);
	h = fBoxBottom - y - h2 - spacer - (m_nNumProcs * (m_iProceduralHeight + 2));
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_LEFT], x, y, w, h);

	// box center
	x += w;
	w = iRight - x - w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_CENTER], x, y, w, h);

	// box right
	x += w;
	w = w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_RIGHT], x, y, w, h);

	// box bottom left
	x = fBoxLeft;
	y += h;
	w = (float)GetPieceWidth(BOX_BOTTOM_LEFT);
	h = h2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_BOTTOM_LEFT], x, y, w, h);

	// box bottom
	x += w;
	w = iRight - x - w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_BOTTOM], x, y, w, h);

	// box bottom right
	x += w;
	w = w2;
	g_pDrawPrim->SetXYWH(&m_PrimArray[BOX_BOTTOM_RIGHT], x, y, w, h);
}

void CHUDProgress::BuildProcRect(int iProc, LTRect rcProc)
{
	float x,y;
	float wLeft, wCenter, wRight;
	float hTop, hCenter, hBottom;

	wLeft = (float)GetPieceWidth(BOX_TOP_LEFT);
	wRight = (float)GetPieceWidth(BOX_TOP_RIGHT);
	wCenter = (float)(rcProc.right - rcProc.left) - wLeft - wRight;

	hTop = (float)GetPieceHeight(BOX_TOP_LEFT);
	hBottom = (float)GetPieceHeight(BOX_BOTTOM_LEFT);
	hCenter = (float)(rcProc.bottom - rcProc.top) - hTop - hBottom;

//	g_pDrawPrim->SetXYWH(&m_ProcPrim[iProc], (float)rcProc.left , (float)rcProc.top, 64.0f, 64.0f);
//	g_pDrawPrim->SetXYWH(&m_SubPrim[i], (float)rcProc.right - 64.0f , (float)rcProc.top, 64.0f, 64.0f);

	// top left
	x = (float)rcProc.left;
	y = (float)rcProc.top;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_TOP_LEFT], x, y, wLeft, hTop);

	// top
	x += wLeft;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_TOP], x, y, wCenter, hTop);

	// top right
	x += wCenter;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_TOP_RIGHT], x, y, wRight, hTop);

	// left
	x = (float)rcProc.left;
	y += hTop;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_LEFT], x, y, wLeft, hCenter);

	// center
	x += wLeft;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_CENTER], x, y, wCenter, hCenter);

	// right
	x += wCenter;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_RIGHT], x, y, wRight, hCenter);

	// bottom left
	x = (float)rcProc.left;
	y += hCenter;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_BOTTOM_LEFT], x, y, wLeft, hBottom);

	// bottom
	x += wLeft;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_BOTTOM], x, y, wCenter, hBottom);

	// bottom right
	x += wCenter;
	g_pDrawPrim->SetXYWH(&m_ProcPrimArray[iProc][PROC_BOTTOM_RIGHT], x, y, wRight, hBottom);
}


void CHUDProgress::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	LTRect Rect;

	// Overall dimensions.
	// Note that these dimensions are for a base box in the bottom left corner
	// and that the box can be grown up vertically as needed.
	m_rcProgress = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressBoxRect("ProgressRect");

	m_iTitleFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleFont();
	m_iTitleFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleFontSize();
	m_iTitleFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressTitleColor();

	m_iObjectiveFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveFont();
	m_iObjectiveFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveFontSize();
	m_iObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressObjectiveColor();
	m_iCompletedObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressCompletedObjectiveColor();
	m_iSecondaryObjectiveFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressSecondaryObjectiveColor();

	m_iProceduralFont = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralFont();
	m_iProceduralFontSize = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralFontSize();
	m_iProceduralFontColor = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralColor();
	m_iProceduralHeight = ((CTronLayoutMgr *)g_pLayoutMgr)->GetProgressProceduralHeight();

	// Position the boxes for procedurals
	int iTop = g_pInterfaceResMgr->GetScreenHeight() - 64; // bottom of screen, but above armor meter
	iTop -= (m_rcPrimDims[FRAME_BOTTOM].bottom - m_rcPrimDims[BOX_BOTTOM].bottom + 1);
	LTRect rcProc;
	rcProc.left = m_rcProgress.left;
	rcProc.left += 	(m_rcPrimDims[FRAME_LEFT].right - m_rcPrimDims[FRAME_LEFT].left + 1) -2;

	rcProc.right = m_rcProgress.right;

	for (int i = 0; i < 5; i++)
	{
		rcProc.top = iTop - m_iProceduralHeight;
		rcProc.bottom = iTop;
		BuildProcRect(i, rcProc);
		iTop -= m_iProceduralHeight + 2;
	}
}

void CHUDProgress::SetUVWH(LT_POLYFT4 * pPrim, LTRect rect)
{
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	g_pDrawPrim->SetUVWH(pPrim, rect.left / 512.0f, rect.top / 512.0f, rect.right / 512.0f, rect.bottom / 512.0f);
}
