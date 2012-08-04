// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoystick.cpp
//
// PURPOSE : Interface screen for joystick/gamepad settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoystick.h"
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

CScreenJoystick::CScreenJoystick()
{
	memset(m_nAxis,0,sizeof(m_nAxis));
	memset(m_nPOV,0,sizeof(m_nPOV));

}

CScreenJoystick::~CScreenJoystick()
{

}

// Build the screen
LTBOOL CScreenJoystick::Build()
{
	CreateTitle(IDS_TITLE_JOYSTICK);
	
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOYSTICK,"ColumnWidth");
	uint8 nLarge = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOYSTICK,"HeaderFontSize");

	
	CLTGUICycleCtrl *pCtrl = AddCycle(IDS_JOYSTICK_AXIS, NULL, kGap, NULL, kDefaultPos, LTTRUE );
	pCtrl->AddString(LoadTempString(IDS_JOYSTICK_ACTION));
	pCtrl->SetFont(NULL,nLarge);
	m_nextPos.y += 8;

	for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
	{
		CDeviceAxisData *pAxisData = g_pProfileMgr->GetAxisData(axis);
		if (!pAxisData || !strlen(pAxisData->m_sName)) continue;

		CLTGUICycleCtrl *pCtrl = AddCycle(pAxisData->m_sName, IDS_HELP_AXIS, kGap, &m_nAxis[axis] );
		pCtrl->SetParam1(axis);
		pCtrl->AddString(LoadTempString(IDS_JOYSTICK_AXISNONE));

		switch (pAxisData->m_nType)
		{
			case CONTROLTYPE_XAXIS:
			case CONTROLTYPE_RXAXIS:
			case CONTROLTYPE_ZAXIS:
				pCtrl->AddString(LoadTempString(IDS_JOYSTICK_TURN));
				pCtrl->AddString(LoadTempString(IDS_JOYSTICK_STRAFE));
				break;
			case CONTROLTYPE_YAXIS:
			case CONTROLTYPE_RYAXIS:
			case CONTROLTYPE_RZAXIS:
				pCtrl->AddString(LoadTempString(IDS_JOYSTICK_LOOK));
				pCtrl->AddString(LoadTempString(IDS_JOYSTICK_MOVE));
				pCtrl->AddString(LoadTempString(IDS_JOYSTICK_INVERT));
				break;
		}

//		pCtrl->NotifyOnChange(CMD_UPDATE,this);
	}

	for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
	{
		CDevicePOVData *pPOVData = g_pProfileMgr->GetPOVData(POV);
		if (!pPOVData || !strlen(pPOVData->m_sName)) continue;

		CLTGUICycleCtrl *pCtrl = AddCycle(pPOVData->m_sName, IDS_HELP_POV, kGap, &m_nPOV[POV] );
		pCtrl->SetParam1(POV);
		pCtrl->AddString(LoadTempString(IDS_JOYSTICK_AXISNONE));

		pCtrl->AddString(LoadTempString(IDS_POV_LOOK));
		pCtrl->AddString(LoadTempString(IDS_POV_MOVE));
	}


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenJoystick::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			UpdateData();

		
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CScreenJoystick::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetJoystick();
		for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
		{
			m_nAxis[axis] = pProfile->m_nAxis[axis];
		}
		for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
		{
			m_nPOV[POV] = pProfile->m_nPOV[POV];
		}

	
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		for (int axis = 0; axis < g_pProfileMgr->GetNumAxis(); axis++)
		{
			pProfile->m_nAxis[axis] = m_nAxis[axis];
		};
		for (int POV = 0; POV < g_pProfileMgr->GetNumPOV(); POV++)
		{
			pProfile->m_nPOV[POV] = m_nPOV[POV];
		};

		pProfile->ApplyJoystick();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}