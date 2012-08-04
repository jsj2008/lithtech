// MenuLoadGame.cpp: implementation of the CMenuLoadGame class.
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

CMenuLoadGame::CMenuLoadGame()
{

}

CMenuLoadGame::~CMenuLoadGame()
{

}

// Build the menu
void CMenuLoadGame::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\loadgame.pcx", IDS_MENU_TITLE_LOADGAME, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);		
}

// Add the menu items
void CMenuLoadGame::InitSavedGameSlots()
{		
	// Remove all of the menu options
	RemoveAllOptions();

	// Save game info
	CSavedGameInfo *pSGInfo = DNULL;
	pSGInfo = g_pBloodClientShell->GetSavedGameInfo();
	
	// This is "RESTART: %s" in English
	HSTRING hRestartFormat=m_pClientDE->FormatString(IDS_MENU_LOADGAME_RESTART, pSGInfo->gCurrentSaveInfo.szName);			
	AddTextItemOption(hRestartFormat, MENU_CMD_LOAD_SAVE_GAME, m_pMainMenus->GetSmallFont());		
	m_pClientDE->FreeString(hRestartFormat);

	// This is "QUICKLOAD: %s" in English
	HSTRING hQuickLoadFormat=m_pClientDE->FormatString(IDS_MENU_LOADGAME_QUICKLOAD, pSGInfo->gQuickSaveInfo.szName);		
	AddTextItemOption(hQuickLoadFormat, MENU_CMD_LOAD_SAVE_GAME, m_pMainMenus->GetSmallFont());	
	m_pClientDE->FreeString(hQuickLoadFormat);

	// Add the save game slots
	if ( pSGInfo )
	{
		int i;
		for (i=0; i < MAX_SAVESLOTS; i++)
		{			
			HSTRING hString=m_pClientDE->CreateString(pSGInfo->gSaveSlotInfo[i].szName);
			AddTextItemOption(hString, MENU_CMD_LOAD_SAVE_GAME, m_pMainMenus->GetSmallFont());	
			m_pClientDE->FreeString(hString);
		}
	}	
}

DDWORD CMenuLoadGame::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_LOAD_SAVE_GAME:
		{
			int nIndex=m_listOption.GetSelectedItem();

			// Special case quickload and restart last level
			if ( nIndex == 0 )
			{
				g_pBloodClientShell->MenuLoadGame(SLOT_CURRENT);
			}
			else if ( nIndex == 1 )
			{
				g_pBloodClientShell->MenuLoadGame(SLOT_QUICK);
			}
			else
			{
				// Load the saved game
				g_pBloodClientShell->MenuLoadGame(nIndex-2);
			}
			break;
		}
	}
	return 0;
}