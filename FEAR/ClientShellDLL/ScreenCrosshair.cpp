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
#include "InterfaceMgr.h"
#include "VarTrack.h"
#include "HUDCrosshair.h"

namespace
{
	int kGap = 250;
	int kWidth = 200;
	uint32 nMinCrosshairSize = 4;
	uint32 nMaxCrosshairSize = 16;
}
	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenCrosshair::CScreenCrosshair()
{
	m_nColorA = 0;
	m_nColorR = 0;
	m_nColorG = 0;
	m_nColorB = 0;
	m_nCrosshairSize = 16;
	m_pPlaceholder = NULL;

}

CScreenCrosshair::~CScreenCrosshair()
{

}


// Build the screen
bool CScreenCrosshair::Build()
{
	CreateTitle("IDS_TITLE_CROSSHAIR");

	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);

	const wchar_t* szYes = LoadString("IDS_YES");
	const wchar_t* szNo = LoadString("IDS_NO");

	LTVector2n pos = m_DefaultPos;

	//background frame
	CLTGUICtrl_create frameCs;
	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));

	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame, frameCs);
	AddControl(pFrame);

	m_DefaultPos = pos;

	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_CH_R";
	scs.pnValue = &m_nColorR;
	scs.nMin = 15;
	scs.nMax = 255;
	scs.nIncrement = 16;
	CLTGUISlider *pSlider = AddSlider("IDS_CH_R", scs );

	scs.szHelpID = "IDS_HELP_CH_G";
	scs.pnValue = &m_nColorG;
	pSlider = AddSlider("IDS_CH_G", scs );

	scs.szHelpID = "IDS_HELP_CH_B";
	scs.pnValue = &m_nColorB;
	pSlider = AddSlider("IDS_CH_B", scs );

	scs.szHelpID = "ScreenCrosshair_Alpha_Help";
	scs.pnValue = &m_nColorA;
	pSlider = AddSlider( "ScreenCrosshair_Alpha", scs );

	scs.szHelpID = "ScreenCrosshair_Size_Help";
	scs.pnValue = &m_nCrosshairSize;
	scs.nMin = nMinCrosshairSize;
	scs.nMax = nMaxCrosshairSize;
	scs.nIncrement = 2;
	pSlider = AddSlider("ScreenCrosshair_Size", scs );

	m_pPlaceholder = debug_new(CLTGUIFrame);
	uint32 nSz = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,0);
	frameCs.rnBaseRect.m_vMin = m_DefaultPos;
	frameCs.rnBaseRect.m_vMin.x += kGap;
	frameCs.rnBaseRect.m_vMax.x = frameCs.rnBaseRect.m_vMin.x + nSz;
	frameCs.rnBaseRect.m_vMax.y = frameCs.rnBaseRect.m_vMin.y + nSz;

	m_pPlaceholder->Create(hFrame, frameCs, true);
	AddControl(m_pPlaceholder);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
}

bool CScreenCrosshair::OnLeft()
{
    bool bHandled = CBaseScreen::OnLeft();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

bool CScreenCrosshair::OnRight()
{
    bool bHandled = CBaseScreen::OnRight();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

bool CScreenCrosshair::OnLButtonUp(int x, int y)
{
    bool bHandled = CBaseScreen::OnLButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

bool CScreenCrosshair::OnRButtonUp(int x, int y)
{
    bool bHandled = CBaseScreen::OnRButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

// Change in focus
void CScreenCrosshair::OnFocus(bool bFocus)
{
	if (bFocus)
	{
	
		GetConsoleVariables();
		g_pCrosshair->Update();
        UpdateData(false);

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

	m_nColorR = (int)pProfile->m_CrosshairR;
	m_nColorG = (int)pProfile->m_CrosshairG;
	m_nColorB = (int)pProfile->m_CrosshairB;
	m_nColorA = (int)pProfile->m_CrosshairA;
	m_nCrosshairSize = (int)pProfile->m_nCrosshairSize;
}

void CScreenCrosshair::SetConsoleVariables()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_CrosshairR = (uint8)m_nColorR;
	pProfile->m_CrosshairG = (uint8)m_nColorG;
	pProfile->m_CrosshairB = (uint8)m_nColorB;
	pProfile->m_CrosshairA = (uint8)m_nColorA;
	pProfile->m_nCrosshairSize = m_nCrosshairSize;

	pProfile->ApplyCrosshair();
	g_pCrosshair->Update();

}

	
bool CScreenCrosshair::Render ()
{
    bool bRendered = CBaseScreen::Render();

	if (bRendered && m_pPlaceholder)
	{
		LTVector2 vPos( m_pPlaceholder->GetRect().Left() + m_pPlaceholder->GetRect().GetWidth()*0.5f,
						m_pPlaceholder->GetRect().Top() + m_pPlaceholder->GetRect().GetHeight()*0.5f );
		g_pCrosshair->RenderCrosshair(vPos,true);

	}

	return bRendered;
}
