// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenCrosshair.cpp
//
// PURPOSE : Interface screen for setting crosshair options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenCrosshair.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "TronInterfaceMgr.h"
#include "VarTrack.h"

namespace
{
	int kGap = 250;
	int kWidth = 200;
	int nCrosshairSize = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenCrosshair::CScreenCrosshair()
{
    m_bCrosshair = LTFALSE;
	m_nAlpha = 0;
	m_nColorR = 0;
	m_nColorG = 0;
	m_nColorB = 0;
	m_nStyle = 0;
    m_bDynamic = LTFALSE;

}

CScreenCrosshair::~CScreenCrosshair()
{

}


// Build the screen
LTBOOL CScreenCrosshair::Build()
{
	CreateTitle(IDS_TITLE_CROSSHAIR);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CROSSHAIR,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CROSSHAIR,"SliderWidth");

	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	//use crosshair
	CLTGUIToggle* pToggle = AddToggle(IDS_USE_CROSSHAIR, IDS_HELP_CROSSHAIR, kGap, &m_bCrosshair );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	CLTGUISlider *pSlider = AddSlider(IDS_CH_ALPHA, IDS_HELP_CH_ALPHA, kGap, kWidth, -1, &m_nAlpha);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_R, IDS_HELP_CH_R, kGap, kWidth, -1, &m_nColorR);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_G, IDS_HELP_CH_G, kGap, kWidth, -1, &m_nColorG);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_B, IDS_HELP_CH_B, kGap, kWidth, -1, &m_nColorB);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pToggle = AddToggle(IDS_CH_DYNAMIC, IDS_HELP_CH_DYNAMIC, kGap, &m_bDynamic );

	m_pStyle = AddCycle(IDS_CH_STYLE,IDS_HELP_CH_STYLE,kGap,&m_nStyle);
	char szTmpBuffer[kMaxStringBuffer];

    FormatString(IDS_CH_BASIC,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);

    FormatString(IDS_CH_CROSSBAR,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);
    
	FormatString(IDS_CH_DOTCROSS,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);

    FormatString(IDS_CH_POST,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);

    FormatString(IDS_CH_DOT,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);

    FormatString(IDS_CH_CORNER,szTmpBuffer,sizeof(szTmpBuffer));
	m_pStyle->AddString(szTmpBuffer);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

LTBOOL CScreenCrosshair::OnLeft()
{
    LTBOOL bHandled = CBaseScreen::OnLeft();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CScreenCrosshair::OnRight()
{
    LTBOOL bHandled = CBaseScreen::OnRight();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CScreenCrosshair::OnLButtonUp(int x, int y)
{
    LTBOOL bHandled = CBaseScreen::OnLButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CScreenCrosshair::OnRButtonUp(int x, int y)
{
    LTBOOL bHandled = CBaseScreen::OnRButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

// Change in focus
void CScreenCrosshair::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
	
		GetConsoleVariables();
        UpdateData(LTFALSE);
		nCrosshairSize = (int)( (g_pLayoutMgr->GetCrosshairGapMax() + g_pLayoutMgr->GetCrosshairBarMax() ) * g_pInterfaceResMgr->GetXRatio());
	}
	else
	{
		UpdateData();
		SetConsoleVariables();
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		pProfile->Save();
		
	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenCrosshair::GetConsoleVariables()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->SetCrosshair();


	m_bCrosshair = pProfile->m_bCrosshair;

	m_nAlpha = (int)pProfile->m_CrosshairA;
	m_nColorR = (int)pProfile->m_CrosshairR;
	m_nColorG = (int)pProfile->m_CrosshairG;
	m_nColorB = (int)pProfile->m_CrosshairB;
	m_nStyle = pProfile->m_nStyle;
	m_bDynamic = pProfile->m_bDynamic;
}

void CScreenCrosshair::SetConsoleVariables()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_bCrosshair = m_bCrosshair;

	pProfile->m_CrosshairA = (uint8)m_nAlpha;
	pProfile->m_CrosshairR = (uint8)m_nColorR;
	pProfile->m_CrosshairG = (uint8)m_nColorG;
	pProfile->m_CrosshairB = (uint8)m_nColorB;

	pProfile->m_nStyle = m_nStyle;
	pProfile->m_bDynamic = m_bDynamic;

	pProfile->ApplyCrosshair();
}

LTBOOL CScreenCrosshair::Render ( HSURFACE hDestSurf )
{
    LTBOOL bRendered = CBaseScreen::Render(hDestSurf);
	if (bRendered)
	{
        LTIntPt pos = m_pStyle->GetBasePos();
		pos.x += m_nItemSpacing;
		pos.y += m_nItemSpacing;
		g_pInterfaceResMgr->ConvertScreenPos(pos.x,pos.y);
		pos.y += m_pStyle->GetHeight();
        LTRect rect(pos.x,pos.y,pos.x+nCrosshairSize,pos.y+nCrosshairSize);

        g_pLTClient->FillRect(hDestSurf,&rect,LTNULL);

		g_pCrosshair->RenderArmed(LTTRUE, LTIntPt(pos.x+nCrosshairSize/2,pos.y+nCrosshairSize/2) );

	}
	return bRendered;
}
