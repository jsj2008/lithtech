#include "RiotClientShell.h"
#include "iltclient.h"
#include "RiotSettings.h"
#include "stdio.h"
#include "windows.h"
#include "TextHelper.h"
#include "ClientRes.h"

CRiotSettings::CRiotSettings()
{
	m_pClientDE = LTNULL;
	m_pClientShell = LTNULL;
	m_bAllowGore = TRUE;
}

//////////////////////////////////////////////////////////////////
//
//	INIT THE SETTINGS...
//
//////////////////////////////////////////////////////////////////

LTBOOL CRiotSettings::Init (ILTClient* pClientDE, CRiotClientShell* pClientShell)
{
	if (!pClientDE || !pClientShell) return LTFALSE;

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	// check if gore is allowed

	if (TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE")) m_bAllowGore = FALSE;
	else m_bAllowGore = TRUE;

	// init misc settings...

	SAFE_STRCPY(Misc[RS_MISC_VEHICLEMODE].strVarName, "VehicleMode");
	Misc[RS_MISC_VEHICLEMODE].hVar = m_pClientDE->GetConsoleVar (Misc[RS_MISC_VEHICLEMODE].strVarName);
	SAFE_STRCPY(Misc[RS_MISC_SCREENFLASH].strVarName, "ScreenFlash");
	Misc[RS_MISC_SCREENFLASH].hVar = m_pClientDE->GetConsoleVar (Misc[RS_MISC_SCREENFLASH].strVarName);

	// init control settings...

	SAFE_STRCPY(Control[RS_CTRL_MOUSELOOK].strVarName, "MouseLook");
	Control[RS_CTRL_MOUSELOOK].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_MOUSELOOK].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_MOUSEINVERTY].strVarName, "MouseInvertY");
	Control[RS_CTRL_MOUSEINVERTY].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_MOUSEINVERTY].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_MOUSESENSITIVITY].strVarName, "MouseSensitivity");
	Control[RS_CTRL_MOUSESENSITIVITY].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_MOUSESENSITIVITY].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_MOUSEINPUTRATE].strVarName, "inputrate");
	Control[RS_CTRL_MOUSEINPUTRATE].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_MOUSEINPUTRATE].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_JOYLOOK].strVarName, "JoyLook");
	Control[RS_CTRL_JOYLOOK].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_JOYLOOK].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_JOYINVERTY].strVarName, "JoyInvertY");
	Control[RS_CTRL_JOYINVERTY].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_JOYINVERTY].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_LOOKSPRING].strVarName, "LookSpring");
	Control[RS_CTRL_LOOKSPRING].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_LOOKSPRING].strVarName);
	SAFE_STRCPY(Control[RS_CTRL_RUNLOCK].strVarName, "RunLock");
	Control[RS_CTRL_RUNLOCK].hVar = m_pClientDE->GetConsoleVar (Control[RS_CTRL_RUNLOCK].strVarName);

	// init sound settings...

	SAFE_STRCPY(Sound[RS_SND_MUSICENABLED].strVarName, "MusicEnable");
	Sound[RS_SND_MUSICENABLED].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_MUSICENABLED].strVarName);
	SAFE_STRCPY(Sound[RS_SND_MUSICVOL].strVarName, "MusicVolume");
	Sound[RS_SND_MUSICVOL].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_MUSICVOL].strVarName);
	SAFE_STRCPY(Sound[RS_SND_FX].strVarName, "SoundEnable");
	Sound[RS_SND_FX].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_FX].strVarName);
	SAFE_STRCPY(Sound[RS_SND_SOUNDVOL].strVarName, "SoundVolume");
	Sound[RS_SND_SOUNDVOL].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_SOUNDVOL].strVarName);
	SAFE_STRCPY(Sound[RS_SND_CHANNELS].strVarName, "SoundChannels");
	Sound[RS_SND_CHANNELS].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_CHANNELS].strVarName);
	SAFE_STRCPY(Sound[RS_SND_16BIT].strVarName, "Sound16Bit");
	Sound[RS_SND_16BIT].hVar = m_pClientDE->GetConsoleVar (Sound[RS_SND_16BIT].strVarName);

	// init renderer settings

	LTRESULT result = m_pClientDE->GetRenderMode (&CurrentRenderer);
	if (result != LT_OK) return LTFALSE;
	
	SAFE_STRCPY(MasterPaletteMode.strVarName, "MasterPaletteMode");
	MasterPaletteMode.hVar = m_pClientDE->GetConsoleVar (MasterPaletteMode.strVarName);

	// init top-level detail settings

	SAFE_STRCPY(Detail[RS_DET_OVERALL].strVarName, "GlobalDetail");
	Detail[RS_DET_OVERALL].hVar = m_pClientDE->GetConsoleVar (Detail[RS_DET_OVERALL].strVarName);
	SAFE_STRCPY(Detail[RS_DET_GORE].strVarName, "Gore");
	Detail[RS_DET_GORE].hVar = m_pClientDE->GetConsoleVar (Detail[RS_DET_GORE].strVarName);

	// init low-level detail settings

	SAFE_STRCPY(SubDetail[RS_SUBDET_MODELLOD].strVarName, "ModelLOD");
	SubDetail[RS_SUBDET_MODELLOD].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_MODELLOD].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_SHADOWS].strVarName, "MaxModelShadows");
	SubDetail[RS_SUBDET_SHADOWS].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_SHADOWS].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_BULLETHOLES].strVarName, "BulletHoles");
	SubDetail[RS_SUBDET_BULLETHOLES].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_BULLETHOLES].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_TEXTUREDETAIL].strVarName, "TextureDetail");
	SubDetail[RS_SUBDET_TEXTUREDETAIL].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_TEXTUREDETAIL].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_DYNAMICLIGHTING].strVarName, "DynamicLightSetting");
	SubDetail[RS_SUBDET_DYNAMICLIGHTING].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_DYNAMICLIGHTING].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_LIGHTMAPPING].strVarName, "LightMap");
	SubDetail[RS_SUBDET_LIGHTMAPPING].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_LIGHTMAPPING].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_SPECIALFX].strVarName, "SpecialFX");
	SubDetail[RS_SUBDET_SPECIALFX].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_SPECIALFX].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_ENVMAPPING].strVarName, "EnvMapEnable");
	SubDetail[RS_SUBDET_ENVMAPPING].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_ENVMAPPING].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_MODELFB].strVarName, "ModelFullbrite");
	SubDetail[RS_SUBDET_MODELFB].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_MODELFB].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_CLOUDMAPLIGHT].strVarName, "CloudMapLight");
	SubDetail[RS_SUBDET_CLOUDMAPLIGHT].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_CLOUDMAPLIGHT].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_PVWEAPONS].strVarName, "PVWeapons");
	SubDetail[RS_SUBDET_PVWEAPONS].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_PVWEAPONS].strVarName);
	SAFE_STRCPY(SubDetail[RS_SUBDET_POLYGRIDS].strVarName, "PolyGrids");
	SubDetail[RS_SUBDET_POLYGRIDS].hVar = m_pClientDE->GetConsoleVar (SubDetail[RS_SUBDET_POLYGRIDS].strVarName);

	// read the current settings
	
	ReadSettings();

	// init the default detail settings

	Setting temp[RS_SUBDET_LAST + 1];
	memcpy (temp, SubDetail, sizeof(SubDetail));

	pClientDE->ReadConfigFile ("DetailHi.cfg");
	ReadSubDetailSettings();
	memcpy (DefHi, SubDetail, sizeof(SubDetail));
	pClientDE->ReadConfigFile ("DetailMd.cfg");
	ReadSubDetailSettings();
	memcpy (DefMed, SubDetail, sizeof(SubDetail));
	pClientDE->ReadConfigFile ("DetailLo.cfg");
	ReadSubDetailSettings();
	memcpy (DefLow, SubDetail, sizeof(SubDetail));

	// now depending on the global detail setting, set the correct sub-detail settings

	switch ((int)Detail[RS_DET_OVERALL].nValue)
	{
		case 0: memcpy (SubDetail, DefLow, sizeof (SubDetail));	break;
		case 1: memcpy (SubDetail, DefMed, sizeof (SubDetail));	break;
		case 2: memcpy (SubDetail, DefHi, sizeof (SubDetail));	break;
		case 3: memcpy (SubDetail, temp, sizeof(SubDetail)); break;
	}
	
	if (m_pClientShell->AdvancedDisableLightMap())
	{
		Detail[RS_DET_OVERALL].nValue = 3.0f;
		SubDetail[RS_SUBDET_LIGHTMAPPING].nValue = 0.0f;
	}

	if (m_pClientShell->AdvancedDisableModelFB())
	{
		Detail[RS_DET_OVERALL].nValue = 3.0f;
		SubDetail[RS_SUBDET_MODELFB].nValue = 0.0f;
	}

	// now write the SubDetail settings back out so that they get set

	WriteSubDetailSettings();

	// implement settings that need implementing

	ImplementMouseSensitivity();

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
	{
		ImplementDetailSetting (i);
	}
	
	return LTTRUE;
}


//////////////////////////////////////////////////////////////////
//
//	READ/WRITE ALL SETTINGS
//
//////////////////////////////////////////////////////////////////

LTBOOL CRiotSettings::ReadSettings ()
{
	ReadMiscSettings();
	ReadControlSettings();
	ReadSoundSettings();
	ReadDetailSettings();
	ReadSubDetailSettings();
	ReadDisplayModeSettings();

	return LTTRUE;
}

void CRiotSettings::WriteSettings ()
{
	WriteMiscSettings();
	WriteControlSettings();
	WriteSoundSettings();
	WriteDetailSettings();
	WriteSubDetailSettings();
	WriteDisplayModeSettings();
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE MISC SETTINGS
//
//////////////////////////////////////////////////////////////////
	
void CRiotSettings::ReadMiscSettings()
{
	if (!m_pClientDE) return;

	for (int i = RS_MISC_FIRST; i <= RS_MISC_LAST; i++)
	{
		if (!Misc[i].hVar)
		{
			Misc[i].hVar = m_pClientDE->GetConsoleVar (Misc[i].strVarName);
			if (!Misc[i].hVar) continue;
		}
		
		if (Misc[i].bStringVar)
		{
			SAFE_STRCPY(Misc[i].strValue, m_pClientDE->GetVarValueString (Misc[i].hVar));
		}
		else
		{
			Misc[i].nValue = m_pClientDE->GetVarValueFloat (Misc[i].hVar);
		}
	}
}

void CRiotSettings::WriteMiscSettings()
{
	if (!m_pClientDE) return;

	char str[128];
	for (int i = RS_MISC_FIRST; i <= RS_MISC_LAST; i++)
	{
		if (Misc[i].bStringVar)
		{
			sprintf (str, "+%s \"%s\"", Misc[i].strVarName, Misc[i].strValue);
		}
		else
		{
			sprintf (str, "+%s %f", Misc[i].strVarName, Misc[i].nValue);
		}
		m_pClientDE->RunConsoleString (str);
	}
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE CONTROL SETTINGS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::ReadControlSettings()
{
	if (!m_pClientDE) return;

	for (int i = RS_CTRL_FIRST; i <= RS_CTRL_LAST; i++)
	{
		if (!Control[i].hVar)
		{
			Control[i].hVar = m_pClientDE->GetConsoleVar (Control[i].strVarName);
			if (!Control[i].hVar) continue;
		}
		
		if (Control[i].bStringVar)
		{
			SAFE_STRCPY(Control[i].strValue, m_pClientDE->GetVarValueString (Control[i].hVar));
		}
		else
		{
			Control[i].nValue = m_pClientDE->GetVarValueFloat (Control[i].hVar);
		}
	}
}

void CRiotSettings::WriteControlSettings()
{
	if (!m_pClientDE) return;

	char str[128];
	for (int i = RS_CTRL_FIRST; i <= RS_CTRL_LAST; i++)
	{
		if (Control[i].bStringVar)
		{
			sprintf (str, "+%s \"%s\"", Control[i].strVarName, Control[i].strValue);
		}
		else
		{
			sprintf (str, "+%s %f", Control[i].strVarName, Control[i].nValue);
		}
		m_pClientDE->RunConsoleString (str);
	}
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE SOUND SETTINGS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::ReadSoundSettings()
{
	if (!m_pClientDE) return;

	for (int i = RS_SND_FIRST; i <= RS_SND_LAST; i++)
	{
		if (!Sound[i].hVar)
		{
			Sound[i].hVar = m_pClientDE->GetConsoleVar (Sound[i].strVarName);
			if (!Sound[i].hVar) continue;
		}

		if (Sound[i].bStringVar)
		{
			SAFE_STRCPY(Sound[i].strValue, m_pClientDE->GetVarValueString (Sound[i].hVar));
		}
		else
		{
			Sound[i].nValue = m_pClientDE->GetVarValueFloat (Sound[i].hVar);
		}
	}

	// hack to keep sound volume reasonable
	if (Sound[RS_SND_SOUNDVOL].nValue > 90.0f) Sound[RS_SND_SOUNDVOL].nValue = 90.0f;

	ImplementMusicSource();
	ImplementMusicVolume();
	ImplementSoundEnabled();
	ImplementSoundVolume();
	ImplementSoundQuality();
}

void CRiotSettings::WriteSoundSettings()
{
	if (!m_pClientDE || !m_pClientShell) return;

	char str[128];
	for (int i = RS_SND_FIRST; i <= RS_SND_LAST; i++)
	{
		if (i == RS_SND_MUSICENABLED)
		{
			if (m_pClientShell->AdvancedDisableMusic())
			{
				m_pClientDE->RunConsoleString ("musicenable 0");
			}
			else
			{
				m_pClientDE->RunConsoleString ("musicenable 1");
			}
			continue;
		}

		if (i == RS_SND_FX)
		{
			if (m_pClientShell->AdvancedDisableSound())
			{
				m_pClientDE->RunConsoleString ("soundenable 0");
			}
			else
			{
				m_pClientDE->RunConsoleString ("soundenable 1");
			}
			continue;
		}

		if (Sound[i].bStringVar)
		{
			sprintf (str, "+%s \"%s\"", Sound[i].strVarName, Sound[i].strValue);
		}
		else
		{
			sprintf (str, "+%s %f", Sound[i].strVarName, Sound[i].nValue);
		}
		m_pClientDE->RunConsoleString (str);
	}
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE DETAIL SETTINGS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::ReadDetailSettings()
{
	if (!m_pClientDE || !m_pClientShell) return;

	for (int i = RS_DET_FIRST; i <= RS_DET_LAST; i++)
	{
		if (!Detail[i].hVar)
		{
			Detail[i].hVar = m_pClientDE->GetConsoleVar (Detail[i].strVarName);
			if (!Detail[i].hVar) continue;
		}
		
		if (Detail[i].bStringVar)
		{
			SAFE_STRCPY(Detail[i].strValue, m_pClientDE->GetVarValueString (Detail[i].hVar));
		}
		else
		{
			Detail[i].nValue = m_pClientDE->GetVarValueFloat (Detail[i].hVar);
		}
	}

	// if they are using any other global detail setting than "advanced", read in the appropriate file...

	if (*DefLow[0].strVarName && *DefMed[0].strVarName && *DefHi[0].strVarName)
	{
		switch ((int)Detail[RS_DET_OVERALL].nValue)
		{
			case 0: memcpy (SubDetail, DefLow, sizeof (SubDetail));	break;
			case 1: memcpy (SubDetail, DefMed, sizeof (SubDetail));	break;
			case 2: memcpy (SubDetail, DefHi, sizeof (SubDetail));	break;
			case 3: ReadSubDetailSettings();
		}
		
	
		if (m_pClientShell->AdvancedDisableLightMap())
		{
			Detail[RS_DET_OVERALL].nValue = 3.0f;
			SubDetail[RS_SUBDET_LIGHTMAPPING].nValue = 0.0f;
		}

		if (m_pClientShell->AdvancedDisableModelFB())
		{
			Detail[RS_DET_OVERALL].nValue = 3.0f;
			SubDetail[RS_SUBDET_MODELFB].nValue = 0.0f;
		}

		WriteSubDetailSettings();
		for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
		{
			ImplementDetailSetting (i);
		}
	}
}

void CRiotSettings::WriteDetailSettings()
{
	if (!m_pClientDE) return;

	char str[128];

	for (int i = RS_DET_FIRST; i <= RS_DET_LAST; i++)
	{
		if (Detail[i].bStringVar)
		{
			sprintf (str, "+%s \"%s\"", Detail[i].strVarName, Detail[i].strValue);
		}
		else
		{
			sprintf (str, "+%s %f", Detail[i].strVarName, Detail[i].nValue);
		}

		m_pClientDE->RunConsoleString (str);
	}
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE SUB-DETAIL SETTINGS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::ReadSubDetailSettings()
{
	if (!m_pClientDE) return;

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
	{
		if (!SubDetail[i].hVar)
		{
			SubDetail[i].hVar = m_pClientDE->GetConsoleVar (SubDetail[i].strVarName);
			if (!SubDetail[i].hVar) continue;
		}

		if (SubDetail[i].bStringVar)
		{
			SAFE_STRCPY(SubDetail[i].strValue, m_pClientDE->GetVarValueString (SubDetail[i].hVar));
		}
		else
		{
			SubDetail[i].nValue = m_pClientDE->GetVarValueFloat (SubDetail[i].hVar);
		}
	}

	// check special-case values

	ImplementDetailSetting (RS_SUBDET_DYNAMICLIGHTING);
	ImplementDetailSetting (RS_SUBDET_TEXTUREDETAIL);
	ImplementDetailSetting (RS_SUBDET_MODELLOD);
}

void CRiotSettings::WriteSubDetailSettings()
{
	if (!m_pClientDE) return;

	char str[128];

	for (int i = RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
	{
		if (SubDetail[i].bStringVar)
		{
			sprintf (str, "+%s \"%s\"", SubDetail[i].strVarName, SubDetail[i].strValue);
		}
		else
		{
			sprintf (str, "+%s %f", SubDetail[i].strVarName, SubDetail[i].nValue);
		}

		m_pClientDE->RunConsoleString (str);
	}
}

//////////////////////////////////////////////////////////////////
//
//	READ/WRITE DISPLAY MODE SETTINGS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::ReadDisplayModeSettings()
{
	if (!m_pClientDE) return;

	if (!MasterPaletteMode.hVar)
	{
		MasterPaletteMode.hVar = m_pClientDE->GetConsoleVar (MasterPaletteMode.strVarName);
		if (!MasterPaletteMode.hVar)
		{
			MasterPaletteMode.nValue = 2.0f;
			ImplementBitDepth();
			return;
		}
	}

	MasterPaletteMode.nValue = m_pClientDE->GetVarValueFloat (MasterPaletteMode.hVar);
	if (MasterPaletteMode.nValue != 1.0f && MasterPaletteMode.nValue != 2.0f)
	{
		MasterPaletteMode.nValue = 2.0f;
		ImplementBitDepth();
	}
}

void CRiotSettings::WriteDisplayModeSettings()
{
	if (!m_pClientDE) return;

	char str[128];

	sprintf (str, "+%s %f", MasterPaletteMode.strVarName, MasterPaletteMode.nValue);
	m_pClientDE->RunConsoleString (str);

/*
	sprintf (str, "+RenderDll \"%s\"", CurrentRenderer.m_RenderDLL);
	m_pClientDE->RunConsoleString (str);

	sprintf (str, "+ScreenWidth %d", CurrentRenderer.m_Width);
	m_pClientDE->RunConsoleString (str);

	sprintf (str, "+ScreenHeight %d", CurrentRenderer.m_Height);
	m_pClientDE->RunConsoleString (str);

	sprintf (str, "+Carddesc \"%s\"", CurrentRenderer.m_InternalName);
	m_pClientDE->RunConsoleString (str);
*/
}

//////////////////////////////////////////////////////////////////
//
//	SET DEFAULT DETAIL LEVELS
//
//////////////////////////////////////////////////////////////////

void CRiotSettings::SetLowDetail()
{
	memcpy (SubDetail, DefLow, sizeof(SubDetail));
	WriteSubDetailSettings();
}

void CRiotSettings::SetMedDetail()
{
	memcpy (SubDetail, DefMed, sizeof(SubDetail));
	WriteSubDetailSettings();
}

void CRiotSettings::SetHiDetail()
{
	memcpy (SubDetail, DefHi, sizeof(SubDetail));
	WriteSubDetailSettings();
}

//////////////////////////////////////////////////////////////////
//
//	IMPLEMENT CURRENT RENDERER SETTING
//
//////////////////////////////////////////////////////////////////

LTBOOL CRiotSettings::ImplementRendererSetting()
{
	if (!m_pClientDE || !m_pClientShell) return LTFALSE;

	// make sure the active mode isn't what we're trying to set...

	RMode current;
	memset (&current, 0, sizeof (RMode));
	if (m_pClientDE->GetRenderMode (&current) != LT_OK) return LTFALSE;

	if (/*strcmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) == 0 &&*/ strcmp (CurrentRenderer.m_Description, current.m_Description) == 0)
	{
		if (CurrentRenderer.m_Width == current.m_Width && CurrentRenderer.m_Height == current.m_Height)
		{
			return LTTRUE;
		}
	}

	// attempt to set the render mode

	m_pClientShell->m_bSwitchingModes = LTTRUE;
	LTRESULT hResult = m_pClientDE->SetRenderMode (&CurrentRenderer);
	m_pClientShell->m_bSwitchingModes = LTFALSE;

	if (hResult == LT_KEPTSAMEMODE || hResult == LT_UNABLETORESTOREVIDEO)
	{
		if (hResult == LT_KEPTSAMEMODE)
		{
			// reset the structure
			m_pClientDE->GetRenderMode (&CurrentRenderer);
		}
		return LTFALSE;
	}

	m_pClientDE->GetRenderMode (&current);
	if (stricmp (CurrentRenderer.m_Description, current.m_Description) != 0 ||
		//stricmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) != 0 ||
		CurrentRenderer.m_Width != current.m_Width || CurrentRenderer.m_Height != current.m_Height)
	{
		// SetRenderMode() returned success, but it really didn't work
		// reset the structure
		m_pClientDE->GetRenderMode (&CurrentRenderer);
		return LTFALSE;
	}

	// adjust the screen and camera rects

	uint32 nScreenWidth = 0;
	uint32 nScreenHeight = 0;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	int nLeft = 0;
	int nTop = 0;
	int nRight = (int)nScreenWidth;
	int nBottom = (int)nScreenHeight;
	bool bFullScreen = true;
	m_pClientDE->SetCameraRect (m_pClientShell->GetCamera(), bFullScreen, nLeft, nTop, nRight, nBottom);
	m_pClientShell->ResetMenuRestoreCamera (0, 0, nRight, nBottom);
	m_pClientShell->GetMenu()->ScreenDimsChanged();

	// make sure the structure is completely filled in

	m_pClientDE->GetRenderMode (&CurrentRenderer);
			

	// Save the new render mode values to the console...

	// WriteDisplayModeSettings();
	

	return LTTRUE;
}

void CRiotSettings::ImplementMusicSource()
{
	if (!m_pClientDE || !m_pClientShell) return;

	char str[64];
	CMusic::EMusicLevel level;

	if (Sound[RS_SND_MUSICENABLED].nValue && !m_pClientShell->AdvancedDisableMusic())
	{
		SAFE_STRCPY(str, "musicenable 1");
		m_pClientDE->RunConsoleString (str);

		if (m_pClientShell->GetMusic()->IsInitialized())
		{
			// Only re-init if not already using ima...
			if (!m_pClientShell->GetMusic()->UsingIMA())
			{
				// preserve the music level, just in case we come back to ima...
				level = m_pClientShell->GetMusic()->GetMusicLevel();
				if (m_pClientShell->GetMusic()->Init (m_pClientDE, LTTRUE))
				{
					// Only start the music if we are in a world...
					if (m_pClientShell->IsInWorld() && !m_pClientShell->IsFirstUpdate())
					{
						m_pClientShell->GetMusic()->InitPlayLists();
						m_pClientDE->PauseMusic();
						m_pClientShell->GetMusic()->PlayMusicLevel (level);
					}
				}
			}
		}
		// music should already be inited, but let try again...
		else if (m_pClientShell->GetMusic()->Init (m_pClientDE, LTTRUE))
		{
			// Only re-init the play lists if we have been in world for a while...
			if (m_pClientShell->IsInWorld() && !m_pClientShell->IsFirstUpdate())
			{
				level = m_pClientShell->GetMusic()->GetMusicLevel();
				m_pClientShell->GetMusic()->InitPlayLists();
				m_pClientDE->PauseMusic();
				m_pClientShell->GetMusic()->PlayMusicLevel (level);
			}
		}
	}
	else if (m_pClientShell->AdvancedDisableMusic())
	{
		Sound[RS_SND_MUSICENABLED].nValue = 0;
		m_pClientDE->RunConsoleString ("musicenable 0");
	}
	else
	{
		Sound[RS_SND_MUSICENABLED].nValue = 0;
		SAFE_STRCPY(str, "musicenable 0");
		m_pClientDE->RunConsoleString (str);
	}
}

void CRiotSettings::ImplementMusicVolume()
{
	if (!m_pClientDE) return;

	LTFLOAT nMusicVolume = Sound[RS_SND_MUSICVOL].nValue;

	m_pClientDE->SetMusicVolume ((short)nMusicVolume);

	char strConsole[64];
	sprintf (strConsole, "+MusicVolume %d", nMusicVolume);
	m_pClientDE->RunConsoleString (strConsole);
}

void CRiotSettings::ImplementSoundEnabled()
{
	if (!m_pClientDE || !m_pClientShell) return;

	if (m_pClientShell->AdvancedDisableSound())
	{
		Sound[RS_SND_FX].nValue = 0;
		m_pClientDE->RunConsoleString ("SoundEnable 0");
		return;
	}

	char strConsole[64];
	sprintf (strConsole, "SoundEnable %f", Sound[RS_SND_FX].nValue);
	m_pClientDE->RunConsoleString (strConsole);
}

void CRiotSettings::ImplementSoundVolume()
{
	if (!m_pClientDE) return;

	LTFLOAT nSoundVolume = Sound[RS_SND_SOUNDVOL].nValue;

	m_pClientDE->SetSoundVolume ((short)nSoundVolume);

	char strConsole[64];
	sprintf (strConsole, "+SoundVolume %f", nSoundVolume);
	m_pClientDE->RunConsoleString (strConsole);
}

void CRiotSettings::ImplementSoundQuality()
{
	if (!m_pClientShell) return;

	m_pClientShell->InitSound();
}

void CRiotSettings::ImplementMouseSensitivity()
{
	if (!m_pClientDE) return;

	LTFLOAT nMouseSensitivity = Control[RS_CTRL_MOUSESENSITIVITY].nValue;

	// get the mouse device name

	char strDevice[128];
	memset (strDevice, 0, 128);
	LTRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_MOUSE, strDevice, 127);
	if (result == LT_OK)
	{
		// get mouse x- and y- axis names

		char strXAxis[32];
		memset (strXAxis, 0, 32);
		char strYAxis[32];
		memset (strYAxis, 0, 32);

		LTBOOL bFoundXAxis = LTFALSE;
		LTBOOL bFoundYAxis = LTFALSE;
		
		DeviceObject* pList = m_pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
		DeviceObject* ptr = pList;
		while (ptr)
		{
			if (ptr->m_ObjectType == CONTROLTYPE_XAXIS)
			{
				SAFE_STRCPY(strXAxis, ptr->m_ObjectName);
				bFoundXAxis = LTTRUE;
			}

			if (ptr->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				SAFE_STRCPY(strYAxis, ptr->m_ObjectName);
				bFoundYAxis = LTTRUE;
			}

			ptr = ptr->m_pNext;
		}
		if (pList) m_pClientDE->FreeDeviceObjects (pList);
		
		if (bFoundXAxis && bFoundYAxis)
		{
			// run the console string

			char strConsole[64];
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strXAxis, 0.00125f + ((LTFLOAT)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strYAxis, 0.00125f + ((LTFLOAT)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
		}
	}
}

void CRiotSettings::ImplementInputRate()
{
	if (!m_pClientDE) return;

	char strConsole[64];
	sprintf (strConsole, "inputrate %f", Control[RS_CTRL_MOUSEINPUTRATE].nValue);
	m_pClientDE->RunConsoleString (strConsole);
}

void CRiotSettings::ImplementDetailSetting (int nSetting)
{
	if (!m_pClientDE || !m_pClientShell) return;

	char str[128];

	switch (nSetting)
	{
		case RS_SUBDET_MODELLOD:
		{
			sprintf (str, "+ModelLOD %d", (int)ModelLOD());
			m_pClientDE->RunConsoleString (str);

			switch ((int)SubDetail[RS_SUBDET_MODELLOD].nValue)
			{
				default:
				{
					m_pClientDE->RunConsoleString ("LODOffset 0");
					m_pClientDE->RunConsoleString ("LODScale 0.9");
				}
				break;

				case 1:
				{
					m_pClientDE->RunConsoleString ("LODOffset 0");
					m_pClientDE->RunConsoleString ("LODScale 1");
				}
				break;

				case 2:
				{
					m_pClientDE->RunConsoleString ("LODOffset -200");
					m_pClientDE->RunConsoleString ("LODScale 2");
				}
				break;
			}
		}
		break;
		
		case RS_SUBDET_SHADOWS:
		{
			sprintf (str, "+MaxModelShadows %d", Shadows() ? 1 : 0);
			m_pClientDE->RunConsoleString (str);
		}
		break;

		case RS_SUBDET_BULLETHOLES:
		{
		}
		break;

		case RS_SUBDET_TEXTUREDETAIL:
		{
			switch ((int)SubDetail[RS_SUBDET_TEXTUREDETAIL].nValue)
			{
				default:
				{
					SAFE_STRCPY(str, "+GroupOffset0 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY(str, "+GroupOffset1 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY(str, "+GroupOffset2 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY(str, "+GroupOffset3 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY(str, "+GroupOffset4 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY(str, "+GroupOffset5 1");
					m_pClientDE->RunConsoleString (str);
				}
				break;

				case 1:
				{
					SAFE_STRCPY(str, "+GroupOffset0 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset1 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset2 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset3 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset4 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset5 0");
					m_pClientDE->RunConsoleString (str);
				}
				break;

				case 2:
				{
					SAFE_STRCPY (str, "+GroupOffset0 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset1 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset2 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset3 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset4 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+GroupOffset5 0");
					m_pClientDE->RunConsoleString (str);
				}
				break;
			}
		}
		break;
		
		case RS_SUBDET_DYNAMICLIGHTING:
		{
			switch ((int)SubDetail[RS_SUBDET_DYNAMICLIGHTING].nValue)
			{
				default:
				{
					SAFE_STRCPY (str, "+DynamicLight 0");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+FastLight 0");
					m_pClientDE->RunConsoleString (str);
				}
				break;
				
				case 1:
				{
					SAFE_STRCPY (str, "+DynamicLight 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+FastLight 1");
					m_pClientDE->RunConsoleString (str);
				}
				break;
				
				case 2:
				{
					SAFE_STRCPY (str, "+DynamicLight 1");
					m_pClientDE->RunConsoleString (str);
					SAFE_STRCPY (str, "+FastLight 0");
					m_pClientDE->RunConsoleString (str);
				}
				break;
			}
		}
		break;
		
		case RS_SUBDET_LIGHTMAPPING:
		{
			if (m_pClientShell->AdvancedDisableLightMap())
			{
				m_pClientDE->RunConsoleString ("+LightMap 0");
				break;
			}

			sprintf (str, "+LightMap %d", SubDetail[RS_SUBDET_LIGHTMAPPING].nValue ? 1 : 0);
			m_pClientDE->RunConsoleString (str);
		}
		break;

		case RS_SUBDET_SPECIALFX:
		{
		}
		break;

		case RS_SUBDET_ENVMAPPING:
		{
			sprintf (str, "+EnvMapEnable %d", EnvironmentMapping() ? 1 : 0);
			m_pClientDE->RunConsoleString (str);
		}
		break;

		case RS_SUBDET_MODELFB:
		{
			if (m_pClientShell->AdvancedDisableModelFB()) 
			{
				m_pClientDE->RunConsoleString ("+ModelFullbrite 0");
				break;
			}

			sprintf (str, "+ModelFullbrite %d", ModelFullBrights() ? 1 : 0);
			m_pClientDE->RunConsoleString (str);
		}
		break;

		case RS_SUBDET_CLOUDMAPLIGHT:
		{
			sprintf (str, "+CloudMapLight %d", CloudMapLight() ? 1 : 0);
			m_pClientDE->RunConsoleString (str);
		}
		break;

		case RS_SUBDET_PVWEAPONS:
		{
		}
		break;

		case RS_SUBDET_POLYGRIDS:
		{
		}
		break;
	}
}

void CRiotSettings::ImplementBitDepth()
{
	if (!m_pClientDE) return;

	if (Textures8Bit())
	{
		m_pClientDE->RunConsoleString ("+MasterPaletteMode 1");
	}
	else
	{
		m_pClientDE->RunConsoleString ("+MasterPaletteMode 2");
	}

	m_pClientDE->RunConsoleString ("rebindtextures");
}
