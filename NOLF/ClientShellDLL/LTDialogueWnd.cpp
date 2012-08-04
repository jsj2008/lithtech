/****************************************************************************
;
;	 MODULE:		CLTDialogueWnd (.cpp)
;
;	PURPOSE:		Class for a dialogue window
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#include "StdAfx.h"
#include "LTDialogueWnd.h"
#include "LTGuiMgr.h"
#include "LTDecisionWnd.h"

#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::Init(DIALOGUEWNDCREATESTRUCT* pcs)
{
	// Sanity checks
	if(!pcs || m_bInitialized)
		return FALSE;

	// Gotta have a font
	if(!pcs->pFont)
		return FALSE;

	// Initialize the base class
	if(!CLTMaskedWnd::Init(pcs))
		return FALSE;

	// Simple vars -- Ya gotta specify a width/height since this is scaleable, bunghole. :)
	m_nWidth = pcs->nWidth;
	m_nHeight = pcs->nHeight;
	m_pFont = pcs->pFont;
	m_bFrame = pcs->bFrame;

	// Init the frame if necessary
	if(!InitFrame(pcs->szFrame))
	{
		Term();
		return FALSE;
	}

	SetAlpha(pcs->fAlpha);

	// Decision window
	DECISIONWNDCREATESTRUCT dwcs;
	dwcs.nControlID = pcs->nControlID;
	dwcs.pParentWnd = pcs->pParentWnd;
	dwcs.szBitmap = pcs->szDecisionBitmap;
	dwcs.hSurf = pcs->hDecisionSurf;
	dwcs.pcollFrames = &m_collFrames;
	dwcs.prcFrame = &m_rcFrame;
	dwcs.prcTotal = &m_rcTotal;
	dwcs.pFont = g_pInterfaceResMgr->GetMediumFont();

	dwcs.bFrame = pcs->bDecisionFrame;
	dwcs.fAlpha = pcs->fDecisionAlpha;
	dwcs.dwFlags = LTWF_SIZEABLE;
	dwcs.dwState = LTWS_CLOSED;

	if(!m_DecisionWnd.Init(&dwcs))
	{
		Term();
		return FALSE;
	}


	Reset();

	// Center us
	MoveWindow((g_pGameClientShell->GetScreenWidth()-m_nWidth)/2,m_yPos);

	// All done!
	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTDialogueWnd::Term()
{
	// Term the frames
	for(int i=0;i<m_collFrames.GetSize();i++)
	{
		TERMSHAREDSURF(m_collFrames[i]);
	}
	m_collFrames.RemoveAll();

	// Term the decision window
	m_DecisionWnd.Term();

	// Term the picture
	m_Pic.Term();

	// Free extra stuff
    m_lfsd.szPrevString = NULL;

	// Base class
	CLTMaskedWnd::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DrawToSurface
//
//	PURPOSE:	Does the dirty work for drawing a single window to a surface
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DrawToSurface(HSURFACE hSurfDest)
{
	ASSERT(m_bInitialized);

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
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_TOPLEFTFRAME],NULL,m_xPos,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_TOPRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_BOTTOMRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos+m_nHeight-m_rcTotal.bottom,g_hColorTransparent);
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_BOTTOMLEFTFRAME],NULL,m_xPos,m_yPos+m_nHeight-m_rcTotal.bottom,g_hColorTransparent);

		// Draw the four scaleable sides
        LTRect rcDest;
		rcDest.left = m_xPos;
		rcDest.top = m_yPos+m_rcTotal.top;
		rcDest.right = m_xPos+m_rcFrame.left;
		rcDest.bottom = m_yPos+m_nHeight-m_rcTotal.bottom;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_LEFTFRAME],&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos + m_nWidth - m_rcFrame.right;
		rcDest.right = m_xPos + m_nWidth;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_RIGHTFRAME],&rcDest,NULL,g_hColorTransparent);
		rcDest.left = m_xPos+m_rcTotal.left;
		rcDest.top = m_yPos;
		rcDest.right = m_xPos+m_nWidth-m_rcTotal.right;
		rcDest.bottom = m_yPos+m_rcFrame.top;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_TOPFRAME],&rcDest,NULL,g_hColorTransparent);
		rcDest.top = m_yPos + m_nHeight - m_rcFrame.bottom;
		rcDest.bottom = m_yPos + m_nHeight;
        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DLG_BOTTOMFRAME],&rcDest,NULL,g_hColorTransparent);
	}

	// Draw the font & clip it
	if(!m_bClosing && !m_bOpening)
	{
        LTIntPt ptPos = g_pLayoutMgr->GetDialogueTextOffset();
		int nTop;
		if(m_lfdd.dwFlags & LTF_TIMED_SCROLL)
			nTop = GetWindowTop() + m_nHeight - ptPos.y - m_pFont->GetHeight();
		else
			nTop = GetWindowTop() + ptPos.y;

		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
		m_pFont->Draw((char *)(LPCSTR)m_csText,hSurfDest,&m_lfdd,GetWindowLeft() + ptPos.x,nTop,&m_lfsd);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);
		return TRUE;
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::SetPic
//
//	PURPOSE:	Sets the picture
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::SetPic(char *szBitmap)
{
	// Make sure it's not already initialized - this will take care of freeing any surface
	// that was previously created
	if(m_Pic.IsInitialized())
		m_Pic.Term();

//  LTIntPt ptPos = g_pLayoutMgr->GetDialoguePicOffset();
	CPoint ptPos = CPoint(0,0);
	if(!m_Pic.InitFromBitmap(0,"DialogueWnd Pic",this,szBitmap,ptPos.x,ptPos.y,LTWF_TRANSPARENT | LTWF_DISABLED))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
		// Center us
	MoveWindow((g_pGameClientShell->GetScreenWidth()-m_nWidth)/2,m_yPos);
	m_pFont = g_pInterfaceResMgr->GetMediumFont();

	if(!bAnimate)
		return CLTMaskedWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	//if(m_bClosing || m_bOpening) // We might be closing and something wants us to open
	if(m_bOpening)
		return FALSE;

	// Call the base class disabling sound
	BOOL bRet = CLTMaskedWnd::ShowWindow(bShow);

	// If we closed, start the closing animation
	if(bRet)
	{
		if(!bShow)
		{
			// We're temporarily visible just so we can close, but we're disabled
			m_bVisible = TRUE;
			m_bEnabled = FALSE;

			m_bClosing = TRUE;
			m_tmrClose.Start(g_pLayoutMgr->GetDialogueCloseTime());
			m_fCloseHeight = (float)m_nHeight;
			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\DialogueClose.wav");
			// Hide the picture
			if(m_Pic.IsInitialized())
				m_Pic.ShowWindow(FALSE);
		}
		else
		{
			// We were visible, and we're supposed to show (we might have been closing tho)
			if(m_bClosing)
			{
				Open();
			}
		}
	}
	else
	{
		if(bShow)
		{
			m_bEnabled = FALSE;
			m_bOpening = TRUE;
			m_tmrClose.Start(g_pLayoutMgr->GetDialogueOpenTime());
			m_fCloseHeight = (float)m_nHeight;
			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\DialogueOpen.wav");
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Update
//
//	PURPOSE:	Main update function
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::Update(float fTimeDelta)
{
	// NOTE: Dialogue window doesn't call down to the base class because we don't care to
	// update its children

	ASSERT(m_bInitialized);

	if(m_bOpening)
	{
		// Open us from the middle
		int nHeight = (int)(m_fCloseHeight - (m_fCloseHeight * (m_tmrClose.GetCountdownTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight >= m_fCloseHeight))
		{
			Open();
			return TRUE;
		}
		int yMid = m_yPos + (m_nHeight / 2);
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
	}
	else if(m_bClosing)
	{
		// Shrink us towards the middle

		int nHeight = (int)(m_fCloseHeight - (m_fCloseHeight * (m_tmrClose.GetElapseTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight < (m_rcTotal.top + m_rcTotal.bottom)))
		{
			Close();
			return TRUE;
		}
		int yMid = m_yPos + (m_nHeight / 2);
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
	}
	else if(m_bDecisions)
	{
		uint8 bySelection = m_DecisionWnd.GetActivatedSelection();
		if(bySelection)
		{
			int nDecision = bySelection;
			if (bySelection > m_collDialogueIDs.GetSize())
				nDecision = m_collDialogueIDs.GetSize();

			DoneShowing(nDecision,m_collDialogueIDs[nDecision-1]);
            g_pLTClient->ClearInput();
		}
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::OnLButtonDown
//
//	PURPOSE:	Left mouse button handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Call the base class
	CLTMaskedWnd::OnLButtonDown(xPos,yPos);

	// Skip
	if (!m_bDecisions)
		Skip();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DisplayText
//
//	PURPOSE:	Sets up the dialogue text
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DisplayText(DWORD dwID, char *szAvatar, BOOL bStayOpen, char *szDecisions)
{
	//if(m_bOpening || m_bClosing)
		//return FALSE;

    HSTRING hString=g_pLTClient->FormatString(dwID);
	CString csText;
	char *szText;

	if(hString)
	{
        szText = g_pLTClient->GetStringData(hString);
	}
	else
	{
		m_lfdd.dwFlags = LTF_DRAW_FORMATTED;
		m_lfdd.fLetterDelay = 0.0f;
		m_lfdd.fLineDelay = 0.0f;

		csText.Format("ERROR - NO TEXT FOUND FOR ID: %d",dwID);
		szText = (char *)(LPCSTR)csText;
	}

	BOOL bRet = DisplayText(szText,szAvatar,bStayOpen,szDecisions);
    g_pLTClient->FreeString(hString);

	if(bRet && m_bImmediateDecisions)
		ShowDecisions();

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DisplayText
//
//	PURPOSE:	Sets up the dialogue text
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DisplayText(char *szText, char *szAvatar, BOOL bStayOpen, char *szDecisions)
{
	if(m_bDecisions)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Show us -- We have to do this up here because it will set our coordinates correctly
	// for the clipping calculations that are below...
	ShowWindow();
	ShowAllChildren();

	// Set the format width
    LTIntPt ptPos = g_pLayoutMgr->GetDialogueTextOffset();
	m_lfdd.dwFormatWidth = m_nWidth - (ptPos.x * 2);
	m_lfdd.dwFlags = LTF_DRAW_FORMATTED;

	if(!szText)
	{
		m_csText = "ERROR - NO TEXT FOUND!";
		m_lfdd.fLetterDelay = 0.0f;
		m_lfdd.fLineDelay = 0.0f;
	}
	else
	{
		// Set the text
		m_csText = szText;

		CRect rcClip;
		GetWindowRect(&rcClip);
		m_lfdd.rcClip.left = rcClip.left + m_rcTotal.left;
		m_lfdd.rcClip.top = rcClip.top + m_rcTotal.top;
		m_lfdd.rcClip.right = rcClip.right - m_rcTotal.right;
		m_lfdd.rcClip.bottom = rcClip.bottom - m_rcTotal.bottom;

		// See if it's larger than our height
        HSTRING hStr = g_pLTClient->CreateString((char *)(LPCSTR)m_csText);
        LTIntPt pt = m_pFont->GetTextExtentsFormat(hStr,m_lfdd.dwFormatWidth);
        g_pLTClient->FreeString(hStr);
        hStr = NULL;
		if(pt.y > (m_nHeight - (ptPos.y * 2)))
		{
			// It's more than we can display all at once so scroll it.
			m_lfdd.dwFlags |= LTF_TIMED_SCROLL | LTF_EXTRA_LOCKLAST;
		}
		else
		{
			m_lfdd.dwFlags &= ~(LTF_TIMED_SCROLL | LTF_EXTRA_LOCKLAST);
		}
	}

	// Term the prev string just in case we drew this string last time timed...
    m_lfsd.szPrevString = NULL;

	// If we weren't visible, reset our command parameters to the default
	m_bCanClose = FALSE;
	m_bMore = bStayOpen;

	// Parse the text for parameters
	//if(!ParseText())
		//return FALSE;

	m_csDecisions.Empty();
	if(szDecisions)
	{
		m_csDecisions = szDecisions;
	}

	// Set the pic
	if(szAvatar && szAvatar[0])
	{
		CString csPic;
		csPic.Format("/interface/avatar/%s.pcx",szAvatar);
		SetPic((char *)(LPCSTR)csPic);
	}

	// Move us to the top and enable us
	m_bEnabled = TRUE;
	SetFocus();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ParseText
//
//	PURPOSE:	Parse the text for commands
//
// ----------------------------------------------------------------------- //
/*BOOL CLTDialogueWnd::ParseText()
{
	CString csText = m_csText;
	m_csText.Empty();
	CString csCmd;

	int nStart = csText.Find(DLG_COMMAND_CHAR);
	int nEnd;
	while(nStart != -1)
	{
		// Add on the non-command text
		m_csText += csText.Left(nStart);

		// Find the command
		csText = csText.Right(csText.GetLength()-nStart-1);

		nEnd = csText.Find(DLG_COMMAND_CHAR);
		if(nEnd == -1)
		{
			// Found start command with no end command
			TRACE("CLTDialogueWnd::ParseText - ERROR - found start command without end command\n");
			return FALSE;
		}

		csCmd = csText.Left(nEnd);

		if(csCmd.IsEmpty())
		{
			// They just wanted the special character, add it on
			m_csText += DLG_COMMAND_CHAR;
		}
		else
		{
			// TODO: Handle the command
		}

		// Trim off the command
		csText = csText.Right(csText.GetLength()-nEnd-1);
		nStart = csText.Find(DLG_COMMAND_CHAR);
	}

	// Add on any remaining non-command text
	m_csText += csText;

	return TRUE;
}*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::OnRButtonDown
//
//	PURPOSE:	Right mouse button handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	if (!m_bDecisions)
		Skip();
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Skip
//
//	PURPOSE:	Skips dialogue playing or skips to the next dialogue
//				if we're done reading.
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::Skip()
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	ASSERT(!m_bDecisions);

	if(!ShowDecisions())
	{
		DoneShowing();
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::InitFrame
//
//	PURPOSE:	Initializes the frame
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::InitFrame(const char *szFrame)
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
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stop.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stopright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomright.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottom.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomleft.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sleft.pcx",szFrame);
		hSurf = g_pInterfaceResMgr->GetSharedSurface((char *)(LPCSTR)csFrame);
        //hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ShowDecisions
//
//	PURPOSE:	Shows the decisions
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::ShowDecisions()
{
	// If we have decisions to make, do them now
	m_bDecisions = FALSE;
	if(!m_csDecisions.IsEmpty())
	{
        ILTCommon* pCommon = g_pLTClient->Common();
		if (!pCommon)
		{
			return FALSE;
		}

		ConParse parse;
		parse.Init((char *)(LPCSTR)m_csDecisions);

		m_collDialogueIDs.RemoveAll();
		CStringArray collDecisions;
		DWORD dwID;
		while (pCommon->Parse(&parse) == LT_OK)
		{
			if (parse.m_nArgs > 0 && parse.m_Args[0])
			{
				dwID = atoi(parse.m_Args[0]);
				DWORD dwTranslatedID = dwID;
                HSTRING hString=g_pLTClient->FormatString(dwTranslatedID);
				if(!hString)
				{
					return FALSE;
				}
				m_collDialogueIDs.Add(dwID);
                collDecisions.Add(g_pLTClient->GetStringData(hString));
                g_pLTClient->FreeString(hString);
			}
		}

		if(collDecisions.GetSize() <= 0)
		{
			return FALSE;
		}

		if(m_DecisionWnd.DisplayText(&collDecisions,this,TRUE))
		{
			// If we immediatelly displayed the decisions, let the server
			// finish talking...

			if (!m_bImmediateDecisions)
			{
				// Tell the server to stop speaking
				HMESSAGEWRITE hMessage;
                hMessage = g_pLTClient->StartMessage(CSM_DIALOGUE_STOP);
                g_pLTClient->EndMessage(hMessage);
			}

			m_bEnabled = FALSE;
			m_bDecisions = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DisplayTempText
//
//	PURPOSE:	Sets up the dialogue text
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DisplayTempText(char *szText)
{
	/*if(m_bDisplaying || m_bDecisions)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if(!szText)
	{
		m_csText = "ERROR - NO TEXT FOUND!";

	}
	else
	{
		// Set the text
		m_csText = szText;
	}

	// If we weren't visible, reset our command parameters to the default
	m_bCanClose = FALSE;
	m_bCanSkip = TRUE;
	m_bMore = bStayOpen;

	// Parse the text for parameters
	if(!ParseText())
		return FALSE;

	m_csDecisions.Empty();
	if(szDecisions)
	{
		m_csDecisions = szDecisions;
	}

	// Set the pic
	if(szAvatar && szAvatar[0])
	{
		CString csPic;
		csPic.Format("/interface/avatar/%s.pcx",szAvatar);
		SetPic((char *)(LPCSTR)csPic);
	}

	// Show us
	ShowWindow();
	ShowAllChildren();

	// Move us to the top and enable us
	m_bEnabled = TRUE;
	SetFocus();*/

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::SetAlpha
//
//	PURPOSE:	Sets the alpha value for the window
//
// ----------------------------------------------------------------------- //
void CLTDialogueWnd::SetAlpha(LTFLOAT fAlpha)
{
	if (m_hSurf)
	{
        g_pLTClient->OptimizeSurface(m_hSurf, g_hColorTransparent);
        g_pLTClient->SetSurfaceAlpha(m_hSurf, fAlpha);
	}

	for(int i=0;i<m_collFrames.GetSize();i++)
	{
		if (m_collFrames[i])
		{
            g_pLTClient->OptimizeSurface(m_collFrames[i], g_hColorTransparent);
            g_pLTClient->SetSurfaceAlpha(m_collFrames[i], fAlpha);
		}
	}

//	m_Pic.SetAlpha(fAlpha);

}