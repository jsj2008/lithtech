#include "clientheaders.h"
#include "DetailSettingsMenu.h"
#include "DisplayOptionsMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include <stdio.h>

extern CommandID g_CommandArray[];

CDetailSettingsMenu::CDetailSettingsMenu() : CBaseMenu()
{
	m_nSecondColumn = 0;
	m_fOriginalTextureDetail = 0.0f;
}

LTBOOL CDetailSettingsMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	CRiotSettings* pSettings = pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 180;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 175;
	}
	else
	{
		m_nSecondColumn = 175;
	}

	return bSuccess;
}

void CDetailSettingsMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
	
	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 180;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 175;
	}
	else
	{
		m_nSecondColumn = 175;
	}

}

void CDetailSettingsMenu::Reset()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	UnloadSurfaces();
	LoadSurfaces();

	m_fOriginalTextureDetail = pSettings->SubDetail[RS_SUBDET_TEXTUREDETAIL].nValue;

	CBaseMenu::Reset();
}

void CDetailSettingsMenu::Up()
{
	CBaseMenu::Up();
}

void CDetailSettingsMenu::Down()
{
	CBaseMenu::Down();
}

void CDetailSettingsMenu::Left()
{
	if (!m_pClientDE || !m_pRiotMenu || m_nSelection ==	RS_SUBDET_LAST + 1) return;
	
	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	
	if (m_nSelection == RS_SUBDET_MODELFB && pClientShell->AdvancedDisableModelFB())
	{
		CBaseMenu::Left();
		return;
	}

	if (m_nSelection == RS_SUBDET_LIGHTMAPPING && pClientShell->AdvancedDisableLightMap())
	{
		CBaseMenu::Left();
		return;
	}

	AdjustSetting (m_nSelection, -1);
	
	CBaseMenu::Left();
}

void CDetailSettingsMenu::Right()
{
	if (!m_pClientDE || m_nSelection ==	RS_SUBDET_LAST + 1) return;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	
	if (m_nSelection == RS_SUBDET_MODELFB && pClientShell->AdvancedDisableModelFB())
	{
		CBaseMenu::Right();
		return;
	}

	if (m_nSelection == RS_SUBDET_LIGHTMAPPING && pClientShell->AdvancedDisableLightMap())
	{
		CBaseMenu::Right();
		return;
	}
	
	AdjustSetting (m_nSelection, 1);

	CBaseMenu::Right();
}

void CDetailSettingsMenu::PageUp()
{
	CBaseMenu::PageUp();
}

void CDetailSettingsMenu::PageDown()
{
	CBaseMenu::PageDown();
}

void CDetailSettingsMenu::Home()
{
	CBaseMenu::Home();
}

void CDetailSettingsMenu::End()
{
	CBaseMenu::End();
}

void CDetailSettingsMenu::Return()
{
	if (m_nSelection == RS_SUBDET_LAST + 1)
	{
		Esc();
	}
}

void CDetailSettingsMenu::Esc()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	// compare current settings to low, med, and hi and set appropriate version in parent menu

	if (memcmp (pSettings->SubDetail, pSettings->DefLow, sizeof(pSettings->SubDetail)) == 0)
	{
		((CDisplayOptionsMenu*)m_pParent)->SetGlobalDetail(0);
	}
	else if (memcmp (pSettings->SubDetail, pSettings->DefMed, sizeof(pSettings->SubDetail)) == 0)
	{
		((CDisplayOptionsMenu*)m_pParent)->SetGlobalDetail(1);
	}
	else if (memcmp (pSettings->SubDetail, pSettings->DefHi, sizeof(pSettings->SubDetail)) == 0)
	{
		((CDisplayOptionsMenu*)m_pParent)->SetGlobalDetail(2);
	}
	else
	{
		((CDisplayOptionsMenu*)m_pParent)->SetGlobalDetail(3);
	}

	if (m_fOriginalTextureDetail != pSettings->SubDetail[RS_SUBDET_TEXTUREDETAIL].nValue)
	{
		m_pClientDE->RunConsoleString ("rebindtextures");
	}

		
	CBaseMenu::Esc();
}

void CDetailSettingsMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE) return;

	CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);

	int y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	for (int i = m_nTopItem; i < RS_SUBDET_LAST + 2; i++)
	{
		if (m_DetailSetting[i].hMenuItem && i != RS_SUBDET_LAST + 1)
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == i ? m_DetailSetting[i].hMenuItemSelected : m_DetailSetting[i].hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		}
		
		y += m_GenericItem[i].szMenuItem.cy + m_nMenuSpacing;
		if (y > GetMenuAreaBottom() - (int)m_GenericItem[i].szMenuItem.cy) break;
	}
}

LTBOOL CDetailSettingsMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	// get the settings class

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// create the menu surfaces

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	m_GenericItem[RS_SUBDET_MODELLOD].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_MODELLOD);
	m_GenericItem[RS_SUBDET_SHADOWS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_SHADOWS);
	m_GenericItem[RS_SUBDET_BULLETHOLES].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_BULLETHOLES);
	m_GenericItem[RS_SUBDET_TEXTUREDETAIL].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_TEXTUREDETAIL);
	m_GenericItem[RS_SUBDET_DYNAMICLIGHTING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_DYNAMICLIGHTING);
	m_GenericItem[RS_SUBDET_LIGHTMAPPING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_LIGHTMAPPING);
	m_GenericItem[RS_SUBDET_SPECIALFX].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_SPECIALFX);
	m_GenericItem[RS_SUBDET_ENVMAPPING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_ENVMAPPING);
	m_GenericItem[RS_SUBDET_MODELFB].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_MODELFB);
	m_GenericItem[RS_SUBDET_CLOUDMAPLIGHT].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_CLOUDMAPLIGHT);
	m_GenericItem[RS_SUBDET_PVWEAPONS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_PVWEAPONS);
	m_GenericItem[RS_SUBDET_POLYGRIDS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SUBDET_POLYGRIDS);
	m_GenericItem[RS_SUBDET_LAST + 1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);

	m_GenericItem[RS_SUBDET_MODELLOD].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_MODELLOD);
	m_GenericItem[RS_SUBDET_SHADOWS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_SHADOWS);
	m_GenericItem[RS_SUBDET_BULLETHOLES].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_BULLETHOLES);
	m_GenericItem[RS_SUBDET_TEXTUREDETAIL].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_TEXTUREDETAIL);
	m_GenericItem[RS_SUBDET_DYNAMICLIGHTING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_DYNAMICLIGHTING);
	m_GenericItem[RS_SUBDET_LIGHTMAPPING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_LIGHTMAPPING);
	m_GenericItem[RS_SUBDET_SPECIALFX].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_SPECIALFX);
	m_GenericItem[RS_SUBDET_ENVMAPPING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_ENVMAPPING);
	m_GenericItem[RS_SUBDET_MODELFB].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_MODELFB);
	m_GenericItem[RS_SUBDET_CLOUDMAPLIGHT].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_CLOUDMAPLIGHT);
	m_GenericItem[RS_SUBDET_PVWEAPONS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_PVWEAPONS);
	m_GenericItem[RS_SUBDET_POLYGRIDS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SUBDET_POLYGRIDS);
	m_GenericItem[RS_SUBDET_LAST + 1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_ADVANCEDDISPLAY);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	// take care of the settings surfaces

	int nStringID = 0;
	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
	{
		nStringID = GetSettingStringID (i);
		m_DetailSetting[i].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
		m_DetailSetting[i].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

		// crop the first item....
		if (i == RS_SUBDET_FIRST)
		{
			m_DetailSetting[i].hMenuItem = CropMenuItemTop (m_DetailSetting[i].hMenuItem);
			m_DetailSetting[i].hMenuItemSelected = CropMenuItemTop (m_DetailSetting[i].hMenuItemSelected);
		}
	}

	// check to see that all the surfaces were created successfully

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST + 1; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}

		if (i == RS_SUBDET_LAST + 1) continue;

		if (!m_DetailSetting[i].hMenuItem || !m_DetailSetting[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}
	
	// get the main surface sizes

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST + 1; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void CDetailSettingsMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST + 1; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;

		if (i == RS_SUBDET_LAST + 1) continue;

		if (m_DetailSetting[i].hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting[i].hMenuItem);
		if (m_DetailSetting[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting[i].hMenuItemSelected);
		m_DetailSetting[i].hMenuItem = LTNULL;
		m_DetailSetting[i].hMenuItemSelected = LTNULL;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CDetailSettingsMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	for (int i = 0; i <= RS_SUBDET_LAST; i++)
	{
		m_pClientDE->GetSurfaceDims (m_DetailSetting[i].hMenuItem, &nSettingWidth, &nSettingHeight);
		if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
		}
	}

	m_nMenuX = 0;
	//if (m_pRiotMenu->InWorld() || m_szScreen.cx < 512)
	//{
		m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx - nMenuMaxWidth) / 2;
	//}
	//else
	//{
	//	m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx / 2);
	//}
}

void CDetailSettingsMenu::AdjustSetting (int nSelection, int nChange)
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();

	// adjust the setting...

	switch (nSelection)
	{
		// 3-position options...
		
		case RS_SUBDET_MODELLOD:
		case RS_SUBDET_TEXTUREDETAIL:
		case RS_SUBDET_DYNAMICLIGHTING:
		case RS_SUBDET_SPECIALFX:
		{
			pSettings->SubDetail[nSelection].nValue += nChange;
			if (pSettings->SubDetail[nSelection].nValue < 0.0f) pSettings->SubDetail[nSelection].nValue = 2.0f;
			if (pSettings->SubDetail[nSelection].nValue > 2.0f) pSettings->SubDetail[nSelection].nValue = 0.0f;
		}
		break;

		// 2-position options...

		case RS_SUBDET_SHADOWS:
		case RS_SUBDET_LIGHTMAPPING:
		case RS_SUBDET_ENVMAPPING:
		case RS_SUBDET_MODELFB:
		case RS_SUBDET_CLOUDMAPLIGHT:
		case RS_SUBDET_POLYGRIDS:
		case RS_SUBDET_PVWEAPONS:
		{
			if (pSettings->SubDetail[nSelection].nValue)
			{
				pSettings->SubDetail[nSelection].nValue = 0.0f;
			}
			else
			{
				pSettings->SubDetail[nSelection].nValue = 1.0f;
			}
		}
		break;

		// real-value options...

		case RS_SUBDET_BULLETHOLES:
		{
			if (pSettings->SubDetail[nSelection].nValue == pSettings->DefLow[nSelection].nValue)
			{
				pSettings->SubDetail[nSelection].nValue = nChange < 0 ? pSettings->DefHi[nSelection].nValue : pSettings->DefMed[nSelection].nValue;
			}
			else if (pSettings->SubDetail[nSelection].nValue == pSettings->DefHi[nSelection].nValue)
			{
				pSettings->SubDetail[nSelection].nValue = nChange < 0 ? pSettings->DefMed[nSelection].nValue : pSettings->DefLow[nSelection].nValue;
			}
			else
			{
				pSettings->SubDetail[nSelection].nValue = nChange < 0 ? pSettings->DefLow[nSelection].nValue : pSettings->DefHi[nSelection].nValue;
			}
		}
		break;
	}

	// re-create the surface

	if (m_DetailSetting[nSelection].hMenuItem) m_pClientDE->DeleteSurface (m_DetailSetting[nSelection].hMenuItem);
	if (m_DetailSetting[nSelection].hMenuItemSelected) m_pClientDE->DeleteSurface (m_DetailSetting[nSelection].hMenuItemSelected);
	int nStringID = GetSettingStringID (nSelection);
	m_DetailSetting[nSelection].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
	m_DetailSetting[nSelection].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

	// implement the new setting if necessary

	ImplementDetailSetting (nSelection);
}

int	CDetailSettingsMenu::GetSettingStringID (int nSetting)
{
	if (!m_pRiotMenu) return IDS_ERROR;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return IDS_ERROR;
	
	switch (nSetting)
	{
		// low, med, high options...

		case RS_SUBDET_MODELLOD:
		case RS_SUBDET_TEXTUREDETAIL:
		case RS_SUBDET_SPECIALFX:
		{
			switch ((int)pSettings->SubDetail[nSetting].nValue)
			{
				default:	return IDS_LOW;
				case 1:		return IDS_MEDIUM;
				case 2:		return IDS_HIGH;
			}
		}
		break;

		// on/off options...

		case RS_SUBDET_SHADOWS:
		case RS_SUBDET_LIGHTMAPPING:
		case RS_SUBDET_ENVMAPPING:
		case RS_SUBDET_MODELFB:
		case RS_SUBDET_CLOUDMAPLIGHT:
		case RS_SUBDET_POLYGRIDS:
		{
			if (pSettings->SubDetail[nSetting].nValue)
			{
				return IDS_ON;
			}
			else
			{
				return IDS_OFF;
			}
		}
		break;

		// yes/no options...

		// other options...

		case RS_SUBDET_BULLETHOLES:
		{
			if (pSettings->SubDetail[nSetting].nValue == pSettings->DefHi[nSetting].nValue)
			{
				return IDS_LOTS;
			}
			else if (pSettings->SubDetail[nSetting].nValue == pSettings->DefMed[nSetting].nValue)
			{
				return IDS_SOME;
			}
			else if (pSettings->SubDetail[nSetting].nValue == pSettings->DefLow[nSetting].nValue)
			{
				return IDS_FEW;
			}
			else
			{
				return IDS_CUSTOM;
			}
		}
		break;

		case RS_SUBDET_DYNAMICLIGHTING:
		{
			switch ((int)pSettings->SubDetail[nSetting].nValue)
			{
				default:	return IDS_OFF;
				case 1:		return IDS_SHARP;
				case 2:		return IDS_SOFT;
			}
		}
		break;
		
		case RS_SUBDET_PVWEAPONS:
		{
			switch ((int)pSettings->SubDetail[nSetting].nValue)
			{
				default:	return IDS_FULL;
				case 1:		return IDS_PULLEDBACK;
			}
		}
		break;
	}

	return IDS_ERROR;
}

void CDetailSettingsMenu::ImplementDetailSetting (int nSetting)
{
	if (!m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	pSettings->ImplementDetailSetting (nSetting);
	switch (nSetting)
	{
		case RS_SUBDET_SPECIALFX:
		case RS_SUBDET_PVWEAPONS:
		case RS_SUBDET_POLYGRIDS:
		{
			// let kevin know somehow that these have changed
		}
		break;

		case RS_SUBDET_BULLETHOLES:
		{
		}
		break;
	}
}
