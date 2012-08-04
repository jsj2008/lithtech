// MenuControls.cpp: implementation of the CMenuControls class.
//
//////////////////////////////////////////////////////////////////////

#include "VKDefs.h"
#include "LTGUIMgr.h"
#include "basedefs_de.h"
#include "MenuLoadGame.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "ClientRes.h"
#include "KeyFixes.h"

#define USE_CONFIGURE_JOYSTICK

// The different columns
#define MENU_COLUMN_ACTION	0
#define MENU_COLUMN_BUFFER	1
#define MENU_COLUMN_EQUALS	2
#define MENU_COLUMN_KEY		3								
	
char	*g_pControlActionName[MENU_CONTROLS_NUM_ACTIONS] =
{
	"Forward",			// 1
	"Backward",			// 2
	"Left",				// 3
	"Right",			// 4
	"Strafe",			// 5
	"StrafeLeft",		// 6
	"StrafeRight",		// 7
	"Run",				// 8
	"RunLock",			// 9
	"Jump",				// 10
	"Duck",				// 11
	"TurnAround",		// 12
	"Fire",				// 13
	"AltFire",			// 14
	"NextWeapon",		// 15
	"PrevWeapon",		// 16
	"DropWeapon",		// 17
	"Open",				// 18
	"InventoryLeft",	// 19
	"InventoryRight",	// 20
	"UseInventory",		// 21
	"Grab",				// 22
	"Message",			// 23
	"Taunt",			// 24
	"LookUp",			// 25
	"LookDown",			// 26
	"MouseAimToggle",	// 27
	"Crosshair",		// 28
//	"ScreenShrink",		// 29
//	"ScreenEnlarge",	// 30
	"Proximities",		// 31
	"Remotes",			// 32
	"Time",				// 33
	"Detonate",			// 34
	"Weapon_0",			// 35
	"Weapon_1",			// 36
	"Weapon_2",			// 37
	"Weapon_3",			// 38
	"Weapon_4",			// 39
	"Weapon_5",			// 40
	"Weapon_6",			// 41
	"Weapon_7",			// 42
	"Weapon_8",			// 43
	"Weapon_9"			// 44
};

int g_nControlActionStringID[MENU_CONTROLS_NUM_ACTIONS] =
{
	IDS_MENU_ACTION_FORWARD,		// 1
	IDS_MENU_ACTION_BACKWARD,		// 2
	IDS_MENU_ACTION_LEFT,			// 3
	IDS_MENU_ACTION_RIGHT,			// 4
	IDS_MENU_ACTION_STRAFE,			// 5
	IDS_MENU_ACTION_STRAFELEFT,		// 6
	IDS_MENU_ACTION_STRAFERIGHT,	// 7
	IDS_MENU_ACTION_RUN,			// 8
	IDS_MENU_ACTION_RUNLOCK,		// 9
	IDS_MENU_ACTION_JUMP,			// 10
	IDS_MENU_ACTION_DUCK,			// 11
	IDS_MENU_ACTION_TURNAROUND,		// 12
	IDS_MENU_ACTION_FIRE,			// 13
	IDS_MENU_ACTION_ALTFIRE,		// 14
	IDS_MENU_ACTION_NEXTWEAPON,		// 15
	IDS_MENU_ACTION_PREVWEAPON,		// 16
	IDS_MENU_ACTION_DROPWEAPON,		// 17
	IDS_MENU_ACTION_OPEN,			// 18
	IDS_MENU_ACTION_INVENTORYLEFT,	// 19
	IDS_MENU_ACTION_INVENTORYRIGHT,	// 20
	IDS_MENU_ACTION_USEINVENTORY,	// 21
	IDS_MENU_ACTION_GRAB,			// 22
	IDS_MENU_ACTION_MESSAGE,		// 23
	IDS_MENU_ACTION_TAUNT,			// 24
	IDS_MENU_ACTION_LOOKUP,			// 25
	IDS_MENU_ACTION_LOOKDOWN,		// 26
	IDS_MENU_ACTION_MOUSEAIMTOGGLE,	// 27
	IDS_MENU_ACTION_CROSSHAIR,		// 28
//	IDS_MENU_ACTION_SCREENSHRINK,	// 29
//	IDS_MENU_ACTION_SCREENENLARGE,	// 30
	IDS_MENU_ACTION_PROXIMITIES,	// 31
	IDS_MENU_ACTION_REMOTES,		// 32
	IDS_MENU_ACTION_TIME,			// 33
	IDS_MENU_ACTION_DETONATE,		// 34
	IDS_MENU_ACTION_WEAPON0,		// 35
	IDS_MENU_ACTION_WEAPON1,		// 36
	IDS_MENU_ACTION_WEAPON2,		// 37
	IDS_MENU_ACTION_WEAPON3,		// 38
	IDS_MENU_ACTION_WEAPON4,		// 39
	IDS_MENU_ACTION_WEAPON5,		// 40
	IDS_MENU_ACTION_WEAPON6,		// 41
	IDS_MENU_ACTION_WEAPON7,		// 42
	IDS_MENU_ACTION_WEAPON8,		// 43
	IDS_MENU_ACTION_WEAPON9			// 44
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuControls::CMenuControls()
{
	for (int i=0; i < MENU_CONTROLS_NUM_DEVICES; i++)
	{
		for (int n=0; n < MENU_CONTROLS_NUM_ACTIONS; n++)
		{
			m_deviceControlArray[i][n] = NULL;
		}
	}	
}

CMenuControls::~CMenuControls()
{

}

// Initialization
DBOOL CMenuControls::Init(CClientDE *pClientDE, CMainMenus *pMainMenus,
						  CMenuBase *pParentMenu, DWORD dwMenuID, int nMenuHeight)
{
	// Call the base class
	if (!CMenuBase::Init(pClientDE, pMainMenus, pParentMenu, dwMenuID, nMenuHeight))
	{
		return DFALSE;
	}
	
	memset(m_deviceControlArray, DNULL, sizeof(m_deviceControlArray));

	return DTRUE;
}

// Termination
void CMenuControls::Term()
{
	// Terminate the key and mouse arrays	
	int i;
	for (i=0; i < MENU_CONTROLS_NUM_DEVICES; i++)
	{
		int n;
		for (n=0; n < MENU_CONTROLS_NUM_ACTIONS; n++)
		{
			if (m_deviceControlArray[i][n])
			{
				delete []m_deviceControlArray[i][n];
				m_deviceControlArray[i][n]=DNULL;
			}
		}
	}	
}

// Builds the menu
void CMenuControls::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);
	SetScrollWrap(DFALSE);
	UseArrows(DTRUE, 298);

	// Add the controls to the menu
	InitControlList();
}

// Maps a device type to an index (DEVICETYPE_KEYBOARD=0, DEVICETYPE_MOUSE=1, etc);
// -1 is returned if the devicetype cannot be mapped.
int CMenuControls::GetDeviceIndexFromType(DWORD dwDeviceType)
{
	int nIndex;
	switch (dwDeviceType)
	{
	case DEVICETYPE_KEYBOARD:
		{
			nIndex=0;
			break;
		}
	case DEVICETYPE_MOUSE:
		{
			nIndex=1;
			break;
		}
	case DEVICETYPE_JOYSTICK:
		{
			nIndex=2;
			break;
		}
	default:
		{
			assert(DFALSE);
			return -1;
		}
	}

	return nIndex;
}

// Returns the device type from an index (ex: 0=DEVICETYPE_KEYBOARD, 1=DEVICETYPE_MOUSE, etc.)
DWORD CMenuControls::GetDeviceTypeFromIndex(int nIndex)
{
	DWORD dwType;
	switch (nIndex)
	{
	case 0:
		{
			dwType=DEVICETYPE_KEYBOARD;
			break;
		}
	case 1:
		{
			dwType=DEVICETYPE_MOUSE;
			break;
		}
	case 2:
		{
			dwType=DEVICETYPE_JOYSTICK;
			break;
		}	
	default:
		{
			assert(DFALSE);
			return -1;
		}
	}

	return dwType;
}

// Intialize the list of controls
DBOOL CMenuControls::InitControlList()
{
	// Remove the current options
	RemoveAllOptions();

	// Add the columns
	InitColumns();

	// Fill the binding arrays
	FillBindingArray(DEVICETYPE_KEYBOARD);
	FillBindingArray(DEVICETYPE_MOUSE);

#ifdef USE_CONFIGURE_JOYSTICK
	FillBindingArray(DEVICETYPE_JOYSTICK);
#endif

	// Fill out the column text for the bindings
	int i;
	for (i=0; i < MENU_CONTROLS_NUM_ACTIONS; i++)
	{
		SetControlText(i);
	}
	return DTRUE;
}

// Sets the key/mouse info for a control at a specific index
void CMenuControls::SetControlText(int nIndex)
{
	// Get the control for this index
	CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)m_listOption.GetControl(nIndex);
	
	char szTemp[1024];
	memset(szTemp, 0, sizeof(szTemp));

	// Go through each device adding its control text
	int i;
	for (i=0; i < MENU_CONTROLS_NUM_DEVICES; i++)
	{		
		if ( m_deviceControlArray[i][nIndex] && _mbstrlen(m_deviceControlArray[i][nIndex]) > 0)
		{
			// Do we need to append "or" to the string?
			if (_mbstrlen(szTemp) > 0)
			{
				_mbscat((unsigned char*)szTemp, (const unsigned char*)" or ");
			}

			// Special case each device type so that it appears correctly on the screen
			switch (GetDeviceTypeFromIndex(i))
			{
			case DEVICETYPE_KEYBOARD:
				{
					// Translate the key message from English to the current keyboard
					char szNewKey[1024];
					ConvertKeyToCurrentKeyboard((char *)&szNewKey, m_deviceControlArray[i][nIndex], sizeof(szNewKey));

					_mbscat((unsigned char*)szTemp, (const unsigned char*)szNewKey);
					break;
				}
			case DEVICETYPE_MOUSE:
				{					
					// Change it from BUTTON to MBUTTON
					_mbscat((unsigned char*)szTemp, (const unsigned char*)"MBUTTON");

					// Get the number that is currently in the string and increment it by one
					// (changing MBUTTON 0 to MBUTTON 1)
					int nButton=atoi(m_deviceControlArray[i][nIndex]+_mbstrlen("button"));
						
					char szTemp2[256]; 
					sprintf(szTemp2, " %d", nButton+1);
					_mbscat((unsigned char*)szTemp, (const unsigned char*)szTemp2);					
					break;
				}
			case DEVICETYPE_JOYSTICK:
				{
					// Prepend a "J" to the control name
					_mbscat((unsigned char*)szTemp, (const unsigned char*)"J");				
					_mbscat((unsigned char*)szTemp, (const unsigned char*)m_deviceControlArray[i][nIndex]);
					break;
				}
			}			
		}
	}
	
	// If the key is unassigned, then just set the text to the unassigned message
	if ( _mbstrlen(szTemp) == 0 )
	{
		pCtrl->SetString(MENU_COLUMN_KEY, IDS_MENU_KEY_UNASSIGNED);		
	}
	else
	{
		// Set the text in the control
		HSTRING hString=m_pClientDE->CreateString(szTemp);
		pCtrl->SetString(MENU_COLUMN_KEY, hString);
		m_pClientDE->FreeString(hString);
	}
}

// Fills out the binding array (m_keyArray and m_mouseArray) for a specific input device.
void CMenuControls::FillBindingArray(DWORD dwDeviceType)
{
	// Get the bindings
	DeviceBinding *pBinding=m_pClientDE->GetDeviceBindings(dwDeviceType);

	// Iterate through each binding setting the control text if necessary
	DeviceBinding *pCurrentBinding=pBinding;
	while ( pCurrentBinding != NULL )
	{
		if ( pCurrentBinding->pActionHead )
		{
			char *lpszActionName=pCurrentBinding->pActionHead->strActionName;
			if ( lpszActionName )
			{
				// Find the action in the global list
				int nIndex=GetActionIndex(lpszActionName);
				if ( nIndex != -1 )
				{	
					SetBindingInfo(nIndex, pCurrentBinding->strTriggerName, dwDeviceType);					
				}				
			}
		}
		pCurrentBinding=pCurrentBinding->pNext;
		if ( pCurrentBinding == pBinding )
		{
			break;
		}
	}

	// Free the device bindings
	m_pClientDE->FreeDeviceBindings(pBinding);
}

// Adds the columns to the controls
void CMenuControls::InitColumns()
{
	// Create an empty string
	HSTRING hEmpty=m_pClientDE->CreateString("");

	int i;
	for (i=0; i < MENU_CONTROLS_NUM_ACTIONS; i++)
	{
		// The initial column (contains the action)
		CLTGUIColumnTextCtrl *pCtrl=AddColumnTextOption(MENU_CMD_CHANGE_CONTROL, m_pMainMenus->GetSmallFont());

		// The "action" column		
		pCtrl->AddColumn(g_nControlActionStringID[i], 100, CF_JUSTIFY_RIGHT);		

		// This is a buffer column to space things out on the screen		
		pCtrl->AddColumn(hEmpty, 11, CF_JUSTIFY_LEFT);

		// The equals column.  Changes from "" to "=" when the user is binding the key		
		pCtrl->AddColumn(hEmpty, 20, CF_JUSTIFY_LEFT);

		// The column that contains the key which is assigned to the control!		
		pCtrl->AddColumn(IDS_MENU_KEY_UNASSIGNED, 100, CF_JUSTIFY_LEFT);
	}

	m_pClientDE->FreeString(hEmpty);
}

// Sets the binding array info for a device based on an action index
void CMenuControls::SetBindingInfo(int nIndex, char *lpszNewControl, DWORD dwDeviceType)
{
	if (lpszNewControl == DNULL)
	{
		assert(DFALSE);
		return;
	}

	// Pointer to the text
	char *pDeviceText=m_deviceControlArray[GetDeviceIndexFromType(dwDeviceType)][nIndex];

	// Delete the current string
	if ( pDeviceText != DNULL )
	{
		delete []pDeviceText;
		pDeviceText=DNULL;
	}

	// Allocate the new string
	int nBufferLength=_mbstrlen(lpszNewControl)+1;
	pDeviceText=new char[nBufferLength];
	memset(pDeviceText, 0, nBufferLength);

	// Copy the string
	_mbscpy((unsigned char*)pDeviceText, (const unsigned char*)lpszNewControl);
	m_deviceControlArray[GetDeviceIndexFromType(dwDeviceType)][nIndex]=pDeviceText;
}

// This finds lpszString in g_pControlActionName and returns the index to it.
// -1 is returned if the string cannot be found.
int CMenuControls::GetActionIndex(char *lpszString)
{	
	int i;
	for (i=0; i < MENU_CONTROLS_NUM_ACTIONS; i++)
	{
		if ( _mbsicmp((const unsigned char*)g_pControlActionName[i], (const unsigned char*)lpszString) == 0 )
		{
			return i;
		}
	}

	// Couldn't find the key
	return -1;
}

// Finds the index of an action that has a specific control assigned to it.
// -1 is returned if the action cannot be found
int CMenuControls::GetControlIndex(char *lpszAction, DWORD dwDeviceType)
{
	int nDeviceIndex=GetDeviceIndexFromType(dwDeviceType);
	if (nDeviceIndex == -1)
	{
		assert(FALSE);
		return -1;
	}

	int i;
	for (i=0; i < MENU_CONTROLS_NUM_ACTIONS; i++)
	{		
		if ( m_deviceControlArray[nDeviceIndex][i] && _mbsicmp((const unsigned char*)m_deviceControlArray[nDeviceIndex][i], (const unsigned char*)lpszAction) == 0 )
		{
			return i;
		}
	}

	// Couldn't find the string
	return -1;	

}

// Handle device tracking
DBOOL CMenuControls::HandleDeviceTrack(DeviceInput *pInput, DDWORD dwNumDevices)
{	
	DBOOL bStopTracking=DFALSE;
	if ( !pInput )
	{
		assert(DFALSE);		
		return bStopTracking;
	}

	// Get the control for the selected item
	int nIndex=m_listOption.GetSelectedItem();
	CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)m_listOption.GetControl(nIndex);
	
	// Go through each device
	while (dwNumDevices--)
	{
		// Only accept buttons or keys on the keyboard
		if (pInput->m_ControlType != CONTROLTYPE_BUTTON &&
			pInput->m_ControlType != CONTROLTYPE_KEY)
		{
			pInput++;
			continue;
		}

		if (pInput->m_DeviceType == DEVICETYPE_KEYBOARD)
		{
			// Ignore the next keyboard message
			g_pBloodClientShell->IgnoreKeyboardMessage(DTRUE);

			// Don't allow the tab key to be bound
			if ( _mbsicmp((const unsigned char*)pInput->m_ControlName, (const unsigned char*)"Tab") == 0 )
			{
				pInput++;
				continue;
			}

			// Cancel this binding of the escape key was pressed
			if ( _mbsicmp((const unsigned char*)pInput->m_ControlName, (const unsigned char*)"Escape") == 0 )
			{			
				// Remove the "=" from the display
				HSTRING hEmpty=m_pClientDE->CreateString("");
				pCtrl->SetString(MENU_COLUMN_EQUALS, hEmpty);
				m_pClientDE->FreeString(hEmpty);

				bStopTracking=DTRUE;
				break;
			}
		}

		// Make sure that this key isn't binded anywhere else
		int nOtherKeyIndex=GetControlIndex(pInput->m_ControlName, pInput->m_DeviceType);
		if (nOtherKeyIndex != -1)
		{
			// "Unbind" that key
			BindControlToAction(pInput->m_ControlName, "", pInput->m_DeviceName);
			SetBindingInfo(nOtherKeyIndex, "", pInput->m_DeviceType);
			SetControlText(nOtherKeyIndex);
		}

		// Remove the binding on the current key
		char *pCurrentKey=m_deviceControlArray[GetDeviceIndexFromType(pInput->m_DeviceType)][nIndex];
		if ( pCurrentKey )
		{
			BindControlToAction(pCurrentKey, "", pInput->m_DeviceName);
		}

		// Bind the key and update the control text
		SetBindingInfo(nIndex, pInput->m_ControlName, pInput->m_DeviceType);
		SetControlText(nIndex);		
		BindControlToAction(pInput->m_ControlName, g_pControlActionName[nIndex], pInput->m_DeviceName);

		// Remove the "=" from the display
		HSTRING hEmpty=m_pClientDE->CreateString("");
		pCtrl->SetString(MENU_COLUMN_EQUALS, hEmpty);
		m_pClientDE->FreeString(hEmpty);

		pInput++;

		bStopTracking=DTRUE;
	}

	return bStopTracking;
}

// Binds a key to a action
void CMenuControls::BindControlToAction(char *lpszControlName, char *lpszActionName, char *lpszDevice)
{
	assert(lpszControlName);
	assert(lpszActionName);
	assert(lpszDevice);

	char tempStr[256];

	// Set the binding
	sprintf(tempStr, "rangebind \"%s\" \"%s\" 0 0 \"%s\"", lpszDevice, lpszControlName, lpszActionName);
	m_pClientDE->RunConsoleString(tempStr);
}

// Handle a keypress
DBOOL CMenuControls::HandleKeyDown(int key, int rep)
{
	// Check to see if the base class is handling this key
	if (CMenuBase::HandleKeyDown(key, rep))
	{
		return DTRUE;
	}

	switch (key)
	{
	case VK_DELETE:
		{
			// Unassign the key
			int nIndex=m_listOption.GetSelectedItem();

			int i;
			for (i=0; i < MENU_CONTROLS_NUM_DEVICES; i++)
			{
				if (m_deviceControlArray[i][nIndex])
				{
					DWORD dwDeviceType=GetDeviceTypeFromIndex(i);					

					// Get the mouse device name
					char szBuffer[2048];
					if (m_pClientDE->GetDeviceName (dwDeviceType, szBuffer, sizeof(szBuffer)) == LT_OK)
					{
						BindControlToAction(m_deviceControlArray[i][nIndex], "", szBuffer);
					}
				}
				SetBindingInfo(nIndex, "", GetDeviceTypeFromIndex(i));
				SetControlText(nIndex);
			}						
			break;
		}
	default:
		{
			// Didn't handle the key
			return DFALSE;
			break;
		}
	}

	// Handled the key
	return DTRUE;
}

DDWORD CMenuControls::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MENU_CMD_CHANGE_CONTROL:
		{			
			// Set the current menu item to ??? while tracking the device.
			int nIndex=m_listOption.GetSelectedItem();
			CLTGUIColumnTextCtrl *pCtrl=(CLTGUIColumnTextCtrl *)m_listOption.GetControl(nIndex);
			
			HSTRING hEquals=m_pClientDE->CreateString("=");
			pCtrl->SetString(MENU_COLUMN_EQUALS, hEquals);
			m_pClientDE->FreeString(hEquals);

			m_pMainMenus->StartTrackingDevices(DEVICETYPE_KEYBOARD);
			m_pMainMenus->StartTrackingDevices(DEVICETYPE_MOUSE);

#ifdef USE_CONFIGURE_JOYSTICK
			if (g_pBloodClientShell->IsUseJoystick())
			{
				m_pMainMenus->StartTrackingDevices(DEVICETYPE_JOYSTICK);
			}
#endif

			break;
		}
	}
	return DTRUE;
}
