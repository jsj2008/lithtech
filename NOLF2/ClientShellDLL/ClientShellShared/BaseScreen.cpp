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
#include "TransitionFXMgr.h"
#include "FXButeMgr.h"

LTBOOL		CBaseScreen::s_bReadLayout = LTFALSE;
LTRect		CBaseScreen::s_HelpRect;
uint8		CBaseScreen::s_HelpSize;
uint16		CBaseScreen::s_HelpWidth;
LTIntPt		CBaseScreen::s_BackPos;
LTIntPt		CBaseScreen::s_NextPos;
LTVector	CBaseScreen::s_vPos; 
LTVector	CBaseScreen::s_vU; 
LTVector	CBaseScreen::s_vR; 
LTVector	CBaseScreen::s_vF;
LTRotation	CBaseScreen::s_rRot;


CLTGUIButton	CBaseScreen::s_BackArrow;

CUIFormattedPolyString*	CBaseScreen::s_pHelpStr = LTNULL;

namespace
{
	LTIntPt offscreen(-64,-64);

#ifdef _DEMO
#ifdef _TRON_E3_DEMO
	CUIPolyString* pDemoBuildVersion = LTNULL;
	LTIntPt DemoBuildPos;
#endif
#endif


	typedef std::map<std::string,CBaseScaleFX*> ScaleFXMap;
	ScaleFXMap g_ScaleFXMap;
	bool bEditSFXMode = false;
	CBaseScaleFX* pEditSFX = NULL;

	LTVector vSFXPos;
	LTVector vSFXScale;
	float fEditDelta = 1.0f;
}

void EditFXFn(int argc, char **argv)
{
	if(argc != 1)
	{
		g_pLTClient->CPrint("EditFX <fxname>");
		return;
}

	char szName[128];
	SAFE_STRCPY(szName,argv[0]);
	_strupr(szName);
	ScaleFXMap::iterator iter = g_ScaleFXMap.find(szName);
	if (iter == g_ScaleFXMap.end())
	{
		g_pLTClient->CPrint("Could not find ScaleFX %s",argv[0]);
		return;
	}

	g_pLTClient->CPrint("Entering SFX edit mode for %s",argv[0]);
	pEditSFX = (*iter).second;

	HOBJECT hObj = pEditSFX->GetObject();

	g_pLTClient->GetObjectPos(hObj,&vSFXPos);
	g_pLTClient->GetObjectScale(hObj,&vSFXScale);
	bEditSFXMode = true;
	fEditDelta = 1.0f;
}

void HandleEditKey(int key);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseScreen::CBaseScreen()
{
    m_bInit = LTFALSE;
    m_bBuilt = LTFALSE;
    m_bBack = LTFALSE;
	m_bHaveFocus = LTFALSE;
	m_bHaveLights = LTFALSE;

    m_pScreenMgr = LTNULL;

    m_pTitleString = LTNULL;
	m_TitlePos.x = 0;
	m_TitlePos.y = 0;
    m_TitleFont = 0;
    m_TitleSize = 32;
    m_TitleColor = argbBlack;

	m_nScreenID = SCREEN_ID_NONE;
	m_nContinueID = SCREEN_ID_NONE;

	m_dwCurrHelpID = 0;

	// Array of controls that this screen owns
	m_controlArray.reserve(5);

	m_nSelection = kNoSelection;
	m_nOldSelection = kNoSelection;
    m_pCaptureCtrl = LTNULL;
	m_nRMouseDownItemSel =  kNoSelection;
	m_nRMouseDownItemSel =  kNoSelection;

	m_nItemSpacing = 0;

	m_SelectedColor		= argbWhite;
	m_NonSelectedColor	= argbBlack;
	m_DisabledColor		= argbGray;

    m_pNext = LTNULL;

	m_nNumAttachments = 0;

	m_fLastScale = 1.0f;
	m_bSelectFXCenter = LTTRUE;

	m_bVisited = LTFALSE;

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

LTBOOL CBaseScreen::Init(int nScreenID)
{
	if (!s_bReadLayout)
	{
        s_bReadLayout   = LTTRUE;
		s_HelpRect		= g_pLayoutMgr->GetHelpRect();
		s_HelpSize		= g_pLayoutMgr->GetHelpSize();
		s_HelpWidth		= (uint16)(s_HelpRect.right - s_HelpRect.left);
		s_BackPos		= g_pLayoutMgr->GetBackPos();
		s_NextPos		= g_pLayoutMgr->GetNextPos();

		LTIntPt arrowBackPos = g_pLayoutMgr->GetArrowBackPos();

		char szTex[128];
		g_pLayoutMgr->GetArrowBackTex(szTex,128);
		HTEXTURE hBack = g_pInterfaceResMgr->GetTexture(szTex);

		g_pLayoutMgr->GetArrowBackTexH(szTex,128);
		HTEXTURE hBackH = g_pInterfaceResMgr->GetTexture(szTex);

		s_BackArrow.Create(CMD_BACK,LTNULL,hBack,hBackH);
		s_BackArrow.SetBasePos(arrowBackPos);
		s_BackArrow.SetScale(g_pInterfaceResMgr->GetXRatio());

		g_pLTClient->RegisterConsoleProgram("EditFX", EditFXFn);
		
#ifdef _DEMO
#ifdef _TRON_E3_DEMO
		DemoBuildPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_MAIN,"VersionPos");
		float bX = (float)DemoBuildPos.x * g_pInterfaceResMgr->GetXRatio();
		float bY = (float)DemoBuildPos.y * g_pInterfaceResMgr->GetYRatio();
		uint8 nFont = g_pLayoutMgr->GetHelpFont();
		uint8 nSize = (uint8)((LTFLOAT)s_HelpSize * g_pInterfaceResMgr->GetXRatio());

		CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
		pDemoBuildVersion = g_pFontManager->CreatePolyString(pFont,"",0.0f, 0.0f);
		pDemoBuildVersion->SetCharScreenHeight(nSize);
		pDemoBuildVersion->SetColor(argbBlack);
		pDemoBuildVersion->SetText(LoadTempString(IDS_DEMOVERSION));

		bX -= pDemoBuildVersion->GetWidth();
		pDemoBuildVersion->SetPosition(bX,bY);
#endif
#endif

//	    g_pLTClient->RegisterConsoleProgram("CalcFXData", CalcFXDataFn);


	}

	m_nScreenID=nScreenID;
	m_pScreenMgr = g_pInterfaceMgr->GetScreenMgr();

	SetTitleColor(argbWhite);

	//set up layout variables
	SetTitlePos(g_pLayoutMgr->GetScreenTitlePos((eScreenID)nScreenID));
	SetTitleFont(g_pLayoutMgr->GetScreenTitleFont((eScreenID)nScreenID));
	SetTitleSize(g_pLayoutMgr->GetScreenTitleSize((eScreenID)nScreenID));

	m_PageRect  = g_pLayoutMgr->GetScreenPageRect((eScreenID)nScreenID);

	SetItemSpacing(g_pLayoutMgr->GetScreenItemSpacing((eScreenID)nScreenID));

	m_nAlignment = g_pLayoutMgr->GetScreenItemAlign((eScreenID)nScreenID);


	m_SelectedColor		= g_pLayoutMgr->GetScreenSelectedColor((eScreenID)nScreenID);
	m_NonSelectedColor	= g_pLayoutMgr->GetScreenNonSelectedColor((eScreenID)nScreenID);
	m_DisabledColor		= g_pLayoutMgr->GetScreenDisabledColor((eScreenID)nScreenID);

	if (!s_pHelpStr)
	{
		
		uint16 nWidth = (uint16)( (LTFLOAT)s_HelpWidth * g_pInterfaceResMgr->GetXRatio());
		float helpX = (float)s_HelpRect.left * g_pInterfaceResMgr->GetXRatio();
		float helpY = (float)s_HelpRect.top * g_pInterfaceResMgr->GetYRatio();
		uint8 nFont = g_pLayoutMgr->GetHelpFont();
		uint8 nSize = (uint8)((LTFLOAT)s_HelpSize * g_pInterfaceResMgr->GetXRatio());

		CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
		s_pHelpStr = g_pFontManager->CreateFormattedPolyString(pFont,"",helpX,helpY);
		s_pHelpStr->SetCharScreenHeight(nSize);
		s_pHelpStr->SetColor(argbWhite);
		s_pHelpStr->SetWrapWidth(nWidth);

	}

	m_nextPos.x = GetPageLeft();
	m_nextPos.y = GetPageTop();


	char szAttName[30];
	char szIntroFXName[128] = "";
	char szShortFXName[128] = "";
	char szLoopFXName[128] = "";
	int nFXNum = 0;
	bool bFound = false;
	do
	{
		szIntroFXName[0] = 0;
		szShortFXName[0] = 0;
		szLoopFXName[0] = 0;
		bFound = false;

		sprintf(szAttName,"IntroFX%d",nFXNum);
		if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
		{
			g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szIntroFXName,128);
		}
		sprintf(szAttName,"ShortIntroFX%d",nFXNum);
		if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
		{
			g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szShortFXName,128);
		}
		sprintf(szAttName,"LoopFX%d",nFXNum);
		if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
		{
			g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szLoopFXName,128);
		}
		if (strlen(szIntroFXName) || strlen(szShortFXName) || strlen(szLoopFXName))
		{
			nFXNum++;
			bFound = true;

			CChainedFX *pChain = debug_new(CChainedFX);
			pChain->Init(szIntroFXName,szShortFXName,szLoopFXName);
			m_Chains.push_back(pChain);
		}

	} while (bFound);


	m_bInit=TRUE;
    return LTTRUE;
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
	if (m_pTitleString)
	{
		g_pFontManager->DestroyPolyString(m_pTitleString);
        m_pTitleString=LTNULL;
	}

	if (s_pHelpStr)
	{
		g_pFontManager->DestroyPolyString(s_pHelpStr);
        s_pHelpStr=LTNULL;
	}

#ifdef _DEMO
#ifdef _TRON_E3_DEMO
	if (pDemoBuildVersion)
	{
		g_pFontManager->DestroyPolyString(pDemoBuildVersion);
        pDemoBuildVersion=LTNULL;
	}
#endif
#endif

	ChainFXList::iterator iter = m_Chains.begin();
	while (iter != m_Chains.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_Chains.clear();


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
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pPlayerMgr->IsPlayerInWorld() &&
			(!g_pPlayerMgr->IsPlayerDead() || IsMultiplayerGame( )) )
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
//	ROUTINE:	CBaseScreen::Render
//
//	PURPOSE:	Renders the screen to a surface
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScreen::Render(HSURFACE hDestSurf)
{
	if (!hDestSurf)
	{
        return LTFALSE;
	}

	LTIntPt tmpPos;

	// Render the title
	if (m_pTitleString)
	{
		float px,py;
		m_pTitleString->GetPosition(&px,&py);
		px += 2.0f;
		py += 2.0f;
		m_pTitleString->SetColor(0xBF000000);
		m_pTitleString->SetPosition(px,py);
		m_pTitleString->Render();

		px -= 2.0f;
		py -= 2.0f;
		m_pTitleString->SetColor(m_TitleColor);
		m_pTitleString->SetPosition(px,py);
		m_pTitleString->Render();
	}

	
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->Render ();
	}

	if (s_pHelpStr)
	{
		float x;
		float y;
		s_pHelpStr->GetPosition(&x,&y);

		//drop shadow
		s_pHelpStr->SetPosition(x+2.0f,y+2.0f);
		s_pHelpStr->SetColor(argbBlack);
		s_pHelpStr->Render();

		s_pHelpStr->SetPosition(x,y);
		s_pHelpStr->SetColor(argbWhite);
		s_pHelpStr->Render();
	}

#ifdef _DEMO
#ifdef _TRON_E3_DEMO
	if (pDemoBuildVersion)
		pDemoBuildVersion->Render();
#endif
#endif


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::CreateTitle
//
//	PURPOSE:	Creates the string to display as the screens title
//
// ----------------------------------------------------------------------- //


LTBOOL CBaseScreen::CreateTitle(char *lpszTitle)
{
	if (m_pTitleString)
	{
		m_pTitleString->SetText(lpszTitle);
	}
	else
	{
		CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFont);
		LTIntPt pos = m_TitlePos;
		g_pInterfaceResMgr->ConvertScreenPos(pos);
		m_pTitleString = g_pFontManager->CreatePolyString(pFont,lpszTitle,(float)pos.x,(float)pos.y);

		if (!m_pTitleString)
			return LTFALSE;

		uint8 nFontSize = (uint8)((LTFLOAT)m_TitleSize * g_pInterfaceResMgr->GetXRatio());
		m_pTitleString->SetCharScreenHeight(nFontSize);
		m_pTitleString->SetColor(m_TitleColor);

		
	}

	return LTTRUE;

}

LTBOOL CBaseScreen::CreateTitle(int nStringID)
{

    LTBOOL created = CreateTitle(LoadTempString(nStringID));

	return created;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScreen::Build
//
//	PURPOSE:	Construct the basic screen elements
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScreen::Build()
{
    m_bBuilt=LTTRUE;

    UseBack(LTTRUE);

	return TRUE;
}

// Handles a user entered character
LTBOOL CBaseScreen::HandleChar(unsigned char c)
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
LTBOOL CBaseScreen::HandleKeyDown(int key, int rep)
{
    LTBOOL handled = LTFALSE;

	if (bEditSFXMode)
	{
		HandleEditKey(key);
		return LTTRUE;
	}


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
					handled = LTFALSE;
			}
			break;
		}
	}

	// Handled the key
	return handled;
}


/******************************************************************/


/******************************************************************/

LTBOOL CBaseScreen::OnUp()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		UpdateHelpText();
		return LTTRUE;
	}

	uint16 sel = m_nSelection;
	return (sel != PreviousSelection());
}

/******************************************************************/

LTBOOL CBaseScreen::OnDown()
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		UpdateHelpText();
		return LTTRUE;
	}

	uint16 sel = m_nSelection;
	return (sel != NextSelection());
}

/******************************************************************/

LTBOOL CBaseScreen::OnLeft()
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

LTBOOL CBaseScreen::OnRight()
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

LTBOOL CBaseScreen::OnEnter()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
	{
		handled = pCtrl->OnEnter();
		if (handled)
		{
			if (pCtrl == &s_BackArrow || pCtrl == m_pNext)
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
	
	CLTGUICtrl* pCtrl = LTNULL;	
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
	
	CLTGUICtrl* pCtrl = LTNULL;	
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
LTBOOL CBaseScreen::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
LTBOOL CBaseScreen::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
					if (pCtrl == &s_BackArrow || pCtrl == m_pNext)
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
LTBOOL CBaseScreen::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
LTBOOL CBaseScreen::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
				if (pCtrl == &s_BackArrow || pCtrl == m_pNext)
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
LTBOOL CBaseScreen::OnLButtonDblClick(int x, int y)
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
LTBOOL CBaseScreen::OnRButtonDblClick(int x, int y)
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
        return LTFALSE;

	if (pCtrl)
		return pCtrl->OnRButtonDblClick(x, y);
	else
        return LTFALSE;
}


void CBaseScreen::GetHelpString(uint32 dwHelpId, uint16 nControlIndex, char *buffer, int bufLen)
{
    LoadString(dwHelpId,buffer,bufLen);
}


/******************************************************************/
LTBOOL CBaseScreen::OnMouseMove(int x, int y)
{
	uint16 nControlUnderPoint=kNoSelection;
    LTBOOL onCtrl = GetControlUnderPoint(x,y,&nControlUnderPoint);
	if (onCtrl)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlUnderPoint);
		if (m_pCaptureCtrl && m_pCaptureCtrl != pCtrl)
            return LTFALSE;

		if (pCtrl->OnMouseMove(x,y))
		{
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
		}
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
		}
        return LTTRUE;
	}
    return LTFALSE;
}

uint16 CBaseScreen::SetSelection(uint16 select, LTBOOL bFindSelectable)
{
	if (select == m_nSelection) return select;

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
		{
			GetControl(nOldSelect)->Select(LTFALSE);
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
		GetControl(nOldSelect)->Select(LTFALSE);
	}

	m_nOldSelection = m_nSelection;
	m_nSelection = select;

	if (m_nSelection == kNoSelection)
	{
		UpdateHelpText();
		return nOldSelect;
	}

	LTIntPt pos = pSelCtrl->GetPos();
	if (m_bSelectFXCenter)
		pos.x += (pSelCtrl->GetWidth() / 2);
	pos.y += (pSelCtrl->GetHeight() / 2);
	g_pInterfaceMgr->ShowSelectFX(pos);
	pSelCtrl->Select(LTTRUE);

	if (m_nSelection != nOldSelect)
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);



	UpdateHelpText();
	OnSelectionChange();
	return m_nSelection;
}


// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
LTBOOL CBaseScreen::GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex)
{
	_ASSERT(pnIndex);

	if (m_pCaptureCtrl && m_pCaptureCtrl->IsOnMe(xPos,yPos))
	{
		*pnIndex = GetIndex(m_pCaptureCtrl);
		return LTTRUE;
	}


	// See if the user clicked on any of the controls.
	for (uint16 i=0; i < m_controlArray.size() ; i++)
	{
		//start with last control
		int ndx = (m_controlArray.size()-1) - i;

		// Check to see if the click is in the bounding box for the control
		if (m_controlArray[i]->IsOnMe(xPos,yPos) && m_controlArray[i]->IsEnabled())
		{
			*pnIndex=i;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

// Return a control at a specific index
CLTGUICtrl *CBaseScreen::GetControl ( uint16 nIndex )
{
	if (nIndex < m_controlArray.size() )
		return m_controlArray[nIndex];
    return LTNULL;
}





void CBaseScreen::RemoveAll(LTBOOL bDelete)
{
	RemoveControl(&s_BackArrow,LTFALSE);
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

	m_nextPos.x = GetPageLeft();
	m_nextPos.y = GetPageTop();

}

uint16 CBaseScreen::AddControl(CLTGUICtrl* pCtrl)
{
	m_controlArray.push_back(pCtrl);
	uint16 num = m_controlArray.size();
	if (num == m_nSelection+1)
        pCtrl->Select(LTTRUE);
	if (num > 0)
		return num-1;
	else
		return kNoSelection;

}


// Calls UpdateData on each control in the menu
void CBaseScreen::UpdateData(LTBOOL bSaveAndValidate)
{
	for (uint16 i=0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}
}



CLTGUITextCtrl* CBaseScreen::CreateTextItem(int stringID, uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUITextCtrl* pCtrl=CreateTextItem(LoadTempString(stringID), commandID, helpID, pos, bFixed, nFont);
	return pCtrl;

}

CLTGUITextCtrl* CBaseScreen::CreateTextItem(char *pString, uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUITextCtrl* pCtrl=debug_new(CLTGUITextCtrl);

    if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);


    if (!pCtrl->Create(pString, commandID, helpID, pFont, nFontSize, this))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetBaseHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());
	

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}


CLTGUIListCtrl* CBaseScreen::CreateList(LTIntPt pos, uint16 nHeight, LTBOOL bUseArrows, uint16 nArrowOffset)
{
	
	CLTGUIListCtrl* pList=debug_new(CLTGUIListCtrl);
    if (pList->Create(nHeight))
	{
		pList->SetBasePos(pos);
		pList->SetScale(g_pInterfaceResMgr->GetXRatio());
		if (bUseArrows)
		{
			HTEXTURE hUp = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup.dtx");
			HTEXTURE hUpH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup_h.dtx");
			HTEXTURE hDown = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn.dtx");
			HTEXTURE hDownH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn_h.dtx");
			pList->UseArrows(nArrowOffset,1.0f,hUp,hUpH,hDown,hDownH);
		}
		pList->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pList;
}



CLTGUICycleCtrl* CBaseScreen::CreateCycle(int stringID, int helpID, int nHeaderWidth, uint8 *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUICycleCtrl* pCtrl=CreateCycle(LoadTempString(stringID), helpID, nHeaderWidth, pnValue, pos, bFixed, nFont);

	return pCtrl;

}

CLTGUICycleCtrl* CBaseScreen::CreateCycle(char *pString, int helpID, int nHeaderWidth, uint8 *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUICycleCtrl* pCtrl=debug_new(CLTGUICycleCtrl);

    if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);


    if (!pCtrl->Create(pString, helpID, pFont, nFontSize, nHeaderWidth, pnValue))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetBaseHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());
	

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}





CLTGUIToggle* CBaseScreen::CreateToggle(int stringID, int helpID, int nHeaderWidth, LTBOOL *pbValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{

	CLTGUIToggle* pCtrl=CreateToggle(LoadTempString(stringID), helpID, nHeaderWidth, pbValue, pos, bFixed, nFont);

	return pCtrl;

}

CLTGUIToggle* CBaseScreen::CreateToggle(char *pString, int helpID, int nHeaderWidth, LTBOOL *pbValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIToggle* pCtrl=debug_new(CLTGUIToggle);

    if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);


    if (!pCtrl->Create(pString, helpID, pFont, nFontSize, nHeaderWidth, pbValue))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetBaseHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	pCtrl->SetOnString(LoadTempString(IDS_ON));
	pCtrl->SetOffString(LoadTempString(IDS_OFF));

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}


CLTGUISlider* CBaseScreen::CreateSlider(int stringID, int helpID, int nHeaderWidth, int nBarWidth,
										int nBarHeight, int *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUISlider* pCtrl=CreateSlider(LoadTempString(stringID), helpID, nHeaderWidth, nBarWidth, nBarHeight, pnValue, pos, bFixed, nFont);

	return pCtrl;

}

CLTGUISlider* CBaseScreen::CreateSlider(char *pString, int helpID, int nHeaderWidth, int nBarWidth,
										int nBarHeight, int *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUISlider* pCtrl=debug_new(CLTGUISlider);

    if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	char szTex[128];
	g_pLayoutMgr->GetSliderTex(szTex,128);
	HTEXTURE hBar = g_pInterfaceResMgr->GetTexture(szTex);

	if (nBarHeight < 0)
	{
		uint32 w,h;
		g_pTexInterface->GetTextureDims(hBar,w,h);
		nBarHeight = h / 3;

	}


    if (!pCtrl->Create(pString, helpID, pFont, nFontSize, hBar, nHeaderWidth, nBarWidth, nBarHeight, pnValue))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetBaseHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());
	

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}


CLTGUIColumnCtrl* CBaseScreen::CreateColumnCtrl(uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIColumnCtrl* pCtrl=debug_new(CLTGUIColumnCtrl);

    if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);


    if (!pCtrl->Create(commandID, helpID, pFont, nFontSize, this))
	{
		debug_delete(pCtrl);
        return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (nFontSize + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());
	

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;

}



void CBaseScreen::UseBack(LTBOOL bBack,LTBOOL bOK,LTBOOL bReturn)
{
	if (bBack)
	{
		CreateBack(bOK,bReturn);

		if (GetIndex(&s_BackArrow) >= m_controlArray.size())
			AddControl(&s_BackArrow);
	}
	else
	{
		RemoveControl(&s_BackArrow,LTFALSE);
	}

	m_bBack = bBack;
}


void CBaseScreen::CreateBack(LTBOOL bOK, LTBOOL bReturn)
{

    int nStr = 0;
    int nHelp = 0;
	if (bOK)
	{
		nStr = IDS_OK;
		nHelp = IDS_HELP_OK;
	}
	else
	{
		nStr = IDS_BACK;
		nHelp = IDS_HELP_BACK;
	}
	uint8 nFont = g_pLayoutMgr->GetBackFont();
	uint8 nFontSize = g_pLayoutMgr->GetBackSize();

	s_BackArrow.SetFont(g_pInterfaceResMgr->GetFont(nFont),nFontSize);
	s_BackArrow.SetHelpID(nHelp);
	s_BackArrow.SetText(LoadTempString(nStr),LTTRUE);
	s_BackArrow.SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		
	if (bReturn)
		s_BackArrow.SetHelpID(IDS_HELP_RETURN);
	else if (bOK)
		s_BackArrow.SetHelpID(IDS_HELP_OK);
	else
		s_BackArrow.SetHelpID(IDS_HELP_BACK);

	s_BackArrow.SetCommandHandler(this);

}


void CBaseScreen::RemoveControl(CLTGUICtrl* pControl,LTBOOL bDelete)
{
	if (!pControl) return;

	ControlArray::iterator iter = m_controlArray.begin();

	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		m_controlArray.erase(iter);
	}

	if (bDelete && pControl != &s_BackArrow)
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

	default:
		return 0;
	}

	return 1;
}


void CBaseScreen::ForceMouseUpdate()
{
//	SetSelection(kNoSelection);
	m_dwCurrHelpID = 0;
    LTIntPt cPos = g_pInterfaceMgr->GetCursorPos();
	OnMouseMove(cPos.x,cPos.y);
}


// This is called when the screen gets or loses focus
void CBaseScreen::OnFocus(LTBOOL bFocus)
{
    m_pCaptureCtrl = LTNULL;

	bEditSFXMode = false;

	if (bFocus)
	{
		if (m_fLastScale != g_pInterfaceResMgr->GetXRatio())
		{
			ScreenDimsChanged();
		}
		m_nOldSelection = kNoSelection;
		if (m_nSelection == kNoSelection)
		{
			if (m_pNext)
				SetSelection(GetIndex(m_pNext));
			else
			{
				SetSelection(0,LTTRUE);
				if (m_nSelection == kNoSelection && m_bBack)
				{
					SetSelection(GetIndex(&s_BackArrow));
				}
		
			}
		}

		
		ForceMouseUpdate();
		UpdateHelpText();
		m_pScreenMgr->GetTransitionFXMgr()->EnterScreen(m_nScreenID);
		CreateInterfaceSFX();
		m_bHaveFocus = LTTRUE;
		m_bVisited = LTTRUE; //set this last
	}
	else
	{
		SetSelection(kNoSelection);
		m_pScreenMgr->GetTransitionFXMgr()->ExitScreen(m_nScreenID);
		RemoveInterfaceSFX();
		m_bHaveFocus = LTFALSE;

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

	int n = 0;
	char szAttName[30];
	char szFXName[128];

	m_bHaveLights = LTFALSE;


	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;


    g_pLTClient->GetObjectPos(hCamera, &s_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &s_rRot);
	s_vU = s_rRot.Up();
	s_vR = s_rRot.Right();
	s_vF = s_rRot.Forward();



	sprintf(szAttName,"Light%d",n);
	while (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
	{
		g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateLightFX(szFXName);
		}

		n++;
		sprintf(szAttName,"Light%d",n);

	}


	n = 0;
	g_ScaleFXMap.clear();
	sprintf(szAttName,"ScaleName%d",n);
	while (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
	{
		g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CBaseScaleFX *pSFX = CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"ScaleName%d",n);

	}

	
	INT_CHAR *pChar = g_pLayoutMgr->GetScreenCharacter((eScreenID)m_nScreenID);
	if (pChar)
	{
		CreateCharFX(pChar);
		if (m_CharSFX.GetObject())
		{
			int reqID[MAX_INT_ATTACHMENTS];
			int numReq = g_pAttachButeMgr->GetRequirementIDs(pChar->szModel,reqID,MAX_INT_ATTACHMENTS);
			int i;
			for (i = 0; i < numReq; i++)
			{
				INT_ATTACH acs;
				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
				g_pAttachButeMgr->GetRequirementSocket(reqID[i],acs.szSocket,sizeof(acs.szSocket));

				CreateAttachFX(&acs);
			}

			int numAtt = g_pLayoutMgr->GetScreenNumAttachments((eScreenID)m_nScreenID);
			for (i = 0; i < numAtt; i++)
			{
				char szTemp[128];
				char *pName = LTNULL;
				char *pSocket = LTNULL;
				g_pLayoutMgr->GetScreenAttachment( (eScreenID)m_nScreenID, i, szTemp, 128);

				pName = strtok(szTemp,";");
				pSocket = strtok(NULL,";");

				INT_ATTACH acs;

				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(pName);
				SAFE_STRCPY(acs.szSocket,pSocket);

				CreateAttachFX(&acs);
			}

		}
	}
	
	szFXName[0] = 0;
	g_pLayoutMgr->GetScreenMouseFX((eScreenID)m_nScreenID,szFXName,sizeof(szFXName));
	g_pInterfaceMgr->SetMouseFX(szFXName);

	szFXName[0] = 0;
	g_pLayoutMgr->GetScreenSelectFX((eScreenID)m_nScreenID,szFXName,sizeof(szFXName));
	m_bSelectFXCenter = g_pLayoutMgr->GetScreenSelectFXCenter((eScreenID)m_nScreenID);
	g_pInterfaceMgr->SetSelectFX(szFXName);

	n = 0;
	sprintf(szAttName,"FX%d",n);
	while (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
	{
		g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			INT_FX* pFX = g_pLayoutMgr->GetFX(szFXName);
			if (pFX)
			{
				g_pInterfaceMgr->AddInterfaceFX(LTNULL, pFX->szFXName,pFX->vPos,pFX->bLoop);
			}
		}

		
		n++;
		sprintf(szAttName,"FX%d",n);

	}

	if (m_pScreenMgr->GetTransitionFXMgr()->HasTransitionFX())
	{
		m_pScreenMgr->GetTransitionFXMgr()->StartTransitionFX(!!m_bVisited);
	}
	else
	{
		// ABM 2/7/02 TODO rip these lines out in favor of letting the TransitionFXMgr handle
		// all the FX.  So take these effects and move their creation into TransitionFXMgr.
		// As a first step, take the Start and Update out, and let BaseScreen still create
		// them but then pass them to the TransitionFXMgr.  As a second step, move their creation
		// into TransitionFXMgr as well.

		ChainFXList::iterator iter = m_Chains.begin();

		while (iter != m_Chains.end())
		{
			(*iter)->Start(!!m_bVisited);
			iter++;
		}
	}
}

void CBaseScreen::RemoveInterfaceSFX()
{
	g_ScaleFXMap.clear();
	for (uint16 i=0; i < m_SFXArray.size(); i++)
	{
		CSpecialFX *pSFX = m_SFXArray[i];
		g_pInterfaceMgr->RemoveInterfaceSFX(pSFX);
		debug_delete(pSFX);
	}
	m_SFXArray.clear();

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_CharSFX);
	
	g_pInterfaceMgr->RemoveInterfaceLights();
	m_bHaveLights = LTFALSE;

	m_CharSFX.Reset();
	m_CharSFX.Term();

	ClearAttachFX();

	ChainFXList::iterator iter = m_Chains.begin();
	while (iter != m_Chains.end())
	{
		(*iter)->End();
		iter++;
	}


	g_pInterfaceMgr->RemoveInterfaceFX();


}

void CBaseScreen::ClearAttachFX()
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

bool CBaseScreen::UpdateInterfaceSFX()
{

	for (int i = 0; i < m_nNumAttachments; i++)
	{
		CBaseScaleFX *pSFX = &m_aAttachment[i].sfx;
		
		HMODELSOCKET hSocket = m_aAttachment[i].socket;
		LTransform transform;
		if (g_pModelLT->GetSocketTransform(m_CharSFX.GetObject(), hSocket, transform, LTTRUE) == LT_OK)
		{
            g_pLTClient->SetObjectPosAndRotation(pSFX->GetObject(), &transform.m_Pos, &transform.m_Rot);

		}
	}

	if (m_pScreenMgr->GetTransitionFXMgr()->HasTransitionFX())
	{
		m_pScreenMgr->GetTransitionFXMgr()->UpdateTransitionFX();
	}
	else
	{
		// ABM TODO 2/7/02 remove this in favor of letting the TransitionFXMgr handle all details.
		ChainFXList::iterator iter = m_Chains.begin();
		while (iter != m_Chains.end())
		{
			(*iter)->Update();
			iter++;
		}
	}

	return true;
}


uint16 CBaseScreen::GetIndex(CLTGUICtrl* pCtrl)
{
	ControlArray::iterator iter = m_controlArray.begin();

	uint16 dwIndex = 0;
	while (iter != m_controlArray.end() && (*iter) != pCtrl)
	{
		++dwIndex;
		iter++;
	}
	if (dwIndex < m_controlArray.size())
		return dwIndex;
	return kNoSelection;
}


CBaseScaleFX* CBaseScreen::CreateScaleFX(char *szFXName)
{
	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		pScaleFX->bUseLight = m_bHaveLights;
		CBaseScaleFX *pSFX = debug_new(CBaseScaleFX);
		g_pFXButeMgr->CreateScaleFX(pScaleFX,s_vPos, s_vF, LTNULL, &s_rRot, pSFX);
		m_SFXArray.push_back(pSFX);
		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);				

		char szTmp[64];
		SAFE_STRCPY(szTmp,szFXName);
		_strupr(szTmp);
		g_ScaleFXMap[szTmp] = pSFX;

		return pSFX;
	}
	return NULL;
}
void CBaseScreen::CreateLightFX(char *szFXName)
{
	INT_LIGHT* pLight = g_pLayoutMgr->GetLight(szFXName);
	if (pLight)
	{
//		pScaleFX->bUseLight = LTTRUE;
//		CBaseScaleFX *pSFX = debug_new(CBaseScaleFX);
//		g_pFXButeMgr->CreateScaleFX(pScaleFX,s_vPos, s_vF, LTNULL, &s_rRot, pSFX);
//		m_SFXArray.push_back(pSFX);
//		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);


		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE | FLAG_ONLYLIGHTOBJECTS;

        createStruct.m_Pos = s_vPos;

		createStruct.m_Pos += pLight->vPos;;

        HOBJECT hLight = g_pLTClient->CreateObject(&createStruct);

		if (hLight)
		{
			g_pLTClient->SetLightColor(hLight, pLight->vColor.x, pLight->vColor.y, pLight->vColor.z);
			g_pLTClient->SetLightRadius(hLight, pLight->fRadius);

			g_pInterfaceMgr->AddInterfaceLight(hLight);
			m_bHaveLights = LTTRUE;
		}

	}
}

void CBaseScreen::CreateCharFX(INT_CHAR *pChar)
{
	if (pChar)
	{

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = s_rRot;

		char modName[128];

		SAFE_STRCPY(modName, pChar->szModel);

		VEC_COPY(vPos,s_vPos);
		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pChar->fScale);

		LTVector vModPos = pChar->vPos;
	    LTFLOAT fRot = pChar->fRot;
		fRot  = MATH_PI + DEG2RAD(fRot);
		rRot.Rotate(s_vU, fRot);

		VEC_MULSCALAR(vTemp, s_vF, vModPos.z);
//		VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, s_vR, vModPos.x);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, s_vU, vModPos.y);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_COPY(bcs.vPos, vPos);
		bcs.rRot = rRot;
		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = modName;
		bcs.pSkinReader = &(pChar->blrSkins);
		bcs.pRenderStyleReader = &(pChar->blrRenderStyles);
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 0.99f;
		bcs.fFinalAlpha = 0.99f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;

		bcs.fMinRotateVel = 1.0f;
		bcs.fMaxRotateVel = 1.0f;

		bcs.nMenuLayer = pChar->nMenuLayer;


		if (m_CharSFX.Init(&bcs))
		{
			m_CharSFX.CreateObject(g_pLTClient);
			if (m_CharSFX.GetObject())
			{
//				g_pLTClient->SetModelAnimation(m_CharSFX.GetObject(), 1);
				g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_NORMAL);
			}
		}

	}
}

void CBaseScreen::CreateAttachFX(INT_ATTACH *pAttach)
{
	if (m_nNumAttachments < MAX_INT_ATTACHMENTS)
	{

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = s_rRot;

		char szModel[128];
//		char szSkin[128];

		g_pAttachButeMgr->GetAttachmentModel(pAttach->nAttachmentID,szModel,sizeof(szModel));

//		str = g_pAttachButeMgr->GetAttachmentSkin(pAttach->nAttachmentID);
//		strncpy(szSkin, (char*)(LPCSTR)str, 128);

		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pAttach->fScale);

		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = szModel;
//		bcs.pSkin[0] = szSkin;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

//		g_pAttachButeMgr->GetAttachmentProperties(pAttach->nAttachmentID);

		CButeListReader blrSkinReader;
		g_pAttachButeMgr->GetAttachmentSkins(pAttach->nAttachmentID, &blrSkinReader);
		bcs.pSkinReader = &blrSkinReader;

		CButeListReader blrRenderStyleReader;
		g_pAttachButeMgr->GetAttachmentRenderStyles(pAttach->nAttachmentID, &blrRenderStyleReader);
		bcs.pRenderStyleReader = &blrRenderStyleReader;

		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.nType = OT_MODEL;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;
		bcs.nMenuLayer = m_CharSFX.GetMenuLayer();

		CBaseScaleFX *pSFX = &m_aAttachment[m_nNumAttachments].sfx;

		if (!pSFX->Init(&bcs)) return;
		
		pSFX->CreateObject(g_pLTClient);
		if (!pSFX->GetObject()) return;

		HOBJECT hChar = m_CharSFX.GetObject();
		if (!hChar) return;
		if (g_pModelLT->GetSocket(hChar, pAttach->szSocket, m_aAttachment[m_nNumAttachments].socket) != LT_OK)
			return;

		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_ATTACH);
		m_nNumAttachments++;
	}
}

void CBaseScreen::UpdateHelpText()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
    uint32 dwID = 0;
	if (pCtrl)
		dwID = pCtrl->GetHelpID();

	if (!dwID)
	{
		s_pHelpStr->SetText("");
		m_dwCurrHelpID = 0;
		return;
	}

	if (dwID != m_dwCurrHelpID)
	{
		m_dwCurrHelpID = dwID;

		int nWidth = s_HelpRect.right - s_HelpRect.left;
		int nHeight = s_HelpRect.bottom - s_HelpRect.top;

		if (m_dwCurrHelpID)
		{
			char szHelpText[256] = "";
			GetHelpString(m_dwCurrHelpID,m_nSelection,szHelpText,sizeof(szHelpText));

			if (s_pHelpStr && strlen(szHelpText))
			{
				s_pHelpStr->SetText(szHelpText);
				uint16 nWidth = (uint16)( (LTFLOAT)s_HelpWidth * g_pInterfaceResMgr->GetXRatio());
				float helpX = (float)s_HelpRect.left * g_pInterfaceResMgr->GetXRatio();
				float helpY = (float)s_HelpRect.top * g_pInterfaceResMgr->GetYRatio();
				uint8 nSize = (uint8)((LTFLOAT)s_HelpSize * g_pInterfaceResMgr->GetXRatio());

				s_pHelpStr->SetPosition(helpX,helpY);
				s_pHelpStr->SetCharScreenHeight(nSize);
				s_pHelpStr->SetWrapWidth(nWidth);
			}

		}
	}
}


void CBaseScreen::ScreenDimsChanged()
{
	m_fLastScale = g_pInterfaceResMgr->GetXRatio();
	unsigned int i;
	for ( i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->SetScale(g_pInterfaceResMgr->GetXRatio());
	}

	if (m_pTitleString)
	{
		LTIntPt pos = m_TitlePos;
		g_pInterfaceResMgr->ConvertScreenPos(pos);
		m_pTitleString->SetPosition((float)pos.x,(float)pos.y);
		uint8 nFontSize = (uint8)((LTFLOAT)m_TitleSize * g_pInterfaceResMgr->GetXRatio());
		m_pTitleString->SetCharScreenHeight(nFontSize);
	}

	if (s_pHelpStr)
	{
		uint16 nWidth = (uint16)( (LTFLOAT)s_HelpWidth * g_pInterfaceResMgr->GetXRatio());
		float helpX = (float)s_HelpRect.left * g_pInterfaceResMgr->GetXRatio();
		float helpY = (float)s_HelpRect.top * g_pInterfaceResMgr->GetYRatio();
		uint8 nSize = (uint8)((LTFLOAT)s_HelpSize * g_pInterfaceResMgr->GetXRatio());

		s_pHelpStr->SetPosition(helpX,helpY);
		s_pHelpStr->SetCharScreenHeight(nSize);
		s_pHelpStr->SetWrapWidth(nWidth);
	}

#ifdef _DEMO
#ifdef _TRON_E3_DEMO
	if (pDemoBuildVersion)
	{
		float bX = (float)DemoBuildPos.x * g_pInterfaceResMgr->GetXRatio();
		float bY = (float)DemoBuildPos.y * g_pInterfaceResMgr->GetYRatio();
		uint8 nSize = (uint8)((LTFLOAT)s_HelpSize * g_pInterfaceResMgr->GetXRatio());

		pDemoBuildVersion->SetCharScreenHeight(nSize);
		bX -= pDemoBuildVersion->GetWidth();
		pDemoBuildVersion->SetPosition(bX,bY);

	}
#endif
#endif

}


// Creates the title for the screen
void CBaseScreen::SetTitlePos(LTIntPt pt)
{
	m_TitlePos = pt;
	if (m_pTitleString)
	{
		LTIntPt pos = m_TitlePos;
		g_pInterfaceResMgr->ConvertScreenPos(pos);
		m_pTitleString->SetPosition((float)pos.x,(float)pos.y);
	}
}

void CBaseScreen::SetTitleFont(uint8 nFont)
{
	m_TitleFont = nFont;
	if (m_pTitleString)
	{
		CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFont);
		m_pTitleString->ApplyFont(pFont);
	}
}


void CBaseScreen::SetTitleSize(uint8 nFontSize)
{
	m_TitleSize = nFontSize;
	if (m_pTitleString)
	{
		CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFont);
		uint8 nFontSize = (uint8)((LTFLOAT)m_TitleSize * g_pInterfaceResMgr->GetXRatio());
		m_pTitleString->SetCharScreenHeight(nFontSize);
	}
}

void CBaseScreen::SetTitleColor(uint32 titleColor)
{
	m_TitleColor = titleColor;
	if (m_pTitleString)
	{
		m_pTitleString->SetColor(titleColor);
	}
}


void CBaseScreen::SetPolyRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}

void CBaseScreen::InitPoly(LTPoly_GT4* pPoly, LTIntPt pos, HTEXTURE hTex)
{
	_ASSERT(pPoly && hTex);
	if (!pPoly || !hTex)
		return;
	ScalePoly(pPoly,pos,hTex);
	SetupQuadUVs(*pPoly, hTex, 0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(pPoly,argbWhite);

}

void CBaseScreen::ScalePoly(LTPoly_GT4* pPoly, LTIntPt pos, HTEXTURE hTex)
{
	_ASSERT(pPoly && hTex);
	if (!pPoly || !hTex)
		return;
	uint32 w,h;
	g_pTexInterface->GetTextureDims(hTex,w,h);
	float x = (float)pos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)pos.y * g_pInterfaceResMgr->GetXRatio();
	float fw = (float)w * g_pInterfaceResMgr->GetXRatio();
	float fh = (float)h * g_pInterfaceResMgr->GetXRatio();

	g_pDrawPrim->SetXYWH(pPoly,x,y,fw,fh);

}



void HandleEditKey(int key)
{
	switch (key)
	{
	case VK_LEFT:
		{
			vSFXPos.x -= fEditDelta;
		} break;
	case VK_RIGHT:
		{
			vSFXPos.x += fEditDelta;
		} break;
	case VK_UP:
		{
			vSFXPos.y += fEditDelta;
		} break;
	case VK_DOWN:
		{
			vSFXPos.y -= fEditDelta;
		} break;
	case VK_A:
		{
			vSFXScale.x -= fEditDelta / 10.0f;
		} break;
	case VK_D:
		{
			vSFXScale.x += fEditDelta / 10.0f;
		} break;
	case VK_W:
		{
			vSFXScale.y -= fEditDelta / 10.0f;
		} break;
	case VK_S:
		{
			vSFXScale.y += fEditDelta / 10.0f;
		} break;
	case VK_PRIOR:
		{
			fEditDelta *= 10.0f;
			g_pLTClient->CPrint("Edit scale = %0.3f",fEditDelta);
		}break;
	case VK_NEXT:
		{
			fEditDelta /= 10.0f;
			g_pLTClient->CPrint("Edit scale = %0.3f",fEditDelta);
		}break;
	case VK_RETURN:
		{
			bEditSFXMode = false;
			g_pLTClient->CPrint("Exiting SFX edit mode.");
			g_pLTClient->CPrint("	SFX Pos = <%0.3f, %0.3f, %0.3f>", vSFXPos.x,vSFXPos.y,vSFXPos.z);
			g_pLTClient->CPrint("	SFX Scale = <%0.3f, %0.3f, %0.3f>", vSFXScale.x,vSFXScale.y,vSFXScale.z);
		} break;
	}

	HOBJECT hObj = pEditSFX->GetObject();
	g_pLTClient->SetObjectPos(hObj,&vSFXPos);
	g_pLTClient->SetObjectScale(hObj,&vSFXScale);

}

