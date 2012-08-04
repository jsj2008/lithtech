// FolderCustomLevel.cpp: implementation of the CFolderCustomLevel class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderCustomLevel.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCustomLevel::CFolderCustomLevel()
{
    m_pFilenames = LTNULL;
	m_nFiles = 0;
}

CFolderCustomLevel::~CFolderCustomLevel()
{
	if (m_pFilenames)
	{
		for (int i = 0; i < m_nFiles; i++)
		{
			if (m_pFilenames[i]) debug_deletea(m_pFilenames[i]);
		}
		debug_deletea(m_pFilenames);
	}

}

// Build the folder
LTBOOL CFolderCustomLevel::Build()
{
	CreateTitle(IDS_TITLE_CUSTOMLEVELS);

    LTBOOL bUseRetailLevels = LTFALSE;

	// Get a list of world names and sort them alphabetically

    FileEntry* pFiles = g_pLTClient->GetFileList("\\");
	m_nFiles = CountDatFiles(pFiles);

#ifndef _DEMO
#ifndef _FINAL

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("EnableRetailLevels");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar) == 1.0f)
	{
        bUseRetailLevels = LTTRUE;
	}
    uint8 nNumPaths = g_pClientButeMgr->GetNumSingleWorldPaths();

    bUseRetailLevels = bUseRetailLevels ? !!(nNumPaths > 0) : LTFALSE;

    FileEntry** pFilesArray = LTNULL;

	char pathBuf[128];

	if (bUseRetailLevels)
	{
		pFilesArray = debug_newa(FileEntry*, nNumPaths);

		if (pFilesArray)
		{
			for (int i=0; i < nNumPaths; ++i)
			{
				pathBuf[0] = '\0';
				g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf));

				if (pathBuf[0])
				{
                    pFilesArray[i] = g_pLTClient->GetFileList(pathBuf);
					m_nFiles += CountDatFiles(pFilesArray[i]);
				}
				else
				{
                    pFilesArray[i] = LTNULL;
				}
			}
		}
	}
#endif
#endif

	m_pFilenames = debug_newa(char*, m_nFiles);
	if (!m_pFilenames)
	{
        g_pLTClient->FreeFileList (pFiles);
        return LTFALSE;
	}
	memset (m_pFilenames, 0, m_nFiles * sizeof(char*));

	int nIndex = 0;
	AddFilesToFilenames(pFiles, "", nIndex);
    g_pLTClient->FreeFileList(pFiles);

#ifndef _DEMO
#ifndef _FINAL
	if (bUseRetailLevels)
	{
		char Buf[255];

		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf));

			if (pathBuf[0] && pFilesArray[i])
			{
				sprintf(Buf, "%s\\", pathBuf);
				AddFilesToFilenames(pFilesArray[i], Buf, nIndex);
                g_pLTClient->FreeFileList(pFilesArray[i]);
			}
		}

		debug_deletea(pFilesArray);
	}
#endif
#endif

	// now sort the array (shellsort)

	for (int i = nIndex / 2; i > 0; i = (i == 2) ? 1 : (int) (i / 2.2))
	{
		for (int j = i; j < nIndex; j++)
		{
			char* pTemp = m_pFilenames[j];

			for (int k = j; k >= i && stricmp (pTemp, m_pFilenames[k - i]) < 0; k -= i)
			{
				m_pFilenames[k] = m_pFilenames[k - i];
			}

			m_pFilenames[k] = pTemp;
		}
	}

	CLTGUITextItemCtrl* pItem;

	for (i = 0; i < m_nFiles; ++i)
	{
		pItem = AddTextItem(m_pFilenames[i], FOLDER_CMD_CUSTOM+i, IDS_HELP_CUSTOMLEVEL, LTFALSE, GetSmallFont());
	}

	// Make sure to call the base class
	return CBaseFolder::Build();
}


void CFolderCustomLevel::OnFocus(LTBOOL bFocus)
{
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderCustomLevel::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= FOLDER_CMD_CUSTOM)
	{
		char strWorld[256];
		DWORD selectNum = dwCommand-FOLDER_CMD_CUSTOM;
		SAFE_STRCPY(strWorld, m_pFilenames[selectNum]);

		//clear out inventory and any mission related data
		g_pInterfaceMgr->GetPlayerStats()->ResetInventory();
		CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
		if (pMissionData)
		{
			pMissionData->Clear();
		}

		//check to see if we're playing multiplayer and disable the diconnected warning if we are
		if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->GetGameType() != SINGLE)
			g_pInterfaceMgr->StartingNewGame();

		if (g_pGameClientShell->LoadWorld(strWorld))
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
	}
	else
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

int CFolderCustomLevel::CountDatFiles(FileEntry* pFiles)
{
	if (!pFiles) return 0;

	int nNum = 0;
	FileEntry* ptr = pFiles;
	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			if (strnicmp (&ptr->m_pBaseFilename[strlen(ptr->m_pBaseFilename) - 4], ".dat", 4) == 0) nNum++;
		}
		ptr = ptr->m_pNext;
	}

	return nNum;
}

void CFolderCustomLevel::AddFilesToFilenames(FileEntry* pFiles, char* pPath, int & nIndex)
{
	if (!pFiles || !pPath) return;

	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	FileEntry* ptr = pFiles;

	while (ptr && nIndex < m_nFiles)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
			pBaseName = strtok (strBaseName, ".");
			pBaseExt = strtok (NULL, "\0");
			if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
			{
				char szString[512];
				sprintf(szString, "%s%s", pPath, pBaseName);

				// add this to the array

				m_pFilenames[nIndex] = debug_newa(char, strlen(szString) + 1);
				SAFE_STRCPY(m_pFilenames[nIndex], szString);
				nIndex++;
			}
		}

		ptr = ptr->m_pNext;
	}
}


// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CFolderCustomLevel::HandleKeyDown(int key, int rep)
{
    LTBOOL handled = LTFALSE;

	switch (key)
	{
	case VK_LEFT:
		{
			handled = OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			handled = OnRight();
			break;
		}
	case VK_UP:
		{
			handled = OnUp();
			break;
		}
	case VK_DOWN:
		{
			handled = OnDown();
			break;
		}
	case VK_RETURN:
		{
			handled = OnEnter();
			break;
		}
	case VK_TAB:
		{
			handled = OnTab();
			break;
		}
	case VK_PRIOR:
		{
			handled = OnPageUp();
			break;
		}
	case VK_NEXT:
		{
			handled = OnPageDown();
			break;
		}
	default:
		{
            handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}

