// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMouse.cpp
//
// PURPOSE : Interface screen for setting mouse options
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMouse.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "VarTrack.h"

extern VarTrack	g_vtMouseMinSensitivity;
extern VarTrack	g_vtMouseMaxSensitivity;
extern VarTrack	g_vtMouseMinInputRate;
extern VarTrack	g_vtMouseMaxInputRate;

namespace
{
	int kGap = 200;
	int kWidth = 200;

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMouse::CScreenMouse()
{

    m_bInvertY = false;
	m_nInputRate = 0;
	m_nSensitivity = 0;
	m_nVehicleTurn	= 100;

    m_pSensitivityCtrl = NULL;
    m_pInputCtrl = NULL;


}

CScreenMouse::~CScreenMouse()
{

}

// Build the screen
bool CScreenMouse::Build()
{
	CreateTitle("IDS_TITLE_MOUSE");
	
	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);

	//background frame
	CLTGUICtrl_create cs;
	cs.rnBaseRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,cs);
	AddControl(pFrame);


	m_DefaultPos = m_ScreenRect.m_vMin;

	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	CLTGUIToggle* pToggle = NULL;

	//invert mouse
	tcs.pbValue = &m_bInvertY;
	tcs.nHeaderWidth = kGap;
	tcs.szHelpID = "IDS_HELP_INVERTY";
	pToggle = AddToggle("IDS_MOUSE_INVERTYAXIS", tcs );
	pToggle->SetOnString(LoadString("IDS_YES"));
	pToggle->SetOffString(LoadString("IDS_NO"));

	//mouse sensitivity
	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_MOUSE_SENSE";
	scs.pnValue = &m_nSensitivity;
	scs.nMin = int(g_vtMouseMinSensitivity.GetFloat());
	scs.nMax = int(g_vtMouseMaxSensitivity.GetFloat());
	scs.nIncrement = 1;
	m_pSensitivityCtrl= AddSlider("IDS_MOUSE_SENSITIVITY", scs );

	//mouse responsiveness
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_MOUSE_INPUT";
	scs.pnValue = &m_nInputRate;
	scs.nMin = int(g_vtMouseMinInputRate.GetFloat());
	scs.nMax = int(g_vtMouseMaxInputRate.GetFloat());
	m_pInputCtrl=AddSlider("IDS_MOUSE_INPUTRATE", scs);


	/*
	CLTGUISlider *pSlider = AddSlider(IDS_VEHICLETURN, IDS_HELP_VEHICLETURN, kGap, kWidth, -1, &m_nVehicleTurn);
	pSlider->SetSliderRange(50, 150);
	pSlider->SetSliderIncrement(10);
*/

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;

}

uint32 CScreenMouse::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};



// Change in focus
void CScreenMouse::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetMouse();
		m_bInvertY = pProfile->m_bInvertY;

		m_nInputRate = pProfile->m_nInputRate;

		m_nSensitivity = pProfile->m_nSensitivity;

        UpdateData(false);
	}
	else
	{
		UpdateData();

		pProfile->m_bInvertY = m_bInvertY;
		pProfile->m_nInputRate = m_nInputRate;
		pProfile->m_nSensitivity = m_nSensitivity;

		pProfile->ApplyMouse();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}