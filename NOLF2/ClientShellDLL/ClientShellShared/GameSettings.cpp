// ----------------------------------------------------------------------- //
//
// MODULE  : GameSettings.cpp
//
// PURPOSE : Handles implementation of various game settings
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "GameSettings.h"
#include "stdio.h"
#include "windows.h"
#include "GameClientShell.h"
#include "VarTrack.h"

VarTrack	g_vtMouseScaleBase;
VarTrack	g_vtMouseScaleInc;

extern CGameClientShell* g_pGameClientShell;

CGameSettings::CGameSettings()
{
    m_pClientDE = LTNULL;
    m_pClientShell = LTNULL;
}

//////////////////////////////////////////////////////////////////
//
//	INIT THE SETTINGS...
//
//////////////////////////////////////////////////////////////////

LTBOOL CGameSettings::Init (ILTClient* pClientDE, CGameClientShell* pClientShell)
{
    if (!pClientDE || !pClientShell) return LTFALSE;

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	g_vtMouseScaleBase.Init(g_pLTClient, "MouseScaleBase", NULL, 0.00125f);
	g_vtMouseScaleInc.Init(g_pLTClient, "MouseScaleIncrement", NULL, 0.001125f);

	// check if gore is allowed

    uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	if (dwAdvancedOptions & AO_MUSIC)
	{
		m_pClientDE->RunConsoleString("musicenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("musicenable 0");
	}
	if (dwAdvancedOptions & AO_SOUND)
	{
		m_pClientDE->RunConsoleString("soundenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("soundenable 0");
	}

	// implement settings that need implementing

    ImplementMouseSensitivity();

	// hack to keep sound volume reasonable
	if (SoundVolume() > 100.0f)
		SetFloatVar("SoundVolume",100.0f);

  	ImplementSoundVolume();
	ImplementSoundQuality();

    return LTTRUE;
}







void CGameSettings::ImplementMusicSource()
{
	if (!m_pClientDE || !m_pClientShell) return;

	ASSERT(!"Don't call this");

    LTBOOL bPlay = MusicEnabled();

	if (bPlay)
	{
		// [KLS 8/11/02] SetupMusic will handle turning on the
		// music and making sure it plays the appropriate music.
		g_pInterfaceMgr->SetupMusic();
	}
	else 
	{
		m_pClientShell->GetMusic()->Term();
	}

}

void CGameSettings::ImplementMusicVolume()
{
	if (!m_pClientDE) return;
	ASSERT(!"Don't call this");

	float nMusicVolume = MusicVolume();

	g_pGameClientShell->GetMusic()->SetMenuVolume((long)nMusicVolume);
}

void CGameSettings::ImplementSoundVolume()
{
	if (!m_pClientDE) return;

	float nSoundVolume = SoundVolume();

	((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetVolume((short)nSoundVolume);

	// set up the sound volume classes for the game
	float fMultiplier;
	fMultiplier = WeaponsMultiplier();
	((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundClassMultiplier( WEAPONS_SOUND_CLASS, fMultiplier );
	fMultiplier = SpeechMultiplier();
	((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundClassMultiplier( SPEECH_SOUND_CLASS, fMultiplier, false );
	fMultiplier = DefaultMultiplier();
	((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundClassMultiplier( DEFAULT_SOUND_CLASS, fMultiplier );

	// now have the sound manager update any playing sounds
	((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->UpdateVolumeSettings();
}

void CGameSettings::ImplementSoundQuality()
{
	if (!m_pClientShell) return;

	m_pClientShell->InitSound();
}

void CGameSettings::ImplementMouseSensitivity()
{
	if (!m_pClientDE) return;

	float nMouseSensitivity = GetFloatVar("MouseSensitivity");

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
				SAFE_STRCPY(strXAxis, "##x-axis");
                bFoundXAxis = LTTRUE;
			}

			if (ptr->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				SAFE_STRCPY(strYAxis, "##y-axis");
                bFoundYAxis = LTTRUE;
			}

			ptr = ptr->m_pNext;
		}
		if (pList) m_pClientDE->FreeDeviceObjects (pList);

		if (bFoundXAxis && bFoundYAxis)
		{
			// run the console string

			char strConsole[64];
			float fBaseScale = g_vtMouseScaleBase.GetFloat();
			float fScaleIncrement = g_vtMouseScaleInc.GetFloat();

			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strXAxis, fBaseScale + ((float)nMouseSensitivity * fScaleIncrement));
			m_pClientDE->RunConsoleString (strConsole);
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strYAxis, fBaseScale + ((float)nMouseSensitivity * fScaleIncrement));
			m_pClientDE->RunConsoleString (strConsole);
		}
	}
}




