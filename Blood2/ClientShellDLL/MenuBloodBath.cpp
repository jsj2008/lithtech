// MenuBloodBath.cpp: implementation of the CMenuBloodBath class.
//
//////////////////////////////////////////////////////////////////////

#include "MainMenus.h"
#include "MenuCommands.h"
#include "MenuBloodBath.h"
#include "BloodClientShell.h"
#include "VKDefs.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuBloodBath::CMenuBloodBath()
{

}

CMenuBloodBath::~CMenuBloodBath()
{

}

// Builds the menu
void CMenuBloodBath::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\bloodbath.pcx", IDS_MENU_TITLE_BLOODBATH, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);

	AddLargeTextItemOption(IDS_MENU_BLOODBATH_JOIN, MENU_CMD_JOIN_GAME);
	AddLargeTextItemOption(IDS_MENU_BLOODBATH_HOST, MENU_CMD_HOST_GAME);
	AddLargeTextItemOption(IDS_MENU_BLOODBATH_CHARSETUP, MENU_CMD_CHARACTER_SETUP_RESVERIFY);
}

// Verifies that the current resolution is high enough for the character setup screen
void CMenuBloodBath::DoResolutionVerify()
{
	RMode currentMode;
	m_pClientDE->GetRenderMode(&currentMode);

	if (currentMode.m_Width < 640 || currentMode.m_Height < 480)
	{
		// Display a message box asking if they would like to switch resolutions
		m_pMainMenus->DoMessageBox(IDS_MENU_MESSAGE_ASK_CHANGE_RESOLUTION, this);
		m_pMainMenus->AddMessageKey(m_pMainMenus->GetYesVKeyCode(), MENU_CMD_CHARACTER_SETUP_SWITCH_RUN);
		m_pMainMenus->AddMessageKey(m_pMainMenus->GetNoVKeyCode(), MENU_CMD_KILL_MESSAGEBOX);
		m_pMainMenus->AddMessageKey(VK_ESCAPE, MENU_CMD_KILL_MESSAGEBOX);
	}
	else
	{
		// Tell the character setup screen not to ask about switching resolutions back
		m_pMainMenus->GetCharacterSetup()->SetResolutionSwitch(DFALSE, 640, 480);

		SendCommand(MENU_CMD_CHARACTER_SETUP, 0, 0);
	}
}

// Switches video resolutions to 640x480
DBOOL CMenuBloodBath::SwitchTo640x480()
{	
	RMode currentMode;
	m_pClientDE->GetRenderMode(&currentMode);
	int nOldScreenWidth=currentMode.m_Width;
	int nOldScreenHeight=currentMode.m_Height;

	if (!m_pMainMenus->SwitchResolution(640, 480))
	{
		m_pMainMenus->DoMessageBox(IDS_MENU_MESSAGE_RES_CHANGE_ERROR, this);
		m_pMainMenus->AddMessageKey(VK_RETURN, MENU_CMD_KILL_MESSAGEBOX);
		
		return DFALSE;
	}

	// Tell the character setup screen to ask about switching resolutions back
	m_pMainMenus->GetCharacterSetup()->SetResolutionSwitch(DTRUE, nOldScreenWidth, nOldScreenHeight);

	return DTRUE;
}

DDWORD CMenuBloodBath::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_HOST_GAME:
		{
			g_pBloodClientShell->MenuHostGame();
			break;
		}
	case MENU_CMD_JOIN_GAME:
		{
			g_pBloodClientShell->MenuJoinGame();
			break;
		}
	case MENU_CMD_CHARACTER_SETUP_RESVERIFY:
		{
			DoResolutionVerify();
			break;
		}
	case MENU_CMD_KILL_MESSAGEBOX:
		{
			m_pMainMenus->KillMessageBox();
			break;
		}
	case MENU_CMD_CHARACTER_SETUP_SWITCH_RUN:
		{
			m_pMainMenus->KillMessageBox();
			if (SwitchTo640x480())
			{
				m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTER);
			}
			break;
		}
	case MENU_CMD_CHARACTER_SETUP:
		{			
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTER);
			break;
		}
	default:
		{
			assert(DFALSE);
			break;
		}
	}
	return 0;
}