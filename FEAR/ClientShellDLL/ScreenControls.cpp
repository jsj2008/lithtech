// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenControls.cpp
//
// PURPOSE : Interface screen for setting control options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenControls.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"
#include "GameClientShell.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;

	void ConfirmCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenControls *pThisScreen = (CScreenControls *)pUserData;
		if (pThisScreen)
		{
			pThisScreen->ConfirmSetting(bReturn);
		}
	}

}

#ifndef IDS_CONFIRM_RESTORE
#	define IDS_CONFIRM_RESTORE	1
#endif
#ifndef IDS_JOYSTICK_UNBOUND
#	define IDS_JOYSTICK_UNBOUND 2
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenControls::CScreenControls()
{
    m_pJoystickCtrl = NULL;

    m_bUseJoystick=true;
	m_nConfirm = 0;
}

CScreenControls::~CScreenControls()
{

}

// Build the screen
bool CScreenControls::Build()
{
	CreateTitle("IDS_TITLE_CONTROLS");
	
	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);


	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kGap,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	cs.nCommandID = CMD_CONFIGURE;
	cs.szHelpID = "IDS_HELP_CUSTOMCONTROLS";
	//customize
	AddTextItem("IDS_CONFIGURE", cs );


	// Mouse
	cs.nCommandID = CMD_MOUSE;
	cs.szHelpID = "IDS_HELP_MOUSE";
	AddTextItem("IDS_MOUSE", cs);

/*
	// keyboard
	AddTextItem(IDS_KEYBOARD, CMD_KEYBOARD, IDS_HELP_KEYBOARD);

	// use joystick
	CLTGUIToggle* pToggle = AddToggle(IDS_JOYSTICK_USE, IDS_HELP_USEJOYSTICK, kGap, &m_bUseJoystick);
	pToggle->NotifyOnChange(CMD_UPDATE,this);
	char szYes[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	char szNo[16];
	FormatString(IDS_NO,szNo,sizeof(szNo));
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);
	pToggle->Enable( bHasJoystick ? true : false );
//	pToggle->Enable(false);


*/

	// joystick
	cs.nCommandID = CMD_JOYSTICK;
	cs.szHelpID = "IDS_HELP_JOYSTICK";
	m_pJoystickCtrl = AddTextItem("IDS_JOYSTICK", cs);
	//	m_pJoystickCtrl->Enable(false);
	
	//restore
	cs.nCommandID = CMD_RESET_DEFAULTS;
	cs.szHelpID = "IDS_HELP_RESTORE";
	AddTextItem("IDS_RESTOREDEFAULTS", cs);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;


	UseBack(true,true);
	return true;

}

uint32 CScreenControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_CONFIGURE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_CONFIGURE);
			break;
		}
	case CMD_UPDATE:
		{

			UpdateData();
			if (m_pJoystickCtrl)
				m_pJoystickCtrl->Enable(m_bUseJoystick);
			break;

		}
	case CMD_MOUSE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MOUSE);
			break;
		}
	case CMD_KEYBOARD:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_KEYBOARD);
			break;
		}
	case CMD_JOYSTICK:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
			break;
		}
	case CMD_RESET_DEFAULTS:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ConfirmCallBack;
			mb.pUserData = this;
			m_nConfirm = IDS_CONFIRM_RESTORE;
			g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRM_RESTORE",&mb);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};


// Change in focus
void CScreenControls::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{

		pProfile->SetControls();

		if (m_pJoystickCtrl)
			m_pJoystickCtrl->Enable(m_bUseJoystick);

        UpdateData(false);
	}
	else
	{
		pProfile->Save();

		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenControls::ConfirmSetting(bool bConfirm)
{
	switch (m_nConfirm)
	{

	case IDS_JOYSTICK_UNBOUND:
/*
		if (bConfirm)
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
		else
		{
			m_bUseJoystick = false;
			UpdateData(false);
		}
*/
		break;

	case IDS_CONFIRM_RESTORE:
		if (bConfirm)
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			pProfile->RestoreDefaults(PROFILE_CONTROLS);
		}
		break;
	}

}