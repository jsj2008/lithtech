// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenKeyboard.cpp
//
// PURPOSE : Interface screen for keyboard settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenKeyboard.h"
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

CScreenKeyboard::CScreenKeyboard()
{
    m_pLookCtrl = LTNULL;


    m_bAutoCenter = LTFALSE;
	m_nNormalTurn	= 15;
	m_nFastTurn		= 23;
	m_nLookUp		= 25;
	m_nVehicleTurn	= 100;

}

CScreenKeyboard::~CScreenKeyboard()
{

}

// Build the screen
LTBOOL CScreenKeyboard::Build()
{
	CreateTitle(IDS_TITLE_KEYBOARD);
	
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_KEYBOARD,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_KEYBOARD,"SliderWidth");

	//turn speed
	CLTGUISlider *pSlider = AddSlider(IDS_NORMALTURN, IDS_HELP_NORMALTURN, kGap, kWidth, -1, &m_nNormalTurn);
	pSlider->SetSliderRange(5, 55);
	pSlider->SetSliderIncrement(5);

	pSlider = AddSlider(IDS_FASTTURN, IDS_HELP_FASTTURN, kGap, kWidth, -1, &m_nFastTurn);
	pSlider->SetSliderRange(8, 58);
	pSlider->SetSliderIncrement(5);

	// look speed
	pSlider = AddSlider(IDS_LOOKUP, IDS_HELP_LOOKUP, kGap, kWidth, -1, &m_nLookUp);
	pSlider->SetSliderRange(5, 50);
	pSlider->SetSliderIncrement(5);

	// auto center
	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	m_pLookCtrl = AddToggle(IDS_MOUSE_LOOKSPRING, IDS_HELP_LOOKSPRING, kGap, &m_bAutoCenter );
	m_pLookCtrl->SetOnString(szYes);
	m_pLookCtrl->SetOffString(szNo);

	pSlider = AddSlider(IDS_VEHICLETURN, IDS_HELP_VEHICLETURN, kGap, kWidth, -1, &m_nVehicleTurn);
	pSlider->SetSliderRange(50, 150);
	pSlider->SetSliderIncrement(5);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenKeyboard::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			UpdateData();
//			m_pLookCtrl->Enable(!m_bMouseLook);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CScreenKeyboard::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetKeyboard();
		m_pLookCtrl->Enable(!pProfile->m_bMouseLook);
		m_bAutoCenter = pProfile->m_bAutoCenter;
		m_nNormalTurn = pProfile->m_nNormalTurn;
		m_nFastTurn = pProfile->m_nFastTurn;
		m_nLookUp = pProfile->m_nLookUp;
		m_nVehicleTurn = pProfile->m_nVehicleTurn;
		
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_bAutoCenter = m_bAutoCenter;
		pProfile->m_nNormalTurn = m_nNormalTurn;
		pProfile->m_nFastTurn = m_nFastTurn;
		pProfile->m_nLookUp	= m_nLookUp;
		pProfile->m_nVehicleTurn = m_nVehicleTurn;

		pProfile->ApplyKeyboard();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}