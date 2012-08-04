// BaseFolder.cpp: implementation of the CBaseFolder class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "BaseFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "SoundMgr.h"
#include "InterfaceMgr.h"
#include "VKDefs.h"
#include "GameClientShell.h"
#include "ClientRes.h"
extern CGameClientShell* g_pGameClientShell;

LTBOOL      CBaseFolder::m_bReadLayout = LTFALSE;
LTRect      CBaseFolder::m_HelpRect;
LTIntPt     CBaseFolder::m_BackPos;
LTIntPt     CBaseFolder::m_ContinuePos;
LTIntPt     CBaseFolder::m_MainPos;
HLTCOLOR    CBaseFolder::m_hShadeColor;
HLTCOLOR    CBaseFolder::m_hBarColor;
int			CBaseFolder::m_nBarHt;
int			CBaseFolder::m_nTopShadeHt;
int			CBaseFolder::m_nBottomShadeHt;
char		CBaseFolder::m_sArrowBack[128];
char		CBaseFolder::m_sArrowNext[128];
LTIntPt		CBaseFolder::m_ArrowBackPos;
LTIntPt     CBaseFolder::m_ArrowNextPos;
HSURFACE	CBaseFolder::m_hHelpSurf = LTNULL;

namespace
{
	LTIntPt offscreen(-64,-64);
    LTVector g_vPos, g_vU, g_vR, g_vF;
    LTRotation g_rRot;

	HSTRING hDemoBuildVersion;
	LTIntPt DemoBuildPos;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseFolder::CBaseFolder()
{
    m_bInit = LTFALSE;
    m_bBuilt = LTFALSE;
	m_bHaveFocus = LTFALSE;

    m_pFolderMgr = LTNULL;

    m_hTitleString = LTNULL;

	m_hTransparentColor = SETRGB_T(255,0,255);
    m_hTitleColor = LTNULL;

	m_titlePos.x = 0;
	m_titlePos.y = 0;
	m_nTitleAlign = LTF_JUSTIFY_LEFT;

	m_nFolderID = FOLDER_ID_NONE;
	m_nContinueID = FOLDER_ID_NONE;

	m_dwCurrHelpID = 0;

	// Array of controls that this folder owns
	m_fixedControlArray.SetSize(0);
	m_skipControlArray.SetSize(0);
	m_controlArray.SetSize(0);

	m_nSelection = kNoSelection;
    m_pCaptureCtrl = LTNULL;
	m_nFirstDrawn = 0;
	m_nLastDrawn = -1;
	m_nRMouseDownItemSel =  kNoSelection;
	m_nRMouseDownItemSel =  kNoSelection;

	m_nItemSpacing = 0;
    m_bScrollWrap = LTTRUE;

    m_sBackground[0] = LTNULL;

	m_hSelectedColor		= kWhite;
	m_hNonSelectedColor		= kBlack;
	m_hDisabledColor		= kGray;
	m_nAlignment			= LTF_JUSTIFY_LEFT;

    m_pUpArrow = LTNULL;
    m_pDownArrow = LTNULL;
    m_pBack = LTNULL;
    m_pContinue = LTNULL;
    m_pMain = LTNULL;

	m_nNumAttachments = 0;

}

CBaseFolder::~CBaseFolder()
{
	if ( m_bInit )
	{
		Term();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Init
//
//	PURPOSE:	initialize the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Init(int nFolderID)
{
	if (!m_bReadLayout)
	{
        m_bReadLayout   = LTTRUE;
		m_HelpRect		= g_pLayoutMgr->GetHelpRect();
		m_BackPos		= g_pLayoutMgr->GetBackPos();
		m_ContinuePos	= g_pLayoutMgr->GetContinuePos();
		m_MainPos		= g_pLayoutMgr->GetMainPos();
		m_hShadeColor	= g_pLayoutMgr->GetShadeColor();
		m_hBarColor		= g_pLayoutMgr->GetBarColor();
		m_nBarHt		= g_pLayoutMgr->GetBarHeight();
		m_nTopShadeHt	= g_pLayoutMgr->GetTopShadeHeight();
		m_nBottomShadeHt= g_pLayoutMgr->GetBottomShadeHeight();
		g_pLayoutMgr->GetArrowBackBmp(m_sArrowBack,128);
		g_pLayoutMgr->GetArrowNextBmp(m_sArrowNext,128);
		m_ArrowBackPos	= g_pLayoutMgr->GetArrowBackPos();
		m_ArrowNextPos	= g_pLayoutMgr->GetArrowNextPos();

#ifdef _DEMO
	    hDemoBuildVersion = g_pLTClient->FormatString(IDS_DEMOVERSION);
		DemoBuildPos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_MAIN,"VersionPos");
#endif

	}
	m_UpArrowPos	= offscreen;
	m_DownArrowPos	= offscreen;

	m_nFolderID=nFolderID;
	m_pFolderMgr = g_pInterfaceMgr->GetFolderMgr();

	SetTitleColor(kWhite);

	//set up layout variables
    LTIntPt tpos = g_pLayoutMgr->GetFolderTitlePos((eFolderID)nFolderID);
	SetTitlePos(tpos.x,tpos.y);

	SetTitleAlignment(g_pLayoutMgr->GetFolderTitleAlign((eFolderID)nFolderID));

	char back[128] = "";
	g_pLayoutMgr->GetFolderBackground((eFolderID)nFolderID,back,sizeof(back));
	SetBackground(back);

	m_PageRect  = g_pLayoutMgr->GetFolderPageRect((eFolderID)nFolderID);

	SetItemSpacing(g_pLayoutMgr->GetFolderItemSpacing((eFolderID)nFolderID));

	m_nAlignment = g_pLayoutMgr->GetFolderItemAlign((eFolderID)nFolderID);

	int nWidth = m_HelpRect.right - m_HelpRect.left;
	int nHeight = m_HelpRect.bottom - m_HelpRect.top;
	if (!m_hHelpSurf)
		m_hHelpSurf = g_pLTClient->CreateSurface((uint32)nWidth,(uint32)nHeight);

	m_bInit=TRUE;
    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Term
//
//	PURPOSE:	Terminate the folder
//
// ----------------------------------------------------------------------- //

void CBaseFolder::Term()
{
	RemoveAll();

	if (!m_bBuilt)
	{
		if (m_pUpArrow)
		{
			debug_delete(m_pUpArrow);
            m_pUpArrow = LTNULL;
		}
		if (m_pDownArrow)
		{
			debug_delete(m_pDownArrow);
            m_pDownArrow = LTNULL;
		}
		if (m_pBack)
		{
			debug_delete(m_pBack);
            m_pBack = LTNULL;
		}
	}

	// Free the title string
	if (m_hTitleString)
	{
        g_pLTClient->FreeString(m_hTitleString);
        m_hTitleString=LTNULL;
	}

	if (m_hHelpSurf)
	{
        g_pLTClient->DeleteSurface (m_hHelpSurf);
        m_hHelpSurf = LTNULL;
	}

	m_bInit=FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Escape
//
//	PURPOSE:	back out of the folder
//
// ----------------------------------------------------------------------- //

void CBaseFolder::Escape()
{
	if (!m_pFolderMgr->PreviousFolder())
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsInWorld() && hPlayerObj && 
			(!g_pGameClientShell->IsPlayerDead() || g_pGameClientShell->GetGameType() != SINGLE))
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
	}
	else
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Render
//
//	PURPOSE:	Renders the folder to a surface
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Render(HSURFACE hDestSurf)
{
	if (!hDestSurf)
	{
        return LTFALSE;
	}


	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

//	HSURFACE hBack = g_pInterfaceResMgr->GetSharedSurface(m_sBackground);
//  g_pLTClient->DrawSurfaceToSurface(hDestSurf, hBack, LTNULL, xo, yo);


    LTRect rect(0,0,g_pInterfaceResMgr->GetScreenWidth(),yo+m_nTopShadeHt);
    g_pLTClient->FillRect(hDestSurf,&rect,m_hShadeColor);

	rect.top = yo+m_nTopShadeHt;
	rect.bottom = rect.top + m_nBarHt;
    g_pLTClient->FillRect(hDestSurf,&rect,m_hBarColor);

	rect.bottom = g_pInterfaceResMgr->GetScreenWidth();
	rect.top = rect.bottom - m_nBottomShadeHt;
    g_pLTClient->FillRect(hDestSurf,&rect,m_hShadeColor);

	rect.bottom = g_pInterfaceResMgr->GetScreenHeight();
	rect.top = (rect.bottom - yo) - m_nBottomShadeHt;
    g_pLTClient->FillRect(hDestSurf,&rect,m_hShadeColor);

	rect.bottom = rect.top;
	rect.top = rect.bottom - m_nBarHt;
    g_pLTClient->FillRect(hDestSurf,&rect,m_hBarColor);

	if (xo > 0)
	{
		rect.left = 0;
		rect.right = xo;
		rect.top = yo+m_nTopShadeHt;
		rect.bottom = (g_pInterfaceResMgr->GetScreenHeight() - yo) - m_nBottomShadeHt;
	    g_pLTClient->FillRect(hDestSurf,&rect,m_hShadeColor);

		rect.right = g_pInterfaceResMgr->GetScreenWidth();
		rect.left = g_pInterfaceResMgr->GetScreenWidth() - xo;
		rect.top = yo+m_nTopShadeHt;
		rect.bottom = (g_pInterfaceResMgr->GetScreenHeight() - yo) - m_nBottomShadeHt;
	    g_pLTClient->FillRect(hDestSurf,&rect,m_hShadeColor);
	
	}


	// Render the title
	CLTGUIFont *pTitleFont=GetTitleFont();

	if (pTitleFont && m_hTitleString)
	{
		int xPos=m_titlePos.x + xo;
		int yPos=m_titlePos.y + yo;

		// Align the text as needed
		switch (m_nTitleAlign)
		{
		case LTF_JUSTIFY_CENTER:
			{
                LTIntPt size=pTitleFont->GetTextExtents(m_hTitleString);
				xPos-=size.x/2;
			} break;
		case LTF_JUSTIFY_RIGHT:
			{
                LTIntPt size=pTitleFont->GetTextExtents(m_hTitleString);
				xPos-=size.x;
			} break;
		}
		pTitleFont->Draw(m_hTitleString, hDestSurf, xPos+2, yPos+2, LTF_JUSTIFY_LEFT,kBlack);
		pTitleFont->Draw(m_hTitleString, hDestSurf, xPos, yPos, LTF_JUSTIFY_LEFT,kWhite);
	}


	if (m_pBack && m_pBack->IsSelected())
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf,g_pInterfaceResMgr->GetSharedSurface(m_sArrowBack), LTNULL, m_ArrowBackPos.x + xo, m_ArrowBackPos.y + yo, m_hTransparentColor);
	}
	if (m_pContinue && m_pContinue->IsSelected())
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf,g_pInterfaceResMgr->GetSharedSurface(m_sArrowNext), LTNULL, m_ArrowNextPos.x + xo, m_ArrowNextPos.y + yo, m_hTransparentColor);
	}

//render list of ctrls
	unsigned int i;
	for ( i = 0; i < m_fixedControlArray.GetSize(); i++ )
	{
        LTIntPt oldPos = m_fixedControlArray[i]->GetPos();
		m_fixedControlArray[i]->SetPos(oldPos.x + xo, oldPos.y + yo);
		m_fixedControlArray[i]->Render ( hDestSurf );
		m_fixedControlArray[i]->SetPos(oldPos);
	}

	int x= GetPageLeft();
	int y= GetPageTop();
    LTIntPt  size;
	for ( i = m_nFirstDrawn; i < m_controlArray.GetSize(); i++ )
	{
		if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
		{
			i++;
			break;
		}
		size.x=m_controlArray[i]->GetWidth();
		size.y=m_controlArray[i]->GetHeight();

		if ( y+size.y <= GetPageBottom() )
		{

			// Set the position for the control
			m_controlArray[i]->SetPos(x+ xo, y+ yo);
			m_controlArray[i]->Render ( hDestSurf );
			m_controlArray[i]->SetPos(x, y);
			y+=size.y+m_nItemSpacing;
		}
		else
		{
			break;
		}
	}

	m_nLastDrawn = i-1;


	if (m_hHelpSurf && m_dwCurrHelpID)
	{
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
        g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hHelpSurf, LTNULL, m_HelpRect.left+xo,m_HelpRect.top+yo);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);
	}

#ifdef _DEMO
	if (hDemoBuildVersion)
	{
		CLTGUIFont* pFont = g_pInterfaceResMgr->GetHelpFont();
		pFont->Draw(hDemoBuildVersion, hDestSurf, DemoBuildPos.x+g_pInterfaceResMgr->GetXOffset(), 
			DemoBuildPos.y+g_pInterfaceResMgr->GetYOffset(), LTF_JUSTIFY_RIGHT, kBlack);
	}
#endif

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::CreateTitle
//
//	PURPOSE:	Creates the string to display as the folders title
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::CreateTitle(HSTRING hString)
{
	if (!hString)
        return LTFALSE;

	// Free the title string if there is one
	if (m_hTitleString)
	{
        g_pLTClient->FreeString(m_hTitleString);
        m_hTitleString=LTNULL;
	}


    m_hTitleString=g_pLTClient->CopyString(hString);


    return LTTRUE;
}

LTBOOL CBaseFolder::CreateTitle(char *lpszTitle)
{
    HSTRING hString=g_pLTClient->CreateString(lpszTitle);

    LTBOOL created = CreateTitle(hString);

    g_pLTClient->FreeString(hString);
	return created;
}

LTBOOL CBaseFolder::CreateTitle(int nStringID)
{
    HSTRING hString=g_pLTClient->FormatString(nStringID);

    LTBOOL created = CreateTitle(hString);

    g_pLTClient->FreeString(hString);
	return created;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::Build
//
//	PURPOSE:	Construct the basic folder elements
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseFolder::Build()
{
	m_UpArrowPos		= g_pLayoutMgr->GetUpArrowPos((eFolderID)m_nFolderID);
	m_DownArrowPos		= g_pLayoutMgr->GetDownArrowPos((eFolderID)m_nFolderID);

    m_bBuilt=LTTRUE;

    UseArrows(LTTRUE);
    UseBack(LTTRUE);
    UseMain(LTFALSE);

	return TRUE;
}

// Handles a user entered character
LTBOOL CBaseFolder::HandleChar(char c)
{
    LTBOOL handled = LTFALSE;

	if (m_pCaptureCtrl)
	{
		if (m_pCaptureCtrl->HandleChar(c))
            handled = LTTRUE;
	}
	return handled;
}

// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CBaseFolder::HandleKeyDown(int key, int rep)
{
    LTBOOL handled = LTFALSE;

	if (m_pCaptureCtrl)
	{
		if (key == VK_RETURN)
			handled = m_pCaptureCtrl->OnEnter();
		else if (m_pCaptureCtrl->HandleKeyDown(key,rep))
            handled = LTTRUE;
		return handled;
	}


	switch (key)
	{
	case VK_LEFT:
		{
			handled = OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			handled = OnRight();
			break;
		}
	case VK_UP:
		{
			handled = OnUp();
			break;
		}
	case VK_DOWN:
		{
			handled = OnDown();
			break;
		}
	case VK_RETURN:
		{
			handled = OnEnter();
			break;
		}
	case VK_PRIOR:
		{
			handled = OnPageUp();
			break;
		}
	case VK_NEXT:
		{
			handled = OnPageDown();
			break;
		}
	default:
		{
            handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}

/******************************************************************/


/******************************************************************/

LTBOOL CBaseFolder::OnUp()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return LTTRUE;

	int sel = m_nSelection;
	return (sel != PreviousSelection());
}

/******************************************************************/

LTBOOL CBaseFolder::OnDown()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return LTTRUE;

	int sel = m_nSelection;
	return (sel != NextSelection());
}

/******************************************************************/

LTBOOL CBaseFolder::OnLeft()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	if (handled)
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	return handled;
}


/******************************************************************/

LTBOOL CBaseFolder::OnRight()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	if (handled)
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	return handled;
}

/******************************************************************/

LTBOOL CBaseFolder::OnTab()
{
    return LTFALSE;
}
/******************************************************************/

LTBOOL CBaseFolder::OnPageUp()
{
	if (!IsFirstPage())
	{
		PreviousPage();
		if (m_nSelection >= 0)
			SetSelection(m_nFirstDrawn,LTTRUE);
        return LTTRUE;
	}
	else
        return LTFALSE;
}

/******************************************************************/

LTBOOL CBaseFolder::OnPageDown()
{
	if (!IsLastPage())
	{
		NextPage();
		if (m_nSelection >= 0)
			SetSelection(m_nFirstDrawn,LTTRUE);
        return LTTRUE;
	}
	else
        return LTFALSE;
}


/******************************************************************/

LTBOOL CBaseFolder::OnEnter()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
	{
		handled = pCtrl->OnEnter();
		if (handled)
		{
			if (pCtrl == m_pContinue || pCtrl == m_pBack || pCtrl == m_pMain)
				g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
			else
				g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		}
	}
	return handled;
}


CLTGUIFont* CBaseFolder::GetTitleFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetTitleFont();
}

CLTGUIFont* CBaseFolder::GetSmallFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetSmallFont();
}

CLTGUIFont* CBaseFolder::GetMediumFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetMediumFont();
}


CLTGUIFont* CBaseFolder::GetHelpFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetHelpFont();
}

CLTGUIFont* CBaseFolder::GetLargeFont()
{
    if (!g_pInterfaceResMgr) return LTNULL;
	return g_pInterfaceResMgr->GetLargeFont();
}

CLTGUIFont* CBaseFolder::GetDefaultFont()
{
	CLTGUIFont *pFont = GetLargeFont();

	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID, "FontSize"))
	{
		if (g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "FontSize") == 0)
			pFont = GetSmallFont();
		else if (g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "FontSize") == 1)
			pFont = GetMediumFont();
	}

	return pFont;
}

int CBaseFolder::NextSelection()
{
	int select = m_nSelection;
	if (select == kNoSelection)
		select = 0;
	int oldSelect = select;
	LTBOOL bFixed = (m_nSelection < 0);
	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		if (bFixed)
		{
			select--;
			int fixSel = FixedIndex(select);
			if (fixSel >= (int)m_fixedControlArray.GetSize())
			{
				if (m_controlArray.GetSize())
				{
					select = 0;
					bFixed = LTFALSE;
				}
				else
					select = -1;
			}
	
		}
		else
		{
			select++;
			if (select >= (int)m_controlArray.GetSize())
			{
				if (m_fixedControlArray.GetSize())
				{
					select = -1;
					bFixed = LTTRUE;
				}
				else
					select = 0;
			}
	
		}
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && (!pCtrl->IsEnabled() || SkipControl(pCtrl) ));


	if (!pCtrl || !pCtrl->IsEnabled() || SkipControl(pCtrl))
		select = m_nSelection;

	return SetSelection(select);

}

int CBaseFolder::PreviousSelection()
{
	int select = m_nSelection;
	if (select == kNoSelection)
		select = 0;
	int oldSelect = select;
	LTBOOL bFixed = (m_nSelection < 0);
	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		if (bFixed)
		{
			select++;
			if (select == 0)
			{
				if (m_controlArray.GetSize())
				{
					select = (int)m_controlArray.GetSize()-1;
					bFixed = LTFALSE;
				}
				else
					select = -1 * (int)m_fixedControlArray.GetSize();
			}
	
		}
		else
		{
			select--;
			if (select < 0)
			{
				if (m_fixedControlArray.GetSize())
				{
					select = -1 * (int)m_fixedControlArray.GetSize();
					bFixed = LTTRUE;
				}
				else
					select = (int)m_controlArray.GetSize()-1;

			}
	
		}
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && (!pCtrl->IsEnabled() || SkipControl(pCtrl) ));


	if (!pCtrl || !pCtrl->IsEnabled() || SkipControl(pCtrl))
		select = m_nSelection;

	return SetSelection(select);

}

/******************************************************************/
LTBOOL CBaseFolder::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLMouseDownItemSel=nControlIndex;
		return pCtrl->OnLButtonDown(x,y);

	}
	else
		m_nLMouseDownItemSel=kNoSelection;

    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;
		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLMouseDownItemSel)
		{
			if (pCtrl->IsEnabled() )
			{
				SetSelection(nControlIndex);
				LTBOOL bHandled = pCtrl->OnLButtonUp(x,y);
				if (bHandled)
				{
					if (pCtrl == m_pContinue || pCtrl == m_pBack || pCtrl == m_pMain)
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
    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRMouseDownItemSel=nControlIndex;

		return pCtrl->OnRButtonDown(x,y);
	}
	else
		m_nRMouseDownItemSel=kNoSelection;

    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "left" message to the control.
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			if (pCtrl->IsEnabled())
			{
				SetSelection(nControlIndex);
				LTBOOL bHandled = pCtrl->OnRButtonUp(x,y);
				if (pCtrl == m_pContinue || pCtrl == m_pBack || pCtrl == m_pMain)
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
    return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnLButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return LTFALSE;

	if (pCtrl)
		return pCtrl->OnLButtonDblClick(x, y);
	else
        return LTFALSE;
}

/******************************************************************/
LTBOOL CBaseFolder::OnRButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return LTFALSE;

	if (pCtrl)
		return pCtrl->OnRButtonDblClick(x, y);
	else
        return LTFALSE;
}

HSTRING CBaseFolder::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
    return g_pLTClient->FormatString(dwHelpId);
}

/******************************************************************/
LTBOOL CBaseFolder::OnMouseMove(int x, int y)
{
	int nControlUnderPoint=kNoSelection;
    LTBOOL onCtrl = GetControlUnderPoint(x,y,&nControlUnderPoint);
	if (onCtrl)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlUnderPoint);
		if (m_pCaptureCtrl && m_pCaptureCtrl != pCtrl)
            return LTFALSE;

		pCtrl->OnMouseMove(x,y);
	}
	else if (m_pCaptureCtrl)
        return LTFALSE;

	if (onCtrl)
	{
		if (GetSelection() != nControlUnderPoint)
		{

			if (GetControl(nControlUnderPoint)->IsEnabled())
			{
				SetSelection(nControlUnderPoint);
			}
			else
			{
//				SetSelection(kNoSelection);
			}
		}
        return LTTRUE;
	}
//	SetSelection(kNoSelection);
    return LTFALSE;
}

int CBaseFolder::SetSelection(int select, LTBOOL bFindSelectable)
{
	if (select == m_nSelection) return select;

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
			GetControl(nOldSelect)->Select(FALSE);
		m_nSelection = kNoSelection;
		UpdateHelpText();
		return kNoSelection;
	}



	int fixedSel = FixedIndex(select);
	CLTGUICtrl *pSelCtrl;


	if (select >= 0)
	{
		if (select >= (int)m_controlArray.GetSize())
			select = m_controlArray.GetSize()-1;
	}
	else
	{
		if (fixedSel >= (int)m_fixedControlArray.GetSize())
			select = FixedIndex((int)m_fixedControlArray.GetSize()-1);
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
		//if we don't know, find the last item on screen
		if (m_nLastDrawn < m_nFirstDrawn)
			CalculateLastDrawn();

		//keep looking until we run out of on screen items or find a selectable one
		while (pSelCtrl && !pSelCtrl->IsEnabled() && select < m_nLastDrawn)
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
		GetControl(nOldSelect)->Select(LTFALSE);


	m_nSelection=select;

	if (m_nSelection == kNoSelection)
	{
		UpdateHelpText();
		return nOldSelect;
	}

	pSelCtrl->Select(TRUE);


	if (m_nSelection != nOldSelect)
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);


	if (select >= 0)
	{
		// Figure out if we should change the page
		while ( m_nSelection < m_nFirstDrawn )
		{
            PreviousPage(LTFALSE);
		}
		if (m_nLastDrawn < m_nFirstDrawn)
			CalculateLastDrawn();

		while (m_nLastDrawn < m_nSelection )
		{
            NextPage(LTFALSE);
			if (m_nLastDrawn < 0)
			{
				//no items are drawn
				_ASSERT(0);
				break;
			}
		}
	}

	CheckArrows();

	UpdateHelpText();
	return m_nSelection;
}

void CBaseFolder::CalculateLastDrawn()
{
    LTIntPt  size;
	int x= GetPageLeft();
	int y= GetPageTop();
	unsigned int i;
	for ( i = m_nFirstDrawn; i < m_controlArray.GetSize(); i++ )
	{
		if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
		{
			i++;
			break;
		}

		size.x=m_controlArray[i]->GetWidth();
		size.y=m_controlArray[i]->GetHeight();

		if ( y+size.y <= GetPageBottom() )
		{

			// Set the position for the control
			m_controlArray[i]->SetPos(x, y);
			y+=size.y+m_nItemSpacing;
		}
		else
		{
			break;
		}
	}
	m_nLastDrawn = i-1;



}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
LTBOOL CBaseFolder::GetControlUnderPoint(int xPos, int yPos, int *pnIndex)
{
	_ASSERT(pnIndex);


	// See if the user clicked on any of the controls.
	int i;
	for (i=m_nFirstDrawn; i <= m_nLastDrawn; i++)
	{
		int nLeft=m_controlArray[i]->GetPos().x;
		int nTop=m_controlArray[i]->GetPos().y;
		int nRight=nLeft+m_controlArray[i]->GetWidth();
		int nBottom=nTop+m_controlArray[i]->GetHeight();

		// Check to see if the click is in the bounding box for the control
		if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
		{
			*pnIndex=i;

            return LTTRUE;
		}
	}
	for (i=0; i < (int)m_fixedControlArray.GetSize(); i++)
	{
		int nLeft=m_fixedControlArray[i]->GetPos().x;
		int nTop=m_fixedControlArray[i]->GetPos().y;
		int nRight=nLeft+m_fixedControlArray[i]->GetWidth();
		int nBottom=nTop+m_fixedControlArray[i]->GetHeight();

		// Check to see if the click is in the bounding box for the control
		if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
		{
			*pnIndex=-1-i;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

// Return a control at a specific index
CLTGUICtrl *CBaseFolder::GetControl ( int nIndex )
{
	if (nIndex >= 0 && nIndex < (int)m_controlArray.GetSize() )
		return m_controlArray[nIndex];
	if (nIndex < 0)
	{
		int nFix = FixedIndex(nIndex);
		if (nFix < (int)m_fixedControlArray.GetSize() )
			return m_fixedControlArray[nFix];
	}
    return LTNULL;
}




void CBaseFolder::SetBackground(char *lpszBitmap)
{
	strncpy(m_sBackground,lpszBitmap,sizeof(m_sBackground));
}

void CBaseFolder::RemoveAll()
{
	// Terminate the ctrls
	RemoveFree();
	RemoveFixed();
}
void CBaseFolder::RemoveFree()
{
	// Terminate the ctrls
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->Destroy();
		debug_delete(m_controlArray[i]);
	}
	m_controlArray.SetSize(0);
	if (m_nSelection >= 0)
		m_nSelection = kNoSelection;
}
void CBaseFolder::RemoveFixed()
{
	// Terminate the ctrls
	unsigned int i;
	for (i=0; i < m_fixedControlArray.GetSize(); i++)
	{
		m_fixedControlArray[i]->Destroy();
		debug_delete(m_fixedControlArray[i]);
	}
	m_skipControlArray.RemoveAll();
	m_fixedControlArray.SetSize(0);
	if (m_nSelection < 0)
		m_nSelection = kNoSelection;
}

LTBOOL CBaseFolder::NextPage(LTBOOL bChangeSelection)
{

	int oldFirst = m_nFirstDrawn;
	if (!IsLastPage())
	{
		m_nFirstDrawn = m_nLastDrawn+1;
		CalculateLastDrawn();
		g_pInterfaceMgr->RequestInterfaceSound(IS_DOWN);

		CheckArrows();

		if (bChangeSelection)
		{
            LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
			OnMouseMove(cPos.x,cPos.y);
		}
	}


	return (oldFirst != m_nFirstDrawn);
}

LTBOOL CBaseFolder::PreviousPage(LTBOOL bChangeSelection)
{
	int oldFirst = m_nFirstDrawn;
	if (!IsFirstPage())
	{
		int height = GetPageBottom() - GetPageTop();
		unsigned int i = m_nFirstDrawn-1;
		if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
		{
			i--;
		}

		while (i > 0)
		{
			if (m_controlArray[i]->GetID() == FOLDER_CMD_BREAK)
			{
				i++;
				break;
			}
			int y=m_controlArray[i]->GetHeight();

			if ( height > y )
			{
				i--;
				height -= y;
			}
			else
			{
				break;
			}
		}

		m_nFirstDrawn = i;
		CalculateLastDrawn();
		g_pInterfaceMgr->RequestInterfaceSound(IS_UP);

		CheckArrows();

		if (bChangeSelection)
		{
            LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
			OnMouseMove(cPos.x,cPos.y);

		}
	}
	return (oldFirst != m_nFirstDrawn);

}

int CBaseFolder::AddFixedControl(CLTGUICtrl* pCtrl, LTIntPt pos, LTBOOL bSelectable)
{
	if (!pCtrl) return 0;

	// Make sure the control wasn't already added...

	for (int i = (int)m_fixedControlArray.GetSize()-1; i >=0; i--)
	{
		if (m_fixedControlArray[i] == pCtrl)
		{
			return 0;
		}
	}

	pCtrl->SetPos(pos);
	m_fixedControlArray.Add(pCtrl);
	if (!bSelectable)
		m_skipControlArray.Add(pCtrl);
	int num = m_fixedControlArray.GetSize();
	return FixedIndex(num-1);

}

int	CBaseFolder::AddFreeControl(CLTGUICtrl* pCtrl)
{
	m_controlArray.Add(pCtrl);
	int num = m_controlArray.GetSize();
	if (num == m_nSelection+1)
        pCtrl->Select(LTTRUE);
	return num-1;

}


// Calls UpdateData on each control in the menu
void CBaseFolder::UpdateData(LTBOOL bSaveAndValidate)
{
	unsigned int i;
	for (i=0; i < m_fixedControlArray.GetSize(); i++)
	{
		m_fixedControlArray[i]->UpdateData(bSaveAndValidate);
	}
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}
}


CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(HSTRING hString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
	CLTGUITextItemCtrl* pCtrl=debug_new(CLTGUITextItemCtrl);

    if (pFont == LTNULL)
		pFont = GetDefaultFont();

    if (!pCtrl->Create(g_pLTClient, commandID, hString, pFont, this, pnValue))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}
	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(int stringID, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl=CreateTextItem(hStr, commandID, helpID, bFixed, pFont, pnValue);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CLTGUITextItemCtrl* CBaseFolder::CreateTextItem(char *pString, uint32 commandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, int *pnValue)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CLTGUITextItemCtrl* pCtrl=CreateTextItem(hStr, commandID, helpID, bFixed, pFont, pnValue);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(HSTRING hString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
	CStaticTextCtrl* pCtrl=debug_new(CStaticTextCtrl);

    if (pFont == LTNULL)
		pFont = GetDefaultFont();

    if (!pCtrl->Create(g_pLTClient,commandID,hString,pFont,this,width,height))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}


	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(SETRGB_T(0,255,0));

	return pCtrl;
}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(int stringID, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CStaticTextCtrl* pCtrl=CreateStaticTextItem(hStr, commandID, helpID, width, height, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

CStaticTextCtrl* CBaseFolder::CreateStaticTextItem(char *pString, uint32 commandID, int helpID, int width, int height, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CStaticTextCtrl* pCtrl=CreateStaticTextItem(hStr, commandID, helpID, width, height, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}



CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(HSTRING hDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{

	CLTGUIEditCtrl* pCtrl=debug_new(CLTGUIEditCtrl);

    if (pFont == LTNULL)
		pFont = GetDefaultFont();

    if (!pCtrl->Create(g_pLTClient, commandID, hDescription, pFont, nTextOffset, nBufferSize, this, pBuffer))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}
	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(int nDescriptionID, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(nDescriptionID);
	CLTGUIEditCtrl* pCtrl=CreateEditCtrl(hStr, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}
CLTGUIEditCtrl* CBaseFolder::CreateEditCtrl(char *pszDescription, uint32 commandID, int helpID, char *pBuffer, int nBufferSize,
									int	nTextOffset, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pszDescription);
	CLTGUIEditCtrl* pCtrl=CreateEditCtrl(hStr, commandID, helpID, pBuffer, nBufferSize, nTextOffset, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;

}

// Adds a Cycle control
CCycleCtrl *CBaseFolder::CreateCycleItem(HSTRING hText, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
	// Create the new menu option
	CCycleCtrl *pCtrl=debug_new(CCycleCtrl);

    if (pFont == LTNULL)
		pFont = GetDefaultFont();

    if ( !pCtrl->Create(g_pLTClient, hText, pFont, nHeaderWidth, nSpacerWidth ,pnValue, m_nAlignment ) )
	{
		debug_delete(pCtrl);
        return LTNULL;
	}
	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;

}

CCycleCtrl *CBaseFolder::CreateCycleItem(int stringID, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CCycleCtrl* pCtrl=CreateCycleItem(hStr, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CCycleCtrl *CBaseFolder::CreateCycleItem(char *pString, int helpID, int nHeaderWidth, int nSpacerWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CCycleCtrl* pCtrl=CreateCycleItem(hStr, helpID, nHeaderWidth, nSpacerWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}



// Adds an on/off control
CToggleCtrl *CBaseFolder::CreateToggle(HSTRING hText, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    if (pFont == LTNULL)
		pFont = GetDefaultFont();

	// Create the new menu control
	CToggleCtrl *pCtrl=debug_new(CToggleCtrl);
    if ( !pCtrl->Create(g_pLTClient, hText, pFont, nRightColumnOffset, pbValue, m_nAlignment ) )
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CToggleCtrl *CBaseFolder::CreateToggle(int stringID, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CToggleCtrl* pCtrl=CreateToggle(hStr, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CToggleCtrl *CBaseFolder::CreateToggle(char *pString, int helpID, int nRightColumnOffset, LTBOOL *pbValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CToggleCtrl* pCtrl=CreateToggle(hStr, helpID, nRightColumnOffset, pbValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}


CSliderCtrl* CBaseFolder::CreateSlider(HSTRING hText, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    if (pFont == LTNULL)
		pFont = GetDefaultFont();

	CSliderCtrl *pCtrl=debug_new(CSliderCtrl);
    if ( !pCtrl->Create(hText, pFont, nSliderOffset, nSliderWidth, LTFALSE, pnValue) )
	{
		debug_delete(pCtrl);
        return LTNULL;
	}
	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CSliderCtrl* CBaseFolder::CreateSlider(int stringID, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->FormatString(stringID);
	CSliderCtrl* pCtrl=CreateSlider(hStr, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CSliderCtrl* CBaseFolder::CreateSlider(char *pString, int helpID, int nSliderOffset, int nSliderWidth, int *pnValue, LTBOOL bFixed, CLTGUIFont *pFont)
{
    HSTRING hStr=g_pLTClient->CreateString(pString);
	CSliderCtrl* pCtrl=CreateSlider(hStr, helpID, nSliderOffset, nSliderWidth, pnValue, bFixed, pFont);
    g_pLTClient->FreeString(hStr);

	return pCtrl;
}

CLTGUIColumnTextCtrl*	CBaseFolder::CreateColumnText(DWORD dwCommandID, int helpID, LTBOOL bFixed, CLTGUIFont *pFont, DWORD dwParam1, DWORD dwParam2)
{
    if (pFont == LTNULL)
		pFont = GetDefaultFont();

	CLTGUIColumnTextCtrl *pCtrl=debug_new(CLTGUIColumnTextCtrl);
    if ( !pCtrl->Create(g_pLTClient, dwCommandID, pFont, this, dwParam1, dwParam2) )
	{
		debug_delete(pCtrl);

        return LTNULL;
	}
	if (bFixed)
		pCtrl->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
	else
		pCtrl->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);
	pCtrl->SetHelpID(helpID);
	pCtrl->SetTransparentColor(m_hTransparentColor);

	return pCtrl;
}

CGroupCtrl*	CBaseFolder::CreateGroup(int nWidth, int nHeight, int helpID)
{
	// Create the new menu option
	CGroupCtrl *pCtrl=debug_new(CGroupCtrl);
	if ( !pCtrl->Create(nWidth,nHeight))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	pCtrl->SetHelpID(helpID);

	return pCtrl;
}

void CBaseFolder::AddPageBreak()
{
	CPageBreakCtrl* pCtrl = debug_new(CPageBreakCtrl);
	pCtrl->Create(FOLDER_CMD_BREAK);
    pCtrl->Enable(LTFALSE);
	AddFreeControl(pCtrl);
}

void CBaseFolder::AddBlankLine()
{
    CStaticTextCtrl* pCtrl = AddStaticTextItem(" ",LTNULL,LTNULL, 0,0, LTTRUE, GetSmallFont());
    pCtrl->Enable(LTFALSE);
}

void CBaseFolder::UseArrows(LTBOOL bArrows, LTBOOL bLeft, LTBOOL bRight)
{
	if (bArrows)
	{
		if (bLeft)
		{
			CreateUpArrow();
		}
		else
		{
			RemoveFixedControl(m_pUpArrow);
			debug_delete(m_pUpArrow);
            m_pUpArrow = LTNULL;
		}

		if (bRight)
		{
			CreateDownArrow();
		}
		else
		{
			RemoveFixedControl(m_pDownArrow);
			debug_delete(m_pDownArrow);
            m_pDownArrow = LTNULL;
		}

		if (IsBuilt())
		{
            AddFixedControl(m_pUpArrow,m_UpArrowPos,LTFALSE);
            AddFixedControl(m_pDownArrow,m_DownArrowPos,LTFALSE);
		}
	}
	else
	{
		if (m_pUpArrow)
		{
			RemoveFixedControl(m_pUpArrow);
			debug_delete(m_pUpArrow);
            m_pUpArrow = LTNULL;
		}
		if (m_pDownArrow)
		{
			RemoveFixedControl(m_pDownArrow);
			debug_delete(m_pDownArrow);
            m_pDownArrow = LTNULL;
		}
	}
}
void CBaseFolder::UseBack(LTBOOL bBack,LTBOOL bOK,LTBOOL bReturn)
{
	if (bBack)
	{
		CreateBack(bOK,bReturn);

		if (IsBuilt())
		{
            AddFixedControl(m_pBack,m_BackPos,LTTRUE);
		}
	}
	else
	{
		if (m_pBack)
		{
			RemoveFixedControl(m_pBack);
			debug_delete(m_pBack);
            m_pBack = LTNULL;
		}
	}
}
void CBaseFolder::UseMain(LTBOOL bMain)
{
	if (bMain)
	{
		CreateMain();

		if (IsBuilt())
		{
            AddFixedControl(m_pMain,m_MainPos,LTTRUE);
		}
	}
	else
	{
		if (m_pMain)
		{
			RemoveFixedControl(m_pMain);
			debug_delete(m_pMain);
            m_pMain = LTNULL;
		}
	}
}
void CBaseFolder::UseContinue(int nContinueID, int nHelpID, int nStringID)
{
	if (nContinueID != FOLDER_ID_NONE)
	{
		CreateContinue(nStringID,nHelpID);
		m_nContinueID = nContinueID;

		if (IsBuilt())
		{
			LTBOOL bBack = (m_pBack != LTNULL);
			if (bBack) UseBack(LTFALSE);
            AddFixedControl(m_pContinue,m_ContinuePos,LTTRUE);
			if (bBack) UseBack(LTTRUE);
		}
	}
	else
	{
		if (m_pContinue)
		{
			RemoveFixedControl(m_pContinue);
			debug_delete(m_pContinue);
            m_pContinue = LTNULL;
		}
	}
}


void CBaseFolder::CreateUpArrow()
{
	if (!m_pUpArrow)
	{
		m_pUpArrow = debug_new(CBitmapCtrl);
        m_pUpArrow->Create(g_pLTClient,"interface\\ArrowUp.pcx","interface\\ArrowUpH.pcx","interface\\ArrowUpD.pcx", this, FOLDER_CMD_LEFT_ARROW);
		m_pUpArrow->SetHelpID(IDS_HELP_LEFT);
	}
}

void CBaseFolder::CreateDownArrow()
{
	if (!m_pDownArrow)
	{
		m_pDownArrow = debug_new(CBitmapCtrl);
        m_pDownArrow->Create(g_pLTClient,"interface\\ArrowDn.pcx","interface\\ArrowDnH.pcx","interface\\ArrowDnD.pcx", this, FOLDER_CMD_RIGHT_ARROW);
		m_pDownArrow->SetHelpID(IDS_HELP_RIGHT);
	}
}

void CBaseFolder::CreateBack(LTBOOL bOK, LTBOOL bReturn)
{
	if (m_pBack)
	{
        HSTRING hStr = LTNULL;
		if (bOK)
			hStr = g_pLTClient->FormatString(IDS_OK);
		else
			hStr = g_pLTClient->FormatString(IDS_BACK);
		m_pBack->RemoveAll();
		m_pBack->AddString(hStr);
		if (bReturn)
			m_pBack->SetHelpID(IDS_HELP_RETURN);
		else if (bOK)
			m_pBack->SetHelpID(IDS_HELP_OK);
		else
			m_pBack->SetHelpID(IDS_HELP_BACK);
        g_pLTClient->FreeString(hStr);
	}
	else
	{
		CLTGUIFont *pFont = GetMediumFont();
        HSTRING hStr = LTNULL;
		if (bOK)
			hStr = g_pLTClient->FormatString(IDS_OK);
		else
			hStr = g_pLTClient->FormatString(IDS_BACK);
		m_pBack = debug_new(CLTGUITextItemCtrl);
        m_pBack->Create(g_pLTClient, FOLDER_CMD_BACK, hStr, pFont, this);

		m_pBack->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);

		if (bOK)
			m_pBack->SetHelpID(IDS_HELP_OK);
		else
			m_pBack->SetHelpID(IDS_HELP_BACK);
        g_pLTClient->FreeString(hStr);
	}
}

void CBaseFolder::CreateContinue(int nStringID, int nHelpID)
{
	if (!nStringID)
		nStringID = IDS_CONTINUE;
    HSTRING hStr=g_pLTClient->FormatString(nStringID);

	if (!nHelpID)
		nHelpID = IDS_HELP_CONTINUE;
	if (m_pContinue)
	{
		m_pContinue->RemoveAll();
		m_pContinue->AddString(hStr);
	}
	else
	{
		CLTGUIFont *pFont = GetMediumFont();
		m_pContinue = debug_new(CLTGUITextItemCtrl);
        m_pContinue->Create(g_pLTClient, FOLDER_CMD_CONTINUE, hStr, pFont, this);

		m_pContinue->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);

        g_pLTClient->FreeString(hStr);
	}
	if (!nHelpID)
		nHelpID = IDS_HELP_CONTINUE;
	m_pContinue->SetHelpID(nHelpID);
}

void CBaseFolder::CreateMain()
{
	if (!m_pMain)
	{
		CLTGUIFont *pFont = GetMediumFont();
        HSTRING hStr=g_pLTClient->FormatString(IDS_MAIN);
		m_pMain = debug_new(CLTGUITextItemCtrl);
        m_pMain->Create(g_pLTClient, FOLDER_CMD_MAIN, hStr, pFont, this);

		m_pMain->SetColor(m_hSelectedColor,m_hNonSelectedColor,m_hDisabledColor);

		m_pMain->SetHelpID(IDS_HELP_MAIN);
        g_pLTClient->FreeString(hStr);
	}
}


void CBaseFolder::RemoveFixedControl(CLTGUICtrl* pControl)
{
	if (!IsBuilt() || !pControl) return;

	uint32 findIndex = m_fixedControlArray.FindElement(pControl);
	if (findIndex < m_fixedControlArray.GetSize())
	{
		m_fixedControlArray.Remove(findIndex);
	}
	findIndex = m_skipControlArray.FindElement(pControl);
	if (findIndex < m_skipControlArray.GetSize())
	{
		m_skipControlArray.Remove(findIndex);
	}


}

uint32 CBaseFolder::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_BACK:
		{
			m_pFolderMgr->EscapeCurrentFolder();
			break;
		}
	case FOLDER_CMD_MAIN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MAIN);
			break;
		}
	case FOLDER_CMD_CONTINUE:
		{
			if (m_nContinueID != FOLDER_ID_NONE)
			{
				m_pFolderMgr->SetCurrentFolder((eFolderID)m_nContinueID);
				return 1;
			}
			else
				return 0;
		} break;

	case FOLDER_CMD_LEFT_ARROW:
		{
			PreviousPage();
			if (m_nSelection >= 0)
				SetSelection(m_nFirstDrawn,LTTRUE);

			break;
		}
	case FOLDER_CMD_RIGHT_ARROW:
		{
			NextPage();
			if (m_nSelection >= 0)
				SetSelection(m_nFirstDrawn,LTTRUE);
			break;
		}
	default:
		return 0;
	}

	return 1;
}


void CBaseFolder::ForceMouseUpdate()
{
//	SetSelection(kNoSelection);
	m_dwCurrHelpID = 0;
    LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
	OnMouseMove(cPos.x,cPos.y);
}

// This is called when the folder gets or loses focus
void CBaseFolder::OnFocus(LTBOOL bFocus)
{
    m_pCaptureCtrl = LTNULL;
	if (bFocus)
	{
		if (m_nSelection == kNoSelection)
		{
			if (m_pContinue)
				SetSelection(GetIndex(m_pContinue));
			else
			{
				SetSelection(0,LTTRUE);
				if (m_nSelection == kNoSelection && m_pBack)
				{
					SetSelection(GetIndex(m_pBack));
				}
		
			}
		}

		
		if (m_nLastDrawn < m_nFirstDrawn)
			CalculateLastDrawn();
		CheckArrows();
		ForceMouseUpdate();
		UpdateHelpText();
		CreateInterfaceSFX();
		m_bHaveFocus = LTTRUE;
	}
	else
	{
		SetSelection(kNoSelection);
		m_nLastDrawn = -1;
		m_nFirstDrawn = 0;
		RemoveInterfaceSFX();
		m_bHaveFocus = LTFALSE;

	}
}

void CBaseFolder::CheckArrows()
{
	if (m_pUpArrow)
	{
		m_pUpArrow->Enable(!IsFirstPage());
		if (m_pUpArrow->IsEnabled())
		{
			m_pUpArrow->SetHelpID(IDS_HELP_LEFT);
			m_pUpArrow->SetPos(m_UpArrowPos);
		}
		else
		{
			m_pUpArrow->SetHelpID(0);
            m_pUpArrow->SetPos(offscreen);
		}
	}

	if (m_pDownArrow)
	{
		m_pDownArrow->Enable(!IsLastPage());
		if (m_pDownArrow->IsEnabled())
		{
			m_pDownArrow->SetHelpID(IDS_HELP_RIGHT);
			m_pDownArrow->SetPos(m_DownArrowPos);
		}
		else
		{
			m_pDownArrow->SetHelpID(0);
            m_pDownArrow->SetPos(offscreen);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseFolder::CreateInterfaceSFX
//
//	PURPOSE:	Create the SFX to render on this screen
//
// ----------------------------------------------------------------------- //

void CBaseFolder::CreateInterfaceSFX()
{

	int n = 0;
	char szAttName[30];
	char szFXName[128];


	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


    g_pLTClient->GetObjectPos(hCamera, &g_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &g_rRot);
    g_pLTClient->GetRotationVectors(&g_rRot, &g_vU, &g_vR, &g_vF);


	sprintf(szAttName,"ScaleName%d",n);
	while (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID,szAttName))
	{
		g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"ScaleName%d",n);

	}

	if (m_pBack)
	{
		g_pLayoutMgr->GetArrowBackSFX(szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}
	}
	if (m_pContinue)
	{
		g_pLayoutMgr->GetArrowNextSFX(szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}
	}

	INT_CHAR *pChar = g_pLayoutMgr->GetFolderCharacter((eFolderID)m_nFolderID);
	if (pChar)
	{
		CreateCharFX(pChar);
		if (m_CharSFX.GetObject())
		{
			int reqID[MAX_INT_ATTACHMENTS];
			int numReq = g_pAttachButeMgr->GetRequirementIDs(pChar->szModel,pChar->szStyle,reqID,MAX_INT_ATTACHMENTS);
			int i;
			for (i = 0; i < numReq; i++)
			{
				INT_ATTACH acs;
				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
				CString socket = g_pAttachButeMgr->GetRequirementSocket(reqID[i]);
				acs.pszSocket = (char *)(LPCTSTR)socket;

				CreateAttachFX(&acs);
			}

			int numAtt = g_pLayoutMgr->GetFolderNumAttachments((eFolderID)m_nFolderID);
			for (i = 0; i < numAtt; i++)
			{
				char szTemp[128];
				char *pName = LTNULL;
				char *pSocket = LTNULL;
				g_pLayoutMgr->GetFolderAttachment( (eFolderID)m_nFolderID, i, szTemp, 128);

				pName = strtok(szTemp,";");
				pSocket = strtok(NULL,";");

				INT_ATTACH acs;

				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(pName);
				acs.pszSocket = pSocket;

				CreateAttachFX(&acs);
			}

		}
	}

	


}

void CBaseFolder::RemoveInterfaceSFX()
{
	while (m_SFXArray.GetSize() > 0)
	{
		CSpecialFX *pSFX = m_SFXArray[0];
		g_pInterfaceMgr->RemoveInterfaceSFX(pSFX);
		debug_delete(pSFX);
		m_SFXArray.Remove(0);
	}

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_CharSFX);
	m_CharSFX.Reset();
	m_CharSFX.Term();

	ClearAttachFX();
}

void CBaseFolder::ClearAttachFX()
{
	for (int i = 0; i < MAX_INT_ATTACHMENTS; i++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_aAttachment[i].sfx);
		m_aAttachment[i].sfx.Reset();
		m_aAttachment[i].sfx.Term();
		m_aAttachment[i].socket = INVALID_MODEL_SOCKET;
	}
	m_nNumAttachments = 0;

}

void CBaseFolder::UpdateInterfaceSFX()
{

	for (int i = 0; i < m_nNumAttachments; i++)
	{
		CBaseScaleFX *pSFX = &m_aAttachment[i].sfx;
		
		HMODELSOCKET hSocket = m_aAttachment[i].socket;
		LTVector vPos;
		LTRotation rRot;
		LTransform transform;
		if (g_pModelLT->GetSocketTransform(m_CharSFX.GetObject(), hSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);
            g_pLTClient->SetObjectPos(pSFX->GetObject(), &vPos, LTTRUE);
            g_pLTClient->SetObjectRotation(pSFX->GetObject(), &rRot);

		}
	}


}


int CBaseFolder::GetIndex(CLTGUICtrl* pCtrl)
{
	uint32 dwIndex = m_controlArray.FindElement(pCtrl);
	if (dwIndex < m_controlArray.GetSize())
		return (int)dwIndex;
	dwIndex = m_fixedControlArray.FindElement(pCtrl);
	if (dwIndex < m_fixedControlArray.GetSize())
		return FixedIndex((int)dwIndex);
	return kNoSelection;
}


void CBaseFolder::CreateScaleFX(char *szFXName)
{
	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		CBaseScaleFX *pSFX = debug_new(CBaseScaleFX);
		g_pFXButeMgr->CreateScaleFX(pScaleFX,g_vPos, g_vF, LTNULL, &g_rRot, pSFX);
		m_SFXArray.Add(pSFX);
		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);				

		//adjust the object's position based on screen res
		HOBJECT hSFX = pSFX->GetObject();
		if (hSFX)
		{
			LTVector vNewPos;
			g_pLTClient->GetObjectPos(hSFX, &vNewPos);
			vNewPos.z *= g_pInterfaceResMgr->GetXRatio();
			g_pLTClient->SetObjectPos(hSFX, &vNewPos);
		}

	}
}

void CBaseFolder::CreateCharFX(INT_CHAR *pChar)
{
	if (pChar)
	{

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = g_rRot;

		char modName[128];
		char skinName[128];
		char skin2Name[128];

		ModelId eModelId = g_pModelButeMgr->GetModelId(pChar->szModel);
		ModelStyle	eModelStyle = g_pModelButeMgr->GetStyle(pChar->szStyle);
		const char* pFilename = g_pModelButeMgr->GetModelFilename(eModelId, eModelStyle);
		SAFE_STRCPY(modName, pFilename);
		const char* pSkin = g_pModelButeMgr->GetBodySkinFilename(eModelId, eModelStyle);
		SAFE_STRCPY(skinName, pSkin);
		const char* pSkin2 = g_pModelButeMgr->GetHeadSkinFilename(eModelId, eModelStyle);
		SAFE_STRCPY(skin2Name, pSkin2);

		VEC_COPY(vPos,g_vPos);
		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pChar->fScale);

		LTVector vModPos = pChar->vPos;
	    LTFLOAT fRot = pChar->fRot;
		fRot  = MATH_PI + DEG2RAD(fRot);
	    g_pLTClient->RotateAroundAxis(&rRot, &g_vU, fRot);

		VEC_MULSCALAR(vTemp, g_vF, vModPos.z);
		VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, g_vR, vModPos.x);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, g_vU, vModPos.y);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_COPY(bcs.vPos, vPos);
		bcs.rRot = rRot;
		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = modName;
		bcs.pSkin = skinName;
		bcs.pSkin2 = skin2Name;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;


		if (m_CharSFX.Init(&bcs))
		{
			m_CharSFX.CreateObject(g_pLTClient);
			if (m_CharSFX.GetObject())
			{
				g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_WORLD);
			}
		}

	}
}

void CBaseFolder::CreateAttachFX(INT_ATTACH *pAttach)
{
	if (m_nNumAttachments < MAX_INT_ATTACHMENTS)
	{

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = g_rRot;

		CString str = "";
		char szModel[128];
		char szSkin[128];

		str = g_pAttachButeMgr->GetAttachmentModel(pAttach->nAttachmentID);
		strncpy(szModel, (char*)(LPCSTR)str, 128);

		str = g_pAttachButeMgr->GetAttachmentSkin(pAttach->nAttachmentID);
		strncpy(szSkin, (char*)(LPCSTR)str, 128);

		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pAttach->fScale);

		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = szModel;
		bcs.pSkin = szSkin;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		str = g_pAttachButeMgr->GetAttachmentProperties(pAttach->nAttachmentID);

		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.nType = OT_MODEL;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;

		CBaseScaleFX *pSFX = &m_aAttachment[m_nNumAttachments].sfx;

		if (!pSFX->Init(&bcs)) return;
		
		pSFX->CreateObject(g_pLTClient);
		if (!pSFX->GetObject()) return;

		HOBJECT hChar = m_CharSFX.GetObject();
		if (!hChar) return;
		if (g_pModelLT->GetSocket(hChar, pAttach->pszSocket, m_aAttachment[m_nNumAttachments].socket) != LT_OK)
			return;


		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_ATTACH);
		m_nNumAttachments++;


	}
}

void CBaseFolder::UpdateHelpText()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
    uint32 dwID = 0;
	if (pCtrl)
		dwID = pCtrl->GetHelpID();

	if (dwID != m_dwCurrHelpID)
	{
		m_dwCurrHelpID = dwID;

		int nWidth = m_HelpRect.right - m_HelpRect.left;
		int nHeight = m_HelpRect.bottom - m_HelpRect.top;
        LTRect rect(0,0,nWidth,nHeight);
        g_pLTClient->FillRect(m_hHelpSurf,&rect,kBlack);

		if (m_dwCurrHelpID)
		{
			HSTRING hHelpTxt = GetHelpString(m_dwCurrHelpID,m_nSelection);

            GetHelpFont()->DrawFormat(hHelpTxt,m_hHelpSurf,0,0,(uint32)nWidth,kWhite);
			g_pLTClient->OptimizeSurface(m_hHelpSurf,kBlack);
            g_pLTClient->FreeString(hHelpTxt);
		}
	}
}

