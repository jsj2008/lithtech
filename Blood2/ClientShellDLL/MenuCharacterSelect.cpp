// MenuCharacterSelect.cpp: implementation of the CMenuCharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "SharedDefs.h"
#include "MenuCommands.h"
#include "MenuCharacterSelect.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuCharacterSelect::CMenuCharacterSelect()
{

}

CMenuCharacterSelect::~CMenuCharacterSelect()
{

}

// Build the menu
void CMenuCharacterSelect::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\singleplayer.pcx", IDS_MENU_TITLE_SINGLEPLAYER, m_pMainMenus->GetTitlePos());	
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);

	AddLargeTextItemOption(IDS_MENU_CHARSELECT_CALEB, MENU_CMD_START_CALEB);
	AddLargeTextItemOption(IDS_MENU_CHARSELECT_GABRIELLA, MENU_CMD_START_GABRIELLA);
	AddLargeTextItemOption(IDS_MENU_CHARSELECT_ISHMAEL, MENU_CMD_START_ISHMAEL);
	AddLargeTextItemOption(IDS_MENU_CHARSELECT_OPHELIA, MENU_CMD_START_OPHELIA);
}

// Switches to the difficulty menu
void CMenuCharacterSelect::SwitchToDifficultyMenu(int nCharacter)
{
	// Change the menu
	m_pMainMenus->SetCurrentMenu(MENU_ID_DIFFICULTY);

	// Tell the difficulty menu about the type of game we are starting
	m_pMainMenus->GetDifficultyMenu()->SetStoryMode(DFALSE);
	m_pMainMenus->GetDifficultyMenu()->SetCharacter(nCharacter);

	// Set the parent menu
	m_pMainMenus->GetDifficultyMenu()->SetParentMenu(this);
}

DDWORD CMenuCharacterSelect::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_START_CALEB:
		{
			SwitchToDifficultyMenu(CHARACTER_CALEB);
			break;
		}
	case MENU_CMD_START_GABRIELLA:
		{
			SwitchToDifficultyMenu(CHARACTER_GABREILLA);
			break;
		}
	case MENU_CMD_START_ISHMAEL:
		{
			SwitchToDifficultyMenu(CHARACTER_ISHMAEL);
			break;
		}
	case MENU_CMD_START_OPHELIA:
		{
			SwitchToDifficultyMenu(CHARACTER_OPHELIA);
			break;
		}
	}
	return 0;
}
