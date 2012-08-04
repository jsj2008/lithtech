#include "clientheaders.h"
#include "DisplayModeMenu.h"
#include "DisplayOptionsMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include <stdio.h>
#include <stdlib.h>

extern CommandID g_CommandArray[];

CDisplayModeMenu::CDisplayModeMenu() : CBaseMenu()
{
	m_nSecondColumn = 0;
	
	m_nRenderDlls = 0;
	
	m_nDrawCount = 0;
	m_bRenderersInited = LTFALSE;
	
	m_nCurrentRenderDll = 0;
	m_nCurrentRenderer = 0;
	m_nCurrentResolution = 0;
	m_nCurrentBitDepth = 0;
	
	m_nOriginalRenderDll = 0;
	m_nOriginalRenderer = 0;
	m_nOriginalResolution = 0;
	m_nOriginalBitDepth = 0;
}

LTBOOL CDisplayModeMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	// call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);

	// adjust some variables

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 100;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 100;
	}
	else
	{
		m_nSecondColumn = 100;
	}
	m_nSelection = 2;

	return bSuccess;
}

void CDisplayModeMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_nDrawCount = 0;
	m_nSelection = 2;
	
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 100;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 100;
	}
	else
	{
		m_nSecondColumn = 100;
	}
}

void CDisplayModeMenu::Reset()
{
	if (!m_pRiotMenu) return;

	CBaseMenu::Reset();

	if (!m_bRenderersInited)
	{
		m_nSelection = 2;
	}

	GetCurrentSettings();

	m_nOriginalRenderDll = m_nCurrentRenderDll;
	m_nOriginalRenderer = m_nCurrentRenderer;
	m_nOriginalResolution = m_nCurrentResolution;
	m_nOriginalBitDepth = m_nCurrentBitDepth;
}

void CDisplayModeMenu::Up()
{
	if (m_nSelection == 2)
	{
		m_nSelection = 0;
		PlayUpSound();
		return;
	}

	CBaseMenu::Up();
}

void CDisplayModeMenu::Down()
{
	if (m_nSelection == 0)
	{
		m_nSelection = 2;
		PlayDownSound();
		return;
	}

	CBaseMenu::Down();
}

void CDisplayModeMenu::Left()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	if (m_nSelection == 0)
	{
		// save the current settings
		int nTempRenderDll = m_nCurrentRenderDll;
		int nTempRenderer = m_nCurrentRenderer;

		// decrement the current driver
		m_nCurrentRenderer--;
		if (m_nCurrentRenderer < 0)
		{
			m_nCurrentRenderDll--;
			if (m_nCurrentRenderDll < 0)
			{
				m_nCurrentRenderDll = m_nRenderDlls - 1;
			}
			m_nCurrentRenderer = m_aRenderDlls[m_nCurrentRenderDll].nRenderers - 1;
		}
		
		// find the closest resolution to the current one in the new renderer
		m_nCurrentResolution = GetClosestResolution (m_aRenderDlls[nTempRenderDll].aRenderers[nTempRenderer].aResolutions[m_nCurrentResolution].nWidth,
													 m_aRenderDlls[nTempRenderDll].aRenderers[nTempRenderer].aResolutions[m_nCurrentResolution].nHeight);
	}
	else if (m_nSelection == 2)
	{
		m_nCurrentResolution--;
		if (m_nCurrentResolution < 0) m_nCurrentResolution = m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].nResolutions - 1;
	}
	else if (m_nSelection == 3)
	{
		int nStringID = m_nCurrentBitDepth ? IDS_DMODE_DEPTH16 : IDS_DMODE_DEPTH8;
		HSURFACE hNormal = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_pRiotMenu->GetFont08n(), nStringID);
		HSURFACE hSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_pRiotMenu->GetFont08s(), nStringID);
		if (hNormal && hSelected)
		{
			m_nCurrentBitDepth = m_nCurrentBitDepth ? 0 : 1;

			if (m_BitDepth.hMenuItem) m_pClientDE->DeleteSurface (m_BitDepth.hMenuItem);
			if (m_BitDepth.hMenuItemSelected) m_pClientDE->DeleteSurface (m_BitDepth.hMenuItemSelected);

			m_BitDepth.hMenuItem = hNormal;
			m_BitDepth.hMenuItemSelected = hSelected;
		}
		else
		{
			if (hNormal) m_pClientDE->DeleteSurface (hNormal);
			if (hSelected) m_pClientDE->DeleteSurface (hSelected);
		}
	}

	CBaseMenu::Left();
}

void CDisplayModeMenu::Right()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == 0)
	{
		// save the current settings
		int nTempRenderDll = m_nCurrentRenderDll;
		int nTempRenderer = m_nCurrentRenderer;

		// increment the current driver
		m_nCurrentRenderer++;
		if (m_nCurrentRenderer == m_aRenderDlls[m_nCurrentRenderDll].nRenderers)
		{
			m_nCurrentRenderDll++;
			if (m_nCurrentRenderDll == m_nRenderDlls)
			{
				m_nCurrentRenderDll = 0;
			}
			m_nCurrentRenderer = 0;
		}

		// find the closest resolution to the current one in the new renderer
		m_nCurrentResolution = GetClosestResolution (m_aRenderDlls[nTempRenderDll].aRenderers[nTempRenderer].aResolutions[m_nCurrentResolution].nWidth,
													 m_aRenderDlls[nTempRenderDll].aRenderers[nTempRenderer].aResolutions[m_nCurrentResolution].nHeight);
	}
	else if (m_nSelection == 2)
	{
		m_nCurrentResolution++;
		if (m_nCurrentResolution == m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].nResolutions) m_nCurrentResolution = 0;
	}
	else if (m_nSelection == 3)
	{
		Left();
		return;
	}

	CBaseMenu::Right();
}

void CDisplayModeMenu::PageUp()
{
	if (!m_bRenderersInited) return;
	CBaseMenu::PageUp();
}

void CDisplayModeMenu::PageDown()
{
	if (!m_bRenderersInited) return;
	CBaseMenu::PageDown();
}

void CDisplayModeMenu::Home()
{
	if (!m_bRenderersInited) return;
	CBaseMenu::Home();
}

void CDisplayModeMenu::End()
{
	if (!m_bRenderersInited) return;
	CBaseMenu::End();
}

void CDisplayModeMenu::Return()
{
	if (m_nSelection == m_nGenericItems - 1)
	{
		Esc();
	}
}

void CDisplayModeMenu::Esc()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	// see if they've changed any options...

	if (m_nCurrentBitDepth != m_nOriginalBitDepth)
	{
		pSettings->MasterPaletteMode.nValue = m_nCurrentBitDepth ? 1.0f : 2.0f;
		pSettings->ImplementBitDepth();
		((CDisplayOptionsMenu*)m_pParent)->SetupCurrentRendererSurfaces();
	}
	
	if ((m_nCurrentRenderDll != m_nOriginalRenderDll) || (m_nCurrentRenderer != m_nOriginalRenderer) || (m_nCurrentResolution != m_nOriginalResolution))
	{
		RMode* pRMode = pSettings->GetRenderMode();
	
		memset (pRMode, 0, sizeof(RMode));
		//SAFE_STRCPY(pRMode->m_RenderDLL, m_aRenderDlls[m_nCurrentRenderDll].strDllName);
		SAFE_STRCPY(pRMode->m_Description, m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].strRenderer);
		SAFE_STRCPY(pRMode->m_InternalName, m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].strInternalName);
		pRMode->m_Width = m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].aResolutions[m_nCurrentResolution].nWidth;
		pRMode->m_Height = m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer].aResolutions[m_nCurrentResolution].nHeight;
		pRMode->m_BitDepth = 16;
		pSettings->ImplementRendererSetting();

		((CDisplayOptionsMenu*)m_pParent)->SetupCurrentRendererSurfaces();
	}

	CBaseMenu::Esc();
}

void CDisplayModeMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);
	
	if (m_nDrawCount == 0)
	{
		m_nDrawCount++;
	}
	else if (m_nDrawCount == 1)
	{
		m_nDrawCount++;
		LoadRealSurfaces();
		m_nSelection = 0;
	}
	else
	{
		RENDERDLL* pRenderDll = &m_aRenderDlls[m_nCurrentRenderDll];
		RENDERER* pRenderer = &pRenderDll->aRenderers[m_nCurrentRenderer];
		RESOLUTION* pResolution = &pRenderer->aResolutions[m_nCurrentResolution];

		int y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
		
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 0 ? pRenderDll->hSurfaceSelected : pRenderDll->hSurface, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		y += m_GenericItem[0].szMenuItem.cy + m_nMenuSpacing;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 0 ? pRenderer->hSurfaceSelected : pRenderer->hSurface, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		y += m_GenericItem[1].szMenuItem.cy + m_nMenuSpacing;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 2 ? pResolution->hSurfaceSelected : pResolution->hSurface, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		y += m_GenericItem[2].szMenuItem.cy + m_nMenuSpacing;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 3 ? m_BitDepth.hMenuItemSelected : m_BitDepth.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
	}
}

LTBOOL CDisplayModeMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	if (m_bRenderersInited)
	{
		return LoadRealSurfaces();
	}

	// create the menu surfaces

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	
	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DMODE_PLEASEWAIT1);
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DMODE_PLEASEWAIT1);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DMODE_PLEASEWAIT2);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DMODE_PLEASEWAIT2);

	for (int i = 0; i < 2; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}

	return CBaseMenu::LoadSurfaces();
}

void CDisplayModeMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < m_nGenericItems; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;
	}
	if (m_BitDepth.hMenuItem) m_pClientDE->DeleteSurface (m_BitDepth.hMenuItem);
	if (m_BitDepth.hMenuItemSelected) m_pClientDE->DeleteSurface (m_BitDepth.hMenuItemSelected);
	m_BitDepth.hMenuItem = LTNULL;
	m_BitDepth.hMenuItemSelected = LTNULL;

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CDisplayModeMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	int nTotalRenderers = 0;
	uint32 nSettingWidth, nSettingHeight;
	if (m_bRenderersInited)
	{
		for (int i = 0; i < m_nRenderDlls; i++)
		{
			for (int nRenderer = 0; nRenderer < m_aRenderDlls[i].nRenderers; nRenderer++)
			{
				m_pClientDE->GetSurfaceDims (m_aRenderDlls[i].aRenderers[nRenderer].hSurface, &nSettingWidth, &nSettingHeight);
				if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
				{
					nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
				}
			}
		}
	}
	else
	{
		if ((int)m_GenericItem[0].szMenuItem.cx > nMenuMaxWidth) nMenuMaxWidth = m_GenericItem[0].szMenuItem.cx;
		if ((int)m_GenericItem[1].szMenuItem.cx > nMenuMaxWidth) nMenuMaxWidth = m_GenericItem[1].szMenuItem.cx;
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

LTBOOL CDisplayModeMenu::LoadRealSurfaces()
{
	if (!m_pRiotMenu || !m_pClientDE) return LTFALSE;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;

	// first unload any surfaces we have

	UnloadSurfaces();

	// create the menu surfaces

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DMODE_RENDERER);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, " ");
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DMODE_RESOLUTION);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_DMODE_BITDEPTH);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);

	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DMODE_RENDERER);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, " ");
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DMODE_RESOLUTION);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_DMODE_BITDEPTH);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);

	int nStringID = pSettings->Textures8Bit() ? IDS_DMODE_DEPTH8 : IDS_DMODE_DEPTH16;
	m_BitDepth.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
	m_BitDepth.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_DISPLAYMODE);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	int nGenericItems = 5;
	
	for (int i = 0; i < nGenericItems; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}
	if (!m_BitDepth.hMenuItem || !m_BitDepth.hMenuItemSelected)
	{
		UnloadSurfaces();
		return LTFALSE;
	}

	for (int i = 0; i < nGenericItems; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	if (!m_bRenderersInited)
	{
		m_bRenderersInited = InitRenderDlls();
		if (!m_bRenderersInited) return LTFALSE;
	}
	
	return CBaseMenu::LoadSurfaces();
}

LTBOOL CDisplayModeMenu::InitRenderDlls()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// get list of renderers

	RMode* pRModeList = m_pClientDE->GetRenderModes();
	if (!pRModeList) return LTFALSE;

	// go through and fill in our structures...

	RMode* pRMode = pRModeList;
	while (pRMode)
	{
		// do we already have this DLL in our array of render dlls?
		LTBOOL bDllFound = LTFALSE;
		for (int i = 0; i < m_nRenderDlls && !bDllFound; i++)
		{
			if (strnicmp (m_aRenderDlls[i].strDllName, pRMode->m_RenderDLL, LEN_RENDERDLL) == 0)
			{
				bDllFound = LTTRUE;

				// do we already have this Renderer in the DLL's array of renderers?
				LTBOOL bRendererFound = LTFALSE;
				for (int j = 0; j < m_aRenderDlls[i].nRenderers && !bRendererFound; j++)
				{
					if (strnicmp (m_aRenderDlls[i].aRenderers[j].strRenderer, pRMode->m_Description, LEN_RENDERER) == 0)
					{
						bRendererFound = LTTRUE;

						// add this resolution to this Renderer's list of resolutions
						RENDERER* pRenderer = &m_aRenderDlls[i].aRenderers[j];
						RESOLUTION* pResolution = &pRenderer->aResolutions[pRenderer->nResolutions];
						pRenderer->nResolutions++;
						pResolution->nWidth = pRMode->m_Width;
						pResolution->nHeight = pRMode->m_Height;
					}
				}

				// if we don't already have this Renderer in this DLL's array, add it and configure the first mode...
				if (!bRendererFound)
				{
					RENDERER* pRenderer = &m_aRenderDlls[i].aRenderers[m_aRenderDlls[i].nRenderers];
					m_aRenderDlls[i].nRenderers++;
					strncpy (pRenderer->strRenderer, pRMode->m_Description, LEN_RENDERER - 1);
					strncpy (pRenderer->strInternalName, pRMode->m_InternalName, LEN_RENDERER - 1);

					RESOLUTION* pResolution = &pRenderer->aResolutions[0];
					pRenderer->nResolutions++;
					pResolution->nWidth = pRMode->m_Width;
					pResolution->nHeight = pRMode->m_Height;
				}
			}
		}

		// if we don't already have this DLL in our array, add it and configure the first mode...
		if (!bDllFound && m_nRenderDlls < MAX_RENDERDLLS)
		{
			strncpy (m_aRenderDlls[m_nRenderDlls].strDllName, pRMode->m_RenderDLL, LEN_RENDERDLL - 1);
			
			RENDERER* pRenderer = &m_aRenderDlls[m_nRenderDlls].aRenderers[0];
			m_aRenderDlls[m_nRenderDlls].nRenderers++;
			strncpy (pRenderer->strRenderer, pRMode->m_Description, LEN_RENDERER - 1);
			strncpy (pRenderer->strInternalName, pRMode->m_InternalName, LEN_RENDERER - 1);

			RESOLUTION* pResolution = &pRenderer->aResolutions[0];
			pRenderer->nResolutions++;
			pResolution->nWidth = pRMode->m_Width;
			pResolution->nHeight = pRMode->m_Height;

			m_nRenderDlls++;
		}

		pRMode = pRMode->m_pNext;
	}

	// we should now have every render mode in our array...now create surfaces for them...

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();

	for (int i = 0; i < m_nRenderDlls; i++)
	{
		RENDERDLL* pRenderDll = &m_aRenderDlls[i];
		for (int j = 0; j < pRenderDll->nRenderers; j++)
		{
			RENDERER* pRenderer = &pRenderDll->aRenderers[j];
			for (int k = 0; k < pRenderer->nResolutions; k++)
			{
				RESOLUTION* pResolution = &pRenderer->aResolutions[k];

				char strRes[24];
				sprintf (strRes, "%d x %d", pResolution->nWidth, pResolution->nHeight);

				pResolution->hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, strRes);
				pResolution->hSurfaceSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, strRes);
			}

			pRenderer->hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pRenderer->strRenderer);
			pRenderer->hSurfaceSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pRenderer->strRenderer);
		}
			
		char str[LEN_RENDERDLL + 2];
		sprintf (str, "(%s)", pRenderDll->strDllName);
		pRenderDll->hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, str);
		pRenderDll->hSurfaceSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, str);
		
		// crop the tops of all of these...
		pRenderDll->hSurface = CropMenuItemTop (pRenderDll->hSurface);
		pRenderDll->hSurfaceSelected = CropMenuItemTop (pRenderDll->hSurfaceSelected);
		
		uint32 nWidth, nHeight;
		m_pClientDE->GetSurfaceDims (pRenderDll->hSurface, &nWidth, &nHeight);
		pRenderDll->nSurfaceWidth = (int)nWidth;
	}

	// now determine the current settings...

	GetCurrentSettings();

	return LTTRUE;
}

void CDisplayModeMenu::GetCurrentSettings()
{
	if (!m_pRiotMenu) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;
	
	/*
	for (int i = 0; i < m_nRenderDlls; i++)
	{
		if (strnicmp (m_aRenderDlls[i].strDllName, pSettings->GetRenderMode()->m_RenderDLL, LEN_RENDERDLL) == 0)
		{
			m_nCurrentRenderDll = i;
			break;
		}
	}
	RENDERDLL* pRenderDll = &m_aRenderDlls[m_nCurrentRenderDll];
	for (int i = 0; i < pRenderDll->nRenderers; i++)
	{
		if (strnicmp (pRenderDll->aRenderers[i].strRenderer, pSettings->GetRenderMode()->m_Description, LEN_RENDERER) == 0)
		{
			m_nCurrentRenderer = i;
			break;
		}
	}
	RENDERER* pRenderer = &pRenderDll->aRenderers[m_nCurrentRenderer];
	for (int i = 0; i < pRenderer->nResolutions; i++)
	{
		if (pRenderer->aResolutions[i].nWidth == (int)pSettings->GetRenderMode()->m_Width &&
			pRenderer->aResolutions[i].nHeight == (int)pSettings->GetRenderMode()->m_Height)
		{
			m_nCurrentResolution = i;
			break;
		}
	}
	*/

	m_nCurrentBitDepth = (int) pSettings->Textures8Bit();
}

int	CDisplayModeMenu::GetClosestResolution (int nWidth, int nHeight)
{
	RENDERER* pRenderer = &m_aRenderDlls[m_nCurrentRenderDll].aRenderers[m_nCurrentRenderer];

	int nClosestHeight = 0;
	int nClosestDiff = abs (pRenderer->aResolutions[0].nHeight - nHeight);

	for (int i = 1; i < pRenderer->nResolutions; i++)
	{
		int nDiff = abs (pRenderer->aResolutions[i].nHeight - nHeight);
		if (nDiff < nClosestDiff)
		{
			nClosestHeight = i;
			nClosestDiff = nDiff;
		}
	}

	return nClosestHeight;
}

