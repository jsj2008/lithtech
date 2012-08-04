// FolderMouse.cpp: implementation of the CFolderMouse class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderMouse.h"
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

CFolderMouse::CFolderMouse()
{

    m_bInvertY = LTFALSE;
    m_bMouseLook = LTFALSE;
	m_nInputRate = 0;
	m_nSensitivity = 0;

    m_pSensitivityCtrl = LTNULL;
    m_pInputCtrl = LTNULL;

	m_nVehicleTurn	= 100;

}

CFolderMouse::~CFolderMouse()
{

}

// Build the folder
LTBOOL CFolderMouse::Build()
{
	CreateTitle(IDS_TITLE_MOUSE);
	
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_MOUSE,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MOUSE,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_MOUSE,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MOUSE,"SliderWidth");

	//Always mouse look
	CToggleCtrl* pToggle = AddToggle(IDS_MOUSE_MOUSELOOK, IDS_HELP_MOUSELOOK, kGap, &m_bMouseLook );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	//mouse sensitivity
	m_pSensitivityCtrl=AddSlider(IDS_MOUSE_SENSITIVITY, IDS_HELP_MOUSE_SENSE, kGap, kWidth, &m_nSensitivity);
	m_pSensitivityCtrl->SetSliderRange(0, 10);
	m_pSensitivityCtrl->SetSliderIncrement(1);

	//mouse responsiveness
	m_pInputCtrl=AddSlider(IDS_MOUSE_INPUTRATE, IDS_HELP_MOUSE_INPUT, kGap, kWidth, &m_nInputRate);
	m_pInputCtrl->SetSliderRange(0, 40);
	m_pInputCtrl->SetSliderIncrement(2);

	//invert mouse
	pToggle = AddToggle(IDS_MOUSE_INVERTYAXIS, IDS_HELP_INVERTY, kGap, &m_bInvertY );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	CSliderCtrl *pSlider = AddSlider(IDS_VEHICLETURN, IDS_HELP_VEHICLETURN, kGap, kWidth, &m_nVehicleTurn);
	pSlider->SetSliderRange(50, 150);
	pSlider->SetSliderIncrement(5);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CFolderMouse::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};



// Change in focus
void CFolderMouse::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	if (bFocus)
	{
		m_bMouseLook = GetConsoleInt("MouseLook",1);
		m_bInvertY = GetConsoleInt("MouseInvertY",0);

		m_nInputRate = GetConsoleInt("InputRate",20);
		if (m_nInputRate < 0) m_nInputRate = 0;
		if (m_nInputRate > 40) m_nInputRate = 40;

		m_nSensitivity = GetConsoleInt("MouseSensitivity",5);
		if (m_nSensitivity < 0) m_nSensitivity = 0;
		if (m_nSensitivity > 10) m_nSensitivity = 10;

		float fTemp = GetConsoleFloat("VehicleTurnRateScale",1.0f);
		m_nVehicleTurn = (int)(100.0f * fTemp);

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		WriteConsoleInt("MouseInvertY",m_bInvertY);
		WriteConsoleInt("MouseLook",m_bMouseLook);
		WriteConsoleFloat("MouseSensitivity",(float)m_nSensitivity);
		pSettings->SetFloatVar("MouseSensitivity",(float)m_nSensitivity);
		pSettings->ImplementMouseSensitivity();
		WriteConsoleFloat("InputRate",(float)m_nInputRate);

		float fTemp = (float)m_nVehicleTurn / 100.0f;
		WriteConsoleFloat("VehicleTurnRateScale",fTemp);

		// Just to be safe save the config incase anything changed...

        g_pLTClient->WriteConfigFile("autoexec.cfg");
	}
	CBaseFolder::OnFocus(bFocus);
}