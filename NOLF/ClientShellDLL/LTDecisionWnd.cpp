/****************************************************************************
;
;	 MODULE:		CLTDecisionWnd (.cpp)
;
;	PURPOSE:		Class for a decision window
;
;	HISTORY:		04/16/99 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#include "StdAfx.h"
#include "LTDecisionWnd.h"
#include "LTGuiMgr.h"
#include "LTWndMessages.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

BOOL CLTDecisionWnd::Init(DECISIONWNDCREATESTRUCT *pcs)
{
	// Sanity checks
	if(!pcs || m_bInitialized)
		return FALSE;

	// Gotta have this stuff
	if(!pcs->pFont || !pcs->pcollFrames || !pcs->prcFrame || !pcs->prcTotal)
		return FALSE;

	// Initialize the base class
	if(!CLTMaskedWnd::Init(pcs))
		return FALSE;

	// Simple vars
	m_pFont = pcs->pFont;
	m_pcollFrames = pcs->pcollFrames;
	m_prcTotal = pcs->prcTotal;
	m_prcFrame = pcs->prcFrame;
	m_bFrame = pcs->bFrame;

	SetAlpha(pcs->fAlpha);

	// Set up the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].Init(i,"Decision",this,m_pFont,0,0,LTWF_FIXEDCHILD);
	}

	Reset();

	// All done!
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTDecisionWnd::Term()
{
	// Term the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].Term();
	}


	CLTMaskedWnd::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::DrawToSurface
//
//	PURPOSE:	Draws the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::DrawToSurface(HSURFACE hSurfDest)
{
	ASSERT(m_bInitialized);

	// Draw the alpha window
	if(m_hSurf)
	{
        LTRect rcDest;
		rcDest.left = GetWindowLeft();
		rcDest.top = GetWindowTop();
		rcDest.right = rcDest.left + m_nWidth;
		rcDest.bottom = rcDest.top + m_nHeight;
        g_pLTClient->ScaleSurfaceToSurface(hSurfDest,m_hSurf,&rcDest,NULL);
	}

	// Draw the frame
	if(m_bFrame && (m_pcollFrames->GetSize() == 8))
	{
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_TOPLEFTFRAME),NULL,m_xPos,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_TOPRIGHTFRAME),NULL,m_xPos+m_nWidth-m_prcTotal->right,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_BOTTOMRIGHTFRAME),NULL,m_xPos+m_nWidth-m_prcTotal->right,m_yPos+m_nHeight-m_prcTotal->bottom,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_BOTTOMLEFTFRAME),NULL,m_xPos,m_yPos+m_nHeight-m_prcTotal->bottom,g_hColorTransparent);

		// Draw the four scaleable sides
        LTRect rcDest;
		rcDest.left = m_xPos;
		rcDest.top = m_yPos+m_prcTotal->top;
		rcDest.right = m_xPos+m_prcFrame->left;
		rcDest.bottom = m_yPos+m_nHeight-m_prcTotal->bottom;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_LEFTFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos + m_nWidth - m_prcFrame->right;
		rcDest.right = m_xPos + m_nWidth;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_RIGHTFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos+m_prcTotal->left;
		rcDest.top = m_yPos;
		rcDest.right = m_xPos+m_nWidth-m_prcTotal->right;
		rcDest.bottom = m_yPos+m_prcFrame->top;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_TOPFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.top = m_yPos + m_nHeight - m_prcFrame->bottom;
		rcDest.bottom = m_yPos + m_nHeight;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_pcollFrames->GetAt(DLG_BOTTOMFRAME),&rcDest,NULL,g_hColorTransparent);
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{

	m_pFont = g_pInterfaceResMgr->GetMediumFont();

	if(!bAnimate)
		return CLTMaskedWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	if(m_bClosing || m_bOpening)
		return FALSE;

	// Call the base class disabling sound
	BOOL bRet = CLTMaskedWnd::ShowWindow(bShow,FALSE);

	// If we closed, start the closing animation
	if(bRet)
	{
		if(!bShow)
		{
			// We're temporarily visible just so we can close, but we're disabled
			m_bVisible = TRUE;
			m_bEnabled = FALSE;

			m_bClosing = TRUE;
			m_tmrClose.Start(g_pLayoutMgr->GetDecisionCloseTime());
			m_fStartPos = (float)m_nHeight;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::Update
//
//	PURPOSE:	Updates the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::Update(float fTimeDelta)
{
	// NOTE: Decision window doesn't call down to the base class because we don't care to
	// update its children

	ASSERT(m_bInitialized);

	if(m_bOpening)
	{
		// Move us towards the center
		int xDest = (g_pGameClientShell->GetScreenWidth()-m_nWidth)/2;
		int xPos = (int)(xDest + ((m_fStartPos - xDest)*m_tmrClose.GetCountdownTime()/m_tmrClose.GetDuration()));
		if(m_tmrClose.Stopped() || (xPos <= xDest))
		{
			m_bEnabled = TRUE;
			m_bOpening = FALSE;
			MoveWindow(xDest,m_yPos);
			return TRUE;
		}
		MoveWindow(xPos,m_yPos);
	}
	else
	if(m_bClosing)
	{
		// Shrink us towards the middle
		int yMid = m_yPos + (m_nHeight / 2);
		int nHeight = (int)(m_fStartPos - (m_fStartPos * (m_tmrClose.GetElapseTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight < (m_prcTotal->top + m_prcTotal->bottom)))
		{
			// We're no longer visible, but we're enabled again
			m_bVisible = FALSE;
			m_bEnabled = TRUE;

			m_bClosing = FALSE;
			m_nHeight = (int)m_fStartPos;
			// Restore us to original size
			MoveWindow(m_xPos,yMid-(m_nHeight / 2));
			return TRUE;
		}
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::OnRButtonDown
//
//	PURPOSE:	Right mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::OnLButtonDown
//
//	PURPOSE:	Left mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Call the base class
	return(CLTMaskedWnd::OnLButtonDown(xPos,yPos));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::DisplayText
//
//	PURPOSE:	Sets up the decisions
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::DisplayText(CStringArray *pcollDecisions, CLTWnd* pWnd, BOOL bBelow)
{
	m_nNumDecisions = 0;
	m_nCurSelection = 0;
	int i;

	m_pFont = g_pInterfaceResMgr->GetMediumFont();


	// Gotta have at least 2 decisions
	if(!pcollDecisions || pcollDecisions->GetSize() < 2)
		return FALSE;

	// Term the decision windows
	for(i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].SetFont(m_pFont);
		m_DecisionWnds[i].SetText(NULL);
		m_DecisionWnds[i].SetSelectable(FALSE);
	}

	// Set the decisions
    LTIntPt ptMax;
    LTIntPt ptCur; 
	ptMax.x = 0;
	ptMax.y = 0;
	m_byActivatedSelection = 0;


    LTIntPt ptPos = g_pLayoutMgr->GetDecisionTextOffset();
	for(i=0;((i<pcollDecisions->GetSize()) && (i<MAX_DECISIONS));i++)
	{
		if(pcollDecisions->GetAt(i).IsEmpty())
			return FALSE;

		// Set the window position
		ptCur = m_DecisionWnds[i].SetText(pcollDecisions->GetAt(i),DECISIONWND_MAX_WIDTH);
		m_DecisionWnds[i].SetSelectable(TRUE);
		m_DecisionWnds[i].MoveWindow(ptPos.x,ptPos.y+ptMax.y);
		ptMax.x = Max(ptMax.x,ptCur.x);
		ptMax.y += ptCur.y;

		m_DecisionWnds[i].Select(i == m_nCurSelection);

		// If we've got another decision, add space inbetween
		if(i+1 < pcollDecisions->GetSize())
			ptMax.y += g_pLayoutMgr->GetDecisionSpacing();
	}

	// Set our number of decisions
	m_nNumDecisions = pcollDecisions->GetSize();

	// Set our size based on the extents of the text
	int nWidth = ptMax.x + ptPos.x + ptPos.x;
	int nHeight = ptMax.y + ptPos.y + ptPos.y;


	// Adjust yPos up or down depending on if we're supposed to be
	// above or below the specified yPos
	int yPos = g_pLayoutMgr->GetDecisionPosition();
	if(pWnd)
	{
		if(bBelow)
		{
			yPos = pWnd->GetRelativeTop() + pWnd->GetHeight() + yPos;
		}
		else
		{

			yPos = pWnd->GetRelativeTop() - nHeight - yPos;
		}
	}

	// Put us at the edge of the screen
	MoveWindow(g_pGameClientShell->GetScreenWidth()-1,yPos,nWidth,nHeight);
	m_fStartPos = (float)m_xPos;

	// Show us
	ShowWindow();
	ShowAllChildren();

	m_bOpening = TRUE;
	m_tmrClose.Start(g_pLayoutMgr->GetDecisionOpenTime());

	// Move us to the top
	SetFocus();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::SendMessage
//
//	PURPOSE:	Handles a sent message
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::SendMessage(CLTWnd* pSender, int nMsg, int nParam1,int nParam2)
{
	if(nMsg == LTWM_COMMAND)
	{
		if(nParam1 == LTWC_SELECT_TEXT)
		{
			if(m_byActivatedSelection)
				return TRUE;

			if(nParam2 >= m_nNumDecisions)
				return TRUE;

			m_byActivatedSelection = uint8(nParam2 + 1);
			TRACE("Choice # %d selected!\n",m_byActivatedSelection);

			return TRUE;
		}
		else if(nParam1 == LTWC_ROLLOVER_TEXT)
		{
			if(m_byActivatedSelection)
				return TRUE;

			if(nParam2 >= m_nNumDecisions)
				return TRUE;

			SetCurSelection(nParam2);
		}

	}
	return(CLTMaskedWnd::SendMessage(pSender,nMsg,nParam1,nParam2));
}

/*BOOL CLTDecisionWnd::InitFrame(CLTSurfaceArray* pcollSurfs)
{
	if(!pcollSurfs || (pcollSurfs->GetSize() < 8))
		return FALSE;

	for(int i=0;i<pcollSurfs->GetSize();i++)
	{
		m_collFrames.Add(pcollSurfs->GetAt(i));
	}

	// Get the frame thicknesses
	DWORD x,y;
    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_LEFTFRAME],&x,&y);
	m_rcFrame.left = x;
    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_TOPFRAME],&x,&y);
	m_rcFrame.top = y;
    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_RIGHTFRAME],&x,&y);
	m_rcFrame.right = x;
    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_BOTTOMFRAME],&x,&y);
	m_rcFrame.bottom = y;

    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_TOPLEFTFRAME],&x,&y);
	m_rcTotal.left = x;
	m_rcTotal.top = y;
    g_pLTClient->GetSurfaceDims(m_collFrames[DECISIONWND_BOTTOMRIGHTFRAME],&x,&y);
	m_rcTotal.right = x;
	m_rcTotal.bottom = y;

	return TRUE;
}*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::SetAlpha
//
//	PURPOSE:	Sets the alpha value for the window
//
// ----------------------------------------------------------------------- //
void CLTDecisionWnd::SetAlpha(LTFLOAT fAlpha)
{
	if (m_hSurf)
	{
        g_pLTClient->OptimizeSurface(m_hSurf, g_hColorTransparent);
        g_pLTClient->SetSurfaceAlpha(m_hSurf, fAlpha);
	}

	for(int i=0;i<m_pcollFrames->GetSize();i++)
	{
		if (m_pcollFrames->GetAt(i))
		{
            g_pLTClient->OptimizeSurface(m_pcollFrames->GetAt(i), g_hColorTransparent);
            g_pLTClient->SetSurfaceAlpha(m_pcollFrames->GetAt(i), fAlpha);
		}
	}

}


void CLTDecisionWnd::SetCurSelection(int nSelection)
{
	if((nSelection != m_nCurSelection) && (nSelection < m_nNumDecisions) && (nSelection >= 0))
	{
		for(int i=0;i<m_nNumDecisions;i++)
		{
			m_DecisionWnds[i].Select(i == nSelection);
		}

		m_nCurSelection = nSelection;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::OnKeyDown
//
//	PURPOSE:	key down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::OnKeyDown(int nKey, int nRep)
{
	// Only handle key down if we're all good
	if(!m_bEnabled || !m_bVisible || m_bOpening || m_bClosing || m_byActivatedSelection)
		return FALSE;

	// Check if it's a special case key...
	switch (nKey)
	{
		case VK_DOWN:
		{
			SetCurSelection(m_nCurSelection+1);
			return TRUE;
		}
		case VK_UP:
		{
			SetCurSelection(m_nCurSelection-1);
			return TRUE;
		}
		case VK_RETURN:
		{
			// Selects the current selection
			SendMessage(this,LTWM_COMMAND,LTWC_SELECT_TEXT,m_nCurSelection);
			return TRUE;
		}
	}

	return FALSE;
}

