// MenuMain.cpp: implementation of the CMenuMain class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MenuMain.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "Splash.h"
#include "VKDefs.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuMain::CMenuMain()
{

}

CMenuMain::~CMenuMain()
{

}

// Build the menu
void CMenuMain::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\mainmenu.pcx", IDS_MENU_TITLE_MAINMENU, m_pMainMenus->GetTitlePos());	
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);	

	AddLargeTextItemOption(IDS_MENU_MAIN_SINGLEPLAYER,	MENU_CMD_SINGLE_PLAYER);
	AddLargeTextItemOption(IDS_MENU_MAIN_BLOODBATH,		MENU_CMD_BLOODBATH);
	AddLargeTextItemOption(IDS_MENU_MAIN_OPTIONS,		MENU_CMD_OPTIONS);

#ifdef _DEMO
	AddLargeTextItemOption(IDS_MENU_MAIN_PRIMEGIBS,		MENU_CMD_CREDITS);
#else
	AddLargeTextItemOption(IDS_MENU_MAIN_CREDITS,		MENU_CMD_CREDITS);
#endif

	AddLargeTextItemOption(IDS_MENU_MAIN_QUIT,			MENU_CMD_QUIT);
}

DDWORD CMenuMain::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{	
	switch(dwCommand)
	{
	case MENU_CMD_SINGLE_PLAYER:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_SINGLEPLAYER);
			break;
		}
	case MENU_CMD_BLOODBATH:
		{
#ifdef _DEMO
			Splash_SetState(m_pClientDE, "\\screens\\bloodbath.pcx");
#else
			m_pMainMenus->SetCurrentMenu(MENU_ID_BLOODBATH);
#endif
			break;
		}
	case MENU_CMD_OPTIONS:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_OPTIONS);
			break;
		}
	case MENU_CMD_CREDITS:
		{
#ifdef _DEMO
			Splash_SetState(m_pClientDE, "\\screens\\info.pcx");
#else
			g_pBloodClientShell->MenuCredits();
#endif
			break;
		}
	case MENU_CMD_QUIT:
		{
#ifdef _DEMO
			Splash_SetState(m_pClientDE, "\\screens\\info.pcx", DTRUE);
#else					
			// Show the confirmation dialogbox
			m_pMainMenus->DoMessageBox(IDS_MENU_MESSAGE_QUIT, this);
			m_pMainMenus->AddMessageKey(m_pMainMenus->GetYesVKeyCode(), MENU_CMD_EXIT);
			m_pMainMenus->AddMessageKey(m_pMainMenus->GetNoVKeyCode(), MENU_CMD_KILL_MESSAGEBOX);
			m_pMainMenus->AddMessageKey(VK_ESCAPE, MENU_CMD_KILL_MESSAGEBOX);						
#endif
			break;
		}
	case MENU_CMD_EXIT:
		{
			m_pMainMenus->KillMessageBox();
			g_pBloodClientShell->MenuQuit();
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