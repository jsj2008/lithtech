#include "clientheaders.h"
#include "OptionsMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "Font28.h"
#include "RiotMenu.h"
#include "RiotClientShell.h"

LTBOOL COptionsMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	if (!m_DisplayOptionsMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_SoundOptionsMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_KeyboardMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_MouseMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_JoystickMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;

	return LTTRUE;
}

void COptionsMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_DisplayOptionsMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_SoundOptionsMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_KeyboardMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_MouseMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_JoystickMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);

	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void COptionsMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	if (m_nSelection == 0)
	{
		m_DisplayOptionsMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_DisplayOptionsMenu);
	}
	else if (m_nSelection == 1)
	{
		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
		if (!pClientShell) return;

		if (!pClientShell->SoundInited())
		{
			pClientShell->DoMessageBox (IDS_SOUNDNOTINITED, TH_ALIGN_CENTER);
			return;
		}

		m_SoundOptionsMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_SoundOptionsMenu);
	}
	else if (m_nSelection == 2)
	{
		m_KeyboardMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_KeyboardMenu);
	}
	else if (m_nSelection == 3)
	{
		m_MouseMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_MouseMenu);
	}
	else if (m_nSelection == 4)
	{
		if (m_JoystickMenu.JoystickEnabled())
		{
			if (!m_JoystickMenu.JoystickMenuDisabled())
			{
				m_JoystickMenu.Reset();
				m_pRiotMenu->SetCurrentMenu (&m_JoystickMenu);
			}
			else
			{
				CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
				if (pClientShell) pClientShell->DoMessageBox (IDS_JOYSTICKMENUDISABLED, TH_ALIGN_CENTER);
			}
		}
		else
		{
			CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
			if (pClientShell) pClientShell->DoMessageBox (IDS_NOJOYSTICKDETECTED, TH_ALIGN_CENTER);
		}
	}
	else if (m_nSelection == 5)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
	}

	CBaseMenu::Return();
}

LTBOOL COptionsMenu::LoadSurfaces()
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

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SCREEN, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_KEYBOARD, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_JOYSTICK, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[5].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SCREEN, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_KEYBOARD, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_JOYSTICK, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[5].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK, IDS_MENUREPLACEMENTFONT);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_OPTIONS, IDS_MENUREPLACEMENTFONT);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	for (int i = 0; i < 6; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void COptionsMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < 6; i++)
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

