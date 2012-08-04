// FolderCustomControls.cpp: implementation of the CFolderCustomControls class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderCustomControls.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "keyfixes.h"
#include "dinput.h"

extern CommandID g_CommandArray[];


// The different columns
#define FOLDER_COLUMN_ACTION		0
#define FOLDER_COLUMN_BUFFER		1
#define FOLDER_COLUMN_EQUALS		2
#define FOLDER_COLUMN_KEY			3

namespace
{

    uint32 devices[FOLDER_CONTROLS_NUM_DEVICES] =
	{
		DEVICETYPE_KEYBOARD,
		DEVICETYPE_MOUSE,
		DEVICETYPE_JOYSTICK
	};

	char strDeviceName[FOLDER_CONTROLS_NUM_DEVICES][256] =
	{
		"","",""
	};

	char strDeviceNiceName[FOLDER_CONTROLS_NUM_DEVICES][256] =
	{
		"","",""
	};

    LTBOOL   bEatMouseButtonUp = LTFALSE;

	char szWheelUp[32] = "";
	char szWheelDown[32] = "";
	char szDebugStr[512] = "";
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCustomControls::CFolderCustomControls()
{
    m_bControlChanged = LTFALSE;
    m_bWaitingForKey = LTFALSE;
	m_nEntries = 0;
	m_fInputPauseTimeLeft = 0.0f;

}

CFolderCustomControls::~CFolderCustomControls()
{

}

void CFolderCustomControls::Escape()
{
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
	{
		return;
	}

	// Make sure we save any config changes...

	if (m_bControlChanged)
	{
        g_pLTClient->WriteConfigFile("autoexec.cfg");
	}

    m_bControlChanged = LTFALSE;

	CBaseFolder::Escape();
}
LTBOOL   CFolderCustomControls::Init(int nFolderID)
{
	return CBaseFolder::Init(nFolderID);
}

// Build the folder
LTBOOL CFolderCustomControls::Build()
{

	CreateTitle(IDS_TITLE_CUSTOM_CONTROLS);

	m_nActionWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"ActionWidth");
	m_nBufferWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"BufferWidth");
	m_nEqualsWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_CUST_CONTROLS,"EqualsWidth");


	HSTRING hStr = g_pLTClient->FormatString(IDS_UP);
	SAFE_STRCPY(szWheelUp,g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DOWN);
	SAFE_STRCPY(szWheelDown,g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DEVICE_MOUSE);
	SAFE_STRCPY(strDeviceNiceName[1],g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);

	hStr = g_pLTClient->FormatString(IDS_DEVICE_JOYSTICK);
	SAFE_STRCPY(strDeviceNiceName[2],g_pLTClient->GetStringData(hStr));
	g_pLTClient->FreeString(hStr);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}


// Intialize the list of controls
LTBOOL CFolderCustomControls::InitControlList()
{
	// Remove the current options
	RemoveFree();


	int navNum = 0;

	m_nEntries = 0;
	for (int nFilter = 0; nFilter < 4; nFilter++)
	{
		int nEntries = 0;
		int nFirstEntry = m_nEntries;
		for (int i = 0; i < g_kNumCommands - 1; i++)
		{
			if (g_CommandArray[i].nCommandType == nFilter)
			{
				m_pEntries[m_nEntries].nStringID = g_CommandArray[i].nStringID;
				m_pEntries[m_nEntries].nAction = g_CommandArray[i].nCommandID;
				m_nEntries++;
				nEntries++;
			}
		}


		// Add the columns
		InitColumns(nFilter,nFirstEntry,nEntries);
	}


	CalculateLastDrawn();

    return LTTRUE;
}

// Sets the key/mouse info for a control at a specific index
void CFolderCustomControls::SetControlText(int nIndex)
{
	CLTGUIColumnTextCtrl *pCtrl = GetControlFromEntryIndex(nIndex);
	if (!pCtrl) 
	{
		_ASSERT(0);
		return;
	}

	char strControls[256] = "";
	int numControls = 0;

	if ( _mbstrlen(m_pEntries[nIndex].strControlName[0]) != 0 )
	{
		SAFE_STRCPY(strControls,m_pEntries[nIndex].strControlName[0]);
		++numControls;
	}

	if ( _mbstrlen(m_pEntries[nIndex].strControlName[1]) != 0 )
	{
		if (numControls)
			strcat(strControls,", ");
		strcat(strControls,strDeviceNiceName[1]);
		strcat(strControls," ");
		strcat(strControls,m_pEntries[nIndex].strControlName[1]);
	}

	if ( _mbstrlen(m_pEntries[nIndex].strControlName[2]) != 0 )
	{
		if (numControls)
			strcat(strControls,", ");
		strcat(strControls,strDeviceNiceName[2]);
		strcat(strControls," ");
		strcat(strControls,m_pEntries[nIndex].strControlName[2]);
	}


	// If the key is unassigned, then just set the text to the unassigned message
	if ( _mbstrlen(strControls) == 0 )
	{
		pCtrl->SetString(FOLDER_COLUMN_KEY, IDS_CONTROL_UNASSIGNED);
	}
	else
	{
		// Set the text in the control
        HSTRING hString=g_pLTClient->CreateString(strControls);
		pCtrl->SetString(FOLDER_COLUMN_KEY, hString);
        g_pLTClient->FreeString(hString);
	}

	if (m_pEntries[nIndex].nAction == COMMAND_ID_MOUSEAIMTOGGLE)
	{
		CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
		if (pSettings)
		{
			pCtrl->Enable(!pSettings->MouseLook());
		}

	}
	if (m_pEntries[nIndex].nAction == COMMAND_ID_CROSSHAIRTOGGLE)
	{
		pCtrl->Enable(g_pInterfaceMgr->IsCrosshairEnabled());
	}
}

// Adds the columns to the controls
void CFolderCustomControls::InitColumns(int nFilter,int nFirstEntry,int nEntries)
{
	// Create an empty string
    HSTRING hEmpty=g_pLTClient->CreateString("");

    CStaticTextCtrl *pCtrl=LTNULL;


	switch (nFilter)
	{
	case COM_MOVE:
		pCtrl = AddStaticTextItem(IDS_MOVE_CONTROLS,LTNULL,LTNULL,200,0,LTTRUE,GetMediumFont());
		pCtrl->Enable(LTFALSE);
		break;
	case COM_INV:
		AddPageBreak();
		pCtrl = AddStaticTextItem(IDS_INV_CONTROLS,LTNULL,LTNULL,200,0,LTTRUE,GetMediumFont());
		pCtrl->Enable(LTFALSE);
		break;
	case COM_VIEW:
		AddPageBreak();
		pCtrl = AddStaticTextItem(IDS_VIEW_CONTROLS,LTNULL,LTNULL,200,0,LTTRUE,GetMediumFont());
		pCtrl->Enable(LTFALSE);
		break;
	case COM_MISC:
		AddPageBreak();
		pCtrl = AddStaticTextItem(IDS_MISC_CONTROLS,LTNULL,LTNULL,200,0,LTTRUE,GetMediumFont());
		pCtrl->Enable(LTFALSE);
		break;
	}

	int i;
	for (i=0; i < nEntries; i++)
	{
		// The initial column (contains the action)
		CLTGUIColumnTextCtrl *pCtrl=AddColumnText(FOLDER_CMD_CHANGE_CONTROL, IDS_HELP_SETCONTROL, LTFALSE);

		// The "action" column
		pCtrl->AddColumn(m_pEntries[nFirstEntry+i].nStringID, m_nActionWidth, LTF_JUSTIFY_LEFT);

		// This is a buffer column to space things out on the screen
		pCtrl->AddColumn(hEmpty, m_nBufferWidth, LTF_JUSTIFY_LEFT);

		// The equals column.  Changes from "" to "=" when the user is binding the key
		pCtrl->AddColumn(hEmpty, m_nEqualsWidth, LTF_JUSTIFY_LEFT);

		// The column that contains the key which is assigned to the control!
		pCtrl->AddColumn(IDS_CONTROL_UNASSIGNED, 100, LTF_JUSTIFY_LEFT);

		pCtrl->SetParam1(1+nFirstEntry+i);

	}

    g_pLTClient->FreeString(hEmpty);
}
// Unbinds an action
void CFolderCustomControls::UnBind(int commandIndex, uint32 deviceType)
{

    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (deviceType);
	if (!pBindings)
	{
		return;
	}

	char szDevice[2048];
    if (!g_pLTClient->GetDeviceName (deviceType, szDevice, sizeof(szDevice)) == LT_OK)
	{
		_ASSERT(0);
		return;
	}


	int dev = 0;
	while (dev < FOLDER_CONTROLS_NUM_DEVICES && devices[dev] != deviceType)
		++dev;


	if (dev < FOLDER_CONTROLS_NUM_DEVICES)
		SAFE_STRCPY (m_pEntries[commandIndex].strControlName[dev], "");


    LTBOOL bFound = LTFALSE;
	DeviceBinding* ptr = pBindings;
	GameAction* pUpAction = LTNULL;
	GameAction* pDownAction = LTNULL;
	LTBOOL bRebindWheel  = LTFALSE;

	while (ptr)
	{
		GameAction* pAction = ptr->pActionHead;
		while (pAction)
		{
			uint32 contType = GetControlType(deviceType, ptr->strTriggerName);
			if (deviceType == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
			{
				if (pAction->nRangeHigh > 0)
				{
					pUpAction = pAction;

				}
				if (pAction->nRangeHigh < 0)
				{
					pDownAction = pAction;

				}

			}
			if (pAction->nActionCode == m_pEntries[commandIndex].nAction)
			{
                
				if (contType == CONTROLTYPE_KEY)
				{
					char tempStr[256];

					// Set the binding
					if (g_pInterfaceResMgr->IsEnglish() && ptr->strTriggerName[0] == ';')
						sprintf(tempStr, "rangebind \"%s\" \"##39\" 0 0 \"\"", szDevice);
					else
						sprintf(tempStr, "rangebind \"%s\" \"%s\" 0 0 \"\"", szDevice, ptr->strTriggerName);
                    g_pLTClient->CPrint(tempStr);
                    g_pLTClient->RunConsoleString(tempStr);
                    m_bControlChanged = LTTRUE;

                    bFound = LTTRUE;
				}
				else if (contType == CONTROLTYPE_BUTTON)
				{
					char tempStr[256];

					// Set the binding
					sprintf(tempStr, "rangebind \"%s\" \"%s\" 0 0 \"\"", szDevice, ptr->strTriggerName);
                    g_pLTClient->CPrint(tempStr);
                    g_pLTClient->RunConsoleString(tempStr);
                    m_bControlChanged = LTTRUE;

                    bFound = LTTRUE;
				}
				else if (deviceType == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
				{
					bRebindWheel = LTTRUE;
					if (pAction == pUpAction) pUpAction = LTNULL;
					if (pAction == pDownAction) pDownAction = LTNULL;
				}
				break;
			}

			pAction = pAction->pNext;
		}


		ptr = ptr->pNext;
	}
	if (bRebindWheel)
	{
		int nUpAction = -1;
		int nDownAction = -1;
		if (pUpAction)
		{
			nUpAction = pUpAction->nActionCode;
		}
		if (pDownAction)
		{
			nDownAction = pDownAction->nActionCode;
		}

		RebindWheel(nUpAction,nDownAction);

	}

    g_pLTClient->FreeDeviceBindings (pBindings);


}

// Binds a key to a action
void CFolderCustomControls::Bind(int commandIndex, char *lpszControlName, uint32 deviceType, uint8 diCode)
{
	_ASSERT(lpszControlName);

	char szDevice[2048];
    if (!g_pLTClient->GetDeviceName (deviceType, szDevice, sizeof(szDevice)) == LT_OK)
	{
		_ASSERT(0);
		return;
	}

	UnBind(commandIndex,deviceType);

	int dev = 0;
	while (dev < FOLDER_CONTROLS_NUM_DEVICES && devices[dev] != deviceType)
		++dev;


	// Set the binding
    uint32 contType = GetControlType(deviceType, lpszControlName);
	if (contType == CONTROLTYPE_BUTTON || contType == CONTROLTYPE_KEY || !strlen(lpszControlName))
	{
		if (dev < FOLDER_CONTROLS_NUM_DEVICES)
			SAFE_STRCPY (m_pEntries[commandIndex].strControlName[dev], lpszControlName);

		char tempStr[256];

		if (diCode)
			sprintf(tempStr, "rangebind \"%s\" \"##%d\" 0 0 \"%s\"", szDevice, diCode, CommandName(m_pEntries[commandIndex].nAction));
		else
			sprintf(tempStr, "rangebind \"%s\" \"%s\" 0 0 \"%s\"", szDevice, lpszControlName, CommandName(m_pEntries[commandIndex].nAction));
		g_pLTClient->CPrint(tempStr);
		SAFE_STRCPY(szDebugStr,tempStr);
		g_pLTClient->RunConsoleString(tempStr);
	
		SetControlText(commandIndex);

	}



}

// Handle a keypress
LTBOOL CFolderCustomControls::HandleKeyDown(int key, int rep)
{

    LTBOOL handled = LTFALSE;
	switch (key)
	{
	case VK_DELETE:
		{
			// Unassign the key
			if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
			{
				int nIndex=GetSelectedControl()->GetParam1()-1;
				if (nIndex >= 0 && nIndex < m_nEntries)
				{
					Bind(nIndex, "", DEVICETYPE_KEYBOARD);
					Bind(nIndex, "", DEVICETYPE_MOUSE);
					Bind(nIndex, "", DEVICETYPE_JOYSTICK);
					handled = LTTRUE;
				}
			}
			break;
		}
	}
	// Check to see if the base class is handling this key
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
		return handled;
	if (!handled)
	{
		handled = CBaseFolder::HandleKeyDown(key, rep);
	}

	// Handled the key
	return handled;
}


uint32 CFolderCustomControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_CUSTOM:
			break;

	case FOLDER_CMD_CHANGE_CONTROL:
		{

			if (m_bWaitingForKey)
				break;
			// Set the current folder item to ??? while tracking the device.
			int nIndex=GetSelectedControl()->GetParam1()-1;
			if (nIndex < 0 || nIndex >= m_nEntries)
				break;

			CLTGUIColumnTextCtrl *pCtrl=GetControlFromEntryIndex(nIndex);

            HSTRING hEquals=g_pLTClient->CreateString("=");
			pCtrl->SetString(FOLDER_COLUMN_EQUALS, hEquals);
            g_pLTClient->FreeString(hEquals);

            HSTRING hEmpty=g_pLTClient->CreateString("");
			pCtrl->SetString(FOLDER_COLUMN_KEY, hEmpty);
            g_pLTClient->FreeString(hEmpty);

			// see if we can track the input devices
            LTRESULT hResult = g_pLTClient->StartDeviceTrack (DEVICETYPE_KEYBOARD | DEVICETYPE_MOUSE | DEVICETYPE_JOYSTICK, TRACK_BUFFER_SIZE);
			if (hResult != LT_OK)
			{
                g_pLTClient->EndDeviceTrack();
			}
			else
			{
                m_bWaitingForKey = LTTRUE;
			}


			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

LTBOOL CFolderCustomControls::Render(HSURFACE hDestSurf)
{
	//no actual rendering here... just wait for keypress

	// see if we are pausing input
	if (m_fInputPauseTimeLeft)
	{
        m_fInputPauseTimeLeft -= g_pGameClientShell->GetFrameTime();
		if (m_fInputPauseTimeLeft < 0.0f) m_fInputPauseTimeLeft = 0.0f;
	}

	// see if we are waiting for a keypress
	if (m_bWaitingForKey)
	{
        uint32 nArraySize = TRACK_BUFFER_SIZE;
        g_pLTClient->TrackDevice (m_pInputArray, &nArraySize);
		if (nArraySize > 0)
		{
			// find the first key down event
            for (uint32 i = 0; i < nArraySize; i++)
			{
				if (m_pInputArray[i].m_InputValue)
				{
					if (SetCurrentSelection (&m_pInputArray[i]))
					{
						if (m_pInputArray[i].m_ControlType == CONTROLTYPE_BUTTON)
                            bEatMouseButtonUp = LTTRUE;
                        m_bWaitingForKey = LTFALSE;
                        g_pLTClient->EndDeviceTrack();
						m_fInputPauseTimeLeft = 0.2f;
					}
				}
			}
		}
	}
	LTBOOL bRend = CBaseFolder::Render(hDestSurf);

//	if (bRend)
//		GetMediumFont()->Draw(szDebugStr, hDestSurf, 0, 55, LTF_JUSTIFY_LEFT);

	return bRend;
}


LTBOOL CFolderCustomControls::SetCurrentSelection (DeviceInput* pInput)
{
    if (!g_pLTClient) return LTFALSE;

	int nIndex=GetSelectedControl()->GetParam1()-1;
	if (nIndex < 0 || nIndex >= m_nEntries)
		 return LTFALSE;

	if (pInput->m_DeviceType == DEVICETYPE_MOUSE && pInput->m_ControlType == CONTROLTYPE_ZAXIS)
	{
		return CheckMouseWheel(pInput);
	}

	if (pInput->m_ControlType != CONTROLTYPE_BUTTON &&
		pInput->m_ControlType != CONTROLTYPE_KEY)
        return LTFALSE;

	char sNewKey[256];
	SAFE_STRCPY(sNewKey,pInput->m_ControlName);
	if (pInput->m_DeviceType == DEVICETYPE_KEYBOARD && (_mbstrlen(pInput->m_ControlName) == 1))
	{
		ConvertKeyToCurrentKeyboard(sNewKey, pInput->m_ControlName, 256);
	}

	// see if this key is bound to something not in the keyboard configuration folder...
	if (KeyRemappable(pInput))
	{


		if (pInput->m_DeviceType == DEVICETYPE_KEYBOARD)
		{
			uint8 diCode = (uint8)pInput->m_ControlName[63];
			g_pLTClient->CPrint("diCode:%d",diCode);
			sprintf(szDebugStr,"diCode:%d",diCode);

			if (_mbstrlen(pInput->m_ControlName) != 1)
			{
				Bind(nIndex, pInput->m_ControlName, pInput->m_DeviceType, diCode);
			}
			else
			{
				Bind(nIndex, sNewKey, pInput->m_DeviceType, diCode);
			}
		}
		else
		{
			Bind(nIndex, pInput->m_ControlName, pInput->m_DeviceType);
		}


		// see if this control is being used for any other commands...if so, disable that command.

		int dev = 0;
		while (dev < FOLDER_CONTROLS_NUM_DEVICES && devices[dev] != pInput->m_DeviceType)
			++dev;

		if (dev < FOLDER_CONTROLS_NUM_DEVICES)
		{
			for (int i = 0; i < m_nEntries; i++)
			{
				if (i == nIndex) continue;
				if (stricmp (m_pEntries[i].strControlName[dev], sNewKey) == 0)
				{
					Bind(i,"", pInput->m_DeviceType);
				}
			}
		}

	};
	SetControlText(nIndex);
	CLTGUIColumnTextCtrl *pCtrl=GetControlFromEntryIndex(nIndex);
    HSTRING hEmpty=g_pLTClient->CreateString("");
	pCtrl->SetString(FOLDER_COLUMN_EQUALS, hEmpty);
    g_pLTClient->FreeString(hEmpty);

    return LTTRUE;
}

LTBOOL CFolderCustomControls::KeyRemappable (DeviceInput* pInput)
{
    if (!g_pLTClient) return LTFALSE;


//*********************************************************************************
	uint8 nDIK = (uint8)pInput->m_ControlName[63];
	if (nDIK == DIK_ESCAPE)
        return LTFALSE;
	if (nDIK == DIK_PAUSE)
        return LTFALSE;
	if (nDIK >= DIK_F1 && nDIK <= DIK_F10)
        return LTFALSE;
	if (nDIK >= DIK_F11 && nDIK <= DIK_F15)
        return LTFALSE;
	char sNewKey[256];
	ConvertKeyToCurrentKeyboard(sNewKey, pInput->m_ControlName, 256);
	if (sNewKey[0] == ';')
		return LTFALSE;
//*********************************************************************************


    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
    if (!pBindings) return LTTRUE;

	// see if this input object is already bound to something...

	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		if (strcmp (ptr->strTriggerName, pInput->m_ControlName) == 0)
		{
			// see if this binding is in the folders or if it exists outside the folders (like a weapon-select action)
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
                LTBOOL bFoundFolderMatch = LTFALSE;
				for (int i = 0; i < m_nEntries; i++)
				{
					if (m_pEntries[i].nAction == pAction->nActionCode)
					{
                        bFoundFolderMatch = LTTRUE;
						break;
					}
				}

				if (!bFoundFolderMatch)
				{
					// this key is already bound to something that doesn't exist in the folders...can't bind it.
                    g_pLTClient->FreeDeviceBindings (pBindings);
                    return LTFALSE;
				}

				pAction = pAction->pNext;
			}

			// binding must already exist in the folders...
			break;
		}
		ptr = ptr->pNext;
	}

	// either the binding exists in the folders or this key is not currently bound...therefore it's remappable

    g_pLTClient->FreeDeviceBindings (pBindings);
    return LTTRUE;
}


LTBOOL CFolderCustomControls::OnUp()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnUp();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnDown()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnDown();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnLeft()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLeft();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRight()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRight();
	else
        return LTTRUE;
}
LTBOOL CFolderCustomControls::OnEnter()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnEnter();
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnLButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnLButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseFolder::OnLButtonUp(x,y);
}

LTBOOL CFolderCustomControls::OnLButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnLButtonDblClick(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CFolderCustomControls::OnRButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseFolder::OnRButtonUp(x,y);
}

LTBOOL CFolderCustomControls::OnRButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnRButtonDblClick(x,y);
	else
        return LTTRUE;
}


LTBOOL CFolderCustomControls::OnMouseMove(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseFolder::OnMouseMove(x, y);
	else
        return LTTRUE;
}

void CFolderCustomControls::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Add the controls to the folder
		InitControlList();
		FillControlList();
	}
	else
	{
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


uint32 CFolderCustomControls::GetControlType(uint32 deviceType, char *controlName)
{
    DeviceObject* pObj = g_pLTClient->GetDeviceObjects(deviceType);
    uint32 type = CONTROLTYPE_UNKNOWN;
	bool bFound = false;
	while (pObj != NULL && !bFound)
	{
		if (pObj->m_ObjectName != NULL)
		{
			bFound = (strcmp (controlName, pObj->m_ObjectName) == 0);
			if (bFound)
				type = pObj->m_ObjectType;
		}
		pObj = pObj->m_pNext;
	}
	return type;
}



// fill the list of controls with binding data
void CFolderCustomControls::FillControlList()
{
	for (int dev = 0; dev < FOLDER_CONTROLS_NUM_DEVICES; ++dev)
	{
        if (!g_pLTClient->GetDeviceName (devices[dev],strDeviceName[dev], sizeof(strDeviceName[dev])) == LT_OK)
		{
			continue;
		}
		for (int i = 0; i < m_nEntries; i++)
		{
			SAFE_STRCPY (m_pEntries[i].strControlName[dev], "");
		}
        uint32 devType = devices[dev];
        DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (devType);
		if (!pBindings)
		{
			continue;
		}
		for (i = 0; i < m_nEntries; i++)
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
                        uint32 contType = GetControlType(devType, ptr->strTriggerName);
						if (contType == CONTROLTYPE_KEY)
						{
							char sNewKey[256];
							ConvertKeyToCurrentKeyboard(sNewKey, ptr->strTriggerName, 256);

							SAFE_STRCPY (m_pEntries[i].strControlName[dev], sNewKey);

                            bFound = LTTRUE;
						}
						else if (contType == CONTROLTYPE_BUTTON)
						{

							SAFE_STRCPY (m_pEntries[i].strControlName[dev], ptr->strTriggerName);

                            bFound = LTTRUE;
						}
						else if (devType == DEVICETYPE_MOUSE && contType == CONTROLTYPE_ZAXIS)
						{
							if (pAction->nRangeHigh > 0)
								sprintf(m_pEntries[i].strControlName[dev],"%s %s",ptr->strTriggerName,szWheelUp);
							else if (pAction->nRangeHigh < 0)
								sprintf(m_pEntries[i].strControlName[dev],"%s %s",ptr->strTriggerName,szWheelDown);
						}
						break;
					}

					pAction = pAction->pNext;
				}

				ptr = ptr->pNext;
			}

		}
        g_pLTClient->FreeDeviceBindings (pBindings);
	}

	// Fill out the column text for the bindings
	for (int n=0; n < m_nEntries; n++)
	{
		SetControlText(n);
	}

}

CLTGUIColumnTextCtrl *CFolderCustomControls::GetControlFromEntryIndex(int nIndex)
{
	// Get the control for this index
	int nTest = 0;
	LTBOOL bFound = LTFALSE;
	CLTGUIColumnTextCtrl *pCtrl = LTNULL;
	while (!bFound && nTest < (int)m_controlArray.GetSize())
	{
		pCtrl=(CLTGUIColumnTextCtrl *)GetControl(nTest);
		if ((int)pCtrl->GetParam1() == (nIndex+1))
		{
			bFound = LTTRUE;
		}
		else
			nTest++;
	}
	return pCtrl;
}


LTBOOL CFolderCustomControls::CheckMouseWheel (DeviceInput* pInput)
{
    if (!g_pLTClient) return LTFALSE;

	if (pInput->m_DeviceType != DEVICETYPE_MOUSE || pInput->m_ControlType != CONTROLTYPE_ZAXIS)
		return LTFALSE;

	LTBOOL bWheelUp = ((int)pInput->m_InputValue > 0);

	char szDevice[256];
    if (!g_pLTClient->GetDeviceName (pInput->m_DeviceType, szDevice, sizeof(szDevice)) == LT_OK)
	{
		_ASSERT(0);
		return LTFALSE;
	}


    LTBOOL bFound = LTFALSE;
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (pInput->m_DeviceType);
	if (!pBindings) return LTFALSE;

	DeviceBinding* ptr = pBindings;

	DeviceBinding* pBinding = LTNULL;
	GameAction* pUpAction = LTNULL;
	GameAction* pDownAction = LTNULL;

	while (!bFound && ptr)
	{


		if (stricmp(ptr->strDeviceName,pInput->m_DeviceName) == 0 &&
			stricmp(ptr->strTriggerName,pInput->m_ControlName) == 0)
		{
			pBinding = ptr;
			bFound = LTTRUE;
		
			GameAction* pAction = ptr->pActionHead;

			while (pAction)
			{
				if (pAction->nRangeHigh > 0)
				{
					pUpAction = pAction;

				}
				if (pAction->nRangeHigh < 0)
				{
					pDownAction = pAction;

				}
				pAction = pAction->pNext;
			}
		}

		ptr = ptr->pNext;
	}


	int nIndex=GetSelectedControl()->GetParam1()-1;
	if (nIndex < 0 || nIndex >= m_nEntries)
		 return LTFALSE;

	int dev = 0;
	while (dev < FOLDER_CONTROLS_NUM_DEVICES && devices[dev] != pInput->m_DeviceType)
		++dev;

	UnBind(nIndex,pInput->m_DeviceType);

	// see if this control is being used for any other commands...if so, disable that command.
	char tempStr[512];
	if (bWheelUp)
		sprintf(tempStr,"%s %s",pInput->m_ControlName,szWheelUp);
	else
		sprintf(tempStr,"%s %s",pInput->m_ControlName,szWheelDown);

	for (int i = 0; i < m_nEntries; i++)
	{
		if (i == nIndex) continue;
		if (stricmp (m_pEntries[i].strControlName[dev], tempStr) == 0)
		{
			Bind(i,"", pInput->m_DeviceType);
		}
	}

	Bind(nIndex,tempStr, pInput->m_DeviceType);
	SAFE_STRCPY (m_pEntries[nIndex].strControlName[dev], tempStr);
	int nUpAction = -1;
	int nDownAction = -1;
	if (pUpAction)
	{
		nUpAction = pUpAction->nActionCode;
	}
	if (pDownAction)
	{
		nDownAction = pDownAction->nActionCode;
	}

	if (bWheelUp)
	{
		nUpAction = m_pEntries[nIndex].nAction;
	}
	else
	{
		nDownAction = m_pEntries[nIndex].nAction;
	}

	RebindWheel(nUpAction,nDownAction);

	SetControlText(nIndex);
	CLTGUIColumnTextCtrl *pCtrl=GetControlFromEntryIndex(nIndex);
    HSTRING hEmpty=g_pLTClient->CreateString("");
	pCtrl->SetString(FOLDER_COLUMN_EQUALS, hEmpty);
    g_pLTClient->FreeString(hEmpty);

	g_pLTClient->FreeDeviceBindings (pBindings);


    return LTTRUE;
}


void CFolderCustomControls::RebindWheel(int nUpAction,int nDownAction)
{
	char szDevice[64];
    g_pLTClient->GetDeviceName (DEVICETYPE_MOUSE, szDevice, sizeof(szDevice));

	// Set the binding
	//rangebind "##mouse" "##z-axis" 0.100000 255.000000 "Forward" -255.000000 -0.100000 "Backward" 
	char tempStr[512] = "";
	char upStr[64] = "";
	if (nUpAction >= 0)
	{
		sprintf(upStr,"0.10 255.0 \"%s\"",CommandName(nUpAction));
	}
	char downStr[64] = "";
	if (nDownAction >= 0)
	{
		sprintf(downStr,"-0.10 -255.0 \"%s\"",CommandName(nDownAction));
	}

	sprintf(tempStr, "rangebind \"%s\" \"##z-axis\" %s %s", szDevice, upStr, downStr);
    g_pLTClient->CPrint(tempStr);
    g_pLTClient->RunConsoleString(tempStr);
}


