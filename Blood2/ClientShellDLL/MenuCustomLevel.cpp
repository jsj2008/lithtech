// MenuCustomLevel.cpp: implementation of the CMenuCustomLevel class.
//
//////////////////////////////////////////////////////////////////////

#include "LTGUIMgr.h"
#include "MenuCustomLevel.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "LTGUIMgr.h"
#include "BloodClientShell.h"
#include "VKDefs.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuCustomLevel::CMenuCustomLevel()
{

}

CMenuCustomLevel::~CMenuCustomLevel()
{

}

// Build the menu
void CMenuCustomLevel::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\singleplayer.pcx", IDS_MENU_TITLE_SINGLEPLAYER, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);
	SetScrollWrap(DFALSE);

	// Add the custom levels to the menu
	InitCustomLevels();
}

// The focus has changed
void CMenuCustomLevel::OnFocus(DBOOL bFocus)
{
	if (bFocus)
	{
		if (m_listOption.GetNum() == 0)
		{
			// Display a message box and leave the custom levels menu
			m_pMainMenus->DoMessageBox(IDS_MENU_MESSAGE_NO_CUSTOM_LEVELS, this);			
			m_pMainMenus->AddMessageKey(VK_ESCAPE, MENU_CMD_KILL_MESSAGEBOX);
			m_pMainMenus->AddMessageKey(VK_SPACE, MENU_CMD_KILL_MESSAGEBOX);
		}
	}
}

// Initialize the custom levels
void CMenuCustomLevel::InitCustomLevels()
{
	// Remove all of the menu options
	RemoveAllOptions();

	// Add the custom levels
	AddCustomLevels("", "");

	HCONSOLEVAR hVar=m_pClientDE->GetConsoleVar("EnableRetailLevels");
	if (hVar && m_pClientDE->GetVarValueFloat(hVar) == 1.0f)
	{
		AddCustomLevels("Worlds", "");
		AddCustomLevels("Worlds\\Test", "\\test\\");
		AddCustomLevels("Worlds\\Multi", "\\multi\\");
#ifdef _ADDON
		AddCustomLevels("Worlds_ao", "\\Worlds_ao\\");
		AddCustomLevels("Worlds_ao\\Multi_ao", "\\multi_ao\\");
#endif // _ADDON
	}		
}

// Add custom levels to the menu
void CMenuCustomLevel::AddCustomLevels(char *lpszPath, char *lpszItemPrefix)
{
	FileEntry *pfe, *pfindex;

	// Get the files
	pfe = m_pClientDE->GetFileList(lpszPath);
	if (!pfe)
	{
		return;
	}

	// Sort the list
	pfe = pfindex = SortFileList(pfe);	
	
	while(pfindex)
	{
		// Conver to lower case
		_strlwr(pfindex->m_pBaseFilename);

		// Only add .dat files
		if (_mbsstr((const unsigned char*)pfindex->m_pBaseFilename, (const unsigned char*)".dat"))
		{
			char szString[512];
			sprintf(szString, "%s%s", lpszItemPrefix, pfindex->m_pBaseFilename);

			// Remove the .dat from the string
			char szString2[512];
			memset(szString2, NULL, sizeof(szString2));
			_mbsncpy((unsigned char*)szString2, (const unsigned char*)szString, _mbstrlen(szString)-4);

			HSTRING hString=m_pClientDE->CreateString(szString2);
			AddTextItemOption(hString, MENU_CMD_CUSTOM_LEVEL_LOAD, m_pMainMenus->GetSmallFont());
			m_pClientDE->FreeString(hString);
		}

		// Next item
		pfindex = pfindex->m_pNext;
	}

	// Free the file list
	m_pClientDE->FreeFileList(pfe);
}

DDWORD CMenuCustomLevel::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_CUSTOM_LEVEL_LOAD:
		{
			CLTGUITextItemCtrl *pCtrl=(CLTGUITextItemCtrl *)m_listOption.GetControl(m_listOption.GetSelectedItem());
			if ( pCtrl )
			{
				// Display a "please wait" message
				m_pMainMenus->ShowSyncMessage(IDS_MENU_MESSAGE_PLEASEWAIT);

				// Load the custom level from the string				
				g_pBloodClientShell->MenuLoadCustomLevel(m_pClientDE->GetStringData(pCtrl->GetString(0)), DIFFICULTY_EASY);
			}

			break;
		}
	case MENU_CMD_KILL_MESSAGEBOX:
		{
			m_pMainMenus->KillMessageBox();
			m_pMainMenus->SetCurrentMenu(MENU_ID_SINGLEPLAYER);
			break;
		}
	}
	return DTRUE;
}
