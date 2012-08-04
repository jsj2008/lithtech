#include "clientheaders.h"
#include "LoadSavedLevelMenu.h"
#include "TextHelper.h"
#include "RiotClientShell.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

LTBOOL CLoadSavedLevelMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	m_nSecondColumn = 100;

	return LTTRUE;
}

void CLoadSavedLevelMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CLoadSavedLevelMenu::Reset()
{
	CBaseMenu::Reset();
}

void CLoadSavedLevelMenu::Up()
{
	if (m_nSelection == 2 && !m_GenericItem[1].hMenuItem)
	{
		m_nSelection = 1;
	}

	CBaseMenu::Up();
}

void CLoadSavedLevelMenu::Down()
{
	if (m_nSelection == 0 && !m_GenericItem[1].hMenuItem)
	{
		m_nSelection = 1;
	}

	CBaseMenu::Down();
}

void CLoadSavedLevelMenu::End()
{
	if (!m_pRiotMenu) return;

	CBaseMenu::End();
	return;

	if (m_nSelection == (m_pRiotMenu->InWorld() ? m_nGenericItems - 1 : m_nGenericItems - 1)) return;

	m_nSelection = (m_pRiotMenu->InWorld() ? m_nGenericItems - 1 : m_nGenericItems - 1);
	
	PlayEndSound();
}

void CLoadSavedLevelMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	if (m_nSelection == MAXLOADLEVELS)
	{
		Esc();
		return;
	}

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (m_nSelection == 0)
	{
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];
		SAFE_STRCPY(strKey, "SaveGame00");
		CWinUtil::WinGetPrivateProfileString ("Shogo", strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);		
		if (!*strSaveGameSetting)
		{
			pClientShell->DoMessageBox (IDS_NOQUICKSAVEGAME, TH_ALIGN_CENTER);
			CBaseMenu::Return();
			return;
		}
		if (!pClientShell->LoadGame (strSaveGameSetting, QUICKSAVE_FILENAME))
		{
			pClientShell->DoMessageBox (IDS_LOADGAMEFAILED, TH_ALIGN_CENTER);
			CBaseMenu::Return();
			return;
		}
	}
	else if (m_nSelection == 1)
	{
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		CWinUtil::WinGetPrivateProfileString("Shogo", "Reload", "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);		
		if (!*strSaveGameSetting) return;

		if (!pClientShell->LoadGame(strSaveGameSetting, RELOADLEVEL_FILENAME))
		{
			pClientShell->DoMessageBox (IDS_LOADGAMEFAILED, TH_ALIGN_CENTER);
			CBaseMenu::Return();
			return;
		}
	}
	else
	{
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];
		sprintf (strKey, "SaveGame%02d", m_nSelection - 1);
		CWinUtil::WinGetPrivateProfileString ("Shogo", strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);		
		
		if (!*strSaveGameSetting) return;

		char strWorldName[128];
		SAFE_STRCPY(strWorldName, strSaveGameSetting);
		char* ptr = strWorldName;
		while (*ptr && *ptr != '|') ptr++;
		*ptr = '\0';
		
		char strFilename[128];
		sprintf (strFilename, "Save\\Slot%02d.sav", m_nSelection - 1);
		
		if (!pClientShell->LoadGame (strWorldName, strFilename))
		{
			pClientShell->DoMessageBox (IDS_LOADGAMEFAILED, TH_ALIGN_CENTER);
			CBaseMenu::Return();
			return;
		}
	}
	
	CBaseMenu::Return();
}

void CLoadSavedLevelMenu::Esc()
{
	if (!m_pRiotMenu || !m_pRiotMenu->GetClientShell()) return;

	CBaseMenu::Esc();
}

void CLoadSavedLevelMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE || !m_pRiotMenu) return;

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
			if (i > 0 && i < MAXLOADLEVELS && m_DateTime[i].hMenuItem)
			{
				m_pClientDE->GetSurfaceDims (m_DateTime[i].hMenuItem, &nDateTimeWidth, &nDateTimeHeight);
			}

			uint32 nWidth, nHeight;
			m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &nWidth, &nHeight);
			if (i == 0 || i == 1 || i == m_nGenericItems - 1 /*(m_pRiotMenu->InWorld() ? m_nGenericItems - 1 : m_nGenericItems)*/ || (i > 0 && i < MAXLOADLEVELS && nDateTimeWidth < 30))
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, ((int)m_szScreen.cx - (int)nWidth) / 2, nCurrentY, LTNULL);
			}
			else
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, m_nMenuX - 10 - (int)nWidth, nCurrentY, LTNULL);
			}
			
			if (i > 0 && i < MAXLOADLEVELS && m_DateTime[i].hMenuItem)
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

LTBOOL CLoadSavedLevelMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return LTFALSE;
	
	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	// go through each saved world and get the level name and date and time at which it was saved

	for (int i = 2; i < MAXLOADLEVELS; i++)
	{
		// see if this filename exists...
		
		LTBOOL bFileExists = LTFALSE;
		char strFilename[128];
		sprintf (strFilename, "Save\\Slot%02d.sav", i - 1);
		if (CWinUtil::FileExist (strFilename))
		{
			bFileExists = LTTRUE;
		}

		// see if the setting exists...

		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];
		sprintf (strKey, "SaveGame%02d", i - 1);
		CWinUtil::WinGetPrivateProfileString ("Shogo", strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
		
		char* pWorldName = LTNULL;
		struct tm* pTimeDate = LTNULL;

		if (*strSaveGameSetting && bFileExists)
		{
			if (strlen (strSaveGameSetting) > 0)
			{
				pWorldName = strSaveGameSetting;

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
		}
		else if (*strSaveGameSetting)
		{
			CWinUtil::WinWritePrivateProfileString ("Shogo", strKey, "", SAVEGAMEINI_FILENAME);
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
	
	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_QUICKLOAD);
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_QUICKLOAD);
	
	char strSaveGameSetting[256];
	memset (strSaveGameSetting, 0, 256);
	CWinUtil::WinGetPrivateProfileString("Shogo", "Reload", "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);		
	if (strSaveGameSetting[0])
	{
		char strNiceName[128];
		SAFE_STRCPY(strNiceName, strSaveGameSetting);
		pClientShell->GetNiceWorldName (strSaveGameSetting, strNiceName, 127);

		HSTRING hString = m_pClientDE->FormatString (IDS_LOADCURRENT, strNiceName);
		char* strLoadCurrent = const_cast<char *>(m_pClientDE->GetStringData (hString));
		m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strLoadCurrent);
		m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strLoadCurrent);
		m_pClientDE->FreeString (hString);
	}
	
	m_GenericItem[MAXLOADLEVELS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);
	m_GenericItem[MAXLOADLEVELS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_LOADGAME);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	for (int i = 2; i < MAXLOADLEVELS; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected || !m_DateTime[i].hMenuItem || !m_DateTime[i].hMenuItemSelected )
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	if (!m_GenericItem[0].hMenuItem || !m_GenericItem[0].hMenuItemSelected || !m_GenericItem[MAXLOADLEVELS].hMenuItem || !m_GenericItem[MAXLOADLEVELS].hMenuItemSelected)
	{
		UnloadSurfaces();
		return LTFALSE;
	}

	for (int i = 0; i < MAXLOADLEVELS; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	m_pClientDE->GetSurfaceDims (m_GenericItem[0].hMenuItem, &m_GenericItem[0].szMenuItem.cx, &m_GenericItem[0].szMenuItem.cy);
	m_pClientDE->GetSurfaceDims (m_GenericItem[MAXLOADLEVELS].hMenuItem, &m_GenericItem[MAXLOADLEVELS].szMenuItem.cx, &m_GenericItem[MAXLOADLEVELS].szMenuItem.cy);
	
	return CBaseMenu::LoadSurfaces();
}

void CLoadSavedLevelMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < MAXLOADLEVELS; i++)
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
	
	if (m_GenericItem[MAXLOADLEVELS].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[MAXLOADLEVELS].hMenuItem);
	if (m_GenericItem[MAXLOADLEVELS].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[MAXLOADLEVELS].hMenuItemSelected);
	m_GenericItem[MAXLOADLEVELS].hMenuItem = LTNULL;
	m_GenericItem[MAXLOADLEVELS].hMenuItemSelected = LTNULL;
	m_GenericItem[MAXLOADLEVELS].szMenuItem.cx = m_GenericItem[MAXLOADLEVELS].szMenuItem.cy = 0;

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CLoadSavedLevelMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	for (int i = 0; i < MAXLOADLEVELS; i++)
	{
		m_pClientDE->GetSurfaceDims (m_DateTime[i].hMenuItem, &nSettingWidth, &nSettingHeight);
		if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
		}
	}

	m_nGenericItems = 13;
	m_nMenuX = (int)m_szScreen.cx / 2;
}

