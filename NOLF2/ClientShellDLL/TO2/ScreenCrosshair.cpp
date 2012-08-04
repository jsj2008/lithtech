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
#include "TO2InterfaceMgr.h"
#include "VarTrack.h"

namespace
{
	int kGap = 250;
	int kWidth = 200;
}
	uint32 nCrosshairSize = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenCrosshair::CScreenCrosshair()
{
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

	//background frame
	LTRect frameRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_CROSSHAIR,"FrameRect");
	LTIntPt pos(frameRect.left,frameRect.top);
	int nHt = frameRect.bottom - frameRect.top;
	int nWd = frameRect.right - frameRect.left;

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_CROSSHAIR,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,nWd,nHt+8,LTTRUE);
	pFrame->SetBasePos(pos);
	pFrame->SetBorder(2,m_SelectedColor);
	AddControl(pFrame);

	CLTGUISlider *pSlider = AddSlider(IDS_CH_R, IDS_HELP_CH_R, kGap, kWidth, -1, &m_nColorR);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_G, IDS_HELP_CH_G, kGap, kWidth, -1, &m_nColorG);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_B, IDS_HELP_CH_B, kGap, kWidth, -1, &m_nColorB);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	CLTGUIToggle* pToggle = AddToggle(IDS_CH_DYNAMIC, IDS_HELP_CH_DYNAMIC, kGap, &m_bDynamic );

	m_pStyle = AddCycle(IDS_CH_STYLE,IDS_HELP_CH_STYLE,kGap,&m_nStyle);
	char szTmp[kMaxStringBuffer];

	uint8 style = 0;
	char *szTag = "HUDCrosshair";
	char szAtt[32];
	sprintf(szAtt,"Crosshair%d",style);

	while (g_pLayoutMgr->HasValue(szTag,szAtt))
	{
		g_pLayoutMgr->GetString(szTag,szAtt,szTmp,sizeof(szTmp));
		strcat(szTmp,"_A.dtx");
		m_styles.push_back(szTmp);

		sprintf(szTmp,"%d",style);
//		m_pStyle->AddString(szTmp);
		m_pStyle->AddString(" ");

		style++;
		sprintf(szAtt,"Crosshair%d",style);

	}

	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);

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

		uint32 th;
		m_hCrosshair = g_pInterfaceResMgr->GetTexture(m_styles[m_nStyle].c_str());
		SetupQuadUVs(m_Poly, m_hCrosshair, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pTexInterface->GetTextureDims(m_hCrosshair,nCrosshairSize,th);

		uint8 cr = (uint8)m_nColorR;
		uint8 cg = (uint8)m_nColorG;
		uint8 cb = (uint8)m_nColorB;
		uint8 ca = 0xFF;
		uint32 crosscolor = SET_ARGB(ca,cr,cg,cb);
		g_pDrawPrim->SetRGBA(&m_Poly,crosscolor);

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
	m_nStyle = pProfile->m_nStyle;
	m_bDynamic = pProfile->m_bDynamic;
}

void CScreenCrosshair::SetConsoleVariables()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_CrosshairR = (uint8)m_nColorR;
	pProfile->m_CrosshairG = (uint8)m_nColorG;
	pProfile->m_CrosshairB = (uint8)m_nColorB;

	pProfile->m_nStyle = m_nStyle;
	pProfile->m_bDynamic = m_bDynamic;

	pProfile->ApplyCrosshair();

	uint32 th;
	m_hCrosshair = g_pInterfaceResMgr->GetTexture(m_styles[m_nStyle].c_str());
	g_pTexInterface->GetTextureDims(m_hCrosshair,nCrosshairSize,th);
	
	uint8 cr = (uint8)m_nColorR;
	uint8 cg = (uint8)m_nColorG;
	uint8 cb = (uint8)m_nColorB;
	uint8 ca = 0xFF;
	uint32 crosscolor = SET_ARGB(ca,cr,cg,cb);
	g_pDrawPrim->SetRGBA(&m_Poly,crosscolor);


}

LTBOOL CScreenCrosshair::Render ( HSURFACE hDestSurf )
{
    LTBOOL bRendered = CBaseScreen::Render(hDestSurf);

	if (bRendered)
	{
        LTIntPt pos = m_pStyle->GetBasePos();
		pos.x += kGap;
		g_pInterfaceResMgr->ConvertScreenPos(pos.x,pos.y);


		uint32 nSize = (uint32)((float)nCrosshairSize * g_pInterfaceResMgr->GetXRatio());
		uint8  nIndent = (uint8)(8.0f * g_pInterfaceResMgr->GetXRatio());
        LTRect rect(pos.x,pos.y,pos.x+nSize+(nIndent*2),pos.y+nSize+(nIndent*2));

        g_pLTClient->FillRect(hDestSurf,&rect,LTNULL);

		float x = (float)(pos.x + nIndent);
		float y = (float)(pos.y + nIndent);
		float sz = (float)nSize;
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,sz,sz);

		g_pDrawPrim->SetTexture(m_hCrosshair);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	return bRendered;
}
