// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoystick.cpp
//
// PURPOSE : Interface screen for joystick/gamepad settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoystick.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "BaseScreen.h"
#include "BindMgr.h"
#include "iltinput.h"
#include "CommandIDs.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;

	struct SAxisData
	{
		const char* m_szStringID;
		uint32 m_nCommandID;
		float m_fScale;
	};

	SAxisData g_aAxisData[] = 
	{
		{ "IDS_JOYSTICK_AXISNONE", COMMAND_ID_UNASSIGNED, 0.0f },
		{ "IDS_JOYSTICK_TURN", COMMAND_ID_YAW_ACCEL, 1.0f },
		{ "IDS_JOYSTICK_LOOK", COMMAND_ID_PITCH_ACCEL, 1.0f },
		{ "IDS_JOYSTICK_INVERT", COMMAND_ID_PITCH_ACCEL, -1.0f },
		{ "IDS_JOYSTICK_STRAFE", COMMAND_ID_STRAFE_AXIS, 1.0f },
		{ "IDS_JOYSTICK_MOVE", COMMAND_ID_FORWARD_AXIS, -1.0f },
		{ "IDS_CONTROL_FIRE", COMMAND_ID_FIRING, 1.0f },
#ifdef PROJECT_DARK
		{ "IDS_CONTROL_FOCUS", COMMAND_ID_FOCUS, 1.0f },
#endif
		{ "IDS_CONTROL_THROWGRENADE", COMMAND_ID_THROW_GRENADE, 1.0f },
	};

	uint32 g_nNumAxisData = LTARRAYSIZE(g_aAxisData);

	// Find the axis data for a command
	SAxisData *FindAxisData(uint32 nCommand, float fScale)
	{
		uint32 nClosestData = 0;
		float fClosestScale = FLT_MAX;

		for (uint32 nAxis = 0; nAxis < g_nNumAxisData; ++nAxis)
		{
			float fScaleDiff = fabsf(g_aAxisData[nAxis].m_fScale - fScale);
			if ((g_aAxisData[nAxis].m_nCommandID == nCommand) && (fScaleDiff < fClosestScale))
			{
				nClosestData = nAxis;
				fClosestScale = fScaleDiff;
			}
		}

		return &g_aAxisData[nClosestData];
	}
}

static ILTInput *g_pLTInput = NULL;
define_holder(ILTInput, g_pLTInput);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoystick::CScreenJoystick() :
	m_nJoystickDevice(ILTInput::k_InvalidIndex)
{
}

CScreenJoystick::~CScreenJoystick()
{

}

// Build the screen
bool CScreenJoystick::Build()
{
	CreateTitle("IDS_TITLE_JOYSTICK");
	
	kGap = 200;//g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	uint8 nLarge = 10;//g_pLayoutDB->GetInt32(m_hLayout,LDB_HeaderFontSize,0);

	CLTGUICycleCtrl_create ccs;
	ccs.nHeaderWidth = kGap;
	ccs.rnBaseRect.m_vMin.Init();
	uint32 nFontSize = 16;//g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize, (eScreenID)m_nScreenID);
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nFontSize);
  		
	ccs.szHelpID = "IDS_JOYSTICK_AXIS";
	CLTGUICycleCtrl *pCtrl = AddCycle("IDS_JOYSTICK_AXIS", ccs);
  	pCtrl->AddString(LoadString("IDS_JOYSTICK_ACTION"));

	uint32 nNumAxes = 0;

	// Look up the joystick
	if (g_pLTInput->FindFirstDeviceByCategory(ILTInput::eDC_Gamepad, &m_nJoystickDevice) == LT_OK)
	{
		// Get the axis info
		g_pLTInput->GetNumDeviceObjectsByCategory(m_nJoystickDevice, ILTInput::eDOC_Axis, &nNumAxes);
		if (nNumAxes)
			g_pLTInput->FindFirstDeviceObjectByCategory(m_nJoystickDevice, ILTInput::eDOC_Axis, &m_nFirstAxis);
	}
	
	m_aAxisBindings.resize(nNumAxes, 0);
  
	// Fill in the controls for the axes
	// Note : The controls are added in reverse so they're in the correct order
	for (int32 nAxis = (int32)nNumAxes - 1; nAxis >= 0; --nAxis)
  	{
		// Get the axis data
		ILTInput::SDeviceObjectDesc sAxisDesc;
		if (g_pLTInput->GetDeviceObjectDesc(m_nJoystickDevice, nAxis + m_nFirstAxis, &sAxisDesc) != LT_OK)
			continue;

		// Override the display name on axis.  DirectInput doesn't properly translate joystick objects.
		if( LTStrEquals( sAxisDesc.m_sName, L"X Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_XAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Y Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_YAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Z Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_ZAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Slider"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_SLIDER"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"POV"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_POV"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Rx Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_RXAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Ry Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_RYAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
		else if( LTStrEquals( sAxisDesc.m_sName, L"Rz Axis"))
			LTStrCpy(sAxisDesc.m_sDisplayName, LoadString("IDS_JOYSTICK_RZAXIS"), LTARRAYSIZE(sAxisDesc.m_sDisplayName) );
  
		ccs.szHelpID = "IDS_HELP_AXIS";
		ccs.pnValue = &m_aAxisBindings[nAxis];
		CLTGUICycleCtrl *pCtrl = AddCycle(sAxisDesc.m_sDisplayName, ccs);
		pCtrl->SetParam1(nAxis);
  
		for (uint32 nOption = 0; nOption < g_nNumAxisData; ++nOption)
  		{
			pCtrl->AddString(LoadString(g_aAxisData[nOption].m_szStringID));
  		}
  
  //		pCtrl->NotifyOnChange(CMD_UPDATE,this);
  	}
  
	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;

}

uint32 CScreenJoystick::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			UpdateData();

		
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};



// Change in focus
void CScreenJoystick::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->ApplyBindings();
		pProfile->ApplyJoystick();
  
		LoadBindings();

	
        UpdateData(false);
	}
	else
	{
		UpdateData();

		SaveBindings();

		pProfile->SetJoystick();
		pProfile->SetBindings();
		pProfile->Save();	
	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenJoystick::LoadBindings()
{
	for (Tuint8List::iterator iCurAxis = m_aAxisBindings.begin(); iCurAxis != m_aAxisBindings.end(); ++iCurAxis)
	{
		uint32 nAxisObject = (iCurAxis - m_aAxisBindings.begin()) + m_nFirstAxis;
		CBindMgr::SBinding sBinding;
		if (CBindMgr::GetSingleton().GetDeviceBinding(m_nJoystickDevice, nAxisObject, 0, &sBinding))
  		{
			SAxisData *pAxisData = FindAxisData(sBinding.m_nCommand, sBinding.m_fScale / 2.0f);
			if (pAxisData)
				*iCurAxis = pAxisData - g_aAxisData;
			else
				*iCurAxis = 0;
		}
		else
			*iCurAxis = 0;
	}
}

void CScreenJoystick::SaveBindings()
{
	for (Tuint8List::iterator iCurAxis = m_aAxisBindings.begin(); iCurAxis != m_aAxisBindings.end(); ++iCurAxis)
	{
		uint32 nAxisObject = (iCurAxis - m_aAxisBindings.begin()) + m_nFirstAxis;
		if (!*iCurAxis)
			CBindMgr::GetSingleton().ClearDeviceBindings(m_nJoystickDevice, nAxisObject);
		else
  		{
			CBindMgr::SBinding sBinding;
			sBinding.m_nCommand = g_aAxisData[*iCurAxis].m_nCommandID;
			sBinding.m_nDevice = m_nJoystickDevice;
			sBinding.m_nObject = nAxisObject;
			sBinding.m_fDefaultValue = 0.0f;

			float fJoystickSensitivity = g_aAxisData[*iCurAxis].m_fScale;
			const float k_fDeadZone = 0.4f;
			sBinding.m_fOffset = -(1.0f + k_fDeadZone) * fJoystickSensitivity;
			sBinding.m_fScale = (2.0f + k_fDeadZone * 2.0f) * fJoystickSensitivity;

			sBinding.m_fDeadZoneMin = -k_fDeadZone * fabsf(fJoystickSensitivity);
			sBinding.m_fDeadZoneMax = k_fDeadZone * fabsf(fJoystickSensitivity);
			sBinding.m_fDeadZoneValue = 0.0f;
			CBindMgr::GetSingleton().SetBinding(sBinding);
		}
  	}
}