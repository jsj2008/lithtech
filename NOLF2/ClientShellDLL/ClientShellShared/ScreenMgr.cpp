// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMgr.cpp
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMgr.h"
#include "ClientButeMgr.h"
#include "SoundMgr.h"
#include "TransitionFXMgr.h"

//screens
#include "BaseScreen.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMgr::CScreenMgr()
{
    m_pCurrentScreen = LTNULL;
	m_eCurrentScreenID = (eScreenID)0;
	m_eLastScreenID = (eScreenID)0;
	m_nHistoryLen = 0;
}

CScreenMgr::~CScreenMgr()
{

}


//////////////////////////////////////////////////////////////////////
// Function name	: CScreenMgr::Init
// Description	    :
// Return type      : LTBOOL
//////////////////////////////////////////////////////////////////////

LTBOOL CScreenMgr::Init()
{
	m_pTransitionFXMgr = debug_new(CTransitionFXMgr);
	m_pTransitionFXMgr->Init();
    return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CScreenMgr::Term
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CScreenMgr::Term()
{
	// Term the screens
	for (uint16 i=0; i < m_screenArray.size(); ++i)
	{
		m_screenArray[i]->Term();
		debug_delete(m_screenArray[i]);
	}
	m_screenArray.clear();

	if(m_pTransitionFXMgr)
	{
		m_pTransitionFXMgr->Term();
		debug_delete(m_pTransitionFXMgr);
		m_pTransitionFXMgr = LTNULL;
	}
}



// Renders the screen to a surface
LTBOOL CScreenMgr::Render(HSURFACE hDestSurf)
{
	if (m_pCurrentScreen)
	{
		return m_pCurrentScreen->Render(hDestSurf);
	}
    return LTFALSE;
}

bool CScreenMgr::UpdateInterfaceSFX()
{
	if (m_pCurrentScreen)
	{
		return m_pCurrentScreen->UpdateInterfaceSFX();
	}

	return true;
}

void CScreenMgr::HandleKeyDown (int vkey, int rep)
{
	if (m_pCurrentScreen)
	{
		if (vkey == VK_ESCAPE)
		{
			m_pCurrentScreen->Escape();
		}
		else
			m_pCurrentScreen->HandleKeyDown(vkey,rep);
	}
}

void CScreenMgr::HandleKeyUp (int vkey)
{
	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->HandleKeyUp(vkey);
	}
}

void CScreenMgr::HandleChar (unsigned char c)
{
	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->HandleChar(c);
	}
}


eScreenID CScreenMgr::GetFromHistory( int nHistoryIndex )
{ 
	if( nHistoryIndex < MAX_SCREEN_HISTORY )
		return m_eScreenHistory[m_nHistoryLen - nHistoryIndex - 1];
	return SCREEN_ID_NONE;
}

LTBOOL CScreenMgr::PreviousScreen()
{
	if (m_nHistoryLen < 1) return LTFALSE;

	CBaseScreen *pNewScreen=GetScreenFromID(m_eScreenHistory[m_nHistoryLen-1]);
	if (pNewScreen)
	{
		SwitchToScreen(pNewScreen);

		// The music may change per screen...
        //g_pInterfaceMgr->SetupMusic();

        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CScreenMgr::SetCurrentScreen(eScreenID screenID)
{

	CBaseScreen *pNewScreen=GetScreenFromID(screenID);
	if (pNewScreen)
	{
		SwitchToScreen(pNewScreen);

		// The music may change per screen...
        //g_pInterfaceMgr->SetupMusic();

        return LTTRUE;
	}
    return LTFALSE;
}

void CScreenMgr::EscapeCurrentScreen()
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->Escape();
}

void CScreenMgr::ExitScreens()
{
	// Tell the old screen that it is losing focus
	if (m_pCurrentScreen)
	{
        m_pCurrentScreen->OnFocus(LTFALSE);
	}

	//clear our screen history (no longer relevant)
	ClearHistory();
	
}


void CScreenMgr::SwitchToScreen(CBaseScreen *pNewScreen)
{

	CBaseScreen *pOldScreen = m_pCurrentScreen;
	// Tell the old screen that it is losing focus
	if (m_pCurrentScreen && pNewScreen != m_pCurrentScreen)
	{
        m_pCurrentScreen->OnFocus(LTFALSE);
		int insert = 0;

		//look through the list of screens we've visited, if we find the
		// one we're going to cut the history back to that point.
		// otherwise add it at the end.
		eScreenID nextScreenID = (eScreenID)pNewScreen->GetScreenID();
		eScreenID currentScreenID = (eScreenID)m_pCurrentScreen->GetScreenID();
		while (insert < m_nHistoryLen && m_eScreenHistory[insert] != nextScreenID)
			++insert;
		if (insert == m_nHistoryLen)
		{
			if (m_nHistoryLen < MAX_SCREEN_HISTORY)
			{
				++m_nHistoryLen;
				m_eScreenHistory[insert] = currentScreenID;
			}
		}
		else
		{
			m_nHistoryLen = insert;
		}
	}

	m_pCurrentScreen=pNewScreen;
	m_eLastScreenID = m_eCurrentScreenID;

	m_eCurrentScreenID = (eScreenID)m_pCurrentScreen->GetScreenID();

	// If the new screen hasn't been built yet... better build it!
	if (!m_pCurrentScreen->IsBuilt())
	{

		m_pCurrentScreen->Build();
	}

	// Tell the new screen that it is gaining focus
	if (pNewScreen && pNewScreen != pOldScreen)
	{
        pNewScreen->OnFocus(LTTRUE);
	}
}


// Returns a screen based on an ID
CBaseScreen *CScreenMgr::GetScreenFromID(eScreenID screenID)
{
	CBaseScreen *pScreen=NULL;

	ScreenArray::iterator iter = m_screenArray.begin();
	while (iter != m_screenArray.end() && (*iter)->GetScreenID() != (int)screenID)
		iter++;

	if (iter != m_screenArray.end())
		pScreen = (*iter);

	return pScreen;

}


// Mouse messages
void	CScreenMgr::OnLButtonDown(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonDown(x,y);
}

void	CScreenMgr::OnLButtonUp(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonUp(x,y);
}

void	CScreenMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonDblClick(x,y);
}

void	CScreenMgr::OnRButtonDown(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonDown(x,y);
}

void	CScreenMgr::OnRButtonUp(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonUp(x,y);
}

void	CScreenMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonDblClick(x,y);
}

void	CScreenMgr::OnMouseMove(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnMouseMove(x,y);
}


void CScreenMgr::AddScreen(CBaseScreen* pScreen)
{
	if (pScreen)
	{
		m_screenArray.push_back(pScreen);
	}

}

LTBOOL CScreenMgr::ForceScreenUpdate(eScreenID screenID)
{
    if (!m_pCurrentScreen || m_eCurrentScreenID != screenID) return LTFALSE;

	return m_pCurrentScreen->HandleForceUpdate();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenMgr::ScreenDimsChanged
//
//	PURPOSE:	Handle the screen dims changing
//
// --------------------------------------------------------------------------- //

void CScreenMgr::ScreenDimsChanged()
{
	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->ScreenDimsChanged();
	}

}



void CScreenMgr::ClearHistory()
{
	m_nHistoryLen = 0;
	m_eScreenHistory[0] = (eScreenID)0;
	m_pCurrentScreen = LTNULL;
	m_eCurrentScreenID = (eScreenID)0;

	GetTransitionFXMgr()->ClearScreenHistory();

}

void CScreenMgr::AddScreenToHistory(eScreenID screenID)
{
	int insert = 0;
	while (insert < m_nHistoryLen && m_eScreenHistory[insert] != screenID)
		++insert;
	if (insert == m_nHistoryLen)
	{
		if (m_nHistoryLen < MAX_SCREEN_HISTORY)
		{
			++m_nHistoryLen;
			m_eScreenHistory[insert] = screenID;
		}
	}
	else
	{
		m_nHistoryLen = insert+1;
	}

}
