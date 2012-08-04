#include "clientheaders.h"
#include "DisplayOptionsMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include <stdio.h>

extern CommandID g_CommandArray[];

#define THIS_MENU_SPACING			3
#define CURRENT_SETTING_SPACING		2

#define max(a,b)	((a) > (b) ? (a) : (b))

CDisplayOptionsMenu::CDisplayOptionsMenu() : CBaseMenu()
{
	m_nSecondColumn = 0;
	m_fOriginalDetailLevel = 0.0f;
}

LTBOOL CDisplayOptionsMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	CRiotSettings* pSettings = pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// init the detail settings menu

	if (!m_DisplayModeMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_DetailSettingsMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;

	// call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 135;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 135;
	}
	else
	{
		m_nSecondColumn = 200;
	}

	return bSuccess;
}

void CDisplayOptionsMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_DisplayModeMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_DetailSettingsMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);

	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 135;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 135;
	}
	else
	{
		m_nSecondColumn = 200;
	}
}

void CDisplayOptionsMenu::Reset()
{
	if (!m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	m_fOriginalDetailLevel = pSettings->Detail[RS_DET_OVERALL].nValue;
	CBaseMenu::Reset();
}

void CDisplayOptionsMenu::Up()
{
	if (!TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE"))
	{
		if (m_nSelection == 2) m_nSelection = 1;
	}
	else
	{
		if (m_nSelection == 3) m_nSelection = 1;
	}

	CBaseMenu::Up();
}

void CDisplayOptionsMenu::Down()
{
	if (!TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE"))
	{
		if (m_nSelection == 0) m_nSelection = 1;
	}
	else
	{
		if (m_nSelection == 1) m_nSelection = 2;
	}
	
	CBaseMenu::Down();
}

void CDisplayOptionsMenu::Left()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == 2)
	{
		CBitmapFont* pFontNormal = LTNULL;
		CBitmapFont* pFontSelected = LTNULL;
		if (m_szScreen.cy < 400)
		{
			pFontNormal = m_pRiotMenu->GetFont08n();
			pFontSelected = m_pRiotMenu->GetFont08s();
		}
		else
		{
			pFontNormal = m_pRiotMenu->GetFont12n();
			pFontSelected = m_pRiotMenu->GetFont12s();
		}

		pSettings->Detail[RS_DET_GORE].nValue = !pSettings->Detail[RS_DET_GORE].nValue;

		if (m_GoreSetting.hMenuItem) m_pClientDE->DeleteSurface (m_GoreSetting.hMenuItem);
		if (m_GoreSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_GoreSetting.hMenuItemSelected);

		m_GoreSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Detail[RS_DET_GORE].nValue ? IDS_ON : IDS_OFF);
		m_GoreSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Detail[RS_DET_GORE].nValue ? IDS_ON : IDS_OFF);

		pSettings->WriteDetailSettings();
	}
	else if (m_nSelection == 3)
	{
		CBitmapFont* pFontNormal = LTNULL;
		CBitmapFont* pFontSelected = LTNULL;
		if (m_szScreen.cy < 400)
		{
			pFontNormal = m_pRiotMenu->GetFont08n();
			pFontSelected = m_pRiotMenu->GetFont08s();
		}
		else
		{
			pFontNormal = m_pRiotMenu->GetFont12n();
			pFontSelected = m_pRiotMenu->GetFont12s();
		}

		pSettings->Misc[RS_MISC_SCREENFLASH].nValue = !pSettings->Misc[RS_MISC_SCREENFLASH].nValue;

		if (m_ScreenFlash.hMenuItem) m_pClientDE->DeleteSurface (m_ScreenFlash.hMenuItem);
		if (m_ScreenFlash.hMenuItemSelected) m_pClientDE->DeleteSurface (m_ScreenFlash.hMenuItemSelected);

		m_ScreenFlash.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->ScreenFlash() ? IDS_ON : IDS_OFF);
		m_ScreenFlash.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->ScreenFlash() ? IDS_ON : IDS_OFF);

		HLTCOLOR hTrans = m_pClientDE->SetupColor2 (0.0f, 0.0f, 0.0f, LTTRUE);

		pSettings->WriteDetailSettings();
	}
	else if (m_nSelection == 4)
	{
		CBitmapFont* pFontNormal = LTNULL;
		CBitmapFont* pFontSelected = LTNULL;
		if (m_szScreen.cy < 400)
		{
			pFontNormal = m_pRiotMenu->GetFont08n();
			pFontSelected = m_pRiotMenu->GetFont08s();
		}
		else
		{
			pFontNormal = m_pRiotMenu->GetFont12n();
			pFontSelected = m_pRiotMenu->GetFont12s();
		}

		pSettings->Detail[RS_DET_OVERALL].nValue--;
		if (pSettings->Detail[RS_DET_OVERALL].nValue < 0) pSettings->Detail[RS_DET_OVERALL].nValue = 2;

		if (m_DetailSetting.hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItem);
		if (m_DetailSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItemSelected);

		int nDetailStringID = 0;
		switch ((int)pSettings->Detail[RS_DET_OVERALL].nValue)
		{
			default:	nDetailStringID = IDS_LOW;		pSettings->SetLowDetail();	break;
			case 1:		nDetailStringID = IDS_MEDIUM;	pSettings->SetMedDetail();	break;
			case 2:		nDetailStringID = IDS_HIGH;		pSettings->SetHiDetail();	break;
		}
		m_DetailSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nDetailStringID);
		m_DetailSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nDetailStringID);

		// implement the detail settings that might have changed...

		for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
		{
			pSettings->ImplementDetailSetting (i);
		}
	}
	
	CBaseMenu::Left();
}

void CDisplayOptionsMenu::Right()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == 2)
	{
		Left();
		return;
	}
	else if (m_nSelection == 3)
	{
		Left();
		return;
	}
	else if (m_nSelection == 4)
	{
		CBitmapFont* pFontNormal = LTNULL;
		CBitmapFont* pFontSelected = LTNULL;
		if (m_szScreen.cy < 400)
		{
			pFontNormal = m_pRiotMenu->GetFont08n();
			pFontSelected = m_pRiotMenu->GetFont08s();
		}
		else
		{
			pFontNormal = m_pRiotMenu->GetFont12n();
			pFontSelected = m_pRiotMenu->GetFont12s();
		}

		pSettings->Detail[RS_DET_OVERALL].nValue++;
		if (pSettings->Detail[RS_DET_OVERALL].nValue > 2) pSettings->Detail[RS_DET_OVERALL].nValue = 0;

		if (m_DetailSetting.hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItem);
		if (m_DetailSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItemSelected);

		int nDetailStringID = 0;
		switch ((int)pSettings->Detail[RS_DET_OVERALL].nValue)
		{
			default:	nDetailStringID = IDS_LOW;		pSettings->SetLowDetail();	break;
			case 1:		nDetailStringID = IDS_MEDIUM;	pSettings->SetMedDetail();	break;
			case 2:		nDetailStringID = IDS_HIGH;		pSettings->SetHiDetail();	break;
		}
		m_DetailSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nDetailStringID);
		m_DetailSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nDetailStringID);

		// implement the detail settings that might have changed...

		for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
		{
			pSettings->ImplementDetailSetting (i);
		}
	}
	
	CBaseMenu::Right();
}

void CDisplayOptionsMenu::PageUp()
{
	CBaseMenu::PageUp();
}

void CDisplayOptionsMenu::PageDown()
{
	CBaseMenu::PageUp();
}

void CDisplayOptionsMenu::Home()
{
	CBaseMenu::Home();
}

void CDisplayOptionsMenu::End()
{
	CBaseMenu::End();
}

void CDisplayOptionsMenu::Return()
{
	if (!m_pRiotMenu) return;

	if (m_nSelection == 0)
	{
		m_DisplayModeMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_DisplayModeMenu);
		CBaseMenu::Return();
	}
	else if (m_nSelection == 5)
	{
		m_DetailSettingsMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_DetailSettingsMenu);
		CBaseMenu::Return();
	}
	else if (m_nSelection == 6)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
		CBaseMenu::Return();
	}
}

void CDisplayOptionsMenu::Esc()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	if (m_fOriginalDetailLevel != pSettings->Detail[RS_DET_OVERALL].nValue)
	{
		m_pClientDE->RunConsoleString ("rebindtextures");
	}

	CBaseMenu::Esc();
}

void CDisplayOptionsMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE) return;

	if (!TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE"))
	{
		CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);
	}
	else
	{
		DrawNoGoreVersion (hScreen, nScreenWidth, nScreenHeight, nTextOffset);
	}

	int x = m_nMenuX;
	int nCurrentSettingX = x + 30;
	int y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing + m_GenericItem[0].szMenuItem.cy + (int) (m_nMenuSpacing + CURRENT_SETTING_SPACING);
	
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_RendererLine1.hMenuItem, LTNULL, nCurrentSettingX, y, LTNULL);
	if (m_RendererLine1.hMenuItem) y += m_RendererLine1.szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_RendererLine2.hMenuItem, LTNULL, nCurrentSettingX, y, LTNULL);
	if (m_RendererLine2.hMenuItem) y += m_RendererLine2.szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_Resolution.hMenuItem, LTNULL, nCurrentSettingX, y, LTNULL);
	if (m_Resolution.hMenuItem) y += m_Resolution.szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_TextureDepth.hMenuItem, LTNULL, nCurrentSettingX, y, LTNULL);

	y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing + m_GenericItem[0].szMenuItem.cy + m_GenericItem[1].szMenuItem.cy + (2 * m_nMenuSpacing);

	if (!TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE"))
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 2 ? m_GoreSetting.hMenuItemSelected : m_GoreSetting.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
	}

	y += m_GenericItem[2].szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 3 ? m_ScreenFlash.hMenuItemSelected : m_ScreenFlash.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
	
	y += m_GenericItem[3].szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 4 ? m_DetailSetting.hMenuItemSelected : m_DetailSetting.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
}

void CDisplayOptionsMenu::SetGlobalDetail (int nSetting)
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CBitmapFont* pFontNormal = LTNULL;
	CBitmapFont* pFontSelected = LTNULL;
	if (m_szScreen.cy < 400)
	{
		pFontNormal = m_pRiotMenu->GetFont08n();
		pFontSelected = m_pRiotMenu->GetFont08s();
	}
	else
	{
		pFontNormal = m_pRiotMenu->GetFont12n();
		pFontSelected = m_pRiotMenu->GetFont12s();
	}
	
	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	pSettings->Detail[RS_DET_OVERALL].nValue = (LTFLOAT)nSetting;
	pSettings->WriteDetailSettings();

	if (m_DetailSetting.hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItem);
	if (m_DetailSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItemSelected);

	int nDetailStringID = 0;
	switch ((int)pSettings->GlobalDetail())
	{
		default:	nDetailStringID = IDS_LOW;		break;
		case 1:		nDetailStringID = IDS_MEDIUM;	break;
		case 2:		nDetailStringID = IDS_HIGH;		break;
		case 3:		nDetailStringID = IDS_ADVANCED;	break;
	}

	m_DetailSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nDetailStringID);
	m_DetailSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nDetailStringID);
}

void CDisplayOptionsMenu::SetupCurrentRendererSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	
	RMode* pRMode = pSettings->GetRenderMode();
	if (!pRMode) return;

	if (m_RendererLine1.hMenuItem) m_pClientDE->DeleteSurface (m_RendererLine1.hMenuItem);
	if (m_RendererLine2.hMenuItem) m_pClientDE->DeleteSurface (m_RendererLine2.hMenuItem);
	if (m_Resolution.hMenuItem) m_pClientDE->DeleteSurface (m_Resolution.hMenuItem);
	if (m_TextureDepth.hMenuItem) m_pClientDE->DeleteSurface (m_TextureDepth.hMenuItem);

	char str[256];

	//sprintf (str, "(%s)", pRMode->m_RenderDLL);
	//m_RendererLine1.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, str);
	
	sprintf (str, "%s", pRMode->m_Description);
	m_RendererLine2.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, str);

	sprintf (str, "%d x %d", pRMode->m_Width, pRMode->m_Height);
	m_Resolution.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, str);

	int nStringID = pSettings->Textures8Bit() ? IDS_DISPLAY_8BIT : IDS_DISPLAY_16BIT;
	m_TextureDepth.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);

	m_pClientDE->GetSurfaceDims (m_RendererLine1.hMenuItem, &m_RendererLine1.szMenuItem.cx, &m_RendererLine1.szMenuItem.cy);
	m_pClientDE->GetSurfaceDims (m_RendererLine2.hMenuItem, &m_RendererLine2.szMenuItem.cx, &m_RendererLine2.szMenuItem.cy);
	m_pClientDE->GetSurfaceDims (m_Resolution.hMenuItem, &m_Resolution.szMenuItem.cx, &m_Resolution.szMenuItem.cy);
	m_pClientDE->GetSurfaceDims (m_TextureDepth.hMenuItem, &m_TextureDepth.szMenuItem.cx, &m_TextureDepth.szMenuItem.cy);
}

LTBOOL CDisplayOptionsMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	// determine the correct setting for the music source string

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// get detail string id

	int nDetailStringID = 0;
	switch ((int)pSettings->GlobalDetail())
	{
		default:	nDetailStringID = IDS_LOW;		break;
		case 1:		nDetailStringID = IDS_MEDIUM;	break;
		case 2:		nDetailStringID = IDS_HIGH;		break;
		case 3:		nDetailStringID = IDS_ADVANCED;	break;
	}

	// create the menu surfaces

	CBitmapFont* pFontNormal = LTNULL;
	CBitmapFont* pFontSelected = LTNULL;
	if (m_szScreen.cy < 400)
	{
		pFontNormal = m_pRiotMenu->GetFont08n();
		pFontSelected = m_pRiotMenu->GetFont08s();
	}
	else
	{
		pFontNormal = m_pRiotMenu->GetFont12n();
		pFontSelected = m_pRiotMenu->GetFont12s();
	}
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DISPLAY_CHANGE);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DISPLAY_GORE);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DISPLAY_SCREENFLASH);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DISPLAY_DETAIL);
	m_GenericItem[5].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DISPLAY_ADVANCED);
	m_GenericItem[6].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);

	m_GoreSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Gore() ? IDS_ON : IDS_OFF);
	m_ScreenFlash.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->ScreenFlash() ? IDS_ON : IDS_OFF);
	m_DetailSetting.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nDetailStringID);

	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DISPLAY_CHANGE);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DISPLAY_GORE);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DISPLAY_SCREENFLASH);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DISPLAY_DETAIL);
	m_GenericItem[5].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DISPLAY_ADVANCED);
	m_GenericItem[6].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);

	m_GoreSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Gore() ? IDS_ON : IDS_OFF);
	m_ScreenFlash.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->ScreenFlash() ? IDS_ON : IDS_OFF);
	m_DetailSetting.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nDetailStringID);

	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_DISPLAYOPTIONS);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	SetupCurrentRendererSurfaces();
	
	uint32 nTempWidth, nTempHeight;
	m_pClientDE->GetSurfaceDims (m_RendererLine1.hMenuItem, &nTempWidth, &nTempHeight);
	m_GenericItem[1].hMenuItem = m_pClientDE->CreateSurface (1, (nTempHeight * 4) + (3 * THIS_MENU_SPACING) + (2 * CURRENT_SETTING_SPACING));
	m_GenericItem[1].hMenuItemSelected = m_pClientDE->CreateSurface (1, (nTempHeight * 4) + (3 * THIS_MENU_SPACING) + (2 * CURRENT_SETTING_SPACING));
	m_pClientDE->FillRect (m_GenericItem[1].hMenuItem, LTNULL, LTNULL);
	m_pClientDE->FillRect (m_GenericItem[1].hMenuItemSelected, LTNULL, LTNULL);

	for (int i = 0; i < 7; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	if (!m_GoreSetting.hMenuItem || !m_DetailSetting.hMenuItem || !m_ScreenFlash.hMenuItem ||
		!m_GoreSetting.hMenuItemSelected || !m_DetailSetting.hMenuItemSelected || !m_ScreenFlash.hMenuItem)
	{
		UnloadSurfaces();
		return LTFALSE;
	}

	for (int i = 0; i < 7; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}

	return CBaseMenu::LoadSurfaces();
}

void CDisplayOptionsMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < 7; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;
	}
	
	if (m_GoreSetting.hMenuItem) m_pClientDE->DeleteSurface (m_GoreSetting.hMenuItem);
	if (m_GoreSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_GoreSetting.hMenuItemSelected);
	m_GoreSetting.hMenuItem = LTNULL;
	m_GoreSetting.hMenuItemSelected = LTNULL;

	if (m_ScreenFlash.hMenuItem) m_pClientDE->DeleteSurface (m_ScreenFlash.hMenuItem);
	if (m_ScreenFlash.hMenuItemSelected) m_pClientDE->DeleteSurface (m_ScreenFlash.hMenuItemSelected);
	m_ScreenFlash.hMenuItem = LTNULL;
	m_ScreenFlash.hMenuItemSelected = LTNULL;

	if (m_DetailSetting.hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItem);
	if (m_DetailSetting.hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting.hMenuItemSelected);
	m_DetailSetting.hMenuItem = LTNULL;
	m_DetailSetting.hMenuItemSelected = LTNULL;

	if (m_RendererLine1.hMenuItem) m_pClientDE->DeleteSurface (m_RendererLine1.hMenuItem);
	if (m_RendererLine2.hMenuItem) m_pClientDE->DeleteSurface (m_RendererLine2.hMenuItem);
	if (m_Resolution.hMenuItem) m_pClientDE->DeleteSurface (m_Resolution.hMenuItem);
	if (m_TextureDepth.hMenuItem) m_pClientDE->DeleteSurface (m_TextureDepth.hMenuItem);
	m_RendererLine1.hMenuItem = LTNULL;
	m_RendererLine2.hMenuItem = LTNULL;
	m_Resolution.hMenuItem = LTNULL;
	m_TextureDepth.hMenuItem = LTNULL;

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CDisplayOptionsMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	m_pClientDE->GetSurfaceDims (m_GoreSetting.hMenuItem, &nSettingWidth, &nSettingHeight);
	if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
	m_pClientDE->GetSurfaceDims (m_ScreenFlash.hMenuItem, &nSettingWidth, &nSettingHeight);
	if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
	m_pClientDE->GetSurfaceDims (m_DetailSetting.hMenuItem, &nSettingWidth, &nSettingHeight);
	if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + nSettingWidth;

	m_nMenuX = 0;
	//if (m_pRiotMenu->InWorld() || m_szScreen.cx < 512)
	//{
		m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx - nMenuMaxWidth) / 2;
	//}
	//else
	//{
	//	m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx / 2);
	//}

	m_nMenuSpacing = THIS_MENU_SPACING;
}

void CDisplayOptionsMenu::DrawNoGoreVersion(HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE || !m_pRiotMenu || !hScreen) return;

	// first draw the menu title if there is one

	int nCurrentY = m_nMenuY;
	if (m_hMenuTitle)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMenuTitle, LTNULL, m_nMenuX, nCurrentY, LTNULL);
		nCurrentY += m_szMenuTitle.cy + m_nMenuTitleSpacing;
	}

	if (m_nTopItem > 0)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetUpArrow(), LTNULL, m_nMenuX + 2, nCurrentY - m_pRiotMenu->GetArrowHeight() - 3, LTNULL);
	}

	LTBOOL bDrawDownArrow = LTFALSE;
	for (int i = m_nTopItem; i < MAX_GENERIC_ITEMS; i++)
	{
		if (m_GenericItem[i].hMenuItem)
		{
			// Don't draw the Gore menu item...
			if (i!=2)
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, m_nMenuX, nCurrentY, LTNULL);
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
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetDownArrow(), LTNULL, m_nMenuX + 2, nCurrentY, LTNULL);
	}
}