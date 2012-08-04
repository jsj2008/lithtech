#include "clientheaders.h"
#include "SinglePlayerMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "RiotClientShell.h"

LTBOOL CSinglePlayerMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	if (!m_NewGameMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_LoadLevelMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_LoadSavedLevelMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_SaveLevelMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;

	return LTTRUE;
}

void CSinglePlayerMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_NewGameMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_LoadLevelMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_LoadSavedLevelMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_SaveLevelMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);

	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CSinglePlayerMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CBaseMenu::Return();
	
	if (m_nSelection == 0)
	{
//#ifdef _DEMO
//		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
//		if (pClientShell)
//		{
//			pClientShell->StartGame(GD_HARD);
//		}
//#else
		m_NewGameMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_NewGameMenu);
//#endif
	}
	else if (m_nSelection == 1)
	{
		m_LoadSavedLevelMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_LoadSavedLevelMenu);
	}
	else if (m_nSelection == 2)
	{
		if (!m_pRiotMenu->InWorld()) return;

		m_SaveLevelMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_SaveLevelMenu);
	}
	else if (m_nSelection == 3)
	{
#ifdef _DEMO  // No custom worlds available
		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
		if (!pClientShell) return;
		pClientShell->DoMessageBox (IDS_DEMONOCUSTOMWORLDS, TH_ALIGN_CENTER);
		return;
#endif
		if (m_LoadLevelMenu.GetNumFiles() == 0)
		{
			CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
			if (!pClientShell) return;
			pClientShell->DoMessageBox (IDS_NOCUSTOMWORLDS, TH_ALIGN_CENTER);
		}
		else
		{
			m_LoadLevelMenu.Reset();
			m_pRiotMenu->SetCurrentMenu (&m_LoadLevelMenu);
		}
	}
	else if (m_nSelection == 4)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
	}
}

LTBOOL CSinglePlayerMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CBitmapFont* pFontNormal = LTNULL;
	CBitmapFont* pFontSelected = LTNULL;
	if (m_szScreen.cx < 512)
	{
		pFontNormal = m_pRiotMenu->GetFont12n();
		pFontSelected = m_pRiotMenu->GetFont12s();
	}
	else if (m_szScreen.cx < 640)
	{
		pFontNormal = m_pRiotMenu->GetFont18n();
		pFontSelected = m_pRiotMenu->GetFont18s();
	}
	else
	{
		pFontNormal = m_pRiotMenu->GetFont28n();
		pFontSelected = m_pRiotMenu->GetFont28s();
	}

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_NEWGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_LOADGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SAVEGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_WARP, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_NEWGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_LOADGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SAVEGAME, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_WARP, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_GAME, IDS_MENUREPLACEMENTFONT);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	for (int i = 0; i < 5; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void CSinglePlayerMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < 5; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

