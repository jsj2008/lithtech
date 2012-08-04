#include "clientheaders.h"
#include "MouseMenu.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "ClientUtilities.h"
#include <stdio.h>

extern CommandID g_CommandArray[];

#define ID_MOUSELOOK		0
#define ID_LOOKSPRING		1
#define ID_INVERTYAXIS		2
#define ID_SENSITIVITY		3
#define ID_INPUTRATE		4
#define ID_LEFTBUTTON		5
#define ID_RIGHTBUTTON		6
#define ID_MIDDLEBUTTON		7
#define ID_BACK				8

CMouseMenu::CMouseMenu() : CBaseMenu()
{
	m_nSecondColumn = 0;

	m_nLeftButtonSelection = 0;
	m_nRightButtonSelection = 0;
	m_nMiddleButtonSelection = 0;
}

LTBOOL CMouseMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu) return LTFALSE;

	CRiotSettings* pSettings = pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// init the sliders

	m_sliderMouseSensitivity.Init (pClientDE, 60, 11);
	m_sliderMouseSensitivity.SetEnabled();
	m_sliderMouseSensitivity.SetPos ((int)pSettings->Control[RS_CTRL_MOUSESENSITIVITY].nValue);
	
	m_sliderMouseInputRate.Init (pClientDE, 60, 41);
	m_sliderMouseInputRate.SetEnabled();
	m_sliderMouseInputRate.SetPos ((int)pSettings->Control[RS_CTRL_MOUSEINPUTRATE].nValue);

	// init button selections

	m_nLeftButtonSelection = NUM_COMMANDS - 1;		// unassigned
	m_nMiddleButtonSelection = NUM_COMMANDS - 1;
	m_nRightButtonSelection = NUM_COMMANDS - 1;

	// get button object names

	char strLeftButton[64];
	char strMiddleButton[64];
	char strRightButton[64];
	memset (strLeftButton, 0, 64);
	memset (strMiddleButton, 0, 64);
	memset (strRightButton, 0, 64);

	DeviceObject* pObjects = pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
	DeviceObject* pObj = pObjects;
	int nButton = 0;
	while (pObj)
	{
		if (pObj->m_ObjectType == CONTROLTYPE_BUTTON)
		{
			if (nButton == 0) SAFE_STRCPY(strLeftButton, pObj->m_ObjectName);
			if (nButton == 1) SAFE_STRCPY(strRightButton, pObj->m_ObjectName);
			if (nButton == 2) SAFE_STRCPY(strMiddleButton, pObj->m_ObjectName);

			nButton++;
		}
		pObj = pObj->m_pNext;
	}
	pClientDE->FreeDeviceObjects (pObjects);

	// load the current button bindings

	DeviceBinding* pBindings = pClientDE->GetDeviceBindings (DEVICETYPE_MOUSE);
	DeviceBinding* pBinding = pBindings;
	while (pBinding)
	{
		if (stricmp (pBinding->strTriggerName, strLeftButton) == 0 && pBinding->pActionHead)
		{
			m_nLeftButtonSelection = CommandToArrayPos (pBinding->pActionHead->nActionCode);
		}

		if (stricmp (pBinding->strTriggerName, strMiddleButton) == 0 && pBinding->pActionHead)
		{
			m_nMiddleButtonSelection = CommandToArrayPos (pBinding->pActionHead->nActionCode);
		}

		if (stricmp (pBinding->strTriggerName, strRightButton) == 0 && pBinding->pActionHead)
		{
			m_nRightButtonSelection = CommandToArrayPos (pBinding->pActionHead->nActionCode);
		}

		pBinding = pBinding->pNext;
	}
	pClientDE->FreeDeviceBindings (pBindings);

	// call the base class Init() function

	LTBOOL bSuccess = CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight);
	
	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 155;
	}
	else if (nScreenWidth < 640)
	{
		m_nSecondColumn = 135;
	}
	else
	{
		m_nSecondColumn = 155;
	}

	return bSuccess;
}

void CMouseMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);

	if (nScreenWidth < 512)
	{
		m_nSecondColumn = 155;
	}
	if (nScreenWidth < 640)
	{
		m_nSecondColumn = 135;
	}
	else
	{
		m_nSecondColumn = 155;
	}
}

void CMouseMenu::Reset()
{
	if (!m_pRiotMenu || !m_pClientDE) return;

	m_sliderMouseSensitivity.SetSelected (LTFALSE);
	m_sliderMouseInputRate.SetSelected (LTFALSE);

	CBaseMenu::Reset();
}

void CMouseMenu::Up()
{
	if (m_nSelection == ID_SENSITIVITY) m_sliderMouseSensitivity.SetSelected (LTFALSE);
	if (m_nSelection == ID_INPUTRATE) m_sliderMouseInputRate.SetSelected (LTFALSE);

	CBaseMenu::Up();

	if (m_nSelection == ID_SENSITIVITY) m_sliderMouseSensitivity.SetSelected();
	if (m_nSelection == ID_INPUTRATE) m_sliderMouseInputRate.SetSelected();
}

void CMouseMenu::Down()
{
	if (m_nSelection == ID_SENSITIVITY) m_sliderMouseSensitivity.SetSelected (LTFALSE);
	if (m_nSelection == ID_INPUTRATE) m_sliderMouseInputRate.SetSelected (LTFALSE);

	CBaseMenu::Down();

	if (m_nSelection == ID_SENSITIVITY) m_sliderMouseSensitivity.SetSelected();
	if (m_nSelection == ID_INPUTRATE) m_sliderMouseInputRate.SetSelected();
}

void CMouseMenu::Left()
{
	if (!m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == ID_MOUSELOOK)
	{
		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();
		
		pSettings->Control[RS_CTRL_MOUSELOOK].nValue = !pSettings->Control[RS_CTRL_MOUSELOOK].nValue;

		if (m_MouseSettings[ID_MOUSELOOK].hMenuItem) m_pClientDE->DeleteSurface (m_MouseSettings[ID_MOUSELOOK].hMenuItem);
		if (m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected) m_pClientDE->DeleteSurface (m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected);

		int nStringID = pSettings->MouseLook() ? IDS_ON : IDS_OFF;
		m_MouseSettings[ID_MOUSELOOK].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
		m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);
		
		// crop the mouselook surface...
		//m_MouseSettings[ID_MOUSELOOK].hMenuItem = CropMenuItemTop (m_MouseSettings[ID_MOUSELOOK].hMenuItem);
		//m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected = CropMenuItemTop (m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected);
	}
	else if (m_nSelection == ID_LOOKSPRING)
	{
		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();
		
		pSettings->Control[RS_CTRL_LOOKSPRING].nValue = !pSettings->Control[RS_CTRL_LOOKSPRING].nValue;

		if (m_MouseSettings[ID_LOOKSPRING].hMenuItem) m_pClientDE->DeleteSurface (m_MouseSettings[ID_LOOKSPRING].hMenuItem);
		if (m_MouseSettings[ID_LOOKSPRING].hMenuItemSelected) m_pClientDE->DeleteSurface (m_MouseSettings[ID_LOOKSPRING].hMenuItemSelected);

		int nStringID = pSettings->Lookspring() ? IDS_ON : IDS_OFF;
		m_MouseSettings[ID_LOOKSPRING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
		m_MouseSettings[ID_LOOKSPRING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);
	}
	else if (m_nSelection == ID_INVERTYAXIS)
	{
		CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
		CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();
		
		pSettings->Control[RS_CTRL_MOUSEINVERTY].nValue = !pSettings->Control[RS_CTRL_MOUSEINVERTY].nValue;

		if (m_MouseSettings[ID_INVERTYAXIS].hMenuItem) m_pClientDE->DeleteSurface (m_MouseSettings[ID_INVERTYAXIS].hMenuItem);
		if (m_MouseSettings[ID_INVERTYAXIS].hMenuItemSelected) m_pClientDE->DeleteSurface (m_MouseSettings[ID_INVERTYAXIS].hMenuItemSelected);

		int nStringID = pSettings->MouseInvertY() ? IDS_YES : IDS_NO;
		m_MouseSettings[ID_INVERTYAXIS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
		m_MouseSettings[ID_INVERTYAXIS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);
	}
	else if (m_nSelection == ID_SENSITIVITY)
	{
		int nPos = m_sliderMouseSensitivity.GetPos();
		if (nPos == m_sliderMouseSensitivity.GetMin()) return;

		m_sliderMouseSensitivity.SetPos (nPos - 1);
		pSettings->Control[RS_CTRL_MOUSESENSITIVITY].nValue = (LTFLOAT)m_sliderMouseSensitivity.GetPos();
		pSettings->ImplementMouseSensitivity();
	}
	else if (m_nSelection == ID_INPUTRATE)
	{
		int nPos = m_sliderMouseInputRate.GetPos();
		if (nPos == m_sliderMouseInputRate.GetMin()) return;

		m_sliderMouseInputRate.SetPos (nPos - 2);
		pSettings->Control[RS_CTRL_MOUSEINPUTRATE].nValue = (LTFLOAT)m_sliderMouseInputRate.GetPos();
		pSettings->ImplementInputRate();
	}
	else if (m_nSelection == ID_LEFTBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nLeftButtonSelection, -1);
		BindButtonToCommand (0, m_nLeftButtonSelection);
	}
	else if (m_nSelection == ID_RIGHTBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nRightButtonSelection, -1);
		BindButtonToCommand (1, m_nRightButtonSelection);
	}
	else if (m_nSelection == ID_MIDDLEBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nMiddleButtonSelection, -1);
		BindButtonToCommand (2, m_nMiddleButtonSelection);
	}
	
	CBaseMenu::Left();
}

void CMouseMenu::Right()
{
	if (!m_pClientDE) return;

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return;

	if (m_nSelection == ID_MOUSELOOK || m_nSelection == ID_LOOKSPRING || m_nSelection == ID_INVERTYAXIS)
	{
		Left();
		return;
	}
	else if (m_nSelection == ID_SENSITIVITY)
	{
		int nPos = m_sliderMouseSensitivity.GetPos();
		if (nPos == m_sliderMouseSensitivity.GetMax()) return;

		m_sliderMouseSensitivity.SetPos (nPos + 1);
		pSettings->Control[RS_CTRL_MOUSESENSITIVITY].nValue = (LTFLOAT)m_sliderMouseSensitivity.GetPos();
		pSettings->ImplementMouseSensitivity();
	}
	else if (m_nSelection == ID_INPUTRATE)
	{
		int nPos = m_sliderMouseInputRate.GetPos();
		if (nPos == m_sliderMouseInputRate.GetMax()) return;

		m_sliderMouseInputRate.SetPos (nPos + 2);
		pSettings->Control[RS_CTRL_MOUSEINPUTRATE].nValue = (LTFLOAT)m_sliderMouseInputRate.GetPos();
		pSettings->ImplementInputRate();
	}
	else if (m_nSelection == ID_LEFTBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nLeftButtonSelection, 1);
		BindButtonToCommand (0, m_nLeftButtonSelection);
	}
	else if (m_nSelection == ID_RIGHTBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nRightButtonSelection, 1);
		BindButtonToCommand (1, m_nRightButtonSelection);
	}
	else if (m_nSelection == ID_MIDDLEBUTTON)
	{
		ChangeButtonSettingSurface (m_nSelection, &m_nMiddleButtonSelection, 1);
		BindButtonToCommand (2, m_nMiddleButtonSelection);
	}
	
	CBaseMenu::Right();
}

void CMouseMenu::PageUp()
{
	Home();
}

void CMouseMenu::PageDown()
{
	End();
}

void CMouseMenu::Home()
{
	CBaseMenu::Home();
}

void CMouseMenu::End()
{
	CBaseMenu::End();
}

void CMouseMenu::Return()
{
	if (!m_pRiotMenu) return;

	if (m_nSelection == ID_BACK)
	{
		m_pRiotMenu->SetCurrentMenu (m_pParent);
		CBaseMenu::Return();
	}
}

void CMouseMenu::Esc()
{
	CBaseMenu::Esc();
}

void CMouseMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE) return;

	CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);

	int y = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	for (int i = m_nTopItem; i < 9; i++)
	{
		if (m_MouseSettings[i].hMenuItem)
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_nSelection == i ? m_MouseSettings[i].hMenuItemSelected : m_MouseSettings[i].hMenuItem, LTNULL, m_nMenuX + m_nSecondColumn, y, LTNULL);
		}

		if (i == ID_SENSITIVITY)
		{
			m_sliderMouseSensitivity.Draw (hScreen, m_nMenuX + m_nSecondColumn, y);
		}

		if (i == ID_INPUTRATE)
		{
			m_sliderMouseInputRate.Draw (hScreen, m_nMenuX + m_nSecondColumn, y);
		}

		y += m_GenericItem[i].szMenuItem.cy + m_nMenuSpacing;
		if (y > GetMenuAreaBottom() - (int)m_GenericItem[i].szMenuItem.cy) break;
	}
}

LTBOOL CMouseMenu::LoadSurfaces()
{
	if (!m_pClientDE) return LTFALSE;

	// get the settings class

	CRiotSettings* pSettings = m_pRiotMenu->GetSettings();
	if (!pSettings) return LTFALSE;
	
	// create the menu surfaces

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();

	m_GenericItem[ID_MOUSELOOK].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_MOUSELOOK);
	m_GenericItem[ID_LOOKSPRING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_LOOKSPRING);
	m_GenericItem[ID_INVERTYAXIS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_INVERTYAXIS);
	m_GenericItem[ID_SENSITIVITY].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_SENSITIVITY);
	m_GenericItem[ID_INPUTRATE].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_INPUTRATE);
	m_GenericItem[ID_LEFTBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_LEFTBUTTON);
	m_GenericItem[ID_RIGHTBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_RIGHTBUTTON);
	m_GenericItem[ID_MIDDLEBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MOUSE_MIDDLEBUTTON);
	m_GenericItem[ID_BACK].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_BACK);

	m_GenericItem[ID_MOUSELOOK].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_MOUSELOOK);
	m_GenericItem[ID_LOOKSPRING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_LOOKSPRING);
	m_GenericItem[ID_INVERTYAXIS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_INVERTYAXIS);
	m_GenericItem[ID_SENSITIVITY].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_SENSITIVITY);
	m_GenericItem[ID_INPUTRATE].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_INPUTRATE);
	m_GenericItem[ID_LEFTBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_LEFTBUTTON);
	m_GenericItem[ID_RIGHTBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_RIGHTBUTTON);
	m_GenericItem[ID_MIDDLEBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MOUSE_MIDDLEBUTTON);
	m_GenericItem[ID_BACK].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_BACK);

	// take care of the settings surfaces

	int nStringID = pSettings->MouseLook() ? IDS_ON : IDS_OFF;
	m_MouseSettings[ID_MOUSELOOK].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
	m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

	// crop the mouselook surface...
	//m_MouseSettings[ID_MOUSELOOK].hMenuItem = CropMenuItemTop (m_MouseSettings[ID_MOUSELOOK].hMenuItem);
	//m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected = CropMenuItemTop (m_MouseSettings[ID_MOUSELOOK].hMenuItemSelected);

	nStringID = pSettings->Lookspring() ? IDS_ON : IDS_OFF;
	m_MouseSettings[ID_LOOKSPRING].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
	m_MouseSettings[ID_LOOKSPRING].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

	nStringID = pSettings->MouseInvertY() ? IDS_YES : IDS_NO;
	m_MouseSettings[ID_INVERTYAXIS].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nStringID);
	m_MouseSettings[ID_INVERTYAXIS].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nStringID);

	// create button settings surfaces

	m_MouseSettings[ID_LEFTBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, g_CommandArray[m_nLeftButtonSelection].nStringID);
	m_MouseSettings[ID_LEFTBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, g_CommandArray[m_nLeftButtonSelection].nStringID);
	
	m_MouseSettings[ID_MIDDLEBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, g_CommandArray[m_nMiddleButtonSelection].nStringID);
	m_MouseSettings[ID_MIDDLEBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, g_CommandArray[m_nMiddleButtonSelection].nStringID);
	
	m_MouseSettings[ID_RIGHTBUTTON].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, g_CommandArray[m_nRightButtonSelection].nStringID);
	m_MouseSettings[ID_RIGHTBUTTON].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, g_CommandArray[m_nRightButtonSelection].nStringID);

	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_MOUSE);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	
	// check to see that all the surfaces were created successfully

	for (int i = 0; i < 9; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}

		if (i == ID_BACK || i == ID_SENSITIVITY || i == ID_INPUTRATE) continue;

		if (!m_MouseSettings[i].hMenuItem || !m_MouseSettings[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}
	
	// get the main surface sizes

	for (int i = 0; i < 9; i++)
	{
		m_pClientDE->GetSurfaceDims (m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void CMouseMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < 9; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;

		if (m_MouseSettings[i].hMenuItem) m_pClientDE->DeleteSurface (m_MouseSettings[i].hMenuItem);
		if (m_MouseSettings[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_MouseSettings[i].hMenuItemSelected);
		m_MouseSettings[i].hMenuItem = LTNULL;
		m_MouseSettings[i].hMenuItemSelected = LTNULL;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;
	
	CBaseMenu::UnloadSurfaces();
}

void CMouseMenu::PostCalculateMenuDims()
{
	if (!m_pClientDE) return;

	// get the maximum width of the menu

	int nMenuMaxWidth = 0;
	uint32 nSettingWidth, nSettingHeight;
	for (int i = ID_MOUSELOOK; i <= ID_MIDDLEBUTTON; i++)
	{
		if (i == ID_SENSITIVITY)
		{
			if (m_sliderMouseSensitivity.GetWidth() > nMenuMaxWidth)
			{
				nMenuMaxWidth = m_sliderMouseSensitivity.GetWidth();
			}
		}
		else if (i == ID_INPUTRATE)
		{
			if (m_sliderMouseInputRate.GetWidth() > nMenuMaxWidth)
			{
				nMenuMaxWidth = m_sliderMouseInputRate.GetWidth();
			}
		}
		else
		{
			m_pClientDE->GetSurfaceDims (m_MouseSettings[i].hMenuItem, &nSettingWidth, &nSettingHeight);
			if (m_nSecondColumn + (int)nSettingWidth > nMenuMaxWidth)
			{
				nMenuMaxWidth = m_nSecondColumn + nSettingWidth;
			}
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

LTBOOL CMouseMenu::ChangeButtonSettingSurface (int nMenuSelection, int* pSelection, int nChange)
{
	if (!pSelection || !m_pClientDE) return LTFALSE;

	CBitmapFont* pFontNormal = m_pRiotMenu->GetFont12n();
	CBitmapFont* pFontSelected = m_pRiotMenu->GetFont12s();
	
	if (m_MouseSettings[nMenuSelection].hMenuItem) m_pClientDE->DeleteSurface (m_MouseSettings[nMenuSelection].hMenuItem);
	if (m_MouseSettings[nMenuSelection].hMenuItemSelected) m_pClientDE->DeleteSurface (m_MouseSettings[nMenuSelection].hMenuItemSelected);
	
	*pSelection += nChange;
	if (*pSelection < 0) *pSelection = NUM_COMMANDS - 1;
	if (*pSelection >= NUM_COMMANDS) *pSelection = 0;

	m_MouseSettings[nMenuSelection].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, g_CommandArray[*pSelection].nStringID);
	m_MouseSettings[nMenuSelection].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, g_CommandArray[*pSelection].nStringID);
	
	return (m_MouseSettings[nMenuSelection].hMenuItem != LTNULL && m_MouseSettings[nMenuSelection].hMenuItemSelected != LTNULL);
}

LTBOOL CMouseMenu::BindButtonToCommand (int nButton, int nSelection)
{
	if (!m_pClientDE) return LTFALSE;

	// find the button

	DeviceObject* pObjects = m_pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
	DeviceObject* pObj = pObjects;
	int nButtonCount = 0;
	while (pObj)
	{
		if (pObj->m_ObjectType == CONTROLTYPE_BUTTON)
		{
			if (nButtonCount == nButton) break;
			nButtonCount++;
		}

		pObj = pObj->m_pNext;
	}
	if (!pObj) 
	{
		m_pClientDE->FreeDeviceObjects (pObjects);
		return LTFALSE;
	}

	// remove old binding and set new binding

	char str[128];
	sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"\"", pObj->m_DeviceName, pObj->m_ObjectName);
	m_pClientDE->RunConsoleString (str);
	
	sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"%s\"", pObj->m_DeviceName, pObj->m_ObjectName, CommandName (g_CommandArray[nSelection].nCommandID));
	m_pClientDE->RunConsoleString (str);
	
	m_pClientDE->FreeDeviceObjects (pObjects);

	return LTTRUE;
}
