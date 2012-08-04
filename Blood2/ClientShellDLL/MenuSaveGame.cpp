// MenuSaveGame.cpp: implementation of the CMenuSaveGame class.
//
//////////////////////////////////////////////////////////////////////

#include "LTGUIMgr.h"
#include "MenuLoadGame.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuSaveGame::CMenuSaveGame()
{

}

CMenuSaveGame::~CMenuSaveGame()
{

}

// Build the menu
void CMenuSaveGame::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\savegame.pcx", IDS_MENU_TITLE_SAVEGAME, m_pMainMenus->GetTitlePos());
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);		
}

// Add the menu items
void CMenuSaveGame::InitSavedGameSlots()
{		
	// Remove all of the menu options
	RemoveAllOptions();

	// Save game info
	CSavedGameInfo *pSGInfo = DNULL;
	pSGInfo = g_pBloodClientShell->GetSavedGameInfo();

	// Add the save game slots
	if ( pSGInfo )
	{
		int i;
		for (i=0; i < MAX_SAVESLOTS; i++)
		{
			HSTRING hString=m_pClientDE->CreateString(pSGInfo->gSaveSlotInfo[i].szName);
			AddTextItemOption(hString, MENU_CMD_SAVE_GAME, m_pMainMenus->GetSmallFont());	
			m_pClientDE->FreeString(hString);
		}
	}	
}

DDWORD CMenuSaveGame::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_SAVE_GAME:
		{
			int nIndex=m_listOption.GetSelectedItem();
			
			// Save the saved game
			g_pBloodClientShell->MenuSaveGame(nIndex);
			break;
		}
	}
	return 0;
}