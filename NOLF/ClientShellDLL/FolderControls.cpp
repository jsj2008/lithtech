// FolderControls.cpp: implementation of the CFolderControls class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderControls.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "FolderCustomControls.h"
#include "ClientRes.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"
#include "FolderJoystick.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;

	void ConfirmCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderControls *pThisFolder = (CFolderControls *)pData;
		if (pThisFolder)
		{
			pThisFolder->ConfirmSetting(bReturn);
		}
	}

	const int FOLDER_CMD_WEAPON_CONTROLS = (FOLDER_CMD_CUSTOM + 1);

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderControls::CFolderControls()
{
    m_pJoystickCtrl = LTNULL;

    m_bUseJoystick=LTFALSE;
	m_nConfirm = 0;


}

CFolderControls::~CFolderControls()
{

}

// Build the folder
LTBOOL CFolderControls::Build()
{
	CreateTitle(IDS_TITLE_CONTROLS);
	
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_CONTROLS,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CONTROLS,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_CONTROLS,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CONTROLS,"SliderWidth");

	//customize
	AddTextItem(IDS_CUSTOM_CONTROLS, FOLDER_CMD_CUSTOM_CONTROLS, IDS_HELP_CUSTOMCONTROLS);

	//customize
	AddTextItem(IDS_WEAPON_CONTROLS, FOLDER_CMD_WEAPON_CONTROLS, IDS_HELP_WEAPON_CONTROLS);

	// Mouse
	AddTextItem(IDS_MOUSE, FOLDER_CMD_MOUSE, IDS_HELP_MOUSE);

	// Mouse
	AddTextItem(IDS_KEYBOARD, FOLDER_CMD_KEYBOARD, IDS_HELP_KEYBOARD);

	// use joystick
	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	CToggleCtrl* pToggle = AddToggle(IDS_JOYSTICK_USE, IDS_HELP_USEJOYSTICK, kGap, &m_bUseJoystick);
	pToggle->NotifyOnChange(FOLDER_CMD_UPDATE,this);
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);
	pToggle->Enable( (dwAdvancedOptions & AO_JOYSTICK) );


	// joystick
	m_pJoystickCtrl = AddTextItem(IDS_JOYSTICK, FOLDER_CMD_JOYSTICK, IDS_HELP_JOYSTICK);
	m_pJoystickCtrl->Enable( (dwAdvancedOptions & AO_JOYSTICK) );

	//restore
	AddTextItem(IDS_RESTOREDEFAULTS, FOLDER_CMD_RESET_DEFAULTS, IDS_HELP_RESTORE);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	CFolderJoystick *pJoy = (CFolderJoystick *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOYSTICK);
	pJoy->Build();

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CFolderControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_CUSTOM_CONTROLS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CUST_CONTROLS);
			break;
		}
	case FOLDER_CMD_WEAPON_CONTROLS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_WPN_CONTROLS);
			break;
		}
	case FOLDER_CMD_UPDATE:
		{
			UpdateData();
			m_pJoystickCtrl->Enable(m_bUseJoystick);
			if (m_bUseJoystick)
			{
				CFolderJoystick *pJoy = (CFolderJoystick *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOYSTICK);
				if (!pJoy->IsJoystickConfigured())
				{
					HSTRING hString = g_pLTClient->FormatString(IDS_JOYSTICK_UNBOUND);
					g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,ConfirmCallBack,this,LTFALSE);
					g_pLTClient->FreeString(hString);
					m_nConfirm = IDS_JOYSTICK_UNBOUND;
				}					
			}
			break;
		}
	case FOLDER_CMD_MOUSE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MOUSE);
			break;
		}
	case FOLDER_CMD_KEYBOARD:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_KEYBOARD);
			break;
		}
	case FOLDER_CMD_JOYSTICK:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOYSTICK);
			break;
		}
	case FOLDER_CMD_RESET_DEFAULTS:
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_RESTORE);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,ConfirmCallBack,this,LTFALSE);
			g_pLTClient->FreeString(hString);
			m_nConfirm = IDS_CONFIRM_RESTORE;
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};


void CFolderControls::ClearBindings()
{
    if (!g_pLTClient) return;

    uint32 devices[3] =
	{
		DEVICETYPE_KEYBOARD,
		DEVICETYPE_MOUSE,
		DEVICETYPE_JOYSTICK
	};


	for (int i = 0; i < 3; ++i)
	{
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devices[i]);
		if (!pBindings)
		{
			continue;
		}

		char str[128];
		DeviceBinding* ptr = pBindings;
		while (ptr)
		{
			if (ptr->strTriggerName[0] == ';')
				sprintf(str, "rangebind \"%s\" \"##39\" 0 0 \"\"", ptr->strDeviceName);
			else
				sprintf(str, "rangebind \"%s\" \"%s\" 0 0 \"\"", ptr->strDeviceName, ptr->strTriggerName);
            g_pLTClient->RunConsoleString (str);

			ptr = ptr->pNext;
		}

        g_pLTClient->FreeDeviceBindings (pBindings);
	}
}


// Change in focus
void CFolderControls::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	if (bFocus)
	{
	// Load the folder options from the console
		m_bUseJoystick=pSettings->UseJoystick();

		CFolderJoystick *pJoy = (CFolderJoystick *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_JOYSTICK);
		if (!pJoy->IsJoystickConfigured()) m_bUseJoystick = LTFALSE;

		m_pJoystickCtrl->Enable(m_bUseJoystick);

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
		pSettings->SetBoolVar("UseJoystick",m_bUseJoystick);
		if (m_bUseJoystick)
		{
			char strJoystick[128];
			memset (strJoystick, 0, 128);
            LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
			if (result == LT_OK)
			{
				char strConsole[256];
				sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
                g_pLTClient->RunConsoleString (strConsole);
			}
			else
			{
                pSettings->SetBoolVar("UseJoystick",LTFALSE);
			}
		}

		// Just to be safe save the config incase anything changed...

        g_pLTClient->WriteConfigFile("autoexec.cfg");
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderControls::ConfirmSetting(LTBOOL bConfirm)
{
	switch (m_nConfirm)
	{

	case IDS_JOYSTICK_UNBOUND:
		if (bConfirm)
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOYSTICK);
		else
		{
			m_bUseJoystick = LTFALSE;
			UpdateData(LTFALSE);
		}
		break;

	case IDS_CONFIRM_RESTORE:
		if (bConfirm)
		{
			// we need to read the config file twice - since multiple keys can be bound to the
			// same action, we need to clear all bindings before reading in the file.  we need
			// to read it in the first time to make sure it's there and works.
			// if multiple keys are bound to the same action, when we get the bindings again in
			// a subfolder they could be reported in any order and we might end up displaying
			// a mixture of the current bindings and the default bindings.
			LTRESULT result = g_pLTClient->ReadConfigFile ("defctrls.cfg");
			if (result != LT_ERROR)
			{
				ClearBindings();
				g_pLTClient->ReadConfigFile ("defctrls.cfg");
			}
	        g_pLTClient->WriteConfigFile("autoexec.cfg");

		}
		break;
	}

}