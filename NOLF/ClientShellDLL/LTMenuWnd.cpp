/****************************************************************************
;
;	 MODULE:		CLTMenuWnd (.cpp)
;
;	PURPOSE:		Class for a menu window
;
;	HISTORY:		03/20/00 [jrg] This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Productions, Inc.
;
****************************************************************************/
#include "StdAfx.h"
#include "LTMenuWnd.h"
#include "LTGuiMgr.h"
#include "LTWndMessages.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

BOOL CLTMenuWnd::Init(MENUWNDCREATESTRUCT *pcs)
{
	// Sanity checks
	if(!pcs || m_bInitialized)
		return FALSE;

	// Gotta have this stuff
	if(!pcs->pFont)
		return FALSE;

	// Initialize the base class
	if(!CLTMaskedWnd::Init(pcs))
		return FALSE;


	// Simple vars
	m_pFont = pcs->pFont;
	m_bFrame = pcs->bFrame;

	// Init the frame if necessary
	if(!InitFrame(pcs->szFrame))
	{
		Term();
		return FALSE;
	}

	SetAlpha(pcs->fAlpha);

	// Set up the decision windows
	for(int i=0;i<MAX_ITEMS;i++)
	{
		m_ItemWnds[i].Init(i,"Item",this,m_pFont,0,0,LTWF_FIXEDCHILD);
	}

	Reset();

	m_nCurSelection = 0;

	// All done!
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::Term()
{
	// Term the decision windows
	for(int i=0;i<MAX_ITEMS;i++)
	{
		m_ItemWnds[i].Term();
	}


	CLTMaskedWnd::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::DrawToSurface
//
//	PURPOSE:	Draws the window
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::DrawToSurface(HSURFACE hSurfDest)
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
	if(m_bFrame && (m_collFrames.GetSize() == 8))
	{
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_TOPLEFTFRAME),NULL,m_xPos,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_TOPRIGHTFRAME),NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_BOTTOMRIGHTFRAME),NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos+m_nHeight-m_rcTotal.bottom,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_BOTTOMLEFTFRAME),NULL,m_xPos,m_yPos+m_nHeight-m_rcTotal.bottom,g_hColorTransparent);

		// Draw the four scaleable sides
        LTRect rcDest;
		rcDest.left = m_xPos;
		rcDest.top = m_yPos+m_rcTotal.top;
		rcDest.right = m_xPos+m_rcFrame.left;
		rcDest.bottom = m_yPos+m_nHeight-m_rcTotal.bottom;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_LEFTFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos + m_nWidth - m_rcFrame.right;
		rcDest.right = m_xPos + m_nWidth;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_RIGHTFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos+m_rcTotal.left;
		rcDest.top = m_yPos;
		rcDest.right = m_xPos+m_nWidth-m_rcTotal.right;
		rcDest.bottom = m_yPos+m_rcFrame.top;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_TOPFRAME),&rcDest,NULL,g_hColorTransparent);
		rcDest.top = m_yPos + m_nHeight - m_rcFrame.bottom;
		rcDest.bottom = m_yPos + m_nHeight;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames.GetAt(DLG_BOTTOMFRAME),&rcDest,NULL,g_hColorTransparent);
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
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
			m_tmrClose.Start(g_pLayoutMgr->GetMenuCloseTime());
			m_fStartPos = (float)m_nHeight;
            m_pMenu = LTNULL;
			m_byActivatedSelection = 0;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::Update
//
//	PURPOSE:	Updates the window
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::Update(float fTimeDelta)
{
	// NOTE: Decision window doesn't call down to the base class because we don't care to
	// update its children

	ASSERT(m_bInitialized);

	if(m_bOpening)
	{
		// Move us towards the center
		int xDest = (g_pGameClientShell->GetScreenWidth()-m_nWidth) / 2;
		int yDest = (g_pGameClientShell->GetScreenHeight()-m_nHeight) / 2;
		m_bEnabled = TRUE;
		m_bOpening = FALSE;
		MoveWindow(xDest,yDest);
	}
	else if(m_bClosing)
	{
		// Shrink us towards the middle
		int yMid = m_yPos + (m_nHeight / 2);
		int nHeight = (int)(m_fStartPos - (m_fStartPos * (m_tmrClose.GetElapseTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight < (m_rcTotal.top + m_rcTotal.bottom)))
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
	else if (m_byActivatedSelection && m_pMenu)
	{
		m_pMenu->Select(m_ItemCmds[m_byActivatedSelection-1]);
		m_byActivatedSelection = 0;
        g_pLTClient->ClearInput();
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::OnRButtonDown
//
//	PURPOSE:	Right mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::OnLButtonDown
//
//	PURPOSE:	Left mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Call the base class
	return(CLTMaskedWnd::OnLButtonDown(xPos,yPos));
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::AddItem
//
//	PURPOSE:	Adds an item
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::AddItem(HSTRING hText, uint8 byCommand)
{
    AddItem(g_pLTClient->GetStringData(hText),byCommand);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::AddItem
//
//	PURPOSE:	Adds an item
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::AddItem(uint32 nTextID, uint8 byCommand)
{
    HSTRING hStr = g_pLTClient->FormatString(nTextID);
    AddItem(g_pLTClient->GetStringData(hStr),byCommand);
    g_pLTClient->FreeString(hStr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::AddItem
//
//	PURPOSE:	Adds an item
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::AddItem(const char *szText, uint8 byCommand)
{
	if (m_nNumItems >= MAX_ITEMS)
		return;


    LTIntPt ptPos = g_pLayoutMgr->GetMenuTextOffset();

	if (m_nNumItems > 0)
		m_curSz.y += g_pLayoutMgr->GetMenuSpacing();

	// Set the window position
    LTIntPt ptCur = m_ItemWnds[m_nNumItems].SetText(szText,MENUWND_MAX_WIDTH);
	m_ItemWnds[m_nNumItems].SetSelectable(TRUE);
	m_ItemWnds[m_nNumItems].MoveWindow(ptPos.x,ptPos.y+m_curSz.y);
	m_curSz.x = Max(m_curSz.x,ptCur.x);
	m_curSz.y += ptCur.y;

	m_ItemCmds[m_nNumItems] = byCommand;

	m_nNumItems++;

}

void CLTMenuWnd::ClearItems()
{
	m_nNumItems = 0;
    m_curSz = LTIntPt(0,0);

	// Term the decision windows
	for(int i=0;i<MAX_ITEMS;i++)
	{
		m_ItemWnds[i].SetText(NULL);
		m_ItemWnds[i].Select(FALSE);
		m_ItemWnds[i].SetSelectable(FALSE);
		m_ItemCmds[i] = 0;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::Open
//
//	PURPOSE:	Opens the menu
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::Open()
{

	if(m_nNumItems < 1)
		return FALSE;

	m_byActivatedSelection = 0;


    LTIntPt ptPos = g_pLayoutMgr->GetMenuTextOffset();

	// Set our size based on the extents of the text
	int nWidth = m_curSz.x + ptPos.x + ptPos.x;
	int nHeight = m_curSz.y + ptPos.y + ptPos.y;


	// Adjust yPos up or down depending on if we're supposed to be
	// above or below the specified yPos
	int yPos = g_pLayoutMgr->GetMenuPosition();

	// Put us at the edge of the screen
	MoveWindow(g_pGameClientShell->GetScreenWidth()-1,yPos,nWidth,nHeight);
	m_fStartPos = (float)m_xPos;

	// Show us
	ShowWindow();
	ShowAllChildren();

	m_nCurSelection = 0;

	for (int i = 0; i < m_nNumItems; i++)
	{
		m_ItemWnds[i].Select(i == m_nCurSelection);
	}
	m_bOpening = TRUE;
	m_tmrClose.Start(g_pLayoutMgr->GetMenuOpenTime());

	// Move us to the top
	SetFocus();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::Close
//
//	PURPOSE:	Closess the menu
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::Close()
{
    ShowWindow(LTFALSE,LTTRUE,LTFALSE);
	ClearItems();
    SetMenu(LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::SendMessage
//
//	PURPOSE:	Handles a sent message
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::SendMessage(CLTWnd* pSender, int nMsg, int nParam1,int nParam2)
{
	if(nMsg == LTWM_COMMAND)
	{
		if(nParam1 == LTWC_SELECT_TEXT)
		{
			if(m_byActivatedSelection)
				return TRUE;

			if(nParam2 >= m_nNumItems)
				return TRUE;

			m_byActivatedSelection = BYTE(nParam2 + 1);
			TRACE("Choice # %d selected!\n",m_byActivatedSelection);

			return TRUE;
		}
		else if(nParam1 == LTWC_ROLLOVER_TEXT)
		{
			if(m_byActivatedSelection)
				return TRUE;

			if(nParam2 >= m_nNumItems)
				return TRUE;

			SetCurSelection(nParam2);
		}

	}
	return(CLTMaskedWnd::SendMessage(pSender,nMsg,nParam1,nParam2));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::SetAlpha
//
//	PURPOSE:	Sets the alpha value for the window
//
// ----------------------------------------------------------------------- //
void CLTMenuWnd::SetAlpha(LTFLOAT fAlpha)
{
	if (m_hSurf)
	{
        g_pLTClient->OptimizeSurface(m_hSurf, g_hColorTransparent);
        g_pLTClient->SetSurfaceAlpha(m_hSurf, fAlpha);
	}

	for(int i=0;i<m_collFrames.GetSize();i++)
	{
		if (m_collFrames.GetAt(i))
		{
            g_pLTClient->OptimizeSurface(m_collFrames.GetAt(i), g_hColorTransparent);
            g_pLTClient->SetSurfaceAlpha(m_collFrames.GetAt(i), fAlpha);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::InitFrame
//
//	PURPOSE:	Initializes the frame
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::InitFrame(const char *szFrame)
{
	// Init the frame
	if(szFrame)
	{
		HSURFACE hSurf;
		// Init the frames
		CString csFrame;
		csFrame.Format("%stopleft.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stop.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stopright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottom.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomleft.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sleft.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTMenuWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);

		// Get the frame thicknesses
		DWORD x,y;
        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_LEFTFRAME],&x,&y);
		m_rcFrame.left = x;
        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_TOPFRAME],&x,&y);
		m_rcFrame.top = y;
        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_RIGHTFRAME],&x,&y);
		m_rcFrame.right = x;
        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_BOTTOMFRAME],&x,&y);
		m_rcFrame.bottom = y;

        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_TOPLEFTFRAME],&x,&y);
		m_rcTotal.left = x;
		m_rcTotal.top = y;
        g_pLTClient->GetSurfaceDims(m_collFrames[DLG_BOTTOMRIGHTFRAME],&x,&y);
		m_rcTotal.right = x;
		m_rcTotal.bottom = y;
		return TRUE;
	}

	return FALSE;
}


void CLTMenuWnd::SetCurSelection(int nSelection)
{
	if((nSelection != m_nCurSelection) && (nSelection < m_nNumItems) && (nSelection >= 0))
	{
		for(int i=0;i<m_nNumItems;i++)
		{
			m_ItemWnds[i].Select(i == nSelection);
		}

		m_nCurSelection = nSelection;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMenuWnd::OnKeyDown
//
//	PURPOSE:	key down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTMenuWnd::OnKeyDown(int nKey, int nRep)
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
