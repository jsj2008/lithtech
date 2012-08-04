#include "stdafx.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "GameSettings.h"
#include "stdio.h"
#include "windows.h"
#include "ClientRes.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

CGameSettings::CGameSettings()
{
    m_pClientDE = LTNULL;
    m_pClientShell = LTNULL;
	m_bAllowGore = LTTRUE;
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

	// check if gore is allowed

	HSTRING hStr = g_pLTClient->FormatString(IDS_ALLOW_NO_GORE);
	m_bAllowGore = (stricmp(g_pLTClient->GetStringData(hStr),"TRUE") != 0);
	g_pLTClient->FreeString(hStr);

	if (!m_bAllowGore)
	{
		m_pClientDE->RunConsoleString("gore 0");
	}


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

	if (dwAdvancedOptions & AO_JOYSTICK && UseJoystick())
	{
		char strJoystick[128];
		memset (strJoystick, 0, 128);
        LTRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
		if (result == LT_OK)
		{
			char strConsole[256];
			sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
			m_pClientDE->RunConsoleString (strConsole);
			m_pClientDE->RunConsoleString("UseJoystick 1");
		}
		else
		{
			m_pClientDE->RunConsoleString("UseJoystick 0");
		}

	}
	else
	{
		m_pClientDE->RunConsoleString("UseJoystick 0");
	}

	// implement settings that need implementing

    ImplementMouseSensitivity();


	ImplementMusicSource();
	ImplementMusicVolume();
	// hack to keep sound volume reasonable
	if (SoundVolume() > 100.0f)
		SetFloatVar("SoundVolume",100.0f);
	ImplementSoundVolume();
	ImplementSoundQuality();

    return LTTRUE;
}







//////////////////////////////////////////////////////////////////
//
//	IMPLEMENT CURRENT RENDERER SETTING
//
//////////////////////////////////////////////////////////////////
LTBOOL CGameSettings::ImplementRendererSetting()
{
    if (!m_pClientDE || !m_pClientShell) return LTFALSE;

	// make sure the active mode isn't what we're trying to set...

	RMode current;
	memset (&current, 0, sizeof (RMode));
    if (m_pClientDE->GetRenderMode (&current) != LT_OK) return LTFALSE;

	if (strcmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) == 0 && strcmp (CurrentRenderer.m_Description, current.m_Description) == 0)
	{
		if (CurrentRenderer.m_Width == current.m_Width && CurrentRenderer.m_Height == current.m_Height)
		{
            return LTTRUE;
		}
	}

	// attempt to set the render mode

    g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
    LTRESULT hResult = m_pClientDE->SetRenderMode(&CurrentRenderer);
    g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);

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
		stricmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) != 0 ||
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
    LTBOOL bFullScreen = LTTRUE;
	m_pClientDE->SetCameraRect (m_pClientShell->GetCamera(), bFullScreen, nLeft, nTop, nRight, nBottom);
	m_pClientDE->SetCameraRect (m_pClientShell->GetInterfaceCamera(), bFullScreen, nLeft, nTop, nRight, nBottom);
	g_pInterfaceMgr->ResetMenuRestoreCamera (0, 0, nRight, nBottom);
	g_pInterfaceMgr->ScreenDimsChanged();

	// make sure the structure is completely filled in

	m_pClientDE->GetRenderMode (&CurrentRenderer);


    return LTTRUE;
}

void CGameSettings::ImplementMusicSource()
{
	if (!m_pClientDE || !m_pClientShell) return;

    uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

    LTBOOL bPlay = MusicEnabled() && (dwAdvancedOptions & AO_MUSIC);

	if (bPlay)
	{
		if (m_pClientShell->GetMusic()->Init(m_pClientDE))
		{
			m_pClientShell->GetMusic()->Play();
		}
	}
	else if (!MusicEnabled())
	{
		m_pClientShell->GetMusic()->Stop();
	}
	else if (!(dwAdvancedOptions & AO_MUSIC))
	{
		m_pClientDE->RunConsoleString("musicenable 0");
		m_pClientShell->GetMusic()->Term();
	}

}

void CGameSettings::ImplementMusicVolume()
{
	if (!m_pClientDE) return;

	float nMusicVolume = MusicVolume();

	g_pGameClientShell->GetMusic()->SetMenuVolume((long)nMusicVolume);

	//m_pClientDE->SetMusicVolume ((short)nMusicVolume);

}


void CGameSettings::ImplementSoundVolume()
{
	if (!m_pClientDE) return;

	float nSoundVolume = SoundVolume();

	m_pClientDE->SetSoundVolume ((short)nSoundVolume);
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
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strXAxis, 0.00125f + ((float)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strYAxis, 0.00125f + ((float)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
		}
	}
}




namespace
{
	const int numDemoSettings = 22;
	char demoSettings[numDemoSettings][32] =
	{
		"MouseLook",
		"MouseInvertY",
		"MouseSensitivity",
		"InputRate",
		"JoyLook",
		"JoyInvertY",
		"LookSpring",
		"RunLock",
		"Gore",
		"ModelLOD",
		"MaxModelShadows",
		"BulletHoles",
		"TextureDetail",
		"DynamicLightSetting",
		"LightMap",
		"SpecialFX",
		"EnvMapEnable",
		"ModelFullbrite",
		"CloudMapLight",
		"PVWeapons",
		"PolyGrids",
	};
};


void CGameSettings::LoadDemoSettings(ILTStream *pStream)
{
	for(int i=0; i <= numDemoSettings; i++)
	{
		float temp;
		(*pStream) >> temp;
		SetFloatVar(demoSettings[i],temp);
	}
}


void CGameSettings::SaveDemoSettings(ILTStream *pStream)
{
	for(int i=0; i <= numDemoSettings; i++)
	{
		float temp = GetFloatVar(demoSettings[i]);
		(*pStream) << temp;
	}
}