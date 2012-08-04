#include "StdAfx.h"
#include "LTTextWnd.h"
#include "LTGUIMgr.h"
#include "LTWndMessages.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTGUIFont* pFont,
					  int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	SetText(NULL);

	// Base class it
	if(!CLTWnd::Init(nControlID,szWndName,pParentWnd,NULL,xPos,yPos,dwFlags,dwState))
		return FALSE;

	m_pFont = pFont;
    m_bSelected = LTFALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::SetText
//
//	PURPOSE:	Sets the text to display
//
// ----------------------------------------------------------------------- //
LTIntPt CLTTextWnd::SetText(const char *szText, int nWidth)
{
    LTIntPt pt;
	pt.x = 0;
	pt.y = 0;

	// Make sure we have text
	if(!szText || !szText[0])
	{
		m_csText.Empty();
		ShowWindow(FALSE,IsInitialized(),IsInitialized());
		return pt;
	}

	// Set the size of the window based on the extents of the text
	m_csText = szText;
    HSTRING hStr = g_pLTClient->CreateString((char *)(LPCSTR)m_csText);
	if(nWidth > 0)
	{
		m_nWidth = nWidth;
		pt = m_pFont->GetTextExtentsFormat(hStr,m_nWidth);
	}
	else
	{
		pt = m_pFont->GetTextExtents(hStr);
	}

	m_nWidth = pt.x;
	m_nHeight = pt.y;
    g_pLTClient->FreeString(hStr);
	m_lfdd.dwFormatWidth = m_nWidth;
	m_lfdd.dwFlags |= LTF_DRAW_FORMATTED;

	// Term the prev string just in case we drew this string last time timed...
    m_lfsd.szPrevString = NULL;
    m_lfsd.byLastState = LTF_STATE_NONE;

	return pt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::DrawToSurface
//
//	PURPOSE:	Draw us
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::DrawToSurface(HSURFACE hSurfDest)
{
	if(m_csText.IsEmpty())
	{
		ASSERT(FALSE);
		return TRUE;
	}

	HLTCOLOR hColor = kBlack;
	if (m_bSelected)
		hColor = kWhite;
	else if (!m_bSelectable)
		hColor = kGray;

	m_pFont->DrawFormat((char *)(LPCSTR)m_csText,hSurfDest,GetWindowLeft(),GetWindowTop(),m_nWidth,hColor);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
	// Call the base class disabling sound
	BOOL bRet = CLTWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	// We can't go visible without text
	if(m_bVisible && m_csText.IsEmpty())
	{
		CLTWnd::ShowWindow(FALSE);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnLButtonDown
//
//	PURPOSE:	Left button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnLButtonDown(int xPos, int yPos)
{
	// Call the base class
	if(!CLTWnd::OnLButtonDown(xPos,yPos))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnRButtonDown
//
//	PURPOSE:	Right button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnRButtonDown(int xPos, int yPos)
{
	// Call the base class
	if(!CLTWnd::OnLButtonDown(xPos,yPos))
		return FALSE;

	s_pWndCapture = this;
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnLButtonUp
//
//	PURPOSE:	Left button Up handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnLButtonUp(int xPos, int yPos)
{
	if(s_pWndCapture != this)
		return CLTWnd::OnLButtonUp(xPos,yPos);

	// We have a click... Tell our parent about it
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	if(pParent)
	{
		pWnd->SendMessage(this,LTWM_COMMAND,LTWC_SELECT_TEXT,m_nControlID);
		if (m_bSelectable)
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		else
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundUnselectable());

	}

	CLTWnd::OnLButtonUp(xPos,yPos);
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnRButtonUp
//
//	PURPOSE:	Right button Up handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnRButtonUp(int xPos, int yPos)
{
	if(s_pWndCapture != this)
		return FALSE;

	// We have a click... Tell our parent about it
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	if(pParent)
	{
		pWnd->SendMessage(this,LTWM_COMMAND,LTWC_SELECT_TEXT,m_nControlID);
		if (m_bSelectable)
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		else
			g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundUnselectable());

	}
	s_pWndCapture = NULL;
	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnMouseEnter
//
//	PURPOSE:	Mouse movement handler
//
// ----------------------------------------------------------------------- //
void CLTTextWnd::OnMouseEnter()
{
	if (m_bSelectable)
	{
		// We have a move... Tell our parent about it
		CLTWnd* pWnd = this;
		CLTWnd* pParent = m_pParentWnd;
		while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
		{
			pWnd = pParent;
			pParent = pWnd->GetParent();
		}

		if(pParent)
		{
			pWnd->SendMessage(this,LTWM_COMMAND,LTWC_ROLLOVER_TEXT,m_nControlID);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnMouseLeave
//
//	PURPOSE:	Mouse movement handler
//
// ----------------------------------------------------------------------- //
void CLTTextWnd::OnMouseLeave()
{
//	if (m_bSelectable)
//       Select(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::Select
//
//	PURPOSE:	Selection change handler
//
// ----------------------------------------------------------------------- //
void CLTTextWnd::Select(LTBOOL bSelect)
{
	if (m_bSelectable)
	{
		m_bSelected = bSelect;
		if (bSelect) g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
	}
}

void CLTTextWnd::SetFont(CLTGUIFont* pFont)
{
	m_pFont = pFont;
}
