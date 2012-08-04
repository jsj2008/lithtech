// MenuSinglePlayer.cpp: implementation of the CMenuSinglePlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "MenuSinglePlayer.h"
#include "BloodClientShell.h"
#include "Splash.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuSinglePlayer::CMenuSinglePlayer()
{
	m_pSaveCtrl=DNULL;
}

CMenuSinglePlayer::~CMenuSinglePlayer()
{

}

// Build the menu
void CMenuSinglePlayer::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\singleplayer.pcx", IDS_MENU_TITLE_SINGLEPLAYER, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);

#ifdef _ADDON
	AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_NIGHTMARES, MENU_CMD_START_NIGHTMARES);
#endif

	AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_CALEB, MENU_CMD_START_STORY);
	AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_CHOSEN, MENU_CMD_START_ACTION);

#ifndef _DEMO
	AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_LOADGAME, MENU_CMD_LOAD_GAME_MENU);

	m_pSaveCtrl = AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_SAVEGAME, MENU_CMD_SAVE_GAME_MENU);
	
	AddLargeTextItemOption(IDS_MENU_SINGLEPLAYER_CUSTOMLEVEL, MENU_CMD_CUSTOM_LEVEL);	
#endif

}

// Enable/Disable the save option
void CMenuSinglePlayer::EnableSave(DBOOL bEnable)
{
	if ( m_pSaveCtrl )
	{
		m_pSaveCtrl->Enable(bEnable);
	}
}

DDWORD CMenuSinglePlayer::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch ( dwCommand )
	{

#ifdef _ADDON
	case MENU_CMD_START_NIGHTMARES:
		{
			// Change the menu
			m_pMainMenus->SetCurrentMenu(MENU_ID_DIFFICULTY);

			// Tell the difficulty menu about the type of game we are starting
			m_pMainMenus->GetDifficultyMenu()->SetNightmaresMode(DTRUE);

			// Set the parent menu		
			m_pMainMenus->GetDifficultyMenu()->SetParentMenu(this);
			break;
		}
#endif

	case MENU_CMD_START_STORY:
		{
			// Change the menu
			m_pMainMenus->SetCurrentMenu(MENU_ID_DIFFICULTY);

			// Tell the difficulty menu about the type of game we are starting
			m_pMainMenus->GetDifficultyMenu()->SetStoryMode(DTRUE);

			// Set the parent menu		
			m_pMainMenus->GetDifficultyMenu()->SetParentMenu(this);
			break;
		}
	case MENU_CMD_START_ACTION:
		{
#ifdef _DEMO
			Splash_SetState(m_pClientDE, "\\screens\\modes.pcx");
#else
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTERSELECT);
#endif
			break;
		}	
	case MENU_CMD_CUSTOM_LEVEL:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_CUSTOM_LEVEL);
			break;
		}	
	case MENU_CMD_LOAD_GAME_MENU:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_LOAD_GAME);
			break;
		}
	case MENU_CMD_SAVE_GAME_MENU:
		{
			m_pMainMenus->SetCurrentMenu(MENU_ID_SAVE_GAME);
			break;
		}
	}
	return 0;
}