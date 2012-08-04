#include "clientheaders.h"
#include "NewGameMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotClientShell.h"
#include "RiotMenu.h"

LTBOOL CNewGameMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	return LTTRUE;
}

void CNewGameMenu::Reset()
{
	CBaseMenu::Reset();

	m_nSelection = 1;
}

void CNewGameMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CNewGameMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	if (m_nSelection == 4)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
	}
	else
	{
		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
		if (!pClientShell) return;

		pClientShell->StartGame((GameDifficulty)m_nSelection);
	}

	CBaseMenu::Return();
}

LTBOOL CNewGameMenu::LoadSurfaces()
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

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_NEW_EASY, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_NEW_MEDIUM, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_NEW_HARD, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_NEW_INSANE, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_NEW_EASY, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_NEW_MEDIUM, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_NEW_HARD, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_NEW_INSANE, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_DIFFICULTY, IDS_MENUREPLACEMENTFONT);
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

void CNewGameMenu::UnloadSurfaces()
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

