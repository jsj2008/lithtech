#include "clientheaders.h"
#include "SaveLevelMenu.h"
#include "TextHelper.h"
#include "RiotClientShell.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "WinUtil.h"
#include "SinglePlayerMenu.h"
#include "LoadSavedLevelMenu.h"
#include <stdio.h>
#include <time.h>

LTBOOL CSaveLevelMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	m_nSecondColumn = 100;
	m_bContentChanged = LTFALSE;

	return LTTRUE;
}

void CSaveLevelMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CSaveLevelMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	// if they're selecting back, just exit this menu

	if (m_nSelection == MAXSAVELEVELS)
	{
		Esc();
		return;
	}
	
	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return;

	// if they're selecting quicksave, then quicksave the level...

	if (m_nSelection == 0)
	{
		char strKey[32];
		SAFE_STRCPY(strKey, "SaveGame00");
		char strSaveGame[256];
		SAFE_STRCPY(strSaveGame, pClientShell->GetCurrentWorldName());
		CWinUtil::WinWritePrivateProfileString ("Shogo", strKey, strSaveGame, SAVEGAMEINI_FILENAME);
		
		if (!pClientShell->SaveGame (QUICKSAVE_FILENAME))
		{
			pClientShell->DoMessageBox (IDS_SAVEGAMEFAILED, TH_ALIGN_CENTER);
		}
		return;
	}

	m_bContentChanged = LTTRUE;

	// create the filename and save the game

	char strFilename[128];
	sprintf (strFilename, "Save\\Slot%02d.sav", m_nSelection);
	if (!pClientShell->SaveGame (strFilename))
	{
		pClientShell->DoMessageBox (IDS_SAVEGAMEFAILED, TH_ALIGN_CENTER);
		return;
	}

	// they're trying to save this level in a slot...set up a few things and get the current time and date

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	
	time_t seconds;
	time (&seconds);
	struct tm* timedate = localtime (&seconds);
	if (!timedate) return;

	// delete the old surfaces...

	if (m_GenericItem[m_nSelection].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[m_nSelection].hMenuItem); m_GenericItem[m_nSelection].hMenuItem = LTNULL;
	if (m_GenericItem[m_nSelection].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[m_nSelection].hMenuItemSelected); m_GenericItem[m_nSelection].hMenuItemSelected = LTNULL;
	if (m_DateTime[m_nSelection].hMenuItem) m_pClientDE->DeleteSurface (m_DateTime[m_nSelection].hMenuItem); m_DateTime[m_nSelection].hMenuItem = LTNULL;
	if (m_DateTime[m_nSelection].hMenuItemSelected) m_pClientDE->DeleteSurface (m_DateTime[m_nSelection].hMenuItemSelected); m_DateTime[m_nSelection].hMenuItemSelected = LTNULL;

	// create the new surfaces...

	char strNiceName[128];
	SAFE_STRCPY(strNiceName, pClientShell->GetCurrentWorldName());
	pClientShell->GetNiceWorldName (pClientShell->GetCurrentWorldName(), strNiceName, 127);
	
	m_GenericItem[m_nSelection].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strNiceName);
	m_GenericItem[m_nSelection].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strNiceName);

	char strTimeDate[128];
	sprintf (strTimeDate, "%02d/%02d/%02d %02d:%02d:%02d", timedate->tm_mon + 1, timedate->tm_mday, (timedate->tm_year + 1900) % 100, timedate->tm_hour, timedate->tm_min, timedate->tm_sec);
	m_DateTime[m_nSelection].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strTimeDate);
	m_DateTime[m_nSelection].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strTimeDate);

	// set the console variable

	char strKey[32];
	sprintf (strKey, "SaveGame%02d", m_nSelection);
	char strSaveGame[256];
	sprintf (strSaveGame, "%s|%ld", pClientShell->GetCurrentWorldName(), (long)seconds);
	CWinUtil::WinWritePrivateProfileString ("Shogo", strKey, strSaveGame, SAVEGAMEINI_FILENAME);

	CBaseMenu::Return();
}

void CSaveLevelMenu::Esc()
{
	if (!m_pRiotMenu || !m_pRiotMenu->GetClientShell()) return;

	if (m_bContentChanged)
	{
		CLoadSavedLevelMenu* pLoadSaved = ((CSinglePlayerMenu*)m_pParent)->GetLoadSavedLevelMenu();
		if (pLoadSaved)
		{
			pLoadSaved->UnloadAllSurfaces();
			pLoadSaved->LoadAllSurfaces();
		}
	}

	CBaseMenu::Esc();
}

void CSaveLevelMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE) return;

	// first draw the menu title if there is one

	int nCurrentY = m_nMenuY;
	if (m_hMenuTitle)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMenuTitle, LTNULL, ((int)m_szScreen.cx - (int)m_szMenuTitle.cx) / 2, nCurrentY, LTNULL);
		nCurrentY += m_szMenuTitle.cy + m_nMenuTitleSpacing;
	}

	if (m_nTopItem > 0)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetUpArrow(), LTNULL, m_nMenuX - (m_pRiotMenu->GetArrowWidth() / 2), nCurrentY - m_pRiotMenu->GetArrowHeight() - 3, LTNULL);
	}
	
	// now draw the saved game slots...

	LTBOOL bDrawDownArrow = LTFALSE;
	for (int i = m_nTopItem; i < MAX_GENERIC_ITEMS; i++)
	{
		if (m_GenericItem[i].hMenuItem)
		{
			uint32 nDateTimeWidth = 0;
			uint32 nDateTimeHeight = 0;
			if (i > 0 && i < MAXSAVELEVELS && m_DateTime[i].hMenuItem)
			{
				m_pClientDE->GetSurfaceDims (m_DateTime[i].hMenuItem, &nDateTimeWidth, &nDateTimeHeight);
			}

			uint32 nWidth, nHeight;
			m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &nWidth, &nHeight);
			if (i == 0 || i == m_nGenericItems - 1 || (i > 0 && i < MAXSAVELEVELS && nDateTimeWidth < 30))
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, ((int)m_szScreen.cx - (int)nWidth) / 2, nCurrentY, LTNULL);
			}
			else
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, m_nMenuX - 10 - nWidth, nCurrentY, LTNULL);
			}
			
			if (i > 0 && i < MAXSAVELEVELS && m_DateTime[i].hMenuItem)
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == i ? m_DateTime[i].hMenuItemSelected : m_DateTime[i].hMenuItem, LTNULL, m_nMenuX + 10, nCurrentY, LTNULL);
			}

			nCurrentY += m_GenericItem[i].szMenuItem.cy + m_nMenuSpacing;
			if (nCurrentY > GetMenuAreaBottom() - (int)m_GenericItem[i].szMenuItem.cy)
			{
				if (i < m_nGenericItems - 1)
				{
					bDrawDownArrow = LTTRUE;
				}
				break;
			}
		}
	}

	if (bDrawDownArrow)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetDownArrow(), LTNULL, m_nMenuX - (m_pRiotMenu->GetArrowWidth() / 2), nCurrentY, LTNULL);
	}
}

LTBOOL CSaveLevelMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return LTFALSE;
	
	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	// go through each saved world and get the level name and date and time at which it was saved

	for (int i = 1; i < MAXSAVELEVELS; i++)
	{
		struct tm* pTimeDate = LTNULL;

		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];
		sprintf (strKey, "SaveGame%02d", i);
		CWinUtil::WinGetPrivateProfileString ("Shogo", strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);		
			
		char* pWorldName = strSaveGameSetting;

		if (strlen (strSaveGameSetting) > 0)
		{
			char* ptr = &strSaveGameSetting[strlen (strSaveGameSetting) - 1];
			while (*ptr != '|' && ptr != strSaveGameSetting) ptr--;

			if (*ptr == '|')
			{
				*ptr = '\0';
				ptr++;
				time_t nSeconds = (time_t) atol (ptr);
				pTimeDate = localtime (&nSeconds);
			}
		}

		if (pTimeDate)
		{
			char strNiceName[128];
			SAFE_STRCPY(strNiceName, pWorldName);
			pClientShell->GetNiceWorldName (pWorldName, strNiceName, 127);

			m_GenericItem[i].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strNiceName);
			m_GenericItem[i].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strNiceName);

			char strDateTime[32];
			sprintf (strDateTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
			m_DateTime[i].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strDateTime);
			m_DateTime[i].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strDateTime);
		}
		else
		{
			m_GenericItem[i].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_EMPTY);
			m_GenericItem[i].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_EMPTY);
			m_DateTime[i].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, " ");
			m_DateTime[i].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, " ");
		}
	}

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_QUICKSAVE);
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_QUICKSAVE);
	m_GenericItem[MAXSAVELEVELS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);
	m_GenericItem[MAXSAVELEVELS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_SAVEGAME);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	for (int i = 1; i < MAXSAVELEVELS; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected || !m_DateTime[i].hMenuItem || !m_DateTime[i].hMenuItemSelected )
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	if (!m_GenericItem[0].hMenuItem || !m_GenericItem[0].hMenuItemSelected || !m_GenericItem[MAXSAVELEVELS].hMenuItem || !m_GenericItem[MAXSAVELEVELS].hMenuItemSelected)
	{
		UnloadSurfaces();
		return LTFALSE;
	}

	for (int i = 1; i < MAXSAVELEVELS; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}

	m_pClientDE->GetSurfaceDims (m_GenericItem[0].hMenuItem, &m_GenericItem[0].szMenuItem.cx, &m_GenericItem[0].szMenuItem.cy);
	m_pClientDE->GetSurfaceDims (m_GenericItem[MAXSAVELEVELS].hMenuItem, &m_GenericItem[MAXSAVELEVELS].szMenuItem.cx, &m_GenericItem[MAXSAVELEVELS].szMenuItem.cy);
	
	return CBaseMenu::LoadSurfaces();
}

void CSaveLevelMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < MAXSAVELEVELS; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;

		if (m_DateTime[i].hMenuItem) m_pClientDE->DeleteSurface (m_DateTime[i].hMenuItem);
		if (m_DateTime[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_DateTime[i].hMenuItemSelected);
		m_DateTime[i].hMenuItem = LTNULL;
		m_DateTime[i].hMenuItemSelected = LTNULL;
	}

	if (m_GenericItem[MAXSAVELEVELS].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[MAXSAVELEVELS].hMenuItem);
	if (m_GenericItem[MAXSAVELEVELS].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[MAXSAVELEVELS].hMenuItemSelected);
	m_GenericItem[MAXSAVELEVELS].hMenuItem = LTNULL;
	m_GenericItem[MAXSAVELEVELS].hMenuItemSelected = LTNULL;
	m_GenericItem[MAXSAVELEVELS].szMenuItem.cx = m_GenericItem[MAXSAVELEVELS].szMenuItem.cy = 0;

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CSaveLevelMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	for (int i = 0; i < MAXSAVELEVELS; i++)
	{
		m_pClientDE->GetSurfaceDims (m_DateTime[i].hMenuItem, &nSettingWidth, &nSettingHeight);
		if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
		}
	}

	m_nMenuX = ((int)m_szScreen.cx) / 2;
}

