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

// The key binding configuration screen is not necessary in the Xenon build
#if !defined(PLATFORM_XENON)

#include "ScreenConfigure.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "CommandIDs.h"
#include "HUDSwap.h"
#include "HUDWeaponList.h"
#include "sys/win/mpstrconv.h"
#include "iltinput.h"
#include "dinput.h" // For the DIK_* codes
  
#include <algorithm>

// The different columns
#define SCREEN_COLUMN_ACTION		0
#define SCREEN_COLUMN_EQUALS		1
#define SCREEN_COLUMN_KEY			2

namespace
{
    bool   bEatMouseButtonUp = false;
	void ConfirmCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenConfigure *pThisScreen = (CScreenConfigure *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_DELETE);
	}
}

static ILTInput *g_pLTInput = NULL;
define_holder(ILTInput, g_pLTInput);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenConfigure::CScreenConfigure()
{
    m_bWaitingForKey = false;
	m_fInputPauseTimeLeft = 0.0f;
	m_nType = 0;
	m_pProfile = NULL;

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

	CBaseScreen::Escape();
}

bool   CScreenConfigure::Init(int nScreenID)
{
	return CBaseScreen::Init(nScreenID);
}

// Build the screen
bool CScreenConfigure::Build()
{

	CreateTitle("IDS_TITLE_CONFIGURE");

	m_nActionWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	m_nEqualsWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);
	m_nCommandWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,2);
	m_ListRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	m_nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

	LTVector2n topPos(m_ScreenRect.Left(),m_ListRect.Top());
	LTVector2n sz(m_ListRect.Left() - topPos.x,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin = topPos;
	cs.rnBaseRect.m_vMax = topPos + sz;

	cs.nCommandID = CMD_MOVE_COM;
	cs.szHelpID = "";
	CLTGUITextCtrl *pCtrl = AddTextItem("IDS_MOVE_CONTROLS",cs);

	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = sz;
	cs.nCommandID = CMD_INV_COM;
	pCtrl = AddTextItem("IDS_INV_CONTROLS",cs);

	cs.nCommandID = CMD_WEAP_COM;
	pCtrl = AddTextItem("IDS_WEAP_CONTROLS",cs);

	cs.nCommandID = CMD_MISC_COM;
	pCtrl = AddTextItem("IDS_MISC_CONTROLS",cs);


	cs.nCommandID = 0;
	cs.rnBaseRect = m_ListRect;

	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	LTVector2n vIndent = g_pLayoutDB->GetListIndent(m_hLayout,0);
	uint8 nExpand = g_pLayoutDB->GetListFrameExpand(m_hLayout,0); 

	CLTGUIListCtrl_create lcs;
	lcs.bArrows = false;
	lcs.rnBaseRect = m_ListRect;
	lcs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	for (int nType = 0; nType < kNumCommandTypes; nType++)
	{
		m_pList[nType] = AddList(lcs);
		m_pList[nType]->SetFrame(hFrame,NULL,nExpand);
		m_pList[nType]->SetIndent(vIndent);
		m_pList[nType]->SetItemSpacing(1);
		m_pList[nType]->Show(false);

		pCtrl = NULL;
		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(m_nActionWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

		switch (nType)
		{
		case COM_MOVE:
			pCtrl = CreateTextItem("IDS_MOVE_CONTROLS",cs,true);
			break;
		case COM_INV:
			pCtrl = CreateTextItem("IDS_INV_CONTROLS",cs,true);
			break;
		case COM_MISC:
			pCtrl = CreateTextItem("IDS_MISC_CONTROLS",cs,true);
			break;
		case COM_WEAP:
			pCtrl = CreateTextItem("IDS_WEAP_CONTROLS",cs,true);
			break;
		}

		if (pCtrl)
			m_pList[nType]->AddControl(pCtrl);

	}

	InitControlList();

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
}

void CScreenConfigure::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		// Note : Apply/set for the profile is opposite from usual, because
		// this screen operates directly on the bindings rather than the profile
		m_pProfile->ApplyBindings();
		UpdateControlList();
	}
	else
	{
		m_pProfile->SetBindings();
		m_pProfile->Save();
		m_pProfile = NULL;
		for (int nType = 0; nType < kNumCommandTypes; nType++)
			m_pList[nType]->Show(false);

		//certain HUD elements report hotkeys, so they might need to get updated here
		if (g_pHUDMgr)
		{
			CHUDSwap::UpdateTriggerName();
			if( g_pHUDWeaponList )
				g_pHUDWeaponList->UpdateTriggerNames();
			if( g_pHUDGrenadeList )
				g_pHUDGrenadeList->UpdateTriggerNames();
		}

	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		SetCurrentType(COM_MOVE);

}


// Adds the columns to the controls
void CScreenConfigure::InitControlList()
{

	LTVector2n pos(m_ListRect.Left(),m_ListRect.Top());


	for (int i=0; i < GetNumCommands(); i++)
	{
		const CommandData *pData = GetCommandData(i);

		// Skip over axis data.  That's another screen.
		if (pData->m_bAxis)
			continue;

		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(m_ListRect.GetWidth(),m_nListFontSize);

		cs.nCommandID = CMD_CHANGE_CONTROL;
		cs.szHelpID = "IDS_HELP_SETCONTROL";
		cs.pCommandHandler = this;

		// The initial column (contains the action)
		CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(cs,false,"",m_nListFontSize);

		// The "action" column
		pCtrl->AddColumn(LoadString(pData->m_szStringID), m_nActionWidth, true);

		// The equals column.  Changes from "" to "=" when the user is binding the key
		pCtrl->AddColumn(L" ", m_nEqualsWidth, true);

		// The column that contains the key which is assigned to the control!
		pCtrl->AddColumn(LoadString("IDS_CONTROL_UNASSIGNED"), m_nCommandWidth, true);

		pCtrl->SetParam1(i);

		m_pList[pData->m_nCommandType]->AddControl(pCtrl);
			

	}

	//add quicksave && quickload
	{
		//add quicksave
		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(m_ListRect.GetWidth(),m_nListFontSize);

		cs.nCommandID = CMD_CHANGE_CONTROL;
		cs.szHelpID = "IDS_HELP_SETCONTROL";
		cs.pCommandHandler = this;

		// The initial column (contains the action)
		CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(cs,false,"",m_nListFontSize);

		// The "action" column
		pCtrl->AddColumn(LoadString("IDS_CONTROL_QUICKSAVE"), m_nActionWidth, true);

		// The equals column.  Changes from "" to "=" when the user is binding the key
		pCtrl->AddColumn(L" ", m_nEqualsWidth, true);

		// The column that contains the key which is assigned to the control!
		pCtrl->AddColumn(LoadString("IDS_CONTROL_QUICKSAVE_KEY"), m_nCommandWidth, true);

		pCtrl->SetParam1(COMMAND_ID_UNASSIGNED);

		pCtrl->Enable(false);

		m_pList[COM_MISC]->AddControl(pCtrl);


		//quickload
		// The initial column (contains the action)
		pCtrl=CreateColumnCtrl(cs,false,"",m_nListFontSize);

		// The "action" column
		pCtrl->AddColumn(LoadString("IDS_CONTROL_QUICKLOAD"), m_nActionWidth, true);

		// The equals column.  Changes from "" to "=" when the user is binding the key
		pCtrl->AddColumn(L" ", m_nEqualsWidth, true);

		// The column that contains the key which is assigned to the control!
		pCtrl->AddColumn(LoadString("IDS_CONTROL_QUICKLOAD_KEY"), m_nCommandWidth, true);

		pCtrl->SetParam1(COMMAND_ID_UNASSIGNED);

		pCtrl->Enable(false);

		m_pList[COM_MISC]->AddControl(pCtrl);
	}

}


// fill the list of controls with binding data
void CScreenConfigure::UpdateControlList()
{
	for (int nType = 0; nType < kNumCommandTypes; nType++)
	{
/*
		int nHt = m_ListRect.GetHeight();

		m_pList[nType]->SetHeight(nHt);
*/
		for (uint16 i = 0; i < m_pList[nType]->GetNumControls(); i++)
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
	if (nCommand == COMMAND_ID_UNASSIGNED)
	{
		return;
	}

	// Get the current binding list
	CBindMgr::TBindingList aBindings;
	CBindMgr::GetSingleton().GetCommandBindings(GetCommandData(nCommand)->m_nCommandID, &aBindings);
  
	// Set the text to "unassigned" if no bindings are set
	if (aBindings.empty())
	{
		pCol->SetString(SCREEN_COLUMN_KEY, LoadString("IDS_CONTROL_UNASSIGNED"));
		return;
	}
  
	std::wstring strControls;
  
	// Add the bindings up into a single string
	for (CBindMgr::TBindingList::const_iterator iCurBinding = aBindings.begin();
		iCurBinding != aBindings.end();
		++iCurBinding)
  	{
		strControls += GetControlName(*iCurBinding);
 
		// Add a separator if it's not the last control
		if ((iCurBinding + 1) != aBindings.end())
  		{
			strControls += L", ";
  		}
  	}
  
	// Set the text in the control
	pCol->SetString(SCREEN_COLUMN_KEY, strControls.c_str());
}

int CScreenConfigure::GetCommand(int nType, int nIndex)
{
	CLTGUIColumnCtrl *pCtrl = (CLTGUIColumnCtrl *) (m_pList[nType]->GetControl(nIndex+1));
	if (!pCtrl) 
	{
		LTERROR( "" );
		return 0;
	}

	return pCtrl->GetParam1();

}

// Handle a keypress
bool CScreenConfigure::HandleKeyDown(int key, int rep)
{

    bool handled = false;

	switch (key)
	{
	case VK_DELETE:
		{
			// Ignore if they haven't selected anything yet.
			if( GetSelection( ) == kNoSelection )
				break;

			// Unassign the key
			if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
			{

				CLTGUICtrl *pCtrl = m_pList[m_nType]->GetSelectedControl();
				if( !pCtrl )
					break;

				int nCommand = pCtrl->GetParam1();
				if (nCommand == COMMAND_ID_UNASSIGNED)
					break;

 				CBindMgr::GetSingleton().ClearCommandBindings(GetCommandData(nCommand)->m_nCommandID);

				SetControlText(pCtrl);

				handled = true;
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
	case CMD_MISC_COM:
		SetCurrentType(COM_MISC);
		break;
	case CMD_WEAP_COM:
		SetCurrentType(COM_WEAP);
		break;

	case CMD_CONFIRM:
		{
			SetCurrentSelection(m_nConfirmDevice,m_nConfirmObject,true);
		} break;

	case CMD_CHANGE_CONTROL:
		{

			if (m_bWaitingForKey)
				break;
			// Set the current screen item to ??? while tracking the device.
			int nIndex=m_pList[m_nType]->GetSelectedIndex()-1;
			if (nIndex < 0)
				break;

			CLTGUIColumnCtrl *pCtrl= (CLTGUIColumnCtrl *)m_pList[m_nType]->GetSelectedControl();

			pCtrl->SetString(SCREEN_COLUMN_EQUALS, L"=");

			pCtrl->SetString(SCREEN_COLUMN_KEY, L" ");

			// Wait for a key
			StartWaitForKeypress();

			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


bool CScreenConfigure::Render()
{
	//no actual rendering here... just wait for keypress

	// see if we are pausing input
	if (m_fInputPauseTimeLeft)
	{
		m_fInputPauseTimeLeft -= RealTimeTimer::Instance().GetTimerElapsedS();
		if (m_fInputPauseTimeLeft < 0.0f) m_fInputPauseTimeLeft = 0.0f;
	}

	if (m_bWaitingForKey && !m_fInputPauseTimeLeft)
  	{
		UpdateWaitForKeypress();
  	}

	return CBaseScreen::Render();
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
	{
		AdjustControlFrame();
		SetSelection(GetIndex(m_pList[nType]));
	}
	else
	{
		SetSelection(0);
	}

	

}

bool CScreenConfigure::SetCurrentSelection(uint32 nDevice, uint32 nObject, bool bForce)
{
	CLTGUIColumnCtrl *pCtrl=(CLTGUIColumnCtrl *)m_pList[m_nType]->GetSelectedControl();
	int nCommand=pCtrl->GetParam1();
	if (nCommand == COMMAND_ID_UNASSIGNED)
	{
		return false;
	}


	//see if we've already bound something to this device
	if (!bForce)
	{
		uint32 nCurBindingIndex = 0;
		bool bFound = false;
		do {
			CBindMgr::SBinding sCurrentBinding;
			bFound = CBindMgr::GetSingleton().GetDeviceBinding(nDevice,nObject, nCurBindingIndex, &sCurrentBinding);
			if (!bFound)
				break;
			++nCurBindingIndex;

			//check for mouse wheel
			bool bWheelMatch = true;
			ILTInput::SDeviceObjectDesc sObject;
			g_pLTInput->GetDeviceObjectDesc(nDevice, nObject, &sObject);
			ILTInput::SDeviceDesc sDevice;
			g_pLTInput->GetDeviceDesc(nDevice, &sDevice);
			float fCurValue;
			g_pLTInput->GetDeviceValues(nDevice, nObject, 1, &fCurValue);

			if (sDevice.m_eCategory == ILTInput::eDC_Mouse)
			{
				if (sObject.m_eCategory == ILTInput::eDOC_Axis)
				{
					// Set the command value to the range [0.5 + half the object's resolution .. FLT_MAX],
					// or the negative version of the range if the axis moved in a negative direction.
					bool bPositive = (fCurValue > 0.5f);

					float fHalfResolution = sObject.m_fResolution / 2.0f;
					if (bPositive)
					{
						bWheelMatch = (sCurrentBinding.m_fCommandMin >= 0);
					}
					else
					{
						bWheelMatch = (sCurrentBinding.m_fCommandMin < 0);
					}
				}
			}

			if (sCurrentBinding.m_nCommand <= COMMAND_ID_UNASSIGNED && sCurrentBinding.m_nCommand != GetCommandData(nCommand)->m_nCommandID && bWheelMatch) 
			{

				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = ConfirmCallBack,
				mb.pData = NULL;
				mb.pUserData = this;

				wchar_t	wsMsg[256];
				FormatString("SCREENCONFIGURE_CONFIRM",wsMsg,LTARRAYSIZE(wsMsg),
					GetControlName(sCurrentBinding),
					LoadString(GetCommandStringID(sCurrentBinding.m_nCommand))
					);
				g_pInterfaceMgr->ShowMessageBox(wsMsg,&mb);

				m_nConfirmDevice = nDevice;
				m_nConfirmObject = nObject;
				m_fConfirmValue = fCurValue;

				pCtrl->SetString(SCREEN_COLUMN_EQUALS, L" ");
				UpdateControlList();

				bEatMouseButtonUp = false; //it will get eaten by dialog box
				return true;
			}
		} while (bFound);
	}


	// Double-check that we're binding a button..
	ILTInput::SDeviceObjectDesc sObject;
	g_pLTInput->GetDeviceObjectDesc(nDevice, nObject, &sObject);
	if (sObject.m_eCategory == ILTInput::eDOC_Button)
	{
		// Make sure we're binding something we're allowed to re-bind
		if (KeyRemappable(nDevice, nObject))
  		{

			//if there is more than one thing already bound, clear the oldest one
			CBindMgr::TBindingList aBindings;
			CBindMgr::GetSingleton().GetCommandBindings(GetCommandData(nCommand)->m_nCommandID, &aBindings);
			if (aBindings.size() > 1)
			{
				CBindMgr::SBinding sOldBinding = aBindings[1];
				CBindMgr::GetSingleton().ClearCommandBindings(GetCommandData(nCommand)->m_nCommandID);
				CBindMgr::GetSingleton().SetBinding(sOldBinding);

			}


			// Set up the binding for it
			CBindMgr::SBinding sBinding;

			sBinding.m_nCommand = GetCommandData(nCommand)->m_nCommandID;
			sBinding.m_nDevice = nDevice;
			sBinding.m_nObject = nObject;
			CBindMgr::GetSingleton().SetBinding(sBinding);

			ILTInput::SDeviceDesc sDevice;
			g_pLTInput->GetDeviceDesc(nDevice, &sDevice);
  		}
	}
	else
	{
  		// Check for a mouse wheel
		ILTInput::SDeviceDesc sDevice;
		g_pLTInput->GetDeviceDesc(nDevice, &sDevice);
		if (sDevice.m_eCategory == ILTInput::eDC_Mouse)
		{
			if (sObject.m_eCategory == ILTInput::eDOC_Axis)
			{
				// Set up the binding for it
				CBindMgr::SBinding sBinding;

				sBinding.m_nCommand = GetCommandData(nCommand)->m_nCommandID;
				sBinding.m_nDevice = nDevice;
				sBinding.m_nObject = nObject;
				// Get the axis offset
				float fCurValue;
				if (bForce)
				{
					fCurValue = m_fConfirmValue;
				}
				else
				{
					if (g_pLTInput->GetDeviceValues(nDevice, nObject, 1, &fCurValue) != LT_OK)
						return false;
				}
				// Set the command value to the range [0.5 + half the object's resolution .. FLT_MAX],
				// or the negative version of the range if the axis moved in a negative direction.
				bool bPositive = (fCurValue > 0.5f);
				float fHalfResolution = sObject.m_fResolution / 2.0f;
				if (bPositive)
				{
					sBinding.m_fCommandMin = 0.5f + fHalfResolution;
					sBinding.m_fCommandMax = FLT_MAX;
				}
				else
				{
					sBinding.m_fCommandMin = -FLT_MAX;
					sBinding.m_fCommandMax = 0.5f - fHalfResolution;
				}
				sBinding.m_fDefaultValue = 0.5f;

				// Save the binding
				CBindMgr::GetSingleton().SetBinding(sBinding);
			}
			else
				return false;
		}
		// Check for a POV
		else if (sDevice.m_eCategory == ILTInput::eDC_Gamepad)
		{
			if (sObject.m_eCategory == ILTInput::eDOC_POV)
			{
				// Set up the binding for it
				CBindMgr::SBinding sBinding;

				sBinding.m_nCommand = GetCommandData(nCommand)->m_nCommandID;
				sBinding.m_nDevice = nDevice;
				sBinding.m_nObject = nObject;
				// Get the POV value
				float fCurValue;
				if (g_pLTInput->GetDeviceValues(nDevice, nObject, 1, &fCurValue) != LT_OK)
					return false;
				// Set the command value to the current value +/- half the object's resolution
				float fHalfResolution = sObject.m_fResolution / 2.0f;
				sBinding.m_fCommandMin = fCurValue - fHalfResolution;
				sBinding.m_fCommandMax = fCurValue + fHalfResolution;
				
				
				
				// Save the binding
				CBindMgr::GetSingleton().SetBinding(sBinding);
			}
			else
				return false;
		}
		else
			return false;
	}
  	
	pCtrl->SetString(SCREEN_COLUMN_EQUALS, L" ");

	UpdateControlList();

    return true;
}

bool CScreenConfigure::KeyRemappable (uint32 nDevice, uint32 nObject)
{
	ILTInput::SDeviceDesc sDevice;
	g_pLTInput->GetDeviceDesc(nDevice, &sDevice);
  
	// Anything goes on non-keyboard devices
	if (sDevice.m_eCategory != ILTInput::eDC_Keyboard)
		return true;

	ILTInput::SDeviceObjectDesc sObject;
	g_pLTInput->GetDeviceObjectDesc(nDevice, nObject, &sObject);
  
	// Check the control code
	uint32 nDIK = sObject.m_nControlCode;
  	if (nDIK == DIK_ESCAPE)
		return false;
  	if (nDIK == DIK_PAUSE)
		return false;
	if (nDIK >= DIK_F1 && nDIK <= DIK_F10)
        return false;
	if (nDIK >= DIK_F11 && nDIK <= DIK_F15)
        return false;

	// Anything else should be fine...
    return true;
}


bool CScreenConfigure::OnUp()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnUp();
	else
        return true;
}

bool CScreenConfigure::OnDown()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnDown();
	else
        return true;
}

bool CScreenConfigure::OnLeft()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLeft();
	else
        return true;
}

bool CScreenConfigure::OnRight()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRight();
	else
        return true;
}
bool CScreenConfigure::OnEnter()
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnEnter();
	else
        return true;
}

bool CScreenConfigure::OnLButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLButtonDown(x,y);
	else
	{
		// Since we captured the mouse down, be sure
		// to capture the mouse up as well.
		bEatMouseButtonUp = true;
        return true;
	}
}

bool CScreenConfigure::OnLButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = false;
        return true;
	}
	else
		return CBaseScreen::OnLButtonUp(x,y);
}

bool CScreenConfigure::OnLButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnLButtonDblClick(x,y);
	else
	{
		// Since we captured the mouse down, be sure
		// to capture the mouse up as well.
		bEatMouseButtonUp = true;
        return true;
	}
}

bool CScreenConfigure::OnRButtonDown(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRButtonDown(x,y);
	else
	{
		// Since we captured the mouse down, be sure
		// to capture the mouse up as well.
		bEatMouseButtonUp = true;
        return true;
	}
}

bool CScreenConfigure::OnRButtonUp(int x, int y)
{
	if (bEatMouseButtonUp)
	{
        bEatMouseButtonUp = false;
        return true;
	}
	else
		return CBaseScreen::OnRButtonUp(x,y);
}

bool CScreenConfigure::OnRButtonDblClick(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnRButtonDblClick(x,y);
	else
	{
		// Since we captured the mouse down, be sure
		// to capture the mouse up as well.
		bEatMouseButtonUp = true;
        return true;
	}
}

bool CScreenConfigure::OnMouseMove(int x, int y)
{
	if (!m_bWaitingForKey && !m_fInputPauseTimeLeft)
		return CBaseScreen::OnMouseMove(x, y);
	else
        return true;
}

void CScreenConfigure::AdjustControlFrame()
{
	if (m_nType >= kNumCommandTypes) 
	{
		return;
	}
	m_pList[m_nType]->CalculatePositions();

	LTVector2n listpos = m_pList[m_nType]->GetBasePos();
	uint16 i = m_pList[m_nType]->GetNumControls() - 1;
	CLTGUICtrl *pCtrl = m_pList[m_nType]->GetControl(i);
	if (pCtrl)
	{
		LTVector2n pos = pCtrl->GetBasePos();
		LTVector2n sz( m_ListRect.GetWidth(), (pos.y - listpos.y) + pCtrl->GetBaseHeight() + 4);

		m_pList[m_nType]->SetSize(sz);

	}
}

void CScreenConfigure::StartWaitForKeypress()
{
	m_bWaitingForKey = true;

	// Remember to get the masked input on the first update

	// Note : The delay is necessary because the event that spawned this process may have happened
	// out of sequence with the input update, meaning the input state still thinks the active button
	// is "up" instead of "down".
	m_bFirstKeyUpdate = true;

	m_fInputPauseTimeLeft = 0.05f;

	CBindMgr::GetSingleton().SetCallbacks(false);
}

void CScreenConfigure::EndWaitForKeypress()
{
	m_bWaitingForKey = false;
	m_aMaskedInput.clear();

	CBindMgr::GetSingleton().SetCallbacks(true);
}

void CScreenConfigure::UpdateWaitForKeypress()
{
	// Get the list of buttons that are already down
	if (m_bFirstKeyUpdate)
	{
		GetButtonsDown(&m_aMaskedInput);
		m_bFirstKeyUpdate = false;
	}

	ASSERT(m_bWaitingForKey);

	TDeviceObjectList aButtonsDown;
	GetButtonsDown(&aButtonsDown);

	// Clear masked input for buttons that went back up
	for (TDeviceObjectList::iterator iCurButton = m_aMaskedInput.begin(); iCurButton != m_aMaskedInput.end();)
	{
		if (std::find(aButtonsDown.begin(), aButtonsDown.end(), *iCurButton) == aButtonsDown.end())
		{
			std::swap(*iCurButton, m_aMaskedInput.back());
			m_aMaskedInput.pop_back();
		}
		else
			++iCurButton;
	}

	// Remove masked input from the list
	for (TDeviceObjectList::iterator iCurButton = aButtonsDown.begin(); iCurButton != aButtonsDown.end();)
	{
		if (std::find(m_aMaskedInput.begin(), m_aMaskedInput.end(), *iCurButton) != m_aMaskedInput.end())
		{
			std::swap(*iCurButton, aButtonsDown.back());
			aButtonsDown.pop_back();
		}
		else
			++iCurButton;
	}

	// Check for candidate keys
	
	for (TDeviceObjectList::iterator iCurButton = aButtonsDown.begin(); iCurButton != aButtonsDown.end(); ++iCurButton)
	{
		if (!SetCurrentSelection(iCurButton->first, iCurButton->second,false))
			continue;
		// We found one, so stop waiting for a keypress
		EndWaitForKeypress();
		m_fInputPauseTimeLeft = 0.2f;
		break;
	}
}

// Special-case function for detecting mouse wheel movement
static bool IsMouseWheelMoving(uint32 *pMouseDevice, uint32 *pMouseWheelObject)
{
	// First axis on a mouse that is considered "the wheel"
	const uint32 nMouseWheelAxis = 2;

	// Get the mouse device
	uint32 nMouseDevice;
	g_pLTInput->FindFirstDeviceByCategory(ILTInput::eDC_Mouse, &nMouseDevice);
	// You do have a mouse, right?
	if (nMouseDevice == ILTInput::k_InvalidIndex)
		return false;
	// Find the wheel(s)
	uint32 nFirstAxis;
	if (g_pLTInput->FindFirstDeviceObjectByCategory(nMouseDevice, ILTInput::eDOC_Axis, &nFirstAxis) != LT_OK)
		return false;
	uint32 nNumAxes;
	if (g_pLTInput->GetNumDeviceObjectsByCategory(nMouseDevice, ILTInput::eDOC_Axis, &nNumAxes) != LT_OK)
		return false;
	// Do you have wheels?
	if (nNumAxes <= nMouseWheelAxis)
		return false;
	// Look for the first active one
	uint32 nEndAxis = nFirstAxis + nNumAxes;
	for (uint32 nCurAxis = nFirstAxis + nMouseWheelAxis; nCurAxis != nEndAxis; ++nCurAxis)
	{
		// Find out where it's sitting
		float fCurValue;
		if (g_pLTInput->GetDeviceValues(nMouseDevice, nCurAxis, 1, &fCurValue) != LT_OK)
			continue;
		// If it's currently at EXACTLY 0.5, we know it's not moving.
		if (fCurValue == 0.5f)
			continue;
		// Query the resolution of this object
		ILTInput::SDeviceObjectDesc sObjectDesc;
		if (g_pLTInput->GetDeviceObjectDesc(nMouseDevice, nCurAxis, &sObjectDesc) != LT_OK)
		{
			// Fall back to checking against a reasonable amount of movement
			sObjectDesc.m_fResolution = 0.1f;
		}
		// If it moves more than half the resolution value from the center point, consider it movement
		if (fabsf(fCurValue - 0.5f) > (sObjectDesc.m_fResolution / 2.0f))
		{
			*pMouseDevice = nMouseDevice;
			*pMouseWheelObject = nCurAxis;
			return true;
		}
	}
	// No active wheels found
	return false;
}

// Special-case function for detecting gamepad POV movement
static bool IsPOVDown(uint32 nDevice, uint32 *pPOVObject)
{
	// Find a POV
	uint32 nFirstPOV;
	if (g_pLTInput->FindFirstDeviceObjectByCategory(nDevice, ILTInput::eDOC_POV, &nFirstPOV) != LT_OK)
		return false;
	// How many POVs do you have?
	uint32 nNumPOVs;
	if (g_pLTInput->GetNumDeviceObjectsByCategory(nDevice, ILTInput::eDOC_POV, &nNumPOVs) != LT_OK)
		return false;
	uint32 nEndPOV = nFirstPOV + nNumPOVs;
	for (uint32 nCurPOV = nFirstPOV; nCurPOV != nEndPOV; ++nCurPOV)
	{
		// Where is this POV currently pointing?
		float fCurValue;
		if (g_pLTInput->GetDeviceValues(nDevice, nCurPOV, 1, &fCurValue) != LT_OK)
			continue;
		// Check for activity
		// Note : POVs return [0,0.5-1.0], so we know we can check for non-zero for activity.
		if (fCurValue != 0.0f)
		{
			*pPOVObject = nCurPOV;
			return true;
		}
	}

	return false;
}

void CScreenConfigure::GetButtonsDown(TDeviceObjectList *pResults)
{
	// Amount of variance from 0 before a "button" is considered "down"
	const float k_fActivityEpsilon = 0.1f;

	pResults->resize(0);

	TfloatList aButtonBuffer;

	// Special-case mouse wheel movement detection
	uint32 nMouseDevice, nMouseWheelObject;
	if (IsMouseWheelMoving(&nMouseDevice, &nMouseWheelObject))
	{
		pResults->push_back(TDeviceObjectList::value_type(nMouseDevice, nMouseWheelObject));
	}

	uint32 nNumDevices;
	g_pLTInput->GetNumDevices(&nNumDevices);
	for (uint32 nCurDevice = 0; nCurDevice < nNumDevices; ++nCurDevice)
	{
		// Special-case POV movement detection
		// Does this device have a POV?
		uint32 nFirstPOV;
		if (g_pLTInput->FindFirstDeviceObjectByCategory(nCurDevice, ILTInput::eDOC_POV, &nFirstPOV) == LT_OK)
		{
			// Find out if any of the POVs on this device are down
			uint32 nPOVObject;
			if (IsPOVDown(nCurDevice, &nPOVObject))
			{
				// Add to the results
				pResults->push_back(TDeviceObjectList::value_type(nCurDevice, nPOVObject));
			}
		}

		// Look for a button
		uint32 nFirstButton;
		if (g_pLTInput->FindFirstDeviceObjectByCategory(nCurDevice, ILTInput::eDOC_Button, &nFirstButton) != LT_OK)
			continue;
		// How many buttons do we have?
		uint32 nNumButtons;
		if (g_pLTInput->GetNumDeviceObjectsByCategory(nCurDevice, ILTInput::eDOC_Button, &nNumButtons) != LT_OK)
			continue;
		// Query them all at once
		aButtonBuffer.resize(nNumButtons);
		if (g_pLTInput->GetDeviceValues(nCurDevice, nFirstButton, nNumButtons, &aButtonBuffer[0]) != LT_OK)
			continue;
		// Check for activity
		for (TfloatList::const_iterator iCurButton = aButtonBuffer.begin(); iCurButton != aButtonBuffer.end(); ++iCurButton)
		{
			if (*iCurButton > k_fActivityEpsilon)
				pResults->push_back(TDeviceObjectList::value_type(nCurDevice, (iCurButton - aButtonBuffer.begin()) + nFirstButton));
		}
	}
}

const wchar_t* CScreenConfigure::GetControlName(const CBindMgr::SBinding& sCurrentBinding)
{
	static std::wstring strControls;
	strControls.clear();

	// Get the device & object info for the given binding
	ILTInput::SDeviceDesc sDevice;
	if (g_pLTInput->GetDeviceDesc(sCurrentBinding.m_nDevice, &sDevice) != LT_OK)
	{
		LTERROR( "Invalid device binding encountered");
		return L"";
	}

	ILTInput::SDeviceObjectDesc sObject;
	if (g_pLTInput->GetDeviceObjectDesc(sCurrentBinding.m_nDevice, sCurrentBinding.m_nObject, &sObject) != LT_OK)
	{
		LTERROR( "Invalid object binding encountered");
		return L"";
	}

	// Don't add the device name for keyboards
	if (sDevice.m_eCategory == ILTInput::eDC_Gamepad)
	{
		strControls += LoadString("IDS_DEVICE_JOYSTICK");
		strControls += L" ";

		// Override the display name on axis.  DirectInput doesn't properly translate joystick objects.
		if (sObject.m_eCategory == ILTInput::eDOC_Axis)
		{
			if( LTStrEquals( sObject.m_sName, L"X Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_XAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Y Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_YAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Z Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_ZAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Slider"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_SLIDER"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"POV"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_POV"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Rx Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_RXAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Ry Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_RYAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
			else if( LTStrEquals( sObject.m_sName, L"Rz Axis"))
				LTStrCpy(sObject.m_sDisplayName, LoadString("IDS_JOYSTICK_RZAXIS"), LTARRAYSIZE(sObject.m_sDisplayName) );
		}
		// Override display name on buttons.
		else if( sObject.m_eCategory == ILTInput::eDOC_Button )
		{
			uint32 nButton = 0;
			swscanf( sObject.m_sName, L"Button %d", &nButton );
			FormatString( "IDS_JOYSTICK_BUTTONNUM", sObject.m_sDisplayName, LTARRAYSIZE( sObject.m_sDisplayName ), nButton );
		}

	}
	else if (sDevice.m_eCategory == ILTInput::eDC_Mouse)
	{
		strControls += LoadString("IDS_DEVICE_MOUSE");
		strControls += L" ";

		// Override the display name on mouse wheels
		if (sObject.m_eCategory == ILTInput::eDOC_Axis)
		{
			const char* szStringID;
			if (sCurrentBinding.m_fCommandMin < 0.0f)
				szStringID = "IDS_WHEEL_DOWN";
			else
				szStringID = "IDS_WHEEL_UP";
			LTStrCpy(sObject.m_sDisplayName, LoadString(szStringID), LTARRAYSIZE(sObject.m_sDisplayName) );
		}
		// Override display name on mouse buttons.
		else if( sObject.m_eCategory == ILTInput::eDOC_Button )
		{
			uint32 nButton = 0;
			swscanf( sObject.m_sName, L"Button %d", &nButton );
			FormatString( "IDS_MOUSE_BUTTONNUM", sObject.m_sDisplayName, LTARRAYSIZE( sObject.m_sDisplayName ), nButton );
		}
	}
	// Add the orientation on POVs
	if (sObject.m_eCategory == ILTInput::eDOC_POV)
	{
		// Find the angle to the nearest 45 degrees
		float fRange = (sCurrentBinding.m_fCommandMax - sCurrentBinding.m_fCommandMin);
		float fMiddle = sCurrentBinding.m_fCommandMin + (fRange * 0.5f);
		uint32 nAngle = (uint32)(((fMiddle - 0.5f) * 16.0f) + 0.5f) * 45;
		wchar_t aNumBuffer[20];
		LTSNPrintF(aNumBuffer, LTARRAYSIZE(aNumBuffer), L" (%d)", nAngle);
		LTStrCat(sObject.m_sDisplayName, aNumBuffer, LTARRAYSIZE(sObject.m_sDisplayName));
	}
	// Add the object name
	strControls += sObject.m_sDisplayName;


	return strControls.c_str();
}




#endif // !PLATFORM_XENON
