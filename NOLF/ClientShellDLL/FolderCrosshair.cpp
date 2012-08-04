// FolderCrosshair.cpp: implementation of the CFolderCrosshair class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderCrosshair.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "VarTrack.h"

namespace
{
	VarTrack g_vtCrosshairStyle;
	VarTrack g_vtCrosshairColorR;
	VarTrack g_vtCrosshairColorG;
	VarTrack g_vtCrosshairColorB;
	VarTrack g_vtCrosshairAlpha;
	VarTrack g_vtCrosshairDynamic;
	VarTrack g_vtCrosshairGapMax;
	VarTrack g_vtCrosshairBarMax;

	int kGap = 250;
	int kWidth = 200;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCrosshair::CFolderCrosshair()
{
    m_bCrosshair = LTFALSE;
	m_nAlpha = 0;
	m_nColorR = 0;
	m_nColorG = 0;
	m_nColorB = 0;
	m_nStyle = 0;
    m_bDynamic = LTFALSE;

}

CFolderCrosshair::~CFolderCrosshair()
{

}


// Build the folder
LTBOOL CFolderCrosshair::Build()
{
	CreateTitle(IDS_TITLE_CROSSHAIR);

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_CROSSHAIR,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CROSSHAIR,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_CROSSHAIR,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CROSSHAIR,"SliderWidth");

	//use crosshair
	CToggleCtrl* pToggle = AddToggle(IDS_USE_CROSSHAIR, IDS_HELP_CROSSHAIR, kGap, &m_bCrosshair );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	CSliderCtrl *pSlider = AddSlider(IDS_CH_ALPHA, IDS_HELP_CH_ALPHA, kGap, kWidth, &m_nAlpha);
	pSlider->SetSliderRange(2,10);
	pSlider->SetSliderIncrement(1);

	pSlider = AddSlider(IDS_CH_R, IDS_HELP_CH_R, kGap, kWidth, &m_nColorR);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_G, IDS_HELP_CH_G, kGap, kWidth, &m_nColorG);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pSlider = AddSlider(IDS_CH_B, IDS_HELP_CH_B, kGap, kWidth, &m_nColorB);
	pSlider->SetSliderRange(15,255);
	pSlider->SetSliderIncrement(16);

	pToggle = AddToggle(IDS_CH_DYNAMIC, IDS_HELP_CH_DYNAMIC, kGap, &m_bDynamic );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	m_pStyle = AddCycleItem(IDS_CH_STYLE,IDS_HELP_CH_STYLE,(kGap-25),25,&m_nStyle);
	m_pStyle->AddString(IDS_CH_BASIC);
	m_pStyle->AddString(IDS_CH_CROSSBAR);
	m_pStyle->AddString(IDS_CH_DOTCROSS);
	m_pStyle->AddString(IDS_CH_POST);
	m_pStyle->AddString(IDS_CH_DOT);
	m_pStyle->AddString(IDS_CH_CORNER);





	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

LTBOOL CFolderCrosshair::OnLeft()
{
    LTBOOL bHandled = CBaseFolder::OnLeft();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CFolderCrosshair::OnRight()
{
    LTBOOL bHandled = CBaseFolder::OnRight();
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CFolderCrosshair::OnLButtonUp(int x, int y)
{
    LTBOOL bHandled = CBaseFolder::OnLButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

LTBOOL CFolderCrosshair::OnRButtonUp(int x, int y)
{
    LTBOOL bHandled = CBaseFolder::OnRButtonUp(x,y);
	if (bHandled)
	{
		UpdateData();
		SetConsoleVariables();
	}
	return bHandled;
}

// Change in focus
void CFolderCrosshair::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_bCrosshair = g_pInterfaceMgr->IsCrosshairEnabled();
		GetConsoleVariables();
        UpdateData(LTFALSE);
		g_pInterfaceMgr->GetPlayerStats()->UpdateCrosshairColors();
	}
	else
	{
		g_pInterfaceMgr->EnableCrosshair(m_bCrosshair);
		UpdateData();
		WriteConsoleVariables();
	}
	CBaseFolder::OnFocus(bFocus);
}

void CFolderCrosshair::GetConsoleVariables()
{
	if (!g_vtCrosshairGapMax.IsInitted())
	{
        g_vtCrosshairGapMax.Init(g_pLTClient, "CrosshairGapMax", NULL, 0.0f);
        g_vtCrosshairBarMax.Init(g_pLTClient, "CrosshairBarMax", NULL, 0.0f);

        g_vtCrosshairStyle.Init(g_pLTClient, "CrosshairStyle", NULL, 0.0f);
        g_vtCrosshairColorR.Init(g_pLTClient, "CrosshairColorR", NULL, 1.0f);
        g_vtCrosshairColorG.Init(g_pLTClient, "CrosshairColorG", NULL, 1.0f);
        g_vtCrosshairColorB.Init(g_pLTClient, "CrosshairColorB", NULL, 1.0f);
        g_vtCrosshairAlpha.Init(g_pLTClient, "CrosshairAlpha", NULL, 1.0f);
        g_vtCrosshairDynamic.Init(g_pLTClient, "CrosshairDynamic", NULL, 1.0f);

	}


	m_nAlpha = (int)(10.0f * g_vtCrosshairAlpha.GetFloat());
	m_nColorR = (int)(255.0f * g_vtCrosshairColorR.GetFloat());
	m_nColorG = (int)(255.0f * g_vtCrosshairColorG.GetFloat());
	m_nColorB = (int)(255.0f * g_vtCrosshairColorB.GetFloat());
	m_nStyle = (int)(g_vtCrosshairStyle.GetFloat());
	m_bDynamic = (g_vtCrosshairDynamic.GetFloat() >= 1.0f );
}

void CFolderCrosshair::SetConsoleVariables()
{
    g_vtCrosshairAlpha.SetFloat( (LTFLOAT)m_nAlpha / 10.0f );
    g_vtCrosshairColorR.SetFloat( (LTFLOAT)m_nColorR / 255.0f);
    g_vtCrosshairColorG.SetFloat( (LTFLOAT)m_nColorG / 255.0f);
    g_vtCrosshairColorB.SetFloat( (LTFLOAT)m_nColorB / 255.0f);
    g_vtCrosshairStyle.SetFloat( (LTFLOAT)m_nStyle);
	g_vtCrosshairDynamic.SetFloat(  (m_bDynamic ? 1.0f : 0.0f) );
	g_pInterfaceMgr->GetPlayerStats()->UpdateCrosshairColors();


}
void CFolderCrosshair::WriteConsoleVariables()
{
    g_vtCrosshairAlpha.WriteFloat( (LTFLOAT)m_nAlpha / 10.0f );
    g_vtCrosshairColorR.WriteFloat( (LTFLOAT)m_nColorR / 255.0f);
    g_vtCrosshairColorG.WriteFloat( (LTFLOAT)m_nColorG / 255.0f);
    g_vtCrosshairColorB.WriteFloat( (LTFLOAT)m_nColorB / 255.0f);
    g_vtCrosshairStyle.WriteFloat( (LTFLOAT)m_nStyle);
	g_vtCrosshairDynamic.WriteFloat(  (m_bDynamic ? 1.0f : 0.0f) );
	g_pInterfaceMgr->GetPlayerStats()->UpdateCrosshairColors();


}

LTBOOL CFolderCrosshair::Render ( HSURFACE hDestSurf )
{
    LTBOOL bRendered = CBaseFolder::Render(hDestSurf);
	if (bRendered)
	{
        LTIntPt pos = m_pStyle->GetPos();
		pos.x += m_nItemSpacing + g_pInterfaceResMgr->GetXOffset();
		pos.y += m_nItemSpacing + m_pStyle->GetHeight() + g_pInterfaceResMgr->GetYOffset();
		int nSize = (int)(g_vtCrosshairGapMax.GetFloat() + g_vtCrosshairBarMax.GetFloat());
        LTRect rect(pos.x,pos.y,pos.x+nSize,pos.y+nSize);

        g_pLTClient->FillRect(hDestSurf,&rect,LTNULL);

        g_pInterfaceMgr->GetPlayerStats()->DrawCrosshair(hDestSurf,pos.x+nSize/2,pos.y+nSize/2,LTTRUE);
	}
	return bRendered;
}
