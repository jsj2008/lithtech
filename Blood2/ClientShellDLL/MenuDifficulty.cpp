// MenuDifficulty.cpp: implementation of the CMenuDifficulty class.
//
//////////////////////////////////////////////////////////////////////

#include "MainMenus.h"
#include "MenuCommands.h"
#include "MenuDifficulty.h"
#include "BloodClientShell.h"
#include "SharedDefs.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuDifficulty::CMenuDifficulty()
{
	m_bStoryMode  = DTRUE;
	m_bNightmares = DFALSE;
}

CMenuDifficulty::~CMenuDifficulty()
{

}

// Build the menu
void CMenuDifficulty::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\singleplayer.pcx", IDS_MENU_TITLE_SINGLEPLAYER, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);

	AddLargeTextItemOption(IDS_MENU_DIFFICULTY_GENOCIDE, MENU_CMD_DIFFICULTY_EASY);
	AddLargeTextItemOption(IDS_MENU_DIFFICULTY_HOMICIDE, MENU_CMD_DIFFICULTY_MEDIUM);
	AddLargeTextItemOption(IDS_MENU_DIFFICULTY_SUICIDE, MENU_CMD_DIFFICULTY_HARD);

	// Select medium difficulty
	SetCurrentItem(1);
}

// Starts the game
void CMenuDifficulty::StartGame(DDWORD dwDifficulty)
{
	// Start a nightmares game if requested
#ifdef _ADDON
	if (m_bNightmares)
	{
		g_pBloodClientShell->MenuNewNightmaresGame(dwDifficulty);
		return;
	}
#endif

	// Start a story mode game
	if (m_bStoryMode)
	{
		g_pBloodClientShell->MenuNewGame(dwDifficulty);
	}
	else
	{
		// Start a game in action mode
		g_pBloodClientShell->MenuNewGame(dwDifficulty, GetCharacter(), GAMETYPE_ACTION);
	}
}

DDWORD CMenuDifficulty::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{	
	switch (dwCommand)
	{	
	case MENU_CMD_DIFFICULTY_EASY:
		{
			StartGame(DIFFICULTY_EASY);
			break;
		}
	case MENU_CMD_DIFFICULTY_MEDIUM:
		{
			StartGame(DIFFICULTY_MEDIUM);
			break;
		}
	case MENU_CMD_DIFFICULTY_HARD:
		{
			StartGame(DIFFICULTY_HARD);
			break;
		}
	}
	return 0;
}
