// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMouse.cpp
//
// PURPOSE : Interface screen for setting mouse options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMouse.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"

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

    m_bInvertY = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;
	m_nVehicleTurn	= 100;

    m_pSensitivityCtrl = LTNULL;
    m_pInputCtrl = LTNULL;


}

CScreenMouse::~CScreenMouse()
{

}

// Build the screen
LTBOOL CScreenMouse::Build()
{
	CreateTitle(IDS_TITLE_MOUSE);
	
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_MOUSE,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_MOUSE,"SliderWidth");

	//Always mouse look
	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));
	CLTGUIToggle* pToggle = AddToggle(IDS_MOUSE_MOUSELOOK, IDS_HELP_MOUSELOOK, kGap, &m_bMouseLook );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	//mouse sensitivity
	m_pSensitivityCtrl= AddSlider(IDS_MOUSE_SENSITIVITY, IDS_HELP_MOUSE_SENSE, kGap, kWidth, -1, &m_nSensitivity);
	m_pSensitivityCtrl->SetSliderRange(0, 10);
	m_pSensitivityCtrl->SetSliderIncrement(1);

	//mouse responsiveness
	m_pInputCtrl=AddSlider(IDS_MOUSE_INPUTRATE, IDS_HELP_MOUSE_INPUT, kGap, kWidth, -1,  &m_nInputRate);
	m_pInputCtrl->SetSliderRange(0, 40);
	m_pInputCtrl->SetSliderIncrement(2);

	//invert mouse
	pToggle = AddToggle(IDS_MOUSE_INVERTYAXIS, IDS_HELP_INVERTY, kGap, &m_bInvertY );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	CLTGUISlider *pSlider = AddSlider(IDS_VEHICLETURN, IDS_HELP_VEHICLETURN, kGap, kWidth, -1, &m_nVehicleTurn);
	pSlider->SetSliderRange(50, 150);
	pSlider->SetSliderIncrement(5);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenMouse::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};



// Change in focus
void CScreenMouse::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetMouse();
		m_bMouseLook = pProfile->m_bMouseLook;
		m_bInvertY = pProfile->m_bInvertY;

		m_nInputRate = pProfile->m_nInputRate;

		m_nSensitivity = pProfile->m_nSensitivity;

		m_nVehicleTurn = pProfile->m_nVehicleTurn;

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_bMouseLook = m_bMouseLook;
		pProfile->m_bInvertY = m_bInvertY;
		pProfile->m_nInputRate = m_nInputRate;
		pProfile->m_nSensitivity = m_nSensitivity;
		pProfile->m_nVehicleTurn = m_nVehicleTurn;

		pProfile->ApplyMouse();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}