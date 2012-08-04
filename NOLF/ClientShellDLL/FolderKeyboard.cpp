// FolderKeyboard.cpp: implementation of the CFolderKeyboard class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderKeyboard.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "FolderCustomControls.h"
#include "ClientRes.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderKeyboard::CFolderKeyboard()
{
    m_pLookCtrl = LTNULL;

    m_bLookspring = LTFALSE;

    m_bMouseLook = LTFALSE;
	m_nNormalTurn	= 15;
	m_nFastTurn		= 23;
	m_nLookUp		= 25;
	m_nVehicleTurn	= 100;

}

CFolderKeyboard::~CFolderKeyboard()
{

}

// Build the folder
LTBOOL CFolderKeyboard::Build()
{
	CreateTitle(IDS_TITLE_KEYBOARD);
	
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_KEYBOARD,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_KEYBOARD,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_KEYBOARD,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_KEYBOARD,"SliderWidth");

	//turn speed
	CSliderCtrl *pSlider = AddSlider(IDS_NORMALTURN, IDS_HELP_NORMALTURN, kGap, kWidth, &m_nNormalTurn);
	pSlider->SetSliderRange(5, 55);
	pSlider->SetSliderIncrement(5);

	pSlider = AddSlider(IDS_FASTTURN, IDS_HELP_FASTTURN, kGap, kWidth, &m_nFastTurn);
	pSlider->SetSliderRange(8, 58);
	pSlider->SetSliderIncrement(5);

	// look speed
	pSlider = AddSlider(IDS_LOOKUP, IDS_HELP_LOOKUP, kGap, kWidth, &m_nLookUp);
	pSlider->SetSliderRange(5, 50);
	pSlider->SetSliderIncrement(5);

	// auto center
	m_pLookCtrl = AddToggle(IDS_MOUSE_LOOKSPRING, IDS_HELP_LOOKSPRING, kGap, &m_bLookspring );
	m_pLookCtrl->SetOnString(IDS_YES);
	m_pLookCtrl->SetOffString(IDS_NO);

	pSlider = AddSlider(IDS_VEHICLETURN, IDS_HELP_VEHICLETURN, kGap, kWidth, &m_nVehicleTurn);
	pSlider->SetSliderRange(50, 150);
	pSlider->SetSliderIncrement(5);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CFolderKeyboard::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_UPDATE:
		{
			UpdateData();
			m_pLookCtrl->Enable(!m_bMouseLook);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CFolderKeyboard::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
    LTBOOL bMouseLook = pSettings->MouseLook();
	if (bFocus)
	{
		m_bMouseLook = bMouseLook;
		m_pLookCtrl->Enable(!bMouseLook);
		if (bMouseLook)
		{
            m_bLookspring = LTFALSE;
		}
		else
			m_bLookspring = pSettings->Lookspring();

		float fTemp = pSettings->GetFloatVar("NormalTurnRate");
		m_nNormalTurn = (int)(10.0f * fTemp);

		fTemp = pSettings->GetFloatVar("FastTurnRate");
		m_nFastTurn = (int)(10.0f * fTemp);

		fTemp = pSettings->GetFloatVar("LookUpRate");
		m_nLookUp = (int)(10.0f * fTemp);

		fTemp = pSettings->GetFloatVar("VehicleTurnRateScale");
		m_nVehicleTurn = (int)(100.0f * fTemp);
		
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		if (!bMouseLook)
			pSettings->SetBoolVar("Lookspring",m_bLookspring);

		float fTemp = (float)m_nNormalTurn / 10.0f;
		pSettings->SetFloatVar("NormalTurnRate",fTemp);

		fTemp = (float)m_nFastTurn / 10.0f;
		pSettings->SetFloatVar("FastTurnRate",fTemp);

		fTemp = (float)m_nLookUp / 10.0f;
		pSettings->SetFloatVar("LookUpRate",fTemp);

		fTemp = (float)m_nVehicleTurn / 100.0f;
		pSettings->SetFloatVar("VehicleTurnRateScale",fTemp);

		// Just to be safe save the config incase anything changed...

        g_pLTClient->WriteConfigFile("autoexec.cfg");
	}
	CBaseFolder::OnFocus(bFocus);
}