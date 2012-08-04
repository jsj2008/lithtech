#include "clientheaders.h"
#include "LoadLevelMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotClientShell.h"
#include "RiotMenu.h"
#include "Font08.h"

#define TITLE_SPACING_MULTIPLIER	3

CLoadLevelMenu::~CLoadLevelMenu()
{
	if (m_pFilenames)
	{
		for (int i = 0; i < m_nFiles; i++)
		{
			if (m_pFilenames[i]) delete [] m_pFilenames[i];
		}
		delete [] m_pFilenames;
	}

	if (m_pSurfaces) delete [] m_pSurfaces;
	if (m_pSurfacesSelected) delete [] m_pSurfacesSelected;
	if (m_szSurfaces) delete [] m_szSurfaces;
}

LTBOOL CLoadLevelMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	LTBOOL bUseRetailLevels = LTFALSE;

	HCONSOLEVAR hVar = pClientDE->GetConsoleVar("EnableRetailLevels");
	if (hVar && pClientDE->GetVarValueFloat(hVar) == 1.0f)
	{
		bUseRetailLevels = LTTRUE;
	}

	// get a list of world names and sort them alphabetically

	FileEntry* pFiles = pClientDE->GetFileList("\\");
	m_nFiles = CountDatFiles(pFiles);

	FileEntry* pWorldFiles = LTNULL;
	FileEntry* pMultiFiles = LTNULL;
	if (bUseRetailLevels)
	{
		pWorldFiles = pClientDE->GetFileList("Worlds");
		m_nFiles += CountDatFiles(pWorldFiles);

		pMultiFiles = pClientDE->GetFileList("Worlds\\Multi");
		m_nFiles += CountDatFiles(pMultiFiles);
	}

	m_pFilenames = new char* [m_nFiles];
	if (!m_pFilenames)
	{
		pClientDE->FreeFileList (pFiles);
		return LTFALSE;
	}
	memset (m_pFilenames, 0, m_nFiles * sizeof(char*));
	
	int nIndex = 0;
	AddFilesToFilenames(pFiles, "", nIndex);
	pClientDE->FreeFileList(pFiles);

	if (bUseRetailLevels && pWorldFiles)
	{
		AddFilesToFilenames(pWorldFiles, "Worlds\\", nIndex);
		pClientDE->FreeFileList(pWorldFiles);

		AddFilesToFilenames(pMultiFiles, "Worlds\\Multi\\", nIndex);
		pClientDE->FreeFileList(pMultiFiles);
	}

	// now sort the array (shellsort)

	for (int i = nIndex / 2; i > 0; i = (i == 2) ? 1 : (int) (i / 2.2))
	{
		for (int j = i; j < nIndex; j++)
		{
			char* pTemp = m_pFilenames[j];
			
			int k;
			for (k = j; k >= i && stricmp (pTemp, m_pFilenames[k - i]) < 0; k -= i)
			{
				m_pFilenames[k] = m_pFilenames[k - i];
			}

			m_pFilenames[k] = pTemp;
		}
	}

	// allocate array of surfaces and surface sizes

	m_pSurfaces = new HSURFACE [m_nFiles];
	if (!m_pSurfaces) return LTFALSE;
	memset (m_pSurfaces, 0, sizeof(HSURFACE) * m_nFiles);

	m_pSurfacesSelected = new HSURFACE [m_nFiles];
	if (!m_pSurfacesSelected) return LTFALSE;
	memset (m_pSurfacesSelected, 0, sizeof(HSURFACE) * m_nFiles);

	m_szSurfaces = new CSize [m_nFiles];
	if (!m_szSurfaces) return LTFALSE;
	memset (m_szSurfaces, 0, sizeof(CSize) * m_nFiles);

	// now call the base class Init() function

	return CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);
}

int CLoadLevelMenu::CountDatFiles(FileEntry* pFiles)
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

void CLoadLevelMenu::AddFilesToFilenames(FileEntry* pFiles, char* pPath, int & nIndex)
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

				m_pFilenames[nIndex] = new char [strlen (szString) + 1];
				SAFE_STRCPY(m_pFilenames[nIndex], szString);
				nIndex++;
			}
		}
		
		ptr = ptr->m_pNext;
	}
}

void CLoadLevelMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CLoadLevelMenu::Up()
{
	if (m_nSelection == 0)
	{
		End();
		return;
	}

	m_nSelection--;
	CheckSelectionOffMenuTop();
	
	PlayUpSound();
}

void CLoadLevelMenu::Down()
{
	if (m_nSelection == m_nFiles - 1)
	{
		Home();
		return;
	}

	m_nSelection++;
	CheckSelectionOffMenuBottom();

	PlayDownSound();
}

void CLoadLevelMenu::PageUp()
{
	CBaseMenu::PageUp();
}

void CLoadLevelMenu::PageDown()
{
	CBaseMenu::PageDown();
}

void CLoadLevelMenu::Home()
{
	if (m_nSelection == 0) return;

	m_nTopItem = 0;
	m_nSelection = 0;

	PlayHomeSound();
}

void CLoadLevelMenu::End()
{
	if (m_nSelection == m_nFiles - 1) return;

	m_nSelection = m_nFiles - 1;
	
	// determine what the top item would be...

	m_nTopItem = 0;
	int nTopY = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	int nCurrentY = GetMenuAreaBottom() - m_szSurfaces[m_nSelection].cy;
	for (int i = m_nSelection - 1; i >= 0; i--)
	{
		nCurrentY -= m_nMenuSpacing + m_szSurfaces[i].cy;
		if (nCurrentY < nTopY)
		{
			m_nTopItem = i + 1;
			break;
		}
	}
	
	PlayEndSound();
}

void CLoadLevelMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CBaseMenu::Return();
	
	char strWorld[256];
	SAFE_STRCPY(strWorld, m_pFilenames[m_nSelection]);

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return;

	if (pClientShell->LoadWorld(strWorld))
	{
		Reset();
		m_pRiotMenu->ExitMenu (LTTRUE);
	}
}

void CLoadLevelMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	int y = m_nMenuY;
	if (m_hMenuTitle)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMenuTitle, LTNULL, m_nMenuX, y, LTNULL);
		y += m_szMenuTitle.cy + m_nMenuTitleSpacing;
	}
	
	if (m_nTopItem > 0)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetUpArrow(), LTNULL, m_nMenuX + 2, y - m_pRiotMenu->GetArrowHeight() - 3, LTNULL);
	}

	LTBOOL bDrawDownArrow = LTFALSE;
	for (int i = m_nTopItem; i < m_nFiles; i++)
	{
		if (m_nSelection == i)
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pSurfacesSelected[i], LTNULL, m_nMenuX, y, LTNULL);
		}
		else
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pSurfaces[i], LTNULL, m_nMenuX, y, LTNULL);
		}

		y += m_szSurfaces[i].cy + m_nMenuSpacing;
		if (y > GetMenuAreaBottom() - (int)m_szSurfaces[i].cy)
		{
			if (i < m_nFiles - 1)
			{
				bDrawDownArrow = LTTRUE;
			}
			break;
		}
	}

	if (bDrawDownArrow)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetDownArrow(), LTNULL, m_nMenuX + 2, y, LTNULL);
	}
}

LTBOOL CLoadLevelMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return LTFALSE;

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	for (int i = 0; i < m_nFiles; i++)
	{
		char strNiceName[128];
		SAFE_STRCPY(strNiceName, m_pFilenames[i]);
		pClientShell->GetNiceWorldName (m_pFilenames[i], strNiceName, 127);
		
		m_pSurfaces[i] = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strNiceName);
		m_pSurfacesSelected[i] = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strNiceName);
		if (!m_pSurfaces || !m_pSurfacesSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
		
		// crop the top surface...

		if (i == 0)
		{
			m_pSurfaces[0] = CropMenuItemTop (m_pSurfaces[i]);
			m_pSurfacesSelected[0] = CropMenuItemTop (m_pSurfacesSelected[i]);
		}

		m_pClientDE->GetSurfaceDims (m_pSurfaces[i], &m_szSurfaces[i].cx, &m_szSurfaces[i].cy);
	}

	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_CUSTOMLEVELS);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	return CBaseMenu::LoadSurfaces();
}

void CLoadLevelMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < m_nFiles; i++)
	{
		if (m_pSurfaces[i]) m_pClientDE->DeleteSurface (m_pSurfaces[i]);
		if (m_pSurfacesSelected[i]) m_pClientDE->DeleteSurface (m_pSurfacesSelected[i]);
		m_pSurfaces[i] = LTNULL;
		m_pSurfacesSelected[i] = LTNULL;
		m_szSurfaces[i].cx = m_szSurfaces[i].cy = 0;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CLoadLevelMenu::PostCalculateMenuDims()
{
	// get the total height of the menu, without any spacing as well as maximum width

	int nMenuHeight = 0;
	int nMenuMaxWidth = 0;
	for (int i = 0; i < m_nFiles; i++)
	{
		nMenuHeight += m_szSurfaces[i].cy;
		if ((int)m_szSurfaces[i].cx > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_szSurfaces[i].cx;
		}
	}

	// determine correct spacing
	// if we can _just_ fit without any spacing, set spacing to default value

	LTBOOL bOffScreen = LTFALSE;
	if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nFiles - 1) * MAX_MENU_SPACING) > m_szMenuArea.cy &&
		nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nFiles - 1) * MIN_MENU_SPACING) <= m_szMenuArea.cy)
	{
		for (int i = MIN_MENU_SPACING; i < MAX_MENU_SPACING; i++)
		{
			if (nMenuHeight + m_szMenuTitle.cy + ((m_nFiles - 1) * (i + 1)) > m_szMenuArea.cy)
			{
				m_nMenuSpacing = i;
				m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
				break;
			}
		}
	}
	else if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nFiles - 1) * MIN_MENU_SPACING) > m_szMenuArea.cy)
	{
		bOffScreen = LTTRUE;
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}
	else
	{
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}

	// determine correct placement

	m_nMenuX = 0;
	//if (m_pRiotMenu->InWorld() || m_szScreen.cx < 512)
	//{
		m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx - nMenuMaxWidth) / 2;
	//}
	//else
	//{
	//	m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx / 2);
	//}
	
	m_nMenuY = GetMenuAreaTop();
	if (!bOffScreen)
	{
		m_nMenuY = GetMenuAreaTop() + (((int)m_szMenuArea.cy - (nMenuHeight + m_nMenuTitleSpacing + (int)m_szMenuTitle.cy + ((m_nFiles - 1) * m_nMenuSpacing))) / 2);
	}
}

void CLoadLevelMenu::CheckSelectionOffMenuTop()
{
	if (m_nSelection < m_nTopItem)
	{
		m_nTopItem = m_nSelection;
	}
}

void CLoadLevelMenu::CheckSelectionOffMenuBottom()
{
	int nCurrentY = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	for (int i = m_nTopItem; i < m_nFiles; i++)
	{
		if (m_pSurfaces[i])
		{
			nCurrentY += m_szSurfaces[i].cy;

			if (nCurrentY > GetMenuAreaBottom())
			{
				m_nTopItem++;
				if (m_nTopItem == m_nFiles)
				{
					// some strangeness happened...
					m_nTopItem = 0;
					return;
				}
				CheckSelectionOffMenuBottom();
				return;
			}

			if (m_nSelection == i) return;

			nCurrentY += m_nMenuSpacing;
		}
	}
}

