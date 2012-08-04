// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreen.cpp
//
// PURPOSE : Base class for interface screens
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScreen.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "SoundMgr.h"
#include "VKDefs.h"
#include "GameClientShell.h"

namespace
{
	LTVector2n offscreen(-64,-64);
}

VarTrack g_vtTextTransitionTime;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseScreen::CBaseScreen()
{
    m_bInit = false;
    m_bBuilt = false;
    m_bBack = false;
	m_bHaveFocus = false;

	m_pScreenMgr = NULL;

	m_BaseTitlePos.x = 0;
	m_BaseTitlePos.y = 0;
    m_TitleSize = 32;
    m_TitleColor = argbBlack;

	m_nScreenID = SCREEN_ID_NONE;
	m_nContinueID = SCREEN_ID_NONE;

	m_szCurrHelpID = "";

	// Array of controls that this screen owns
	m_controlArray.reserve(5);

	m_nSelection = kNoSelection;
	m_nOldSelection = kNoSelection;
    m_pCaptureCtrl = NULL;
	m_bCaptureAlways = false;
	m_nRMouseDownItemSel =  kNoSelection;
	m_nRMouseDownItemSel =  kNoSelection;

	m_nItemSpacing = 0;

	m_SelectedColor		= argbWhite;
	m_NonSelectedColor	= argbBlack;
	m_DisabledColor		= argbGray;

	m_vfLastScale.Init();

	m_bVisited = false;
	m_DefaultPos.Init();

	m_hLayout = NULL;
}

CBaseScreen::~CBaseScreen()
{
	if ( m_bInit )
	{
		Term();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Init
//
//	PURPOSE:	initialize the screen
//
// ----------------------------------------------------------------------- //

bool CBaseScreen::Init(int nScreenID)
{

	if (!g_vtTextTransitionTime.IsInitted())
	{
		HRECORD hShared = g_pLayoutDB->GetSharedRecord();
		g_vtTextTransitionTime.Init(g_pLTClient,"TextTransitionTime",NULL,g_pLayoutDB->GetFloat(hShared,"TextTransitionTime",0,0.25f));
	}

	m_nScreenID=nScreenID;
	m_pScreenMgr = g_pInterfaceMgr->GetScreenMgr();
	m_TransitionTimer.SetEngineTimer(RealTimeTimer::Instance());

	m_hLayout = g_pLayoutDB->GetScreenRecord((eScreenID)nScreenID);

	//set up layout variables
	m_BaseTitlePos = g_pLayoutDB->GetPosition(m_hLayout,LDB_ScreenTitlePos);
	m_TitleFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenTitleFont);
	m_TitleSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenTitleSize);
	m_TitleColor = g_pLayoutDB->GetColor(m_hLayout,LDB_ScreenTitleColor);;

	uint32 nSize = (uint32)((float)m_TitleSize * g_pInterfaceResMgr->GetYRatio());
	m_TitleString.SetPos(g_pInterfaceResMgr->ConvertScreenPos(m_BaseTitlePos));
	m_TitleString.SetAlignment(kRight);
	m_TitleString.SetFont( CFontInfo(m_TitleFont.c_str(),nSize));
	m_TitleString.SetColor(m_TitleColor);
	m_TitleString.SetDropShadow(2);

	m_ScreenRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenRect);

	SetItemSpacing(g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenItemSpace));

	m_SelectedColor		= g_pLayoutDB->GetColor(m_hLayout,LDB_ScreenSelectedColor);
	m_NonSelectedColor	= g_pLayoutDB->GetColor(m_hLayout,LDB_ScreenNonSelectedColor);
	m_DisabledColor		= g_pLayoutDB->GetColor(m_hLayout,LDB_ScreenDisabledColor);

	if (!m_pScreenMgr->GetHelpFont( ).m_nHeight)
	{
		
		uint16 nWidth = (uint16)( (float)m_pScreenMgr->GetHelpWidth( ) * g_pInterfaceResMgr->GetXRatio());
		uint32 nSize = (uint32)((float)m_pScreenMgr->GetHelpSize( ) * g_pInterfaceResMgr->GetYRatio());

		m_pScreenMgr->GetHelpFont( ) = CFontInfo(g_pLayoutDB->GetHelpFont(),nSize);
		m_pScreenMgr->GetHelpStr( ).SetFont(m_pScreenMgr->GetHelpFont( ));
		m_pScreenMgr->GetHelpStr( ).WordWrap(nWidth);
		m_pScreenMgr->GetHelpStr( ).SetColor(m_SelectedColor);

	}

	const char* pszBack = g_pLayoutDB->GetString(m_hLayout,"BackgroundTexture");
	if (pszBack)
	{
		m_hBackTexture.Load(pszBack);
		SetupQuadUVs(m_BackPoly, m_hBackTexture, 0.0f, 0.0f, 1.0f, 1.0f );
		DrawPrimSetRGBA(m_BackPoly,argbWhite);
	}


	m_bInit=TRUE;

	m_DefaultPos = m_ScreenRect.m_vMin;
    return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CBaseScreen::Term()
{
	RemoveAll();

	// Free the title string
	m_TitleString.FlushTexture();

	m_pScreenMgr->GetHelpStr( ).FlushTexture();

	m_bInit=FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Escape
//
//	PURPOSE:	back out of the screen
//
// ----------------------------------------------------------------------- //

void CBaseScreen::Escape()
{
	if (!m_pScreenMgr->PreviousScreen())
	{
       	if (g_pPlayerMgr->IsPlayerInWorld() && 
			(g_pPlayerMgr->IsPlayerAlive() || IsMultiplayerGameClient( )) )
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		else
		{
			m_pScreenMgr->ClearHistory();
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MAIN);
		}
	}
	else
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Render
//
//	PURPOSE:	Renders the screen to a surface
//
// ----------------------------------------------------------------------- //

bool CBaseScreen::Render()
{
	LTVector2n tmpPos;

	if (m_hBackTexture != (HTEXTURE)NULL)
	{
		g_pDrawPrim->SetTexture(m_hBackTexture);
		g_pDrawPrim->DrawPrim(&m_BackPoly, 1);
	}


	if (m_TransitionTimer.IsStarted() && ! m_TransitionTimer.IsTimedOut())
	{
		float fTrans = (float)(m_TransitionTimer.GetElapseTime() / m_TransitionTimer.GetDuration());
		// Render the title
		if (!m_TitleString.IsEmpty())
		{
			m_TitleString.RenderTransition(fTrans);
		}
		for (uint16 i = 0; i < m_controlArray.size(); i++ )
		{
			m_controlArray[i]->RenderTransition(fTrans);
		}

		return true;
	}
	// Render the title
	if (!m_TitleString.IsEmpty())
	{
		m_TitleString.Render();
	}

	
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->Render ();
	}

	if (!m_pScreenMgr->GetHelpStr( ).IsEmpty())
	{
		m_pScreenMgr->GetHelpStr( ).Render();
	}


    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::CreateTitle
//
//	PURPOSE:	Creates the string to display as the screens title
//
// ----------------------------------------------------------------------- //


bool CBaseScreen::CreateTitle(const wchar_t *pszTitle)
{
	uint32 nFontSize = (uint32)((float)m_TitleSize * g_pInterfaceResMgr->GetYRatio());

	m_TitleString.SetFont( CFontInfo(m_TitleFont.c_str(),nFontSize));
	m_TitleString.SetText(pszTitle);
	m_TitleString.SetPos(g_pInterfaceResMgr->ConvertScreenPos(m_BaseTitlePos));
	m_TitleString.SetColor(m_TitleColor);

	return true;

}

bool CBaseScreen::CreateTitle(const char* szStringID)
{
    bool created = CreateTitle(LoadString(szStringID));

	return created;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Build
//
//	PURPOSE:	Construct the basic screen elements
//
// ----------------------------------------------------------------------- //

bool CBaseScreen::Build()
{
    m_bBuilt=true;

    UseBack(true);

	return TRUE;
}

// Handles a user entered character
bool CBaseScreen::HandleChar(wchar_t c)
{
    bool handled = false;

	if (m_pCaptureCtrl)
	{
		if (m_pCaptureCtrl->HandleChar(c))
            handled = true;
	}
	return handled;
}


// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
bool CBaseScreen::HandleKeyDown(int key, int rep)
{
    bool handled = false;

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	switch (key)
	{
	case VK_LEFT:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->OnLeft();
			else
				handled = OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->OnRight();
			else
				handled = OnRight();
			break;
		}
	case VK_UP:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->OnUp();
			else
				handled = OnUp();
			break;
		}
	case VK_DOWN:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->OnDown();
			else
				handled = OnDown();
			break;
		}
	case VK_RETURN:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->OnEnter();
			else
				handled = OnEnter();
			break;
		}
	default:
		{
			if (m_pCaptureCtrl)
				handled = m_pCaptureCtrl->HandleKeyDown(key,rep);
			else
			{
				CLTGUICtrl* pCtrl = GetSelectedControl();
				if (pCtrl)
				{
					handled = pCtrl->HandleKeyDown(key,rep);
					if (handled && (key == VK_NEXT || key == VK_PRIOR))
						g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
				}
				else
					handled = false;
			}
			break;
		}
	}

#endif // !PLATFORM_XENON

	// Handled the key
	return handled;
}


/******************************************************************/


/******************************************************************/

bool CBaseScreen::OnUp()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		UpdateHelpText();
		return true;
	}

	uint16 sel = m_nSelection;
	return (sel != PreviousSelection());
}

/******************************************************************/

bool CBaseScreen::OnDown()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		UpdateHelpText();
		return true;
	}

	uint16 sel = m_nSelection;
	return (sel != NextSelection());
}

/******************************************************************/

bool CBaseScreen::OnLeft()
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	if (handled)
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	return handled;
}


/******************************************************************/

bool CBaseScreen::OnRight()
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	if (handled)
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	return handled;
}


/******************************************************************/

bool CBaseScreen::OnEnter()
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
	{
		handled = pCtrl->OnEnter();
		if (handled)
		{
			if (pCtrl == &m_pScreenMgr->GetBackCtrl( ))
				g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
			else
				g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		}
	}
	return handled;
}


uint16 CBaseScreen::NextSelection()
{
	uint16 select = m_nSelection;
	if (select == kNoSelection)
		select = m_controlArray.size()-1;
	uint16 oldSelect = select;
	
	CLTGUICtrl* pCtrl = NULL;	
	do
	{
		select++;
		if (select >= m_controlArray.size())
		{
			select = 0;
		}
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nSelection;

	return SetSelection(select);

}

uint16 CBaseScreen::PreviousSelection()
{
	uint16 select = m_nSelection;
	if (select == kNoSelection)
		select = 0;
	uint16 oldSelect = select;
	
	CLTGUICtrl* pCtrl = NULL;	
	do
	{
		if (select == 0)
		{
			select = m_controlArray.size()-1;
		}
		else
			select--;
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nSelection;

	return SetSelection(select);

}

/******************************************************************/
bool CBaseScreen::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return false;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLMouseDownItemSel=nControlIndex;
		return pCtrl->OnLButtonDown(x,y);

	}
	else
		m_nLMouseDownItemSel=kNoSelection;

    return false;
}

/******************************************************************/
bool CBaseScreen::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return false;
		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLMouseDownItemSel)
		{
			if (pCtrl->IsEnabled() )
			{
				SetSelection(nControlIndex);
				bool bHandled = pCtrl->OnLButtonUp(x,y);
				if (bHandled)
				{
					if (pCtrl == &m_pScreenMgr->GetBackCtrl( ))
						g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
					else
						g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
				}
				return bHandled;

			}
		}
	}
	else
	{
		m_nLMouseDownItemSel= kNoSelection;
	}
    return false;
}

/******************************************************************/
bool CBaseScreen::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return false;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRMouseDownItemSel=nControlIndex;

		return pCtrl->OnRButtonDown(x,y);
	}
	else
		m_nRMouseDownItemSel=kNoSelection;

    return false;
}

/******************************************************************/
bool CBaseScreen::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return false;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "left" message to the control.
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			if (pCtrl->IsEnabled())
			{
				SetSelection(nControlIndex);
				bool bHandled = pCtrl->OnRButtonUp(x,y);
				if (pCtrl == &m_pScreenMgr->GetBackCtrl( ) )
					g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				else
					g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
				return bHandled;
			}
		}
	}
	else
	{
		m_nRMouseDownItemSel= kNoSelection;
	}
    return false;
}

/******************************************************************/
bool CBaseScreen::OnLButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return false;

	if (pCtrl)
		return pCtrl->OnLButtonDblClick(x, y);
	else
        return false;
}

/******************************************************************/
bool CBaseScreen::OnRButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return false;

	if (pCtrl)
		return pCtrl->OnRButtonDblClick(x, y);
	else
        return false;
}


void CBaseScreen::GetHelpString(const char* szHelpId, uint16 nControlIndex, wchar_t *buffer, int bufLen)
{
	LTStrCpy( buffer, LoadString(szHelpId), bufLen );
}


/******************************************************************/
bool CBaseScreen::OnMouseMove(int x, int y)
{
	bool bReturn = false;

	// restore the cursor to default
	m_hCursorRecord = NULL;

	uint16 nControlUnderPoint=kNoSelection;
    bool onCtrl = GetControlUnderPoint(x,y,&nControlUnderPoint);
	if (onCtrl)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlUnderPoint);
		if ( m_pCaptureCtrl && m_pCaptureCtrl != pCtrl )
		{
			bReturn = false;
			goto DONE;
		}

		if (pCtrl->OnMouseMove(x,y))
		{
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
		}
	}
	else if( !m_pCaptureCtrl )
	{
		bReturn = false;
		goto DONE;
	}
	
	if( onCtrl )
	{
		if (GetSelection() != nControlUnderPoint)
		{
			if (GetControl(nControlUnderPoint)->IsEnabled())
			{
				SetSelection(nControlUnderPoint);
			}
		}

		bReturn = true;
	}

DONE:
	if( g_pCursorMgr )
		g_pCursorMgr->SetCursor( m_hCursorRecord );

    return bReturn;
}

bool CBaseScreen::OnMouseWheel(int x, int y, int zDelta)
{
	uint16 nControlUnderPoint=kNoSelection;
	bool onCtrl = GetControlUnderPoint(x,y,&nControlUnderPoint);
	if (onCtrl)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlUnderPoint);
		if (m_pCaptureCtrl && m_pCaptureCtrl != pCtrl)
			return false;

		if( pCtrl )
			return pCtrl->OnMouseWheel(x,y,zDelta);
	}

	return false;
}

uint16 CBaseScreen::SetSelection(uint16 select, bool bFindSelectable)
{
	if (select == m_nSelection) return select;

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
		{
			GetControl(nOldSelect)->Select(false);
			OnSelectionChange();
		}
		m_nOldSelection = m_nSelection;
		m_nSelection = kNoSelection;
		UpdateHelpText();
		return kNoSelection;
	}


	CLTGUICtrl *pSelCtrl;


	if (select >= 0)
	{
		if (select >= m_controlArray.size())
			select = m_controlArray.size()-1;
	}


	pSelCtrl = GetControl(select);
	if (!pSelCtrl)
	{
		UpdateHelpText();
		return nOldSelect;
	}
	//check to see if we can select this item
	if (!pSelCtrl->IsEnabled())
	{
		//if we don't need to find a selectable item return
		if (!bFindSelectable)
		{
			UpdateHelpText();
			return nOldSelect;
		}

		//keep looking until we run out of on screen items or find a selectable one
		while (pSelCtrl && !pSelCtrl->IsEnabled())
		{
			select++;
			pSelCtrl = GetControl(select);
		}
		if (!pSelCtrl || !pSelCtrl->IsEnabled())
		{
			UpdateHelpText();
			return nOldSelect;
		}
	}


	if (nOldSelect != kNoSelection)
	{
		GetControl(nOldSelect)->Select(false);
	}

	m_nOldSelection = m_nSelection;
	m_nSelection = select;

	if (m_nSelection == kNoSelection)
	{
		UpdateHelpText();
		return nOldSelect;
	}

	pSelCtrl->Select(true);

	if (m_nSelection != nOldSelect)
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);



	UpdateHelpText();
	OnSelectionChange();
	return m_nSelection;
}


// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
bool CBaseScreen::GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex)
{
	LTASSERT(pnIndex,"");

	if( m_pCaptureCtrl && (m_bCaptureAlways || m_pCaptureCtrl->IsOnMe(xPos,yPos)) )
	{
		*pnIndex = GetIndex(m_pCaptureCtrl);
		return true;
	}


	// See if the user clicked on any of the controls.
	for (uint16 i=0; i < m_controlArray.size() ; i++)
	{
		// Check to see if the click is in the bounding box for the control
		if (m_controlArray[i] && m_controlArray[i]->IsOnMe(xPos,yPos))
		{
			if (m_controlArray[i]->IsEnabled())
			{
				*pnIndex=i;
				return true;

			}
		}
	}

    return false;
}

// Return a control at a specific index
CLTGUICtrl *CBaseScreen::GetControl ( uint16 nIndex )
{
	if (nIndex < m_controlArray.size() )
		return m_controlArray[nIndex];
    return NULL;
}





void CBaseScreen::RemoveAll(bool bDelete)
{
	RemoveControl(&m_pScreenMgr->GetBackCtrl( ),false);
	// Terminate the ctrls
	if (bDelete)
	{
		for (uint16 i=0; i < m_controlArray.size(); i++)
		{
			m_controlArray[i]->Destroy();
			debug_delete(m_controlArray[i]);
		}
	}
	m_controlArray.clear();
	if (m_nSelection >= 0)
		m_nSelection = kNoSelection;

	m_DefaultPos = m_ScreenRect.m_vMin;

}

uint16 CBaseScreen::AddControl(CLTGUICtrl* pCtrl)
{
	m_controlArray.push_back(pCtrl);
	uint16 num = m_controlArray.size();
	if (num == m_nSelection+1)
        pCtrl->Select(true);
	if (num > 0)
	{
		m_DefaultPos = pCtrl->GetBasePos();
		m_DefaultPos.y += (pCtrl->GetBaseHeight() + m_nItemSpacing);
		return num-1;
	}
	else
		return kNoSelection;

}


// Calls UpdateData on each control in the menu
void CBaseScreen::UpdateData(bool bSaveAndValidate)
{
	for (uint16 i=0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}
}



CLTGUITextCtrl* CBaseScreen::CreateTextItem(const char* szStringID, CLTGUICtrl_create cs, bool bFixed, const char* szFont, uint32 nFontHeight)
{
	CLTGUITextCtrl* pCtrl=CreateTextItem(LoadString(szStringID), cs, bFixed, szFont, nFontHeight);
	return pCtrl;

}

CLTGUITextCtrl* CBaseScreen::CreateTextItem(const wchar_t *pString, CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUITextCtrl* pCtrl=debug_new(CLTGUITextCtrl);

	if (!szFont)
	{
		szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	}

	if (!nFontHeight)
		nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}
	cs.pCommandHandler = this;

	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();

    if (!pCtrl->Create(pString, CFontInfo(szFont,nFontHeight), cs))
	{
		debug_delete(pCtrl);
        return NULL;
	}


	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	

	if (bFixed)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}


CLTGUIListCtrl* CBaseScreen::CreateList(CLTGUIListCtrl_create cs)
{
	
	if (cs.bArrows)
	{
		if (!cs.hUpNormal)
	{
			cs.hUpNormal.Load("interface\\menu\\sprtex\\arrowup.dds");
			cs.hUpSelected.Load("interface\\menu\\sprtex\\arrowup_h.dds");
			cs.hDownNormal.Load("interface\\menu\\sprtex\\arrowdn.dds");
			cs.hDownSelected.Load("interface\\menu\\sprtex\\arrowdn_h.dds");
			if (cs.vnArrowSz.x == 0 || cs.vnArrowSz.y == 0)
			{
				cs.vnArrowSz = LTVector2n(16,16);
			}
			
		}

		if (!cs.nArrowOffset)
		{
			cs.nArrowOffset = cs.rnBaseRect.GetWidth() - 16;
		}
	}

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
		{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
		}
	
	CLTGUIListCtrl* pList=debug_new(CLTGUIListCtrl);
    if (pList->Create(cs))
	{
		pList->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pList->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pList;
}

CLTGUIListCtrlEx* CBaseScreen::CreateListEx(CLTGUIListCtrlEx_create cs)
{
	cs.nSelectedColumnColor = g_pLayoutDB->GetListSelectedColumnColor();
	cs.nBackgroundColumnColor = g_pLayoutDB->GetListBackgroundColumnColor();
	cs.nHighlightColor = g_pLayoutDB->GetListHighlightColor();

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	cs.pCommandHandler = this;

	CLTGUIListCtrlEx* pList=debug_new(CLTGUIListCtrlEx);
	if (pList->Create(cs))
	{
		pList->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pList->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pList;
}

CLTGUIHeaderCtrl* CBaseScreen::CreateHeaderCtrl(CLTGUIHeaderCtrl_create cs)
{
	cs.nSelectedColor = g_pLayoutDB->GetHeaderCtrlHighlightColor();
	cs.nBackgroundColor = g_pLayoutDB->GetHeaderCtrlBackgroundColor();
	cs.nSortedColor = g_pLayoutDB->GetHeaderCtrlSortedColor();

	//cs.hHeaderTextureNormal.Load( g_pLayoutDB->GetHeaderCtrlTex(0) );
	//cs.hHeaderTextureHot.Load( g_pLayoutDB->GetHeaderCtrlTex(1) );

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	CLTGUIHeaderCtrl* pCtrl = debug_new(CLTGUIHeaderCtrl);
	if( pCtrl->Create(cs) )
	{
		pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pCtrl->SetInputCaptureHandler( this );
	}
	else
	{
		debug_delete( pCtrl );
		pCtrl = NULL;
	}

	return pCtrl;
}


CLTGUIFillFrame* CBaseScreen::CreateFillFrame( CLTGUIFillFrame_create cs )
{
	cs.nSelectedColor = m_SelectedColor;
	cs.nNonSelectedColor = m_NonSelectedColor;
	cs.nBackgroundColor = g_pLayoutDB->GetListBackgroundColumnColor();

	CLTGUIFillFrame* pCtrl = debug_new(CLTGUIFillFrame);
	if( pCtrl->Create(cs) )
	{
		pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pCtrl->SetFrameWidth( 1 );
	}
	else
	{
		debug_delete( pCtrl );
		pCtrl = NULL;
	}

	return pCtrl;

}

CLTGUITabCtrl* CBaseScreen::CreateTabCtrl(CLTGUITabCtrl_create cs)
{
	cs.nSelectedColor = g_pLayoutDB->GetHeaderCtrlHighlightColor();
	cs.nBackgroundColor = g_pLayoutDB->GetHeaderCtrlBackgroundColor();

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	CLTGUITabCtrl* pCtrl = debug_new(CLTGUITabCtrl);
	if( pCtrl->Create(cs) )
	{
		pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}
	else
	{
		debug_delete( pCtrl );
		pCtrl = NULL;
	}

	return pCtrl;
}



CLTGUICycleCtrl* CBaseScreen::CreateCycle(const char* szStringID, CLTGUICycleCtrl_create cs , bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUICycleCtrl* pCtrl=CreateCycle(LoadString(szStringID), cs, bFixed, szFont, nFontHeight);

	return pCtrl;

}

CLTGUICycleCtrl* CBaseScreen::CreateCycle(const wchar_t *pString, CLTGUICycleCtrl_create cs , bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUICycleCtrl* pCtrl=debug_new(CLTGUICycleCtrl);
	if( !pCtrl )
		return NULL;

	if( !CreateCycle( pCtrl, pString, cs, bFixed, szFont, nFontHeight ))
	{
		debug_delete( pCtrl );
		return NULL;
	}

	return pCtrl;
}

bool CBaseScreen::CreateCycle(CLTGUICycleCtrl* pCtrl, const wchar_t *pString, CLTGUICycleCtrl_create cs , bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	if (!szFont)
	{
		szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	}

	if (!nFontHeight)
		nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();


    if (!pCtrl->Create(pString, CFontInfo(szFont,nFontHeight), cs))
	{
		return false;
	}


	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	

	if (bFixed)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return true;
}





CLTGUIToggle* CBaseScreen::CreateToggle(const char* szStringID, CLTGUIToggle_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{

	CLTGUIToggle* pCtrl=CreateToggle(LoadString(szStringID), cs, bFixed, szFont, nFontHeight);

	return pCtrl;

}

CLTGUIToggle* CBaseScreen::CreateToggle(const wchar_t *pString, CLTGUIToggle_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUIToggle* pCtrl=debug_new(CLTGUIToggle);
	if( !CreateToggle( pCtrl, pString, cs, bFixed, szFont, nFontHeight ))
	{
		debug_delete( pCtrl );
		return NULL;
	}

	return pCtrl;
}

bool CBaseScreen::CreateToggle(CLTGUIToggle* pCtrl, const wchar_t *pString, CLTGUIToggle_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	if (!szFont)
	{
		szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	}

	if (!nFontHeight)
		nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();

    if (!pCtrl->Create(pString, CFontInfo(szFont,nFontHeight), cs))
	{
        return false;
	}


	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());

	pCtrl->SetOnString(LoadString("IDS_ON"));
	pCtrl->SetOffString(LoadString("IDS_OFF"));

	if (bFixed)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return true;
}


CLTGUISlider* CBaseScreen::CreateSlider(const char* szStringID, CLTGUISlider_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUISlider* pCtrl=CreateSlider(LoadString(szStringID), cs, bFixed, szFont, nFontHeight);

	return pCtrl;

}

CLTGUISlider* CBaseScreen::CreateSlider(const wchar_t *pString, CLTGUISlider_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUISlider* pCtrl=debug_new(CLTGUISlider);
	if( !CreateSlider( pCtrl, pString, cs, bFixed, szFont, nFontHeight ))
	{
		debug_delete( pCtrl );
		return NULL;
	}

	return pCtrl;
}

bool CBaseScreen::CreateSlider(CLTGUISlider* pCtrl, const wchar_t *pString, CLTGUISlider_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	if (!szFont)
	{
		szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	}

	if (!nFontHeight)
		nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	cs.hBarTexture.Load(g_pLayoutDB->GetSliderTex(0));
	cs.hBarTextureDisabled.Load(g_pLayoutDB->GetSliderTex(1));

	if (!cs.nBarHeight)
	{
		cs.nBarHeight = nFontHeight / 2;
	}

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();

    if (!pCtrl->Create(pString, CFontInfo(szFont,nFontHeight), cs))
	{
        return false;
	}

	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	

	if (bFixed)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return true;

}

// creates a scroll bar
CLTGUIScrollBar* CBaseScreen::CreateScrollBar( CLTGUIScrollBar_create cs )
{
	cs.nSelectedColor = g_pLayoutDB->GetListSelectedColumnColor();
	cs.nBackgroundColor = g_pLayoutDB->GetListBackgroundColumnColor();

	cs.hBarTextureNormal.Load( g_pLayoutDB->GetScrollBarTex(0) );
	cs.hBarTextureHot.Load( g_pLayoutDB->GetScrollBarTex(1) );

	CLTGUIScrollBar* pCtrl = debug_new(CLTGUIScrollBar);
	if( pCtrl->Create(cs) )
	{
		pCtrl->SetScrollDelay( g_pLayoutDB->GetScrollBarDelay() );
		pCtrl->SetScrollSpeed( g_pLayoutDB->GetScrollBarSpeed() );
		pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pCtrl->SetInputCaptureHandler( this );
	}
	else
	{
		debug_delete( pCtrl );
		return NULL;
	}

	return pCtrl;
}

CLTGUIColumnCtrl* CBaseScreen::CreateColumnCtrl(CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUIColumnCtrl* pCtrl=debug_new(CLTGUIColumnCtrl);

	if( !CreateColumnCtrl( pCtrl, cs, bFixed, szFont, nFontHeight ))
	{
		debug_delete( pCtrl );
		return NULL;
	}

	return pCtrl;
}

bool CBaseScreen::CreateColumnCtrl(CLTGUIColumnCtrl* pCtrl, CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	if (!szFont)
	{
		szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	}

	if (!nFontHeight)
		nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	if (cs.rnBaseRect.Left() == 0 && cs.rnBaseRect.Top() == 0)
	{
		LTVector2n sz = cs.rnBaseRect.m_vMax;
		cs.rnBaseRect.m_vMin = m_DefaultPos;
		cs.rnBaseRect.m_vMax = m_DefaultPos + sz;
	}

	if (!pCtrl->Create(CFontInfo(szFont,nFontHeight), cs))
	{
        return false;
	}

	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	

	if (bFixed)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return true;

}



void CBaseScreen::UseBack(bool bBack,bool bOK,bool bReturn)
{
	if (bBack)
	{
		CreateBack(bOK,bReturn);

		if (GetIndex(&m_pScreenMgr->GetBackCtrl( )) >= m_controlArray.size())
			AddControl(&m_pScreenMgr->GetBackCtrl( ));
	}
	else
	{
		RemoveControl(&m_pScreenMgr->GetBackCtrl( ),false);
	}

	m_bBack = bBack;
}


void CBaseScreen::CreateBack(bool bOK, bool bReturn)
{

    const char* szStr = "";
    const char* szHelp = "";
	if (bOK)
	{
		szStr = "IDS_OK";
		szHelp = "IDS_HELP_OK";
	}
	else
	{
		szStr = "IDS_BACK";
		szHelp = "IDS_HELP_BACK";
	}
	uint32 nFontSize = g_pLayoutDB->GetScreenBackSize();

	m_pScreenMgr->GetBackCtrl( ).SetFont(CFontInfo(g_pLayoutDB->GetScreenBackFont(),nFontSize));
	m_pScreenMgr->GetBackCtrl( ).SetHelpID(szHelp);
	m_pScreenMgr->GetBackCtrl( ).SetString(LoadString(szStr));
	m_pScreenMgr->GetBackCtrl( ).SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		
	if (bReturn)
		m_pScreenMgr->GetBackCtrl( ).SetHelpID("IDS_HELP_RETURN");
	else if (bOK)
		m_pScreenMgr->GetBackCtrl( ).SetHelpID("IDS_HELP_OK");
	else
		m_pScreenMgr->GetBackCtrl( ).SetHelpID("IDS_HELP_BACK");

	m_pScreenMgr->GetBackCtrl( ).SetCommandHandler(this);

}


void CBaseScreen::RemoveControl(CLTGUICtrl* pControl,bool bDelete)
{
	if (!pControl) return;

	ControlArray::iterator iter = m_controlArray.begin();

	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		m_controlArray.erase(iter);
	}

	if (bDelete && pControl != &m_pScreenMgr->GetBackCtrl( ))
	{
		debug_delete(pControl);
	}


}

uint32 CBaseScreen::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}
	case CMD_MAIN:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MAIN);
			break;
		}
	case CMD_CONTINUE:
		{
			if (m_nContinueID != SCREEN_ID_NONE)
			{
				m_pScreenMgr->SetCurrentScreen((eScreenID)m_nContinueID);
				return 1;
			}
			else
				return 0;
		} break;
	case eGUICtrlCmd_SetCapture:
		LTASSERT( (CLTGUICtrl*)dwParam1, "Invalid CLTGUICtrl pointer" );
		if( dwParam1 )
			SetCapture( (CLTGUICtrl*)dwParam1, dwParam2 != 0 );
		break;
	case eGUICtrlCmd_ReleaseCapture:
		SetCapture( NULL );
		break;
	case eGUICtrlCmd_PlaySound:
		g_pInterfaceMgr->RequestInterfaceSound( (InterfaceSound)dwParam1 );
		break;
	case eGUICtrlCmd_SetCursor:
		if( g_pCursorMgr )
		{
			m_hCursorRecord = g_pCursorMgr->GetCursorRecordByName( (const char*)dwParam1 );
		}
		break;
	case eGUICtrlCmd_UpdateHelp:
		UpdateHelpText();
		break;

	default:
		return 0;
	}

	return 1;
}


void CBaseScreen::ForceMouseUpdate()
{
//	SetSelection(kNoSelection);
	m_szCurrHelpID = "";
    LTVector2n cPos = g_pInterfaceMgr->GetCursorPos();
	OnMouseMove(cPos.x,cPos.y);
}


// This is called when the screen gets or loses focus
void CBaseScreen::OnFocus(bool bFocus)
{
    m_pCaptureCtrl = NULL;

	if (bFocus)
	{
		float fTransTime = g_vtTextTransitionTime.GetFloat();
		if (fTransTime > 0.0f)
		{
			m_TransitionTimer.Start(fTransTime);
		}
		if (m_vfLastScale != g_pInterfaceResMgr->GetScreenScale())
		{
			ScreenDimsChanged();
		}
		m_nOldSelection = kNoSelection;
		if (m_nSelection == kNoSelection)
		{
			SetSelection(0,true);
			if (m_nSelection == kNoSelection && m_bBack)
			{
				SetSelection(GetIndex(&m_pScreenMgr->GetBackCtrl( )));
			}
		
		}

		
		ForceMouseUpdate();
		UpdateHelpText();
		CreateInterfaceSFX();
		m_bHaveFocus = true;
		m_bVisited = true; //set this last
	}
	else
	{
		SetSelection(kNoSelection);
		RemoveInterfaceSFX();

		m_TitleString.FlushTexture();
		for (uint32 i = 0; i < m_controlArray.size(); i++ )
		{
			m_controlArray[i]->FlushTextureStrings();
		}

		m_pScreenMgr->GetHelpStr( ).FlushTexture();
		m_bHaveFocus = false;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::CreateInterfaceSFX
//
//	PURPOSE:	Create the SFX to render on this screen
//
// ----------------------------------------------------------------------- //

void CBaseScreen::CreateInterfaceSFX()
{
}

void CBaseScreen::RemoveInterfaceSFX()
{
	g_pInterfaceMgr->RemoveInterfaceFX();
}


bool CBaseScreen::UpdateInterfaceSFX()
{
	return true;
}


uint16 CBaseScreen::GetIndex(CLTGUICtrl* pCtrl)
{
	ControlArray::iterator iter = m_controlArray.begin();

	uint32 dwIndex = 0;
	while (iter != m_controlArray.end() && (*iter) != pCtrl)
	{
		++dwIndex;
		iter++;
	}
	if (dwIndex < m_controlArray.size())
		return ( uint16 )dwIndex;
	return kNoSelection;
}


void CBaseScreen::UpdateHelpText()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
    const char* szID = "";
	if (pCtrl)
		szID = pCtrl->GetHelpID();

	if( !szID || (szID[0] == '\0') )
	{
		m_pScreenMgr->GetHelpStr( ).SetText(L"");
		m_szCurrHelpID = "";
		return;
	}

	if( !LTStrIEquals(m_szCurrHelpID, szID) )
	{
		m_szCurrHelpID = szID;

		if( szID && (szID[0] != '\0') )
		{
			wchar_t wszHelpText[256];
			GetHelpString(m_szCurrHelpID,m_nSelection,wszHelpText,LTARRAYSIZE(wszHelpText));

			if( wszHelpText && (wszHelpText[0] != '\0') )
			{
				m_pScreenMgr->GetHelpStr( ).SetText(wszHelpText);
				uint32 nWidth = (uint32)( (float)m_pScreenMgr->GetHelpWidth( ) * g_pInterfaceResMgr->GetXRatio());
				float helpX = (float)m_pScreenMgr->GetHelpRect( ).Left() * g_pInterfaceResMgr->GetXRatio();
				float helpY = (float)m_pScreenMgr->GetHelpRect( ).Top() * g_pInterfaceResMgr->GetYRatio();

				m_pScreenMgr->GetHelpStr( ).SetPos( LTVector2(helpX,helpY));
				m_pScreenMgr->GetHelpStr( ).WordWrap(nWidth);
			}

		}
	}
}


void CBaseScreen::ScreenDimsChanged()
{
	m_vfLastScale = g_pInterfaceResMgr->GetScreenScale();
	unsigned int i;
	for ( i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->FlushTextureStrings();
		m_controlArray[i]->SetScale(g_pInterfaceResMgr->GetScreenScale());
	}

	m_TitleString.FlushTexture();
	m_TitleString.SetPos(g_pInterfaceResMgr->ConvertScreenPos(m_BaseTitlePos));
	uint32 nFontSize = (uint32)((float)m_TitleSize * g_pInterfaceResMgr->GetYRatio());
	m_TitleString.SetFontHeight(nFontSize);

	uint32 nWidth = (uint32)( (float)m_pScreenMgr->GetHelpWidth( ) * g_pInterfaceResMgr->GetXRatio());
	float helpX = (float)m_pScreenMgr->GetHelpRect( ).Left() * g_pInterfaceResMgr->GetXRatio();
	float helpY = (float)m_pScreenMgr->GetHelpRect( ).Top() * g_pInterfaceResMgr->GetYRatio();
	uint32 nSize = (uint32)((float)m_pScreenMgr->GetHelpSize( ) * g_pInterfaceResMgr->GetYRatio());

	m_pScreenMgr->GetHelpStr( ).FlushTexture();
	m_pScreenMgr->GetHelpStr( ).SetPos( LTVector2(helpX,helpY) );
	m_pScreenMgr->GetHelpStr( ).SetFontHeight(nSize);
	m_pScreenMgr->GetHelpStr( ).WordWrap(nWidth);

	LTVector4 vBackgroundRect = g_pLayoutDB->GetVector4(m_hLayout,"BackgroundTextureRect");

	vBackgroundRect.x *= g_pInterfaceResMgr->GetXRatio();
	vBackgroundRect.y *= g_pInterfaceResMgr->GetYRatio();
	vBackgroundRect.z *= g_pInterfaceResMgr->GetXRatio();
	vBackgroundRect.w *= g_pInterfaceResMgr->GetYRatio();

	if( LTNearlyEquals(vBackgroundRect.z - vBackgroundRect.x, 0.0f, 0.1f) || 
		LTNearlyEquals(vBackgroundRect.w - vBackgroundRect.y, 0.0f, 0.1f) )
	{
		float w = float(g_pInterfaceResMgr->GetScreenWidth());
		float offset = (w - float(g_pInterfaceResMgr->GetScreenHeight())) / 2.0f;
		DrawPrimSetXYWH(m_BackPoly,0.0f,-offset,w,w);
	}
	else
	{
		DrawPrimSetXYWH(m_BackPoly,vBackgroundRect.x,vBackgroundRect.y,vBackgroundRect.z,vBackgroundRect.w);
	}
}



