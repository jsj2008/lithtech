#include "clientheaders.h"
#include "KeyboardMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include "vkdefs.h"
#include "RiotClientShell.h"
#include "WinUtil.h"
#include <stdio.h>
#include "KeyFixes.h"

extern CommandID g_CommandArray[];

#define TITLE_SPACING_MULTIPLIER	3

CKeyboardMenu::~CKeyboardMenu()
{
}

LTBOOL CKeyboardMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	m_nSecondColumn = 120;

	// build a list of menu entries

	m_nEntries = 0;
	for (int i = 0; i < NUM_COMMANDS - 1; i++)
	{
		m_pEntries[i].nStringID = g_CommandArray[i].nStringID;
		m_pEntries[i].nAction = g_CommandArray[i].nCommandID;
		m_nEntries++;
	}

	m_pEntries[m_nEntries++].nStringID = IDS_RESTOREDEFAULTS;
	m_pEntries[m_nEntries++].nStringID = IDS_BACK;

	// now call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);

	m_nSpacing = 3;

	return bSuccess;
}

void CKeyboardMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CKeyboardMenu::HandleInput (int vKey)
{
	if (m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;

	if (vKey == VK_DELETE && m_nSelection < m_nEntries - 2)
	{
		// remove current binding...

		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	
		char str[128];
		sprintf (str, "rangebind \"##keyboard\" \"%s\" 0 0 \"\"", m_pEntries[m_nSelection].strControlName);
		m_pClientDE->RunConsoleString (str);
		
		if (m_pEntries[m_nSelection].hSetting) m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSetting);
		if (m_pEntries[m_nSelection].hSettingSelected) m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSettingSelected);

		SAFE_STRCPY (m_pEntries[m_nSelection].strControlName, "");
		m_pEntries[m_nSelection].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_CONTROL_UNASSIGNED);
		m_pEntries[m_nSelection].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_CONTROL_UNASSIGNED);
	}
}

void CKeyboardMenu::Up()
{
	if (m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;

	if (m_nSelection == 0)
	{
		End();
		return;
	}

	m_nSelection--;
	CheckSelectionOffMenuTop();
	
	PlayUpSound();
}

void CKeyboardMenu::Down()
{
	if (m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;

	if (m_nSelection == m_nEntries - 1)
	{
		Home();
		return;
	}

	m_nSelection++;
	CheckSelectionOffMenuBottom();

	PlayDownSound();
}

void CKeyboardMenu::PageUp()
{
	CBaseMenu::PageUp();
}

void CKeyboardMenu::PageDown()
{
	CBaseMenu::PageDown();
}

void CKeyboardMenu::Home()
{
	if (m_nSelection == 0 || m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;

	m_nTopItem = 0;
	m_nSelection = 0;

	PlayHomeSound();
}

void CKeyboardMenu::End()
{
	if (m_nSelection == m_nEntries - 1 || m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;

	m_nSelection = m_nEntries - 1;
	
	// determine what the top item would be...

	m_nTopItem = 0;
	int nTopY = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	int nCurrentY = GetMenuAreaBottom() - m_pEntries[m_nSelection].szSurface.cy;
	for (int i = m_nSelection - 1; i >= 0; i--)
	{
		nCurrentY -= m_nSpacing + m_pEntries[i].szSurface.cy;
		if (nCurrentY < nTopY)
		{
			m_nTopItem = i + 1;
			break;
		}
	}
	
	PlayEndSound();
}

void CKeyboardMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu || m_bWaitingForKeypress || m_fInputPauseTimeLeft) return;
	
	// if they chose "Back" just go back one level

	if (m_nSelection == m_nEntries - 1) 
	{
		Esc();
		return;
	}

	// if they chose "restore defaults" attempt to restore the normal keyboard config

	if (m_nSelection == m_nEntries - 2)
	{
		// we need to read the config file twice - since multiple keys can be bound to the
		// same action, we need to clear all bindings before reading in the file.  we need
		// to read it in the first time to make sure it's there and works. 
		// if multiple keys are bound to the same action, when we get the bindings again in
		// LoadSurfaces() they could be reported in any order and we might end up displaying
		// a mixture of the current bindings and the default bindings.
		
		LTRESULT result = m_pClientDE->ReadConfigFile ("defkeybd.cfg");
		if (result != LT_ERROR)
		{
			UnloadSurfaces();
			ClearKBBindings();
			m_pClientDE->ReadConfigFile ("defkeybd.cfg");
			LoadSurfaces();
		}
		return;
	}

	// attempt to create the "press any key" string

	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();

	HSURFACE hTemp = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_PRESSANYKEY);
	if (!hTemp) return;

	// see if we can track the keyboard device

	LTRESULT hResult = m_pClientDE->StartDeviceTrack (DEVICETYPE_KEYBOARD, TRACK_BUFFER_SIZE);
	if (hResult != LT_OK)
	{
		m_pClientDE->DeleteSurface (hTemp);
		m_pClientDE->EndDeviceTrack();
		return;
	}
	m_bWaitingForKeypress = LTTRUE;
	
	// now change the actual surface

	if (m_pEntries[m_nSelection].hSettingSelected) m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSettingSelected);
	m_pEntries[m_nSelection].hSettingSelected = hTemp;

	PlayReturnSound();
}

void CKeyboardMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	// see if we are pausing input
	if (m_fInputPauseTimeLeft)
	{
		m_fInputPauseTimeLeft -= m_pClientDE->GetFrameTime();
		if (m_fInputPauseTimeLeft < 0.0f) m_fInputPauseTimeLeft = 0.0f;
	}

	// now draw the menu itself

	int y = m_nMenuY;
	if (m_hMenuTitle)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMenuTitle, LTNULL, m_nMenuX, y, LTNULL);
		y += m_szMenuTitle.cy + m_nMenuTitleSpacing;
	}

	if (m_nTopItem > 0)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetUpArrow(), LTNULL, m_nMenuX + 2, y - m_pRiotMenu->GetArrowHeight() - 3, LTNULL);
	}

	LTBOOL bDrawDownArrow = LTFALSE;
	for (int i = m_nTopItem; i < NUM_COMMANDS + 1; i++)
	{
		if (m_nSelection == i)
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pEntries[i].hSurfaceSelected, LTNULL, m_nMenuX, y, LTNULL);
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pEntries[i].hSettingSelected, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		}
		else
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pEntries[i].hSurface, LTNULL, m_nMenuX, y, LTNULL);
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pEntries[i].hSetting, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		}

		y += m_pEntries[i].szSurface.cy + m_nSpacing;
		if (y > GetMenuAreaBottom() - (int)m_pEntries[i].szSurface.cy) 
		{
			if (i < NUM_COMMANDS)
			{
				bDrawDownArrow = LTTRUE;
			}
			break;
		}
	}

	if (bDrawDownArrow)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetDownArrow(), LTNULL, m_nMenuX + 2, y, LTNULL);
	}

	// see if we are waiting for a keypress

	if (m_bWaitingForKeypress)
	{
		uint32 nArraySize = TRACK_BUFFER_SIZE;
		m_pClientDE->TrackDevice (m_pInputArray, &nArraySize);
		if (nArraySize > 0)
		{
			// find the first key down event
			for (uint32 i = 0; i < nArraySize; i++)
			{
				if (m_pInputArray[i].m_InputValue)
				{
					m_bWaitingForKeypress = LTFALSE;
					m_pClientDE->EndDeviceTrack();
					if (!SetCurrentSelection (&m_pInputArray[i])) return;
					m_fInputPauseTimeLeft = 0.2f;
					break;
				}
			}
		}
	}
}


LTBOOL CKeyboardMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	CBitmapFont* pFontTitle = m_pRiotMenu->GetFont12n();

	// load the surfaces for the menu entries

	for (int i = 0; i < m_nEntries; i++)
	{
		m_pEntries[i].hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, m_pEntries[i].nStringID);
		m_pEntries[i].hSurfaceSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, m_pEntries[i].nStringID);
		if (!m_pEntries[i].hSurface || !m_pEntries[i].hSurfaceSelected) return LTFALSE;

		// crop the top surface...
		if (i == 0)
		{
			m_pEntries[0].hSurface = CropMenuItemTop (m_pEntries[0].hSurface);
			m_pEntries[0].hSurfaceSelected = CropMenuItemTop (m_pEntries[0].hSurfaceSelected);
		}

		m_pClientDE->GetSurfaceDims (m_pEntries[i].hSurface, &m_pEntries[i].szSurface.cx, &m_pEntries[i].szSurface.cy);
	}

	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontTitle, IDS_TITLE_KEYBOARD);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	// load the bindings for the keyboard and create the settings surfaces

	DeviceBinding* pBindings = m_pClientDE->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return LTTRUE;
	}
	for (int i = 0; i < m_nEntries - 2; i++)
	{
		// find the binding for this action
		
		LTBOOL bFound = LTFALSE;
		DeviceBinding* ptr = pBindings;
		while (!bFound && ptr)
		{
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				if (pAction->nActionCode == m_pEntries[i].nAction)
				{
					SAFE_STRCPY (m_pEntries[i].strControlName, ptr->strTriggerName);
					if (_mbstrlen(ptr->strTriggerName) != 1)
					{
						m_pEntries[i].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, ptr->strTriggerName);
						m_pEntries[i].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, ptr->strTriggerName);
					}
					else
					{
						char sNewKey[256];
						ConvertKeyToCurrentKeyboard(sNewKey, ptr->strTriggerName, 256);
		
						m_pEntries[i].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, sNewKey);
						m_pEntries[i].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, sNewKey);
					}

					bFound = LTTRUE;
					break;
				}

				pAction = pAction->pNext;
			}

			ptr = ptr->pNext;
		}
		
		if (!bFound)
		{
			SAFE_STRCPY (m_pEntries[i].strControlName, "");
			m_pEntries[i].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_CONTROL_UNASSIGNED);
			m_pEntries[i].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_CONTROL_UNASSIGNED);
		}

		// crop the top surface...

		if (i == 0)
		{
			m_pEntries[0].hSetting = CropMenuItemTop (m_pEntries[0].hSetting);
			m_pEntries[0].hSettingSelected = CropMenuItemTop (m_pEntries[0].hSettingSelected);
		}
	}

	m_pClientDE->FreeDeviceBindings (pBindings);

	return CBaseMenu::LoadSurfaces();
}

void CKeyboardMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < m_nEntries; i++)
	{
		if (m_pEntries[i].hSurface) m_pClientDE->DeleteSurface (m_pEntries[i].hSurface);
		m_pEntries[i].hSurface = LTNULL;
		if (m_pEntries[i].hSurfaceSelected) m_pClientDE->DeleteSurface (m_pEntries[i].hSurfaceSelected);
		m_pEntries[i].hSurfaceSelected = LTNULL;

		if (m_pEntries[i].hSetting) m_pClientDE->DeleteSurface (m_pEntries[i].hSetting);
		m_pEntries[i].hSetting = LTNULL;
		if (m_pEntries[i].hSettingSelected) m_pClientDE->DeleteSurface (m_pEntries[i].hSettingSelected);
		m_pEntries[i].hSettingSelected = LTNULL;

		m_pEntries[i].szSurface.cx = m_pEntries[i].szSurface.cy = 0;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CKeyboardMenu::PostCalculateMenuDims()
{
	// get the total height of the menu, without any spacing as well as maximum width

	int nMenuHeight = 0;
	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	for (int i = 0; i < m_nEntries; i++)
	{
		m_pClientDE->GetSurfaceDims (m_pEntries[i].hSetting, &nSettingWidth, &nSettingHeight);

		nMenuHeight += m_pEntries[i].szSurface.cy;
		if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_nSecondColumn + (int)nSettingWidth;
		}
	}

	// determine correct spacing
	// if we can _just_ fit without any spacing, set spacing to default value

	LTBOOL bOffScreen = LTFALSE;
	if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nEntries - 1) * MAX_MENU_SPACING) > m_szMenuArea.cy &&
		nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nEntries - 1) * MIN_MENU_SPACING) <= m_szMenuArea.cy)
	{
		for (int i = MIN_MENU_SPACING; i < MAX_MENU_SPACING; i++)
		{
			if (nMenuHeight + m_szMenuTitle.cy + ((m_nEntries - 1) * (i + 1)) > m_szMenuArea.cy)
			{
				m_nMenuSpacing = i;
				m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
				break;
			}
		}
	}
	else if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nEntries - 1) * MIN_MENU_SPACING) > m_szMenuArea.cy)
	{
		bOffScreen = LTTRUE;
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}
	else
	{
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}

	// determine correct placement

	m_nMenuX = 0;
	//if (m_pRiotMenu->InWorld() || m_szScreen.cx < 512)
	//{
		m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx - nMenuMaxWidth) / 2;
	//}
	//else
	//{
	//	m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx / 2);
	//}
	
	m_nMenuY = GetMenuAreaTop();
	if (!bOffScreen)
	{
		m_nMenuY = GetMenuAreaTop() + (((int)m_szMenuArea.cy - (nMenuHeight + m_nMenuTitleSpacing + (int)m_szMenuTitle.cy + ((m_nEntries - 1) * m_nMenuSpacing))) / 2);
	}
}

void CKeyboardMenu::CheckSelectionOffMenuTop()
{
	if (m_nSelection < m_nTopItem)
	{
		m_nTopItem = m_nSelection;
	}
}

void CKeyboardMenu::CheckSelectionOffMenuBottom()
{
	int nCurrentY = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	for (int i = m_nTopItem; i < NUM_COMMANDS + 1; i++)
	{
		if (m_pEntries[i].hSurface)
		{
			nCurrentY += m_pEntries[i].szSurface.cy;

			if (nCurrentY > GetMenuAreaBottom())
			{
				m_nTopItem++;
				if (m_nTopItem == NUM_COMMANDS)
				{
					// some strangeness happened...
					m_nTopItem = 0;
					return;
				}
				CheckSelectionOffMenuBottom();
				return;
			}

			if (m_nSelection == i) return;

			nCurrentY += m_nSpacing;
		}
	}
}


LTBOOL CKeyboardMenu::SetCurrentSelection (DeviceInput* pInput)
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont08n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont08s();
	
	// see if this key is bound to something not in the keyboard configuration menu...

	if (!KeyRemappable(pInput))
	{
		// change the selected surface back to the current key name...

		HSURFACE hSurf = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, m_pEntries[m_nSelection].strControlName);
		if (hSurf)
		{
			if (m_pEntries[m_nSelection].hSettingSelected)
			{
				m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSettingSelected);
			}
			m_pEntries[m_nSelection].hSettingSelected = hSurf;
		}

		// now create a messagebox letting the user know they can't map this key...

		CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
		if (pClientShell)
		{
			pClientShell->DoMessageBox (IDS_NOREMAPKEY, TH_ALIGN_CENTER);
		}

		return LTFALSE;
	}

	// delete the old surfaces and create the new surfaces

	if (m_pEntries[m_nSelection].hSetting) m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSetting);
	m_pEntries[m_nSelection].hSetting = LTNULL;

	if (m_pEntries[m_nSelection].hSettingSelected) m_pClientDE->DeleteSurface (m_pEntries[m_nSelection].hSettingSelected);
	m_pEntries[m_nSelection].hSettingSelected = LTNULL;

	if (_mbstrlen(pInput->m_ControlName) != 1)
	{
		m_pEntries[m_nSelection].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, pInput->m_ControlName);
		m_pEntries[m_nSelection].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, pInput->m_ControlName);
	}
	else
	{
		char sNewKey[256];
		ConvertKeyToCurrentKeyboard(sNewKey, pInput->m_ControlName, 256);

		m_pEntries[m_nSelection].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, sNewKey);
		m_pEntries[m_nSelection].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, sNewKey);
	}


	if (!m_pEntries[m_nSelection].hSetting || !m_pEntries[m_nSelection].hSettingSelected) return LTFALSE;

	HLTCOLOR hTransColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, LTTRUE);

	// here we actually need to do the binding so build the console string

	char str[128];
	sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"\"", pInput->m_DeviceName, m_pEntries[m_nSelection].strControlName);
	m_pClientDE->RunConsoleString (str);
	
	SAFE_STRCPY (m_pEntries[m_nSelection].strControlName, pInput->m_ControlName);
	
	sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"%s\"", pInput->m_DeviceName, m_pEntries[m_nSelection].strControlName, CommandName (m_pEntries[m_nSelection].nAction));
	m_pClientDE->RunConsoleString (str);

	// see if this control is being used for any other commands...if so, disable that command.

	for (int i = 0; i < m_nEntries - 2; i++)
	{
		if (i == m_nSelection) continue;

		if (stricmp (m_pEntries[i].strControlName, pInput->m_ControlName) == 0)
		{
			if (m_pEntries[i].hSetting) m_pClientDE->DeleteSurface (m_pEntries[i].hSetting);
			if (m_pEntries[i].hSettingSelected) m_pClientDE->DeleteSurface (m_pEntries[i].hSettingSelected);

			SAFE_STRCPY (m_pEntries[i].strControlName, "");
			m_pEntries[i].hSetting = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_CONTROL_UNASSIGNED);
			m_pEntries[i].hSettingSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_CONTROL_UNASSIGNED);
		}
	}

	return LTTRUE;
}

void CKeyboardMenu::ClearKBBindings()
{
	if (!m_pClientDE) return;

	DeviceBinding* pBindings = m_pClientDE->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return;
	}
	
	char str[128];
	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"\"", ptr->strDeviceName, ptr->strTriggerName);
		m_pClientDE->RunConsoleString (str);

		ptr = ptr->pNext;
	}

	m_pClientDE->FreeDeviceBindings (pBindings);
}

LTBOOL CKeyboardMenu::KeyRemappable (DeviceInput* pInput)
{
	if (!m_pClientDE) return LTFALSE;

	DeviceBinding* pBindings = m_pClientDE->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings) return LTTRUE;

	// see if this input object is already bound to something...

	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		if (strcmp (ptr->strTriggerName, pInput->m_ControlName) == 0)
		{
			// see if this binding is in the menus or if it exists outside the menus (like a weapon-select action)
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
				LTBOOL bFoundMenuMatch = LTFALSE;
				for (int i = 0; i < m_nEntries - 2; i++)
				{
					if (m_pEntries[i].nAction == pAction->nActionCode)
					{
						bFoundMenuMatch = LTTRUE;
						break;
					}
				}

				if (!bFoundMenuMatch)
				{
					// this key is already bound to something that doesn't exist in the menus...can't bind it.
					m_pClientDE->FreeDeviceBindings (pBindings);
					return LTFALSE;
				}

				pAction = pAction->pNext;
			}

			// binding must already exist in the menus...
			break;
		}
		ptr = ptr->pNext;
	}

	// either the binding exists in the menus or this key is not currently bound...therefore it's remappable

	m_pClientDE->FreeDeviceBindings (pBindings);
	return LTTRUE;	
}
