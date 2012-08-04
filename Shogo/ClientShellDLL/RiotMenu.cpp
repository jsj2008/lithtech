#include "iltclient.h"
#include "RiotMenu.h"
#include "RiotClientShell.h"
#include "vkdefs.h"
#include "WinUtil.h"
#include <stdio.h>

CRiotMenu::CRiotMenu()
{
	m_pClientDE = LTNULL;
	m_pClientShell = LTNULL;

	m_pCurrentMenu = LTNULL;

	m_hArt = LTNULL;
	m_szArt.cy = m_szArt.cx = 0;

	m_hUpArrow = LTNULL;
	m_hDnArrow = LTNULL;
	m_nArrowHeight = 0;
	m_nArrowWidth = 0;
	
	SAFE_STRCPY(m_strErrorMsg, "Unspecified Error");
}

CRiotMenu::~CRiotMenu()
{
	Term();
}

LTBOOL CRiotMenu::Init (ILTClient* pClientDE, CRiotClientShell* pClientShell)
{
	if (!pClientDE || !pClientShell) return LTFALSE;

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	InitControls();

#ifdef KEVIN_____
	// setup the menu artwork
	m_hArt = m_pClientDE->CreateSurfaceFromBitmap ("interface//Menu//Half.pcx");
	if (!m_hArt)
	{
		Term();
		return LTFALSE;
	}
#endif

	m_pClientDE->GetSurfaceDims (m_hArt, &m_szArt.cx, &m_szArt.cy);
	HLTCOLOR hTrans = m_pClientDE->SetupColor1 (0.00001f, 0.00001f, 0.00001f, LTTRUE);

	m_hUpArrow = m_pClientDE->CreateSurfaceFromBitmap ("interface/ArrowUp.pcx");
	m_hDnArrow = m_pClientDE->CreateSurfaceFromBitmap ("interface/ArrowDn.pcx");

	uint32 nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (m_hUpArrow, &nWidth, &nHeight);
	m_nArrowHeight = (int)nHeight;
	m_nArrowWidth = (int)nWidth;

	// read in the settings
	m_Settings.Init (m_pClientDE, m_pClientShell);

	SAFE_STRCPY(m_strErrorMsg, "Unspecified Error");

	// initialize the fonts
	m_font08n.Init (m_pClientDE, "Interface/Font08n.pcx");
	m_font08s.Init (m_pClientDE, "Interface/Font08s.pcx");
	m_font12n.Init (m_pClientDE, "Interface/Font12n.pcx");
	m_font12s.Init (m_pClientDE, "Interface/Font12s.pcx");
	m_font18n.Init (m_pClientDE, "Interface/Font18n.pcx");
	m_font18s.Init (m_pClientDE, "Interface/Font18s.pcx");
	m_font28n.Init (m_pClientDE, "Interface/Font28n.pcx");
	m_font28s.Init (m_pClientDE, "Interface/Font28s.pcx");
	
	// init surface sizes and rects
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &m_szScreen.cx, &m_szScreen.cy);

#define __PULTZ
#ifdef __PULTZ
	m_rcArtDest.left = 0;
	m_rcArtDest.right = m_szScreen.cx >> 1;
	m_rcArtDest.top = 0;
	m_rcArtDest.bottom = (int)m_szScreen.cy;
#else
	int nHalfWidth = m_szScreen.cx / 2;
	m_rcArtDest.left = nHalfWidth - (int)m_szArt.cx;
	m_rcArtDest.right = nHalfWidth;
	m_rcArtDest.top = ((int)m_szScreen.cy - (int)m_szArt.cy) / 2;
	m_rcArtDest.bottom = (int)m_szScreen.cy - m_rcArtDest.top;
#endif

	// init the menus
	if (!m_MainMenu.Init (pClientDE, this, LTNULL, m_szScreen.cx, m_szScreen.cy)) return LTFALSE;

	m_pCurrentMenu = &m_MainMenu;


	return LTTRUE;
}

void CRiotMenu::Term()
{
	if (!m_pClientDE) return;

	m_Settings.WriteSettings();

	if (m_hArt) m_pClientDE->DeleteSurface (m_hArt);
	m_hArt = LTNULL;
	
	if (m_hUpArrow) m_pClientDE->DeleteSurface (m_hUpArrow);
	m_hUpArrow = LTNULL;
	if (m_hDnArrow) m_pClientDE->DeleteSurface (m_hDnArrow);
	m_hDnArrow = LTNULL;
	
	m_nArrowHeight = 0;
	m_nArrowWidth = 0;

	m_pClientDE = LTNULL;
	m_pClientShell = LTNULL;
	m_pCurrentMenu = LTNULL;
}

LTBOOL CRiotMenu::LoadAllSurfaces()
{
	return m_MainMenu.LoadAllSurfaces();
}

void CRiotMenu::UnloadAllSurfaces()
{
	m_MainMenu.UnloadAllSurfaces();
}

void CRiotMenu::HandleInput (int vkey)
{
	if (m_pCurrentMenu) m_pCurrentMenu->HandleInput (vkey);

	switch (vkey)
	{
		case VK_UP:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Up();
		}
		break;

		case VK_DOWN:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Down();
		}
		break;

		case VK_LEFT:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Left();
		}
		break;

		case VK_RIGHT:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Right();
		}
		break;

		case VK_PRIOR:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->PageUp();
		}
		break;

		case VK_NEXT:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->PageDown();
		}
		break;

		case VK_HOME:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Home();
		}
		break;

		case VK_END:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->End();
		}
		break;

		case VK_RETURN:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Return();
		}
		break;

		case VK_ESCAPE:
		{
			if (m_pCurrentMenu) m_pCurrentMenu->Esc();
		}
		break;
	}
}

LTBOOL CRiotMenu::InWorld()
{
	if (m_pClientShell) 
	{
		return m_pClientShell->IsInWorld(); 
	}
	return LTFALSE; 
}

void CRiotMenu::ScreenDimsChanged()
{
	if (!m_pClientDE) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &m_szScreen.cx, &m_szScreen.cy);

#ifdef __PULTZ
	m_rcArtDest.left = 0;
	m_rcArtDest.right = m_szScreen.cx >> 1;
	m_rcArtDest.top = 0;
	m_rcArtDest.bottom = (int)m_szScreen.cy;
#else
	int nHalfWidth = m_szScreen.cx / 2;
	m_rcArtDest.left = nHalfWidth - (int)m_szArt.cx;
	m_rcArtDest.right = nHalfWidth;
	m_rcArtDest.top = ((int)m_szScreen.cy - (int)m_szArt.cy) / 2;
	m_rcArtDest.bottom = (int)m_szScreen.cy - m_rcArtDest.top;
#endif

	m_MainMenu.ScreenDimsChanged (m_szScreen.cx, m_szScreen.cy);
}

void CRiotMenu::Draw()
{
	if (!m_pClientDE || !m_pClientShell) return;

	if (m_pCurrentMenu)
	{
		HSURFACE hScreen = m_pClientDE->GetScreenSurface();

#ifdef KEVIN_____
		if (!InWorld())
		{
			if (m_szScreen.cx != 320)
			{
#ifdef __PULTZ
				m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hArt, &m_rcArtDest, LTNULL);
#else
				m_pClientDE->DrawSurfaceToSurface (hScreen, m_hArt, LTNULL, m_rcArtDest.left, m_rcArtDest.top);
#endif
			}
		}
#endif

		m_pCurrentMenu->Draw (hScreen, (int)m_szScreen.cx, (int)m_szScreen.cy);
	}
}

void CRiotMenu::StopMenuMusic()
{
	if (m_pClientShell) 
	{
		m_pClientShell->SetMenuMusic (LTFALSE);
	}
}

void CRiotMenu::ExitMenu (LTBOOL bLoadingLevel)
{
	if (!m_pClientShell) return;

	if (!m_pClientShell->SetMenuMode (LTFALSE, bLoadingLevel)) return;
	
	m_pCurrentMenu = &m_MainMenu;
	m_MainMenu.Reset();
}

void CRiotMenu::SetErrorMsg (uint32 nStringID, char* strFilename, uint32 nLineNumber)
{
	if (!m_pClientDE)
	{
		SAFE_STRCPY(m_strErrorMsg, "SetErrrorMsg() failed!");
		return;
	}

	HSTRING hStr = m_pClientDE->FormatString (nStringID);
	if (!hStr)
	{
		SAFE_STRCPY(m_strErrorMsg, "SetErrrorMsg() failed!");
		return;
	}

	const char* pStr = m_pClientDE->GetStringData (hStr);
	sprintf (m_strErrorMsg, "Error: \"%s\" in file %s at line %lu", pStr, strFilename, nLineNumber);

	m_pClientDE->FreeString (hStr);
}

LTBOOL CRiotMenu::InitControls()
{
	if (!m_pClientDE) return LTFALSE;

	// can we find the devices?

	LTBOOL bFoundKeyboard = LTFALSE;
	LTBOOL bFoundMouse = LTFALSE;
	LTBOOL bFoundJoystick = LTFALSE;

	// get the device string names

	char strKeyboard[128];
	memset (strKeyboard, 0, 128);
	LTRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_KEYBOARD, strKeyboard, 127);
	if (result == LT_OK) bFoundKeyboard = LTTRUE;

	char strMouse[128];
	memset (strMouse, 0, 128);
	result = m_pClientDE->GetDeviceName (DEVICETYPE_MOUSE, strMouse, 127);
	if (result == LT_OK) bFoundMouse = LTTRUE;

	char strJoystick[128];
	memset (strJoystick, 0, 128);
	result = m_pClientDE->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
	if (result == LT_OK) bFoundJoystick = LTTRUE;
	
	// enable the devices if they're not enabled already

	if (bFoundKeyboard)
	{
		bool bEnabled = false;
		m_pClientDE->IsDeviceEnabled (strKeyboard, &bEnabled);
		if (!bEnabled)
		{
			char strConsole[256];
			sprintf (strConsole, "EnableDevice \"%s\"", strKeyboard);
			m_pClientDE->RunConsoleString (strConsole);
		}

		// setup the weapon keys

		char strConsole[256];
		sprintf (strConsole, "rangebind \"%s\" \"1\" 0 0 \"Weapon_0\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"2\" 0 0 \"Weapon_1\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"3\" 0 0 \"Weapon_2\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"4\" 0 0 \"Weapon_3\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"5\" 0 0 \"Weapon_4\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"6\" 0 0 \"Weapon_5\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"7\" 0 0 \"Weapon_6\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"8\" 0 0 \"Weapon_7\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"9\" 0 0 \"Weapon_8\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
		sprintf (strConsole, "rangebind \"%s\" \"0\" 0 0 \"Weapon_9\"", strKeyboard);
		m_pClientDE->RunConsoleString (strConsole);
	}

	if (bFoundMouse)
	{
		bool bEnabled = false;
		m_pClientDE->IsDeviceEnabled (strMouse, &bEnabled);
		if (!bEnabled)
		{
			char strConsole[256];
			sprintf (strConsole, "EnableDevice \"%s\"", strMouse);
			m_pClientDE->RunConsoleString (strConsole);
		}
		
		// set up the mouse axes

		DeviceObject* pObjects = m_pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
		DeviceObject* pObj = pObjects;
		while (pObj)
		{
			char strConsole[256];
			if (pObj->m_ObjectType == CONTROLTYPE_XAXIS)
			{
				sprintf (strConsole, "rangebind \"%s\" \"%s\" 0 0 \"Axis1\"", pObj->m_DeviceName, pObj->m_ObjectName);
				m_pClientDE->RunConsoleString (strConsole);
				sprintf (strConsole, "scale \"%s\" \"%s\" %f", pObj->m_DeviceName, pObj->m_ObjectName, 0.005f);
				m_pClientDE->RunConsoleString (strConsole);
			}
			if (pObj->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				sprintf (strConsole, "rangebind \"%s\" \"%s\" 0 0 \"Axis2\"", pObj->m_DeviceName, pObj->m_ObjectName);
				m_pClientDE->RunConsoleString (strConsole);
				sprintf (strConsole, "scale \"%s\" \"%s\" %f", pObj->m_DeviceName, pObj->m_ObjectName, 0.005f);
				m_pClientDE->RunConsoleString (strConsole);
			}
			pObj = pObj->m_pNext;
		}
		m_pClientDE->FreeDeviceObjects (pObjects);
	}

	if (bFoundJoystick)
	{
		bool bEnabled = false;
		m_pClientDE->IsDeviceEnabled (strJoystick, &bEnabled);
		if (!bEnabled)
		{
			char strConsole[256];
			sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
			m_pClientDE->RunConsoleString (strConsole);
		}
	}

	return LTTRUE;
}
