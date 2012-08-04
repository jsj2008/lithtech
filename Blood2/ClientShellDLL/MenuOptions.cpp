// MenuOptions.cpp: implementation of the CMenuOptions class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "MenuOptions.h"
#include "MenuCommands.h"
#include "ClientRes.h"
#include "vkdefs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuOptions::CMenuOptions()
{

}

CMenuOptions::~CMenuOptions()
{

}

// Build the menu
void CMenuOptions::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);

	AddLargeTextItemOption(IDS_MENU_OPTIONS_DISPLAY,	MENU_CMD_DISPLAY);
	AddLargeTextItemOption(IDS_MENU_OPTIONS_AUDIO,		MENU_CMD_AUDIO);
	AddLargeTextItemOption(IDS_MENU_OPTIONS_CONTROLS,	MENU_CMD_CUSTOMIZE_CONTROLS);
	AddLargeTextItemOption(IDS_MENU_OPTIONS_MOUSE,		MENU_CMD_MOUSE);
	AddLargeTextItemOption(IDS_MENU_OPTIONS_JOYSTICK,	MENU_CMD_JOYSTICK);
	AddLargeTextItemOption(IDS_MENU_OPTIONS_KEYBOARD,	MENU_CMD_KEYBOARD);
}

DDWORD CMenuOptions::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_CUSTOMIZE_CONTROLS:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_CONTROLS);
			break;
		}
	case  MENU_CMD_AUDIO:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_SOUND);
			break;
		}
	case MENU_CMD_DISPLAY:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_DISPLAY);
			break;
		}
	case MENU_CMD_MOUSE:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_MOUSE);
			break;
		}
	case MENU_CMD_KEYBOARD:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_KEYBOARD);
			break;
		}
	case MENU_CMD_JOYSTICK:
		{
			BOOL bJoystickMenuDisabled = FALSE;
			HCONSOLEVAR hVar;
			hVar = m_pClientDE->GetConsoleVar( "JoystickMenuDisabled");
			if (hVar != NULL) 
			{
				if (m_pClientDE->GetVarValueFloat(hVar) == 1) bJoystickMenuDisabled = DTRUE;
			}

			// Show the confirmation dialogbox
			if (!bJoystickMenuDisabled) m_pMainMenus->SetCurrentMenu(MENU_ID_JOYSTICK);

			// display dialog box for joystick menu disbled
			else
			{
				m_pMainMenus->DoMessageBox(IDS_MENU_OPTIONS_NOJOYSTICKMENU, this);
				m_pMainMenus->AddMessageKey(VK_RETURN, MENU_CMD_KILL_MESSAGEBOX);
				m_pMainMenus->AddMessageKey(VK_SPACE, MENU_CMD_KILL_MESSAGEBOX);
				m_pMainMenus->AddMessageKey(VK_ESCAPE, MENU_CMD_KILL_MESSAGEBOX);						
			}

			break;
		}
	case MENU_CMD_KILL_MESSAGEBOX:
		{
			m_pMainMenus->KillMessageBox();
			break;
		}
	}
	return 0;
}