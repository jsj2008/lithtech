#include "clientheaders.h"
#include "SoundOptionsMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include <stdio.h>

extern CommandID g_CommandArray[];

CSoundOptionsMenu::CSoundOptionsMenu() : CBaseMenu()
{
	m_nSecondColumn = 0;

	memset (m_strOrigMusicSource, 0, 32);
}

LTBOOL CSoundOptionsMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	CRiotSettings* pSettings = pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	m_nSecondColumn = 180;
	
	m_sliderMusicVolume.Init (pClientDE, 60, 11);
	m_sliderSoundVolume.Init (pClientDE, 60, 11);

	m_sliderMusicVolume.SetEnabled();
	m_sliderSoundVolume.SetEnabled();

	m_sliderMusicVolume.SetPos ((int)pSettings->Sound[RS_SND_MUSICVOL].nValue / 10);
	m_sliderSoundVolume.SetPos ((int)pSettings->Sound[RS_SND_SOUNDVOL].nValue / 9);
	
	// call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);

	return bSuccess;
}

void CSoundOptionsMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CSoundOptionsMenu::Reset()
{
	if (!m_pRiotMenu) return;

	m_sliderMusicVolume.SetSelected (LTFALSE);
	m_sliderSoundVolume.SetSelected (LTFALSE);

	CBaseMenu::Reset();
}

void CSoundOptionsMenu::Up()
{
	if (m_nSelection == 1)
	{
		m_sliderMusicVolume.SetSelected (LTFALSE);
	}
	else if (m_nSelection == 3)
	{
		m_sliderSoundVolume.SetSelected (LTFALSE);
	}

	CBaseMenu::Up();

	if (m_nSelection == 1)
	{
		m_sliderMusicVolume.SetSelected();
	}
	else if (m_nSelection == 3)
	{
		m_sliderSoundVolume.SetSelected();
	}
}

void CSoundOptionsMenu::Down()
{
	if (m_nSelection == 1)
	{
		m_sliderMusicVolume.SetSelected (LTFALSE);
	}
	else if (m_nSelection == 3)
	{
		m_sliderSoundVolume.SetSelected (LTFALSE);
	}

	CBaseMenu::Down();

	if (m_nSelection == 1)
	{
		m_sliderMusicVolume.SetSelected();
	}
	else if (m_nSelection == 3)
	{
		m_sliderSoundVolume.SetSelected();
	}
}

void CSoundOptionsMenu::Left()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == 0)
	{
		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();

		pSettings->Sound[RS_SND_MUSICENABLED].nValue = pSettings->Sound[RS_SND_MUSICENABLED].nValue ? 0.0f : 1.0f;
		pSettings->ImplementMusicSource();
		
		if (m_MusicSource.hMenuItem) m_pClientDE->DeleteSurface (m_MusicSource.hMenuItem);
		if (m_MusicSource.hMenuItemSelected) m_pClientDE->DeleteSurface (m_MusicSource.hMenuItemSelected);

		m_MusicSource.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_MUSICENABLED].nValue ? IDS_ON : IDS_OFF);
		m_MusicSource.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_MUSICENABLED].nValue ? IDS_ON : IDS_OFF);
		
		// crop the music source setting...
		//m_MusicSource.hMenuItem = CropMenuItemTop (m_MusicSource.hMenuItem);
		//m_MusicSource.hMenuItemSelected = CropMenuItemTop (m_MusicSource.hMenuItemSelected);
	}
	else if (m_nSelection == 1)
	{
		if (!pSettings->MusicEnabled())
		{
			pClientShell->DoMessageBox (IDS_NOMUSICCHANGE, TH_ALIGN_CENTER);
			return;
		}

		int nPos = m_sliderMusicVolume.GetPos();
		if (nPos == m_sliderMusicVolume.GetMin()) return;

		m_sliderMusicVolume.SetPos (nPos - 1);
		pSettings->Sound[RS_SND_MUSICVOL].nValue = (LTFLOAT)m_sliderMusicVolume.GetPos() * 10;
		pSettings->ImplementMusicVolume();
	}
	else if (m_nSelection == 2)
	{
		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();

		pSettings->Sound[RS_SND_FX].nValue = pSettings->Sound[RS_SND_FX].nValue ? 0.0f : 1.0f;
		pSettings->ImplementSoundEnabled();
		
		if (m_SoundFx.hMenuItem) m_pClientDE->DeleteSurface (m_SoundFx.hMenuItem);
		if (m_SoundFx.hMenuItemSelected) m_pClientDE->DeleteSurface (m_SoundFx.hMenuItemSelected);
		
		m_SoundFx.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_FX].nValue == 1 ? IDS_ON : IDS_OFF);
		m_SoundFx.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_FX].nValue == 1 ? IDS_ON : IDS_OFF);
	}
	else if (m_nSelection == 3)
	{
		if (!pSettings->SoundEnabled())
		{
			pClientShell->DoMessageBox (IDS_NOSOUNDCHANGE, TH_ALIGN_CENTER);
			return;
		}

		int nPos = m_sliderSoundVolume.GetPos();
		if (nPos == m_sliderSoundVolume.GetMin()) return;

		m_sliderSoundVolume.SetPos (nPos - 1);
		pSettings->Sound[RS_SND_SOUNDVOL].nValue = (LTFLOAT)m_sliderSoundVolume.GetPos() * 9;
		pSettings->ImplementSoundVolume();
	}
	else if (m_nSelection == 4)
	{
		if (!pSettings->SoundEnabled())
		{
			pClientShell->DoMessageBox (IDS_NOSOUNDCHANGE, TH_ALIGN_CENTER);
			return;
		}

		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();

		if (pSettings->Sound[RS_SND_16BIT].nValue == 1)
		{
			pSettings->Sound[RS_SND_16BIT].nValue = 0;
		}
		else
		{
			pSettings->Sound[RS_SND_16BIT].nValue = 1;
		}

		if (m_SoundQuality.hMenuItem) m_pClientDE->DeleteSurface (m_SoundQuality.hMenuItem);
		if (m_SoundQuality.hMenuItemSelected) m_pClientDE->DeleteSurface (m_SoundQuality.hMenuItemSelected);
		
		m_SoundQuality.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_16BIT].nValue == 1 ? IDS_HIGH : IDS_LOW);
		m_SoundQuality.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_16BIT].nValue == 1 ? IDS_HIGH : IDS_LOW);

		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
		if (pClientShell)
		{
			pClientShell->InitSound();
		}
	}
	
	CBaseMenu::Left();
}

void CSoundOptionsMenu::Right()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == 0)
	{
		Left();
		return;
	}
	else if (m_nSelection == 1)
	{
		if (!pSettings->MusicEnabled())
		{
			pClientShell->DoMessageBox (IDS_NOMUSICCHANGE, TH_ALIGN_CENTER);
			return;
		}

		int nPos = m_sliderMusicVolume.GetPos();
		if (nPos == m_sliderMusicVolume.GetMax()) return;

		m_sliderMusicVolume.SetPos (nPos + 1);
		pSettings->Sound[RS_SND_MUSICVOL].nValue = (LTFLOAT)m_sliderMusicVolume.GetPos() * 10;
		pSettings->ImplementMusicVolume();
	}
	else if (m_nSelection == 2)
	{
		Left();
		return;
	}
	else if (m_nSelection == 3)
	{
		if (!pSettings->SoundEnabled())
		{
			pClientShell->DoMessageBox (IDS_NOSOUNDCHANGE, TH_ALIGN_CENTER);
			return;
		}

		int nPos = m_sliderSoundVolume.GetPos();
		if (nPos == m_sliderSoundVolume.GetMax()) return;

		m_sliderSoundVolume.SetPos (nPos + 1);
		pSettings->Sound[RS_SND_SOUNDVOL].nValue = (LTFLOAT)m_sliderSoundVolume.GetPos() * 9;
		pSettings->ImplementSoundVolume();
	}
	else if (m_nSelection == 4)
	{
		Left();
		return;
	}
	
	CBaseMenu::Right();
}

void CSoundOptionsMenu::PageUp()
{
	Home();
}

void CSoundOptionsMenu::PageDown()
{
	End();
}

void CSoundOptionsMenu::Home()
{
	CBaseMenu::Home();
}

void CSoundOptionsMenu::End()
{
	CBaseMenu::End();
}

void CSoundOptionsMenu::Return()
{
	if (!m_pRiotMenu) return;

	if (m_nSelection == 5)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
		CBaseMenu::Return();
	}
}

void CSoundOptionsMenu::Esc()
{
	CBaseMenu::Esc();
}

void CSoundOptionsMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);

	if (!m_pClientDE) return;

	int y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 0 ? m_MusicSource.hMenuItemSelected : m_MusicSource.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
	y += m_GenericItem[0].szMenuItem.cy + m_nMenuSpacing;
	m_sliderMusicVolume.Draw (hScreen, m_nMenuX + m_nSecondColumn, y);
	y += m_GenericItem[1].szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 2 ? m_SoundFx.hMenuItemSelected : m_SoundFx.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
	y += m_GenericItem[2].szMenuItem.cy + m_nMenuSpacing;
	m_sliderSoundVolume.Draw (hScreen, m_nMenuX + m_nSecondColumn, y);
	y += m_GenericItem[3].szMenuItem.cy + m_nMenuSpacing;
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == 4 ? m_SoundQuality.hMenuItemSelected : m_SoundQuality.hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
}

LTBOOL CSoundOptionsMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	// determine the correct setting for the music source string

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// create the menu surfaces

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND_MUSICENABLED);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND_MUSICVOL);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND_FX);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND_FXVOL);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SOUND_QUALITY);
	m_GenericItem[5].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);

	m_MusicSource.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_MUSICENABLED].nValue ? IDS_ON : IDS_OFF);
	m_SoundFx.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_FX].nValue == 1 ? IDS_ON : IDS_OFF);
	m_SoundQuality.hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pSettings->Sound[RS_SND_16BIT].nValue == 1 ? IDS_HIGH : IDS_LOW);
	
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND_MUSICENABLED);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND_MUSICVOL);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND_FX);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND_FXVOL);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SOUND_QUALITY);
	m_GenericItem[5].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);

	m_MusicSource.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_MUSICENABLED].nValue ? IDS_ON : IDS_OFF);
	m_SoundFx.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_FX].nValue == 1 ? IDS_ON : IDS_OFF);
	m_SoundQuality.hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pSettings->Sound[RS_SND_16BIT].nValue == 1 ? IDS_HIGH : IDS_LOW);

	// crop the music source setting...
	//m_MusicSource.hMenuItem = CropMenuItemTop (m_MusicSource.hMenuItem);
	//m_MusicSource.hMenuItemSelected = CropMenuItemTop (m_MusicSource.hMenuItemSelected);

	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_SOUND);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	for (int i = 0; i < 6; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

	if (!m_MusicSource.hMenuItem || !m_MusicSource.hMenuItemSelected || !m_SoundFx.hMenuItem || !m_SoundFx.hMenuItemSelected || !m_SoundQuality.hMenuItem || !m_SoundQuality.hMenuItemSelected)
	{
		UnloadSurfaces();
		return LTFALSE;
	}

	for (int i = 0; i < 6; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void CSoundOptionsMenu::UnloadSurfaces()
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

	if (m_MusicSource.hMenuItem) m_pClientDE->DeleteSurface (m_MusicSource.hMenuItem);
	if (m_MusicSource.hMenuItemSelected) m_pClientDE->DeleteSurface (m_MusicSource.hMenuItemSelected);
	m_MusicSource.hMenuItem = LTNULL;
	m_MusicSource.hMenuItemSelected = LTNULL;

	if (m_SoundFx.hMenuItem) m_pClientDE->DeleteSurface (m_SoundFx.hMenuItem);
	if (m_SoundFx.hMenuItemSelected) m_pClientDE->DeleteSurface (m_SoundFx.hMenuItemSelected);
	m_SoundFx.hMenuItem = LTNULL;
	m_SoundFx.hMenuItemSelected = LTNULL;
	
	if (m_SoundQuality.hMenuItem) m_pClientDE->DeleteSurface (m_SoundQuality.hMenuItem);
	if (m_SoundQuality.hMenuItemSelected) m_pClientDE->DeleteSurface (m_SoundQuality.hMenuItemSelected);
	m_SoundQuality.hMenuItem = LTNULL;
	m_SoundQuality.hMenuItemSelected = LTNULL;

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CSoundOptionsMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	
	m_pClientDE->GetSurfaceDims (m_MusicSource.hMenuItem, &nSettingWidth, &nSettingHeight);
	if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
	if (m_nSecondColumn + m_sliderMusicVolume.GetWidth() > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + m_sliderMusicVolume.GetWidth();
	m_pClientDE->GetSurfaceDims (m_SoundFx.hMenuItem, &nSettingWidth, &nSettingHeight);
	if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
	if (m_nSecondColumn + m_sliderSoundVolume.GetWidth() > nMenuMaxWidth) nMenuMaxWidth = m_nSecondColumn + m_sliderSoundVolume.GetWidth();
	m_pClientDE->GetSurfaceDims (m_SoundQuality.hMenuItem, &nSettingWidth, &nSettingHeight);
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
}

