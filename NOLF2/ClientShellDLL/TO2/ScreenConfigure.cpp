// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenConfigure.cpp
//
// PURPOSE : Interface screen for binding keys to commands
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenConfigure.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "dinput.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"

// The different columns
#define SCREEN_COLUMN_ACTION		0
#define SCREEN_COLUMN_EQUALS		1
#define SCREEN_COLUMN_KEY			2

namespace
{

    uint32 devices[SCREEN_CONTROLS_NUM_DEVICES] =
	{
		DEVICETYPE_KEYBOARD,
		DEVICETYPE_MOUSE,
		DEVICETYPE_JOYSTICK
	};

	char strDeviceName[SCREEN_CONTROLS_NUM_DEVICES][256] =
	{
		"","",""
	};

	char strDeviceNiceName[SCREEN_CONTROLS_NUM_DEVICES][256] =
	{
		"","",""
	};

    LTBOOL   bEatMouseButtonUp = LTFALSE;

	char szWheelUp[32] = "";
	char szWheelDown[32] = "";


}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenConfigure::CScreenConfigure()
{
    m_bControlChanged = LTFALSE;
    m_bWaitingForKey = LTFALSE;
	m_fInputPauseTimeLeft = 0.0f;
	m_nType = 0;
	m_pProfile = LTNULL;

}

CScreenConfigure::~CScreenConfigure()
{

}

void CScreenConfigure::Escape()
{
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
	{
		return;
	}

	for (int n = 0; n < 4; n++)
	{
		if (GetSelection() == GetIndex(m_pList[n]))
		{
			SetCurrentType(kNumCommandTypes);
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
			return;
		}
	}

    m_bControlChanged = LTFALSE;

	CBaseScreen::Escape();
}

LTBOOL   CScreenConfigure::Init(int nScreenID)
{
	LoadString(IDS_DEVICE_MOUSE,strDeviceNiceName[1],sizeof(strDeviceNiceName[1]));
	LoadString(IDS_DEVICE_JOYSTICK,strDeviceNiceName[2],sizeof(strDeviceNiceName[2]));

	sprintf (szWheelUp,"%s %s",strDeviceNiceName[1], LoadTempString(IDS_WHEEL_UP) );
	sprintf (szWheelDown,"%s %s",strDeviceNiceName[1], LoadTempString(IDS_WHEEL_DOWN) );

	if (g_pGameClientShell->HasGamepad())
	{
		devices[2] = DEVICETYPE_GAMEPAD;
		g_pGameClientShell->EnableGamepad();
	}
	else if (g_pGameClientShell->HasJoystick())
	{
		devices[2] = DEVICETYPE_JOYSTICK;
		g_pGameClientShell->EnableJoystick();
	}
	else
		devices[2] = DEVICETYPE_UNKNOWN;



	for (int dev = 0; dev < 3; dev++)
	{
		g_pLTClient->GetDeviceName (devices[dev],strDeviceName[dev], sizeof(strDeviceName[dev]));
	}


	return CBaseScreen::Init(nScreenID);
}

// Build the screen
LTBOOL CScreenConfigure::Build()
{

	CreateTitle(IDS_TITLE_CONFIGURE);

	m_nActionWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONFIGURE,"ActionWidth");
	m_nEqualsWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONFIGURE,"EqualsWidth");
	m_nCommandWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONFIGURE,"CommandWidth");
	m_ListRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_CONFIGURE,"ListRect");
	m_nListFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONFIGURE,"ListFontSize");


	int nWidth = GetPageRight() - GetPageLeft();
	LTIntPt topPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_CONFIGURE,"MoveControlPos");

	CLTGUITextCtrl *pCtrl = AddTextItem(IDS_MOVE_CONTROLS,CMD_MOVE_COM,LTNULL,topPos);
	pCtrl->SetFixedWidth(nWidth);

	pCtrl = AddTextItem(IDS_INV_CONTROLS,CMD_INV_COM,LTNULL);
	pCtrl->SetFixedWidth(nWidth);

	pCtrl = AddTextItem(IDS_VIEW_CONTROLS,CMD_VIEW_COM,LTNULL);
	pCtrl->SetFixedWidth(nWidth);

	pCtrl = AddTextItem(IDS_MISC_CONTROLS,CMD_MISC_COM,LTNULL);
	pCtrl->SetFixedWidth(nWidth);

	
	LTIntPt pos(m_ListRect.left,m_ListRect.top);
	int nHt = m_ListRect.bottom - m_ListRect.top;
	int nWd = m_nActionWidth + m_nEqualsWidth + m_nCommandWidth;

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_CONFIGURE,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);
	m_pFrame = debug_new(CLTGUIFrame);
	m_pFrame->Create(hFrame,nWd,nHt+8,LTTRUE);
	m_pFrame->SetBasePos(pos);
	AddControl(m_pFrame);


	for (int nType = 0; nType < kNumCommandTypes; nType++)
	{
		m_pList[nType] = AddList(pos,nHt);
		m_pList[nType]->SetFrameWidth(2);
		m_pList[nType]->SetIndent(LTIntPt(4,4));
		m_pList[nType]->Show(LTFALSE);

		pCtrl = LTNULL;
		switch (nType)
		{
		case COM_MOVE:
			pCtrl = CreateTextItem(IDS_MOVE_CONTROLS,LTNULL,LTNULL,kDefaultPos,LTTRUE);
			break;
		case COM_INV:
			pCtrl = CreateTextItem(IDS_INV_CONTROLS,LTNULL,LTNULL,kDefaultPos,LTTRUE);
			break;
		case COM_VIEW:
			pCtrl = CreateTextItem(IDS_VIEW_CONTROLS,LTNULL,LTNULL,kDefaultPos,LTTRUE);
			break;
		case COM_MISC:
			pCtrl = CreateTextItem(IDS_MISC_CONTROLS,LTNULL,LTNULL,kDefaultPos,LTTRUE);
			break;
		}

		if (pCtrl)
			m_pList[nType]->AddControl(pCtrl);

	}

	InitControlList();

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

void CScreenConfigure::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		m_pProfile->SetBindings();
		UpdateControlList();
	}
	else
	{
		m_pProfile->ApplyBindings();
		m_pProfile->Save();
		m_pProfile = LTNULL;
		for (int nType = 0; nType < kNumCommandTypes; nType++)
			m_pList[nType]->Show(LTFALSE);
	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		SetCurrentType(COM_MOVE);

}


// Adds the columns to the controls
void CScreenConfigure::InitControlList()
{

	LTIntPt pos(m_ListRect.left,m_ListRect.top);


	for (int i=0; i < g_kNumCommands; i++)
	{
		const CommandData *pData = GetCommandData(i);

		//check to see that this is a TO2 command
		if (pData->nCommandID < FIRST_TRON_COMMAND)
		{
			// The initial column (contains the action)
			CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(CMD_CHANGE_CONTROL, IDS_HELP_SETCONTROL);
			pCtrl->SetFont(LTNULL,m_nListFontSize);

			// The "action" column
			char szTmp[64];
			FormatString(pData->nStringID,szTmp,sizeof(szTmp));
			pCtrl->AddColumn(szTmp, m_nActionWidth);

			// The equals column.  Changes from "" to "=" when the user is binding the key
			pCtrl->AddColumn(" ", m_nEqualsWidth);

			// The column that contains the key which is assigned to the control!
			FormatString(IDS_CONTROL_UNASSIGNED,szTmp,sizeof(szTmp));
			pCtrl->AddColumn(szTmp, m_nCommandWidth);

			pCtrl->SetParam1(i);

			m_pList[pData->nCommandType]->AddControl(pCtrl);
		}

	}
}


// fill the list of controls with binding data
void CScreenConfigure::UpdateControlList()
{
	for (int nType = 0; nType < kNumCommandTypes; nType++)
	{
		int nHt = m_ListRect.bottom - m_ListRect.top;

		m_pList[nType]->SetHeight(nHt);

		for (int i = 0; i < m_pList[nType]->GetNumControls(); i ++)
		{
			SetControlText(nType,i);
		}
	}

	AdjustControlFrame();

}


// Sets the key/mouse info for a control at a specific index
void CScreenConfigure::SetControlText(int nType, int nIndex)
{

	CLTGUICtrl *pCtrl = m_pList[nType]->GetControl(nIndex+1);
	if (pCtrl) 
	{
		SetControlText(pCtrl);
	}
}
void CScreenConfigure::SetControlText(CLTGUICtrl *pCtrl)
{
	CLTGUIColumnCtrl *pCol = (CLTGUIColumnCtrl *)pCtrl;
	uint32 nCommand = pCtrl->GetParam1();

	CBindingData* pData = m_pProfile->FindBinding(nCommand);

	char strControls[256] = "";
	char szDeviceObjectName[256] = "";
	int numControls = 0;

	if (pData) 
	{
		if (strlen(pData->strTriggerName[0]) != 0 )
		{
			char szTemp[256] = "";

			// Get the name twice and take the shorter version.  WinXP has a problem
			// with occasionally reporting garbage.  Taking the shorter one assumes that
			// the garbage version is full of bad characters.
			g_pLTClient->GetDeviceObjectName( m_pProfile->GetDeviceName( 0 ), pData->nDeviceObjectId[0], 
				szTemp, ARRAY_LEN( szTemp ));
			g_pLTClient->GetDeviceObjectName( m_pProfile->GetDeviceName( 0 ), pData->nDeviceObjectId[0], 
				szDeviceObjectName, ARRAY_LEN( szDeviceObjectName ));

			if( strlen( szTemp ) < strlen( szDeviceObjectName ))
			{
				SAFE_STRCPY(strControls, szTemp );
			}
			else
			{
				SAFE_STRCPY(strControls, szDeviceObjectName );
			}

			++numControls;
		}

		if (strlen(pData->strTriggerName[1]) != 0 )
		{
			if (numControls)
				strcat(strControls,", ");

			if (stricmp("#U",pData->strRealName[1]) == 0)
				strcat(strControls,szWheelUp);
			else if (stricmp("#D",pData->strRealName[1]) == 0)
				strcat(strControls,szWheelDown);
			else if (stricmp("##3",pData->strRealName[1]) == 0)
				strcat(strControls,LoadTempString(IDS_MOUSE_LEFTBUTTON));
			else if (stricmp("##4",pData->strRealName[1]) == 0)
				strcat(strControls,LoadTempString(IDS_MOUSE_RIGHTBUTTON));
			else if (stricmp("##5",pData->strRealName[1]) == 0)
				strcat(strControls,LoadTempString(IDS_MOUSE_MIDDLEBUTTON));
			else
			{
				strcat(strControls,strDeviceNiceName[1]);
				strcat(strControls," ");

				g_pLTClient->GetDeviceObjectName( m_pProfile->GetDeviceName( 1 ), pData->nDeviceObjectId[1], 
					szDeviceObjectName, ARRAY_LEN( szDeviceObjectName ));

				strcat(strControls, szDeviceObjectName );
			}
		}

		if (strlen(pData->strTriggerName[2]) != 0 )
		{
			if (numControls)
				strcat(strControls,", ");
			strcat(strControls,strDeviceNiceName[2]);
			strcat(strControls," ");

			g_pLTClient->GetDeviceObjectName( m_pProfile->GetDeviceName( 2 ), pData->nDeviceObjectId[2], 
				szDeviceObjectName, ARRAY_LEN( szDeviceObjectName ));

			strcat( strControls, szDeviceObjectName );
		}
	}


	// If the key is unassigned, then just set the text to the unassigned message
	if (strlen(strControls) == 0 )
	{
		char szTmp[64];
		FormatString(IDS_CONTROL_UNASSIGNED,szTmp,sizeof(szTmp));
		pCol->SetString(SCREEN_COLUMN_KEY, szTmp);
	}
	else
	{
		// Set the text in the control
		pCol->SetString(SCREEN_COLUMN_KEY, strControls);
	}

}

int CScreenConfigure::GetCommand(int nType, int nIndex)
{
	CLTGUIColumnCtrl *pCtrl = (CLTGUIColumnCtrl *) (m_pList[nType]->GetControl(nIndex+1));
	if (!pCtrl) 
	{
		_ASSERT(0);
		return 0;
	}

	return pCtrl->GetParam1();

}

// Unbinds an action
void CScreenConfigure::UnBind( uint32 nObjectId, char const* pszControlName, uint32 deviceType)
{
	if( !nObjectId && !pszControlName )
	{
		ASSERT( !"CScreenConfigure::UnBind: Invalid inputs." );
		return;
	}

	int dev = 0;
	while (dev < 3 && devices[dev] != deviceType)
		++dev;

	char szTmp[16] ="";
	for (int i = 0; i < g_kNumCommands; i++)
	{
		CBindingData *pData = m_pProfile->FindBinding(i);
		if( nObjectId )
		{
			if( pData->nDeviceObjectId[dev] != nObjectId )
			{
				continue;
			}
		}
		else
		{
			if( stricmp( pData->strRealName[dev], pszControlName ))
				continue;
		}

		strcpy(pData->strRealName[dev],"");
		strcpy(pData->strTriggerName[dev],"");
		pData->nDeviceObjectId[dev] = 0;

	}
}

// Binds a key to a action
void CScreenConfigure::Bind(int nCommand, uint32 nDeviceObjectId, uint16 nControlCode, char *lpszControlName, uint32 deviceType)
{
	_ASSERT(lpszControlName || nControlCode);

	int dev = 0;
	while (dev < 3 && devices[dev] != deviceType)
		++dev;

	CBindingData *pData = m_pProfile->FindBinding(nCommand);
	if (pData)
	{
		pData->nDeviceObjectId[dev] = nDeviceObjectId;

		if (nControlCode && (deviceType != DEVICETYPE_JOYSTICK || deviceType == DEVICETYPE_GAMEPAD))
		{
			sprintf(pData->strRealName[dev],"##%d",nControlCode);
			SAFE_STRCPY(pData->strTriggerName[dev],lpszControlName)
		}
		else
		{
			SAFE_STRCPY(pData->strRealName[dev],lpszControlName)
			SAFE_STRCPY(pData->strTriggerName[dev],lpszControlName)
		}
	}
}

// Handle a keypress
LTBOOL CScreenConfigure::HandleKeyDown(int key, int rep)
{

    LTBOOL handled = LTFALSE;

	switch (key)
	{
	case VK_DELETE:
		{
			// Unassign the key
			if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
			{

				CLTGUICtrl *pCtrl = m_pList[m_nType]->GetSelectedControl();
				int nCommand = pCtrl->GetParam1();
				CBindingData* pData = m_pProfile->FindBinding(nCommand);

				UnBind( pData->nDeviceObjectId[0], NULL, DEVICETYPE_KEYBOARD);
				UnBind( 0, pData->strTriggerName[1], DEVICETYPE_MOUSE);

				if (g_pGameClientShell->HasGamepad())
					UnBind( 0, pData->strTriggerName[2], DEVICETYPE_GAMEPAD);
				else if (g_pGameClientShell->HasJoystick())
					UnBind(0, pData->strTriggerName[2], DEVICETYPE_JOYSTICK);
				else
					UnBind(0, pData->strTriggerName[2], DEVICETYPE_UNKNOWN);

				SetControlText(pCtrl);

				handled = LTTRUE;
			}
			break;
		}
	}
	// Check to see if the base class is handling this key
	if (m_bWaitingForKey || m_fInputPauseTimeLeft)
		return handled;

	if (!handled)
	{
		handled = CBaseScreen::HandleKeyDown(key, rep);
	}

	// Handled the key
	return handled;
}


uint32 CScreenConfigure::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_MOVE_COM:
		SetCurrentType(COM_MOVE);
		break;
	case CMD_INV_COM:
		SetCurrentType(COM_INV);
		break;
	case CMD_VIEW_COM:
		SetCurrentType(COM_VIEW);
		break;
	case CMD_MISC_COM:
		SetCurrentType(COM_MISC);
		break;


	case CMD_CHANGE_CONTROL:
		{

			if (m_bWaitingForKey)
				break;
			// Set the current screen item to ??? while tracking the device.
			int nIndex=m_pList[m_nType]->GetSelectedIndex()-1;
			if (nIndex < 0)
				break;

			CLTGUIColumnCtrl *pCtrl= (CLTGUIColumnCtrl *)m_pList[m_nType]->GetSelectedControl();

			pCtrl->SetString(SCREEN_COLUMN_EQUALS, "=");

			pCtrl->SetString(SCREEN_COLUMN_KEY, " ");



			// see if we can track the input devices
            LTRESULT hResult = g_pLTClient->StartDeviceTrack (DEVICETYPE_KEYBOARD | DEVICETYPE_MOUSE | devices[2], TRACK_BUFFER_SIZE);
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
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


LTBOOL CScreenConfigure::Render(HSURFACE hDestSurf)
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

	return CBaseScreen::Render(hDestSurf);
}


void CScreenConfigure::SetCurrentType(int nType)
{
	if (nType < 0 || nType > kNumCommandTypes)
	{
		nType = kNumCommandTypes;
	}
	for (int n = 0; n < kNumCommandTypes; n++)
	{
		m_pList[n]->Show( (n==nType) );
	}
	m_nType = nType;
	if (nType < kNumCommandTypes)
		SetSelection(GetIndex(m_pList[nType]));
	else
		SetSelection(0);

	AdjustControlFrame();

}

LTBOOL CScreenConfigure::SetCurrentSelection (DeviceInput* pInput)
{
	CLTGUIColumnCtrl *pCtrl=(CLTGUIColumnCtrl *)m_pList[m_nType]->GetSelectedControl();
	int nCommand=pCtrl->GetParam1();
	int nIndex=m_pList[m_nType]->GetSelectedIndex()-1;

	if (pInput->m_DeviceType == DEVICETYPE_JOYSTICK)
	{
		uint16 diCode = pInput->m_ControlCode;
		g_pLTClient->CPrint("joystick diCode:%d",diCode);
	}

	if (pInput->m_DeviceType == DEVICETYPE_MOUSE && pInput->m_ControlType == CONTROLTYPE_ZAXIS)
	{
		return CheckMouseWheel(pInput);
	}

	if (pInput->m_ControlType != CONTROLTYPE_BUTTON &&
		pInput->m_ControlType != CONTROLTYPE_KEY)
        return LTFALSE;



//	char sNewKey[256];
//	SAFE_STRCPY(sNewKey,pInput->m_ControlName);

	// see if this key is bound to something not in the keyboard configuration folder...
	if (KeyRemappable(pInput))
	{
		UnBind( pInput->m_nObjectId, NULL, pInput->m_DeviceType);
		Bind(nCommand, pInput->m_nObjectId, pInput->m_ControlCode, pInput->m_ControlName, pInput->m_DeviceType);

	};

	
	pCtrl->SetString(SCREEN_COLUMN_EQUALS, "");

	UpdateControlList();

    return LTTRUE;
}

LTBOOL CScreenConfigure::KeyRemappable (DeviceInput* pInput)
{

	if (pInput->m_DeviceType != DEVICETYPE_KEYBOARD) return LTTRUE;

	uint16 nDIK = pInput->m_ControlCode;
	if (nDIK == DIK_ESCAPE)
        return LTFALSE;
	if (nDIK == DIK_PAUSE)
        return LTFALSE;
	if (nDIK >= DIK_F1 && nDIK <= DIK_F10)
        return LTFALSE;
	if (nDIK >= DIK_F11 && nDIK <= DIK_F15)
        return LTFALSE;


    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
    if (!pBindings) return LTTRUE;

	// see if this input object is already bound to something...

	DeviceBinding* ptr = pBindings;
	while (ptr)
	{
		if( ptr->m_nObjectId == pInput->m_nObjectId )
		{
			// see if this binding is in the list of mappable ones
			GameAction* pAction = ptr->pActionHead;
			while (pAction)
			{
                LTBOOL bFound = LTFALSE;
				for (int i = 0; i < g_kNumCommands; i++)
				{
					const CommandData *pData = GetCommandData(i);
					if (pData->nCommandID == pAction->nActionCode)
					{
                        bFound = LTTRUE;
						break;
					}
				}

				if (!bFound)
				{
					// this key is already bound to something but is not remappable
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


LTBOOL CScreenConfigure::OnUp()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnUp();
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnDown()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnDown();
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnLeft()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLeft();
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnRight()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRight();
	else
        return LTTRUE;
}
LTBOOL CScreenConfigure::OnEnter()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnEnter();
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnLButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnLButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseScreen::OnLButtonUp(x,y);
}

LTBOOL CScreenConfigure::OnLButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLButtonDblClick(x,y);
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnRButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRButtonDown(x,y);
	else
        return LTTRUE;
}

LTBOOL CScreenConfigure::OnRButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = LTFALSE;
        return LTTRUE;
	}
	else
		return CBaseScreen::OnRButtonUp(x,y);
}

LTBOOL CScreenConfigure::OnRButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRButtonDblClick(x,y);
	else
        return LTTRUE;
}


LTBOOL CScreenConfigure::OnMouseMove(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnMouseMove(x, y);
	else
        return LTTRUE;
}



LTBOOL CScreenConfigure::CheckMouseWheel (DeviceInput* pInput)
{
	
    if (!g_pLTClient) return LTFALSE;

	if (pInput->m_DeviceType != DEVICETYPE_MOUSE || pInput->m_ControlType != CONTROLTYPE_ZAXIS)
		return LTFALSE;

	LTBOOL bWheelUp = ((int)pInput->m_InputValue > 0);
	char szCommand[64];

	CLTGUIColumnCtrl *pCtrl=(CLTGUIColumnCtrl *)m_pList[m_nType]->GetSelectedControl();
	int nCommand=pCtrl->GetParam1();
	uint16 diCode = pInput->m_ControlCode;
	
	if (bWheelUp)
		strcpy(szCommand, "#U");
	else
		strcpy(szCommand, "#D");
	
	UnBind( 0, szCommand, pInput->m_DeviceType );

	Bind(nCommand, pInput->m_nObjectId, 0, szCommand, pInput->m_DeviceType);

	pCtrl->SetString(SCREEN_COLUMN_EQUALS, "");

	UpdateControlList();

    return LTTRUE;
}



void CScreenConfigure::AdjustControlFrame()
{
	if (m_nType >= kNumCommandTypes)
	{
		m_pFrame->Show(LTFALSE);
		return;
	}


	uint16 nWd = m_nActionWidth + m_nEqualsWidth + m_nCommandWidth;
	m_pFrame->Show(LTTRUE);

	m_pList[m_nType]->CalculatePositions();

	LTIntPt listpos = m_pList[m_nType]->GetBasePos();
	uint16 i = m_pList[m_nType]->GetNumControls() - 1;
	CLTGUICtrl *pCtrl = m_pList[m_nType]->GetControl(i);
	if (pCtrl)
	{
		LTIntPt pos = pCtrl->GetBasePos();
		
		uint16 nHt = (pos.y - listpos.y) + pCtrl->GetBaseHeight() + 4;

		m_pFrame->SetSize(nWd+8,nHt);
		m_pList[m_nType]->SetHeight(nHt);
	}
}
