#include "stdafx.h"
#include "FolderMgr.h"
#include "BaseFolder.h"
#include "JoystickAxis.h"
#include "clientres.h"


// Slider ranges
#define DEADZONESLIDERLOW 0
#define DEADZONESLIDERHIGH 9
#define SENSITIVITYSLIDERLOW 0
#define SENSITIVITYSLIDERHIGH 20

namespace
{
	int kGap = 200;
	int kWidth = 200;
}


/**************************************************************************/
// Joystick Axis info class
/**************************************************************************/

void CJoystickAxisInfo::Init(char* sName, uint32 nType, float fRangeLow, float fRangeHigh)
{
	if (sName == NULL) m_sName[0] = '\0';
	else
	{
		strncpy(m_sName, sName, INPUTNAME_LEN);
		m_sName[INPUTNAME_LEN-1] = '\0';
	}
	m_nType = nType;
	m_fRangeLow = fRangeLow;
	m_fRangeHigh = fRangeHigh;
};


/**************************************************************************/
// Joystick Axis base class
/**************************************************************************/

// Constructor
CJoystickAxisBase::CJoystickAxisBase()
{
	// names of possible actions
	m_sActionDigitalLow[0] = '\0';
	m_sActionDigitalHigh[0] = '\0';
	m_sActionAnalog[0] = '\0';
	m_sDeadZoneConsoleVar[0] = '\0';

	// numbers used to make the scale from the slider
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	// set default values for rangebind and rangescale data
	m_sDeviceName[0] = '\0';
	m_sTriggerName[0] = '\0';
	m_fScale = 1.0f;
	m_fRangeScaleMin = -1.0f;
	m_fRangeScaleMax = 1.0f;
	m_fRangeScalePreCenterOffset;
	m_sActionName[0] = '\0';
	m_fRangeLow = 0.0f;
	m_fRangeHigh = 0.0f;
	m_sActionName2[0] = '\0';
	m_fRangeLow2 = 0.0f;
	m_fRangeHigh2 = 0.0f;
	m_fDeadZone = 0.10f;

	// set to TRUE if bindings were read in successfully
    m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTFALSE;

	// Data members
	m_nAxis=JOYSTICK_AXIS_NONE;
	m_nSensitivity=1;
	m_nDeadZone=1;
    m_bAnalog=LTFALSE;
    m_bInvertAxis=LTFALSE;
    m_bCenterOffset=LTFALSE;

	// Stores original value of data members
	m_nOrigAxis=m_nAxis;
	m_nOrigSensitivity=m_nSensitivity;
	m_nOrigDeadZone=m_nDeadZone;
	m_bOrigAnalog=m_bAnalog;
	m_bOrigInvertAxis=m_bInvertAxis;
	m_bOrigCenterOffset=m_bCenterOffset;

	// clear the array of axis name strings
	m_nNumJoystickAxis = 0;

	// Message ID for each control
	m_nAxisID=IDS_JOYSTICK_AXIS;
	m_nSensitivityID=IDS_JOYSTICK_SENSITIVITY;
	m_nDeadZoneID=IDS_JOYSTICK_DEADZONE;
	m_nAnalogID=IDS_JOYSTICK_ANALOG;
	m_nInvertAxisID=IDS_JOYSTICK_INVERT;
	m_nCenterOffsetID=IDS_JOYSTICK_CENTERCORRECTION;

	// The controls
    m_pAxisCtrl=LTNULL;
    m_pSensitivityCtrl=LTNULL;
    m_pAnalogCtrl=LTNULL;
    m_pDeadZoneCtrl=LTNULL;
    m_pInvertAxisCtrl=LTNULL;
    m_pCenterOffsetCtrl=LTNULL;
}

// Build the  items for this class
void CJoystickAxisBase::Build(ILTClient *pClientDE, CBaseFolder *pDestFolder)
{
	if (!pDestFolder)
	{
		return;
	}

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_JOYSTICK,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOYSTICK,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_JOYSTICK,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOYSTICK,"SliderWidth");

	// figure out the name of the joystick device
	SetDeviceName(pClientDE);

	// create the array of possible axis
	CreateAxisArray(pClientDE);

	// The axis option
	m_pAxisCtrl=BuildAxisOption(pClientDE, pDestFolder, m_nAxisID, &m_nAxis);

	// The invert axis control
	m_pInvertAxisCtrl=pDestFolder->AddToggle(m_nInvertAxisID, IDS_HELP_INVERTAXIS,  kGap,
												&m_bInvertAxis, LTFALSE, g_pInterfaceResMgr->GetSmallFont());

	m_pInvertAxisCtrl->SetOnString(IDS_ON);
	m_pInvertAxisCtrl->SetOffString(IDS_OFF);

	// The dead zone control
	m_pDeadZoneCtrl=pDestFolder->AddSlider(m_nDeadZoneID, IDS_HELP_DEADZONE, kGap, kWidth,
											   &m_nDeadZone, LTFALSE, g_pInterfaceResMgr->GetSmallFont());
	if (m_pDeadZoneCtrl)
	{
		m_pDeadZoneCtrl->SetSliderRange(DEADZONESLIDERLOW, DEADZONESLIDERHIGH);
		m_pDeadZoneCtrl->SetSliderIncrement(1);
	}

	// Analog on/off
	m_pAnalogCtrl=pDestFolder->AddToggle(m_nAnalogID, IDS_HELP_ANALOG, kGap, &m_bAnalog, LTFALSE,  g_pInterfaceResMgr->GetSmallFont());
	m_pAnalogCtrl->SetOnString(IDS_ON);
	m_pAnalogCtrl->SetOffString(IDS_OFF);

	// The sensitivity control
	m_pSensitivityCtrl=pDestFolder->AddSlider(m_nSensitivityID, IDS_HELP_JOY_SENSE, kGap, kWidth,
												  &m_nSensitivity, LTFALSE,  g_pInterfaceResMgr->GetSmallFont());
	if (m_pSensitivityCtrl)
	{
		m_pSensitivityCtrl->SetSliderRange(SENSITIVITYSLIDERLOW, SENSITIVITYSLIDERHIGH);
		m_pSensitivityCtrl->SetSliderIncrement(1);
	}


	// The center offset axis control
	m_pCenterOffsetCtrl=pDestFolder->AddToggle(m_nCenterOffsetID, IDS_HELP_JOY_CENTER, kGap,
												  &m_bCenterOffset, LTFALSE, g_pInterfaceResMgr->GetSmallFont());
	m_pCenterOffsetCtrl->SetOnString(IDS_ON);
	m_pCenterOffsetCtrl->SetOffString(IDS_OFF);
}

// Updates the enable/disable status of each control.
// bJoystickOn indicates if the joystick is enabled in the s.
void CJoystickAxisBase::UpdateEnable(LTBOOL bJoystickOn)
{
	// If the joystick is on the enable all of the controls.
	// The analog status is checked down below which may disable some of the controls.
    LTBOOL bEnable=bJoystickOn;

	if (m_pAxisCtrl)			m_pAxisCtrl->Enable(bEnable);
	if (m_pInvertAxisCtrl)		m_pInvertAxisCtrl->Enable(bEnable);
	if (m_pDeadZoneCtrl)		m_pDeadZoneCtrl->Enable(bEnable);
	if (m_pAnalogCtrl)			m_pAnalogCtrl->Enable(bEnable);
	if (m_pSensitivityCtrl)		m_pSensitivityCtrl->Enable(bEnable);
	if (m_pCenterOffsetCtrl)	m_pCenterOffsetCtrl->Enable(bEnable);

	// Return if the joystick is disabled
	if (!bJoystickOn)
	{
		return;
	}

	// If the axis is set to none then disable the rest of the controls
	if (m_nAxis == JOYSTICK_AXIS_NONE)
	{
        bEnable=LTFALSE;
		if (m_pInvertAxisCtrl)		m_pInvertAxisCtrl->Enable(bEnable);
		if (m_pDeadZoneCtrl)		m_pDeadZoneCtrl->Enable(bEnable);
		if (m_pAnalogCtrl)			m_pAnalogCtrl->Enable(bEnable);
		if (m_pSensitivityCtrl)		m_pSensitivityCtrl->Enable(bEnable);
		if (m_pCenterOffsetCtrl)	m_pCenterOffsetCtrl->Enable(bEnable);

		return;
	}

	// Disable the sensitivity and center off controls if in analog mode
	if (!m_bAnalog)
	{
        if (m_pSensitivityCtrl)     m_pSensitivityCtrl->Enable(LTFALSE);
        if (m_pCenterOffsetCtrl)    m_pCenterOffsetCtrl->Enable(LTFALSE);
	}
}

// Build the  option that allows the user to select the axis
CCycleCtrl *CJoystickAxisBase::BuildAxisOption(ILTClient *pClientDE, CBaseFolder *pDestFolder, int nMessageID, int *pnDestValue)
{

	// Load the display string from the string resource
	HSTRING hDisplayString=pClientDE->FormatString(nMessageID);
	if (!hDisplayString)
	{
        return LTNULL;
	}

	// Create the control
	CCycleCtrl *pCtrl=pDestFolder->AddCycleItem(hDisplayString, 0, 75, 25, pnDestValue, LTFALSE, g_pInterfaceResMgr->GetSmallFont());

	// Add the strings to the control
	int i;
	for (i=0; i < m_nNumJoystickAxis; i++)
	{
		// Construct the string for the option (ie "Select axis [axis name]");
		char szBuffer[2048];
		sprintf(szBuffer, "[%s]", m_aryAxisInfo[i].GetName());

		// Add the string to the control
		HSTRING hTemp=pClientDE->CreateString(szBuffer);
		pCtrl->AddString(hTemp);

		// Free the strings
		pClientDE->FreeString(hTemp);
	}

	return pCtrl;
}

// Set the joystick device name
BOOL CJoystickAxisBase::SetDeviceName(ILTClient *pClientDE)
{
	BOOL bDeviceNameFound = FALSE;

	// clear the devicename for the case where we don't find one
	m_sDeviceName[0] = '\0';

	// read in the joystick device to use from the JOYSTICKDEVICENAME console variable or set the default
	char* sValue = NULL;
	HCONSOLEVAR hVar = pClientDE->GetConsoleVar( "JOYSTICKDEVICENAME");
	if (hVar != NULL) sValue = pClientDE->GetVarValueString(hVar);
	if (sValue != NULL)
	{
		strncpy(m_sDeviceName, sValue, INPUTNAME_LEN);
		m_sDeviceName[INPUTNAME_LEN-1] = '\0';
	}
	else
	{
		strncpy(m_sDeviceName, "Joystick 1", INPUTNAME_LEN);
		m_sDeviceName[INPUTNAME_LEN-1] = '\0';
	}

	// verify that this device name exists
	DeviceObject* pObjects = pClientDE->GetDeviceObjects(DEVICETYPE_JOYSTICK);
	DeviceObject* pObj = pObjects;
    LTBOOL bFoundIt = LTFALSE;
	while ((pObj != NULL) && (bDeviceNameFound == FALSE))
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL) && (pObj->m_DeviceType == DEVICETYPE_JOYSTICK))
		{
			if (stricmp(pObj->m_DeviceName, m_sDeviceName) == 0) bDeviceNameFound = TRUE;
		}
		pObj = pObj->m_pNext;
	}

	// if the name did not exist use the first joystick name that we can find
	if ((bDeviceNameFound == FALSE) && (pObjects != NULL))
	{
		if (pObjects->m_DeviceName != NULL)
		{
			strncpy(m_sDeviceName, pObjects->m_DeviceName, INPUTNAME_LEN);
			m_sDeviceName[INPUTNAME_LEN-1] = '\0';
			bDeviceNameFound = TRUE;
		}
	}

	// free all the device objects
	if (pObjects != NULL) pClientDE->FreeDeviceObjects (pObjects);

	// put the enabledevice command out to the console
	char tempStr[512];
	sprintf(tempStr, "enabledevice \"%s\"", m_sDeviceName);
	pClientDE->RunConsoleString(tempStr);

	return bDeviceNameFound;
}

// Set the joystick device name
void CJoystickAxisBase::CreateAxisArray(ILTClient *pClientDE)
{
	DeviceObject* pObjects = pClientDE->GetDeviceObjects(DEVICETYPE_JOYSTICK);
	DeviceObject* pObj = pObjects;
    LTBOOL bFoundIt = LTFALSE;

	// the first axis is always the none axis
	HSTRING hAxisName = pClientDE->FormatString(IDS_JOYSTICK_AXISNONE);
	m_nNumJoystickAxis = 0;
	if (hAxisName != NULL) m_aryAxisInfo[m_nNumJoystickAxis].Init(pClientDE->GetStringData(hAxisName), 0, 0.0f, 0.0f);
	else m_aryAxisInfo[m_nNumJoystickAxis].Init("none", 0, 0.0f, 0.0f);
	m_nNumJoystickAxis++;

	// loop through all joystick objects and store the axis ones with our devicename the aryAxisInfo array
	while ((pObj != NULL) && (m_nNumJoystickAxis < JOYSTICK_MAX_AXIS))
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL))
		{
			if ((pObj->m_DeviceType == DEVICETYPE_JOYSTICK) &&
				(stricmp(pObj->m_DeviceName, m_sDeviceName) == 0) &&
				((pObj->m_ObjectType == CONTROLTYPE_XAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_YAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_ZAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RXAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RYAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_RZAXIS) ||
				 (pObj->m_ObjectType == CONTROLTYPE_SLIDER)))
			{
				m_aryAxisInfo[m_nNumJoystickAxis].Init(pObj->m_ObjectName, pObj->m_ObjectType, pObj->m_RangeLow, pObj->m_RangeHigh);
				m_nNumJoystickAxis++;
			}

		}
		pObj = pObj->m_pNext;
	}

	// free the device objects
	if (pObjects != NULL) pClientDE->FreeDeviceObjects (pObjects);
}

// Set the joystick device name
void CJoystickAxisBase::SaveNewActions(ILTClient *pClientDE)
{
	pClientDE->RunConsoleString("AddAction AxisYaw -10001");
	pClientDE->RunConsoleString("AddAction AxisPitch -10002");
	pClientDE->RunConsoleString("AddAction AxisLeftRight -10003");
	pClientDE->RunConsoleString("AddAction AxisForwardBackward -10004");
}

// get a binding for a joystick trigger
void CJoystickAxisBase::GetJoystickBindingInfo(ILTClient *pClientDE)
{
//	pClientDE->CPrint("Enter GetJoystickBindingInfo. m_sActionAnalog = %s m_sActionDigitalHigh = %s m_sActionDigitalLow = %s", m_sActionAnalog, m_sActionDigitalHigh, m_sActionDigitalLow ); // BLB TEMP

	// Get the bindings
	DeviceBinding *pBinding=pClientDE->GetDeviceBindings(DEVICETYPE_JOYSTICK);

	// Iterate through each binding look for the one with our action name
	DeviceBinding* pCurrentBinding = pBinding;
	while (pCurrentBinding != NULL)
	{
		if ((stricmp(pCurrentBinding->strDeviceName, m_sDeviceName) == 0) &&
			(GetAxisIndex(pClientDE, pCurrentBinding->strTriggerName) > 0))
		{
//			pClientDE->CPrint("Checking device = %s trigger = %s", pCurrentBinding->strDeviceName, pCurrentBinding->strTriggerName ); // BLB TEMP

            LTBOOL       bFound = LTFALSE;
			GameAction* pDigitalLowAction = NULL;
			GameAction* pDigitalHighAction = NULL;
			GameAction* pAnalogAction = NULL;
			GameAction* pAction = pCurrentBinding->pActionHead;

			// go through all actions looking for our actions
			while (pAction != NULL)
			{
				char *sActionName=pAction->strActionName;
				if (sActionName != NULL)
				{
//					pClientDE->CPrint("Checking action = %s", sActionName ); // BLB TEMP
					if (stricmp(sActionName, m_sActionAnalog) == 0) pAnalogAction = pAction;
					if (stricmp(sActionName, m_sActionDigitalHigh) == 0) pDigitalHighAction = pAction;
					if (stricmp(sActionName, m_sActionDigitalLow) == 0) pDigitalLowAction = pAction;
				}
				pAction = pAction->pNext;
				if (pAction == pCurrentBinding->pActionHead) break;
			}

			// if we found an analog action set up the variables for it
			if (pAnalogAction != NULL)
			{
				strncpy(m_sActionName, pAnalogAction->strActionName, MAX_ACTIONNAME_LEN);
				m_sActionName[MAX_ACTIONNAME_LEN-1] = '\0';
				m_fRangeLow = pAnalogAction->nRangeLow;
				m_fRangeHigh = pAnalogAction->nRangeHigh;

				m_bAnalog = TRUE;
                bFound = LTTRUE;

//				pClientDE->CPrint("Digital binding read Device = %s Trigger = %s Action1 = %s", m_sDeviceName, pCurrentBinding->strTriggerName, m_sActionName ); // BLB TEMP
			}

			// otherwise if we found the two digital actions with no analog we are in digital mode
			else if ((pDigitalLowAction != NULL) && (pDigitalHighAction != NULL))
			{
				strncpy(m_sActionName, pDigitalLowAction->strActionName, MAX_ACTIONNAME_LEN);
				m_sActionName[MAX_ACTIONNAME_LEN-1] = '\0';
				m_fRangeLow = pDigitalLowAction->nRangeLow;
				m_fRangeHigh = pDigitalLowAction->nRangeHigh;

				strncpy(m_sActionName2, pDigitalHighAction->strActionName, MAX_ACTIONNAME_LEN);
				m_sActionName2[MAX_ACTIONNAME_LEN-1] = '\0';
				m_fRangeLow2 = pDigitalHighAction->nRangeLow;
				m_fRangeHigh2 = pDigitalHighAction->nRangeHigh;

				m_bAnalog = FALSE;
                bFound = LTTRUE;

//				pClientDE->CPrint("Digital binding read Device = %s Trigger = %s Action1 = %s Action2 = %s", m_sDeviceName, pCurrentBinding->strTriggerName, m_sActionName, m_sActionName2 ); // BLB TEMP
			}

			else
			{
//				pClientDE->CPrint("No binding found!"); // BLB TEMP
			}

			// if we did find some bindings transfer the information
			if (bFound)
			{
				strncpy(m_sTriggerName, pCurrentBinding->strTriggerName, INPUTNAME_LEN);
				m_sTriggerName[INPUTNAME_LEN-1] = '\0';
				m_fScale = pCurrentBinding->nScale;
				m_fRangeScaleMin = pCurrentBinding->nRangeScaleMin;
				m_fRangeScaleMax = pCurrentBinding->nRangeScaleMax;
				m_fRangeScalePreCenterOffset = pCurrentBinding->nRangeScalePreCenterOffset;
                m_bBindingsRead = LTTRUE;
			}
		}
		pCurrentBinding = pCurrentBinding->pNext;
		if (pCurrentBinding == pBinding) break;
	}

	// Free the device bindings
	pClientDE->FreeDeviceBindings(pBinding);

	if (!m_bBindingsRead)
	{
		strncpy(m_sActionName, m_sActionAnalog, MAX_ACTIONNAME_LEN);
		m_sActionName[MAX_ACTIONNAME_LEN-1] = '\0';
	}

//	pClientDE->CPrint("Exit GetJoystickBindingInfo." ); // BLB TEMP
}

// Send out a rangebind to the console
void CJoystickAxisBase::DoRangeBind(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh)
{
	assert(lpszControlName);
	assert(lpszActionName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszActionName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the binding
	sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\"", lpszDevice, lpszControlName, nRangeLow, nRangeHigh, lpszActionName);
	pClientDE->RunConsoleString(tempStr);
}

// Clear a rangebind to the console
void CJoystickAxisBase::ClearRangeBind(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName)
{
	assert(lpszControlName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the binding
	sprintf(tempStr, "rangebind \"%s\" \"%s\" 0 0 \"\"", lpszDevice, lpszControlName);
	pClientDE->RunConsoleString(tempStr);
}

// Send out a rangescale to the console
void CJoystickAxisBase::DoRangeScale(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset)
{
	assert(lpszControlName);
	assert(lpszDevice);

	if (lpszControlName == NULL) return;
	if (lpszDevice == NULL) return;

	char tempStr[512];

	// Set the rangescale
	sprintf(tempStr, "rangescale \"%s\" \"%s\" %f %f %f %f", lpszDevice, lpszControlName, nScale, nRangeScaleMin, nRangeScaleMax, nRangeScalePreCenterOffset);
	pClientDE->RunConsoleString(tempStr);
}

// Set a consolve var to a float
void CJoystickAxisBase::SetConsoleVar(ILTClient *pClientDE, char *lpszName, float nVal)
{
	assert(lpszName);

	if (lpszName == NULL) return;

	char tempStr[512];

	// Set the rangescale
	sprintf(tempStr, "+%s %f", lpszName, nVal);
	pClientDE->RunConsoleString(tempStr);
}

// find the index into the axis array for the given axis name (return 0 if not found)
int CJoystickAxisBase::GetAxisIndex(ILTClient *pClientDE, char* sAxisName)
{
	// loop through all axis trying to find the matching axis (skip 0 becaus it is the none axis)
	for (int i = 1; i < m_nNumJoystickAxis; i++)
	{
		if (stricmp(sAxisName, m_aryAxisInfo[i].GetName()) == 0) return i;
	}
	return 0;
}

// scales a value from one range to another in a linear fashion
float CJoystickAxisBase::ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax)
{
	float fMultiplier = 1.0f;
	float fOffset;

	float fFromMinMinusMax = fFromMin - fFromMax;
	if (fFromMinMinusMax != 0.0f) fMultiplier = (fToMin - fToMax) / fFromMinMinusMax;
	fOffset = fToMin - (fMultiplier * fFromMin);

	float fRetVal = (fFromVal * fMultiplier) + fOffset;
	if (fRetVal < fToMin) fRetVal = fToMin;
	if (fRetVal > fToMax) fRetVal = fToMax;

	return fRetVal;
}


// Load variables from the console
void CJoystickAxisBase::LoadFromConsole(ILTClient *pClientDE)
{

	if (!m_bBindingsRead)
	{
		// try to read digital bindings
		GetJoystickBindingInfo(pClientDE);
	}

	// read in the dead zone console variable
	HCONSOLEVAR hVar = pClientDE->GetConsoleVar(m_sDeadZoneConsoleVar);
	if (m_bAnalog)
	{
		if (hVar != NULL) m_fDeadZone = pClientDE->GetVarValueFloat(hVar) / m_fScale;
		//pClientDE->CPrint("Read Dead Zone Analog : m_fDeadZone = %f m_fScale = %f ", m_fDeadZone, m_fScale ); // BLB TEMP
	}
	else
	{
		if (hVar != NULL) m_fDeadZone = pClientDE->GetVarValueFloat(hVar);
		//pClientDE->CPrint("Read Dead Zone Digital : m_fDeadZone = %f", m_fDeadZone ); // BLB TEMP
	}

	// convert all of the variables read in from the bindings to our internal controls variables
	if (m_bBindingsRead)
	{
		// figure out the index of the axis that we found
		m_nAxis = GetAxisIndex(pClientDE, m_sTriggerName);

		// scale the sensitivity slider bar from the rangescale values that we read in
		if (m_bAnalog) m_nSensitivity = (int)ScaleToRange(m_fScale, m_fScaleCenter-m_fScaleRangeDiv2, m_fScaleCenter+m_fScaleRangeDiv2, (float)SENSITIVITYSLIDERLOW, (float)SENSITIVITYSLIDERHIGH);

		// scale the dead zone that we read in to the dead zone slider bar
		m_nDeadZone = (int)ScaleToRange(m_fDeadZone, 0.0f, 0.90f, (float)DEADZONESLIDERLOW, (float)DEADZONESLIDERHIGH);

		// figure out if the bindings that were read in had inverted axis
		if (m_bAnalog)
		{
            if (m_fRangeScaleMin > m_fRangeScaleMax) m_bInvertAxis = LTTRUE;
            else m_bInvertAxis = LTFALSE;
		}
		else
		{
			if (stricmp(m_sActionName, m_sActionDigitalLow) == 0)
			{
                if (m_fRangeLow < m_fRangeLow2) m_bInvertAxis = LTFALSE;
                else m_bInvertAxis = LTTRUE;
			}
			else
			{
                if (m_fRangeLow <= m_fRangeLow2) m_bInvertAxis = LTTRUE;
                else m_bInvertAxis = LTFALSE;
			}
		}

		// figure out the correct value of the center offset flag from the data read in
        if ((long)m_fRangeScalePreCenterOffset != 100) m_bCenterOffset = LTFALSE;
        else m_bCenterOffset = LTTRUE;
	}

	// save off the original values
	m_nOrigAxis=m_nAxis;
	m_nOrigSensitivity=m_nSensitivity;
	m_nOrigDeadZone=m_nDeadZone;
	m_bOrigInvertAxis=m_bInvertAxis;
	m_bOrigCenterOffset=m_bCenterOffset;
	m_bOrigAnalog = m_bAnalog;

}


// Save an analog binding to the consle
void CJoystickAxisBase::SaveToConsoleAnalog(ILTClient *pClientDE)
{
	// set the new scale if it has been adjusted or was never read in
	m_fScale = ScaleToRange((float)m_nSensitivity, (float)SENSITIVITYSLIDERLOW, (float)SENSITIVITYSLIDERHIGH, m_fScaleCenter-m_fScaleRangeDiv2, m_fScaleCenter+m_fScaleRangeDiv2);
	if (m_fScale < 0.001) m_fScale = 0.001f; // don't let the scale get too small

	// adjust values for the invert axis flag if it has been adjusted or was never read in
	if (m_bInvertAxis)
	{
		if (m_fRangeScaleMin < m_fRangeScaleMax)
		{
			float fTemp;
			fTemp = m_fRangeScaleMin;
			m_fRangeScaleMin = m_fRangeScaleMax;
			m_fRangeScaleMax = fTemp;
		}
	}
	else
	{
		if (m_fRangeScaleMin > m_fRangeScaleMax)
		{
			float fTemp;
			fTemp = m_fRangeScaleMin;
			m_fRangeScaleMin = m_fRangeScaleMax;
			m_fRangeScaleMax = fTemp;
		}
	}

	// set the center offsets if it has changed or was never read in
	if (m_bCenterOffset) m_fRangeScalePreCenterOffset = 100.0f;
	else m_fRangeScalePreCenterOffset = 0.0f;

	// write out the rangebind and rangescale
	if ((m_sDeviceName[0] != '\0') && (m_sTriggerName[0] != '\0') && (m_sActionName[0] != '\0'))
	{
		char tempStr[1024];
		if (m_bAddDigitalBindingsToAnalog)
		{
			if (m_bInvertAxis)
			{
				sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\" %f %f \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog,
					(m_fDeadZone*m_fScale)-0.001f, (1.0f*m_fScale)+0.001f, m_sActionDigitalLow, -((m_fDeadZone*m_fScale)-0.001f), -((1.0f*m_fScale)+0.001f), m_sActionDigitalHigh);
			}
			else
			{
				sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\" %f %f \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog,
					(m_fDeadZone*m_fScale)-0.001f, (1.0f*m_fScale)+0.001f, m_sActionDigitalHigh, -((m_fDeadZone*m_fScale)-0.001f), -((1.0f*m_fScale)+0.001f), m_sActionDigitalLow);
			}
		}
		else
		{
			sprintf(tempStr, "rangebind \"%s\" \"%s\" %f %f \"%s\"", m_sDeviceName, m_sTriggerName, 0.0f, 0.0f, m_sActionAnalog);
		}
		pClientDE->RunConsoleString(tempStr);
		DoRangeScale(pClientDE, m_sDeviceName, m_sTriggerName, m_fScale, m_fRangeScaleMin, m_fRangeScaleMax, m_fRangeScalePreCenterOffset);
		//pClientDE->CPrint("Joystick Analog Binding : %s m_fScale=%f m_fRangeScaleMin=%f m_fRangeScaleMax=%f m_fRangeScalePreCenterOffset=%f ", tempStr, m_fScale, m_fRangeScaleMin, m_fRangeScaleMax, m_fRangeScalePreCenterOffset ); // BLB TEMP
	}
}


// Save a digital binding to the consle
void CJoystickAxisBase::SaveToConsoleDigital(ILTClient *pClientDE)
{
	// save off the range of the device
	float fDeviceRangeHigh = m_aryAxisInfo[m_nAxis].GetRangeHigh();
	float fDeviceRangeLow = m_aryAxisInfo[m_nAxis].GetRangeLow();

	// figure out the intermediate range information
	float fRange = fDeviceRangeHigh - fDeviceRangeLow;
	float fHalfRange = fRange / 2.0f;
	float fActiveRange = fHalfRange - (m_fDeadZone * fRange);

	// figure out the numbers to write out
	float fLeftLow = fDeviceRangeLow;
	float fLeftHigh = fDeviceRangeLow + fActiveRange;
	float fRightLow = fDeviceRangeHigh - fActiveRange;
	float fRightHigh = fDeviceRangeHigh;

	// clear the bindings
	char str[512];
	sprintf (str, "rangebind \"%s\" \"%s\" 0 0 \"\"", m_sDeviceName, m_sTriggerName);
	pClientDE->RunConsoleString (str);

	// write out the new bindings
	if (m_bInvertAxis)
	{
		fLeftLow = fDeviceRangeLow;
		fLeftHigh = fDeviceRangeLow + fActiveRange;
		fRightLow = fDeviceRangeHigh - fActiveRange;
		fRightHigh = fDeviceRangeHigh;

		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
			 fRightLow, fRightHigh, m_sActionDigitalLow, fLeftLow, fLeftHigh, m_sActionDigitalHigh);
	}
	else
	{
		fLeftLow = fDeviceRangeLow;
		fLeftHigh = fDeviceRangeLow + fActiveRange;
		fRightLow = fDeviceRangeHigh - fActiveRange;
		fRightHigh = fDeviceRangeHigh;

		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
			fLeftLow, fLeftHigh, m_sActionDigitalLow, fRightLow, fRightHigh, m_sActionDigitalHigh);
	}

	pClientDE->RunConsoleString (str);
	//pClientDE->CPrint("Joystick Digital Binding : %s", str ); // BLB TEMP
}


// Save variables to the console
void CJoystickAxisBase::SaveToConsole(ILTClient *pClientDE)
{
		// clear the old binding
	if (m_bBindingsRead)
	{
		if (m_nOrigAxis > JOYSTICK_AXIS_NONE)
		{
			strncpy(m_sTriggerName, m_aryAxisInfo[m_nOrigAxis].GetName(), INPUTNAME_LEN);
			m_sTriggerName[INPUTNAME_LEN-1] = '\0';

			ClearRangeBind(pClientDE, m_sDeviceName, m_sTriggerName);
		}
	}

	// figure out the new trigger name
	strncpy(m_sTriggerName, m_aryAxisInfo[m_nAxis].GetName(), INPUTNAME_LEN);
	m_sTriggerName[INPUTNAME_LEN-1] = '\0';

	// set the new dead zone if it has been adjusted or was never read in
	m_fDeadZone = ScaleToRange((float)m_nDeadZone, (float)DEADZONESLIDERLOW, (float)DEADZONESLIDERHIGH, 0.0f, 0.90f);
	if (m_nDeadZone == DEADZONESLIDERLOW) m_fDeadZone = 0.0f;

	// do the analog or digital binding
	if (m_nAxis > JOYSTICK_AXIS_NONE)
	{
		// clear any previous bindings to this axis
		ClearRangeBind(pClientDE, m_sDeviceName, m_sTriggerName);

		// write out the new actions in case they are not in the autoexec.cfg
		SaveNewActions(pClientDE);

		// save out analog options
		if (m_bAnalog) SaveToConsoleAnalog(pClientDE);

		// save out digital options
		else SaveToConsoleDigital(pClientDE);
	}

	// write out the dead zone console variable
	if ((m_sDeviceName[0] != '\0') && (m_sTriggerName[0] != '\0') && (m_sActionName[0] != '\0'))
	{
		float fDeadZone = m_fDeadZone;
		if (m_bAnalog) fDeadZone = m_fDeadZone * m_fScale;
		if (fDeadZone < 0.0f) fDeadZone = 0.0f;
		SetConsoleVar(pClientDE, m_sDeadZoneConsoleVar, fDeadZone);
		//pClientDE->CPrint("Saving Dead Zone : fDeadZone = %f m_fDeadZone = %f", fDeadZone, m_fDeadZone ); // BLB TEMP
	}

	g_pLTClient->WriteConfigFile("autoexec.cfg");

	if (m_bBindingsRead)
	{
		GetJoystickBindingInfo(pClientDE);
	}

}

/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/

// Load from the console
void CJoystickAxisTurn::LoadFromConsole(ILTClient *pClientDE)
{
//    m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTFALSE;

	// set up range of sensitivity scaling
	m_fScaleCenter = 0.51f;
	m_fScaleRangeDiv2 = 0.5f;

	if (!m_bBindingsRead)
	{
		// set up the default variables for this axis
		strncpy(m_sTriggerName, "none", INPUTNAME_LEN);
		m_sTriggerName[INPUTNAME_LEN-1] = '\0';
		m_fRangeScaleMin = -1.0f;
		m_fRangeScaleMax = 1.0f;
		m_nAxis = GetAxisIndex(pClientDE, m_sTriggerName);
        m_bInvertAxis = LTFALSE;
		m_nDeadZone = 2;
        m_bAnalog = LTFALSE;
		m_nSensitivity = 10;
        m_bCenterOffset = LTFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "Left", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "Right", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "Axis1", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisYawDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CJoystickAxisTurn::SaveToConsole(ILTClient *pClientDE)
{
	// call base class to save bindings
	CJoystickAxisBase::SaveToConsole(pClientDE);
}

/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

// Constructor
CJoystickAxisLook::CJoystickAxisLook()
{
	CJoystickAxisBase::CJoystickAxisBase();

	// Members
    m_bFixedPosition=LTTRUE;

	// String IDs
	m_nFixedPositionID=IDS_JOYSTICK_LOOKUPDOWNFIXEDBYPOSITION;

	// Controls
    m_pFixedPositionCtrl=LTNULL;

}

// Build the  items
void CJoystickAxisLook::Build(ILTClient *pClientDE, CBaseFolder *pDestFolder)
{
	// Get the main s pointer

	// Call the base class
	CJoystickAxisBase::Build(pClientDE, pDestFolder);

	// The center offset axis control
	m_pFixedPositionCtrl=pDestFolder->AddToggle(m_nFixedPositionID, 0, kGap,
												   &m_bFixedPosition, LTFALSE, g_pInterfaceResMgr->GetSmallFont());
	m_pFixedPositionCtrl->SetOnString(IDS_ON);
	m_pFixedPositionCtrl->SetOffString(IDS_OFF);
	m_pFixedPositionCtrl->SetHelpID(IDS_HELP_FIXEDPOSITION);
}

// Updates the enable/disable status of each control.
// bJoystickOn indicates if the joystick is enabled in the s.
void CJoystickAxisLook::UpdateEnable(LTBOOL bJoystickOn)
{
	// Call the base class
	CJoystickAxisBase::UpdateEnable(bJoystickOn);

    LTBOOL bEnable=LTTRUE;
	if (bJoystickOn && m_bAnalog)
	{
        bEnable=LTTRUE;
	}
	else
	{
        bEnable=LTFALSE;
	}

	if (m_pFixedPositionCtrl)
	{
		m_pFixedPositionCtrl->Enable(bEnable);
	}
}

// Load from the console
void CJoystickAxisLook::LoadFromConsole(ILTClient *pClientDE)
{
    m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTFALSE;

	// read in and set up the fixed position variable
	HCONSOLEVAR hVar = pClientDE->GetConsoleVar( "FixedAxisPitch");
	if (hVar != NULL)
	{
		if (pClientDE->GetVarValueFloat(hVar) == 1) m_bFixedPosition = TRUE;
		else m_bFixedPosition = FALSE;
	}
	else m_bFixedPosition = FALSE;

	// set up range of sensitivity scaling
	if (m_bFixedPosition) m_fScaleCenter = 1.0f;
	else m_fScaleCenter = 0.51f;
	m_fScaleRangeDiv2 = 0.5f;

	if (!m_bBindingsRead)
	{
		// set up the default variables for this axis
		strncpy(m_sTriggerName, "none", INPUTNAME_LEN);
		m_sTriggerName[INPUTNAME_LEN-1] = '\0';
		m_fRangeScaleMin = -1.0f;
		m_fRangeScaleMax = 1.0f;
		m_nAxis = GetAxisIndex(pClientDE, m_sTriggerName);
        m_bInvertAxis = LTFALSE;
		m_nDeadZone = 2;
        m_bAnalog = LTFALSE;
		m_nSensitivity = 10;
        m_bCenterOffset = LTFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "LookUp", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "LookDown", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "Axis2", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisPitchDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CJoystickAxisLook::SaveToConsole(ILTClient *pClientDE)
{
	// figure out correct rangescale values based on fixedposition
	if (m_bFixedPosition)
	{
		m_fRangeScaleMin = -(3.14159265f/2.0f);
		m_fRangeScaleMax = (3.14159265f/2.0f);
		m_fScaleCenter = 1.0f;
	}
	else
	{
		m_fRangeScaleMin = -1.0;
		m_fRangeScaleMax = 1.0;
		m_fScaleCenter = 0.51f;
	}

	// call base class to save bindings
	CJoystickAxisBase::SaveToConsole(pClientDE);

	// write out the fixed axis pitch console variable
	if ((m_bFixedPosition) && (m_bAnalog) && (m_nAxis > JOYSTICK_AXIS_NONE)) SetConsoleVar(pClientDE, "FixedAxisPitch", 1.0);
	else SetConsoleVar(pClientDE, "FixedAxisPitch", 0.0);
}

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

// Contructor
CJoystickAxisMove::CJoystickAxisMove()
{
	CJoystickAxisBase::CJoystickAxisBase();
}

// Load from the console
void CJoystickAxisMove::LoadFromConsole(ILTClient *pClientDE)
{
    m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTTRUE;

	// set up range of sensitivity scaling
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	if (!m_bBindingsRead)
	{
		// set up the default variables for this axis
		strncpy(m_sTriggerName, "none", INPUTNAME_LEN);
		m_sTriggerName[INPUTNAME_LEN-1] = '\0';
		m_fRangeScaleMin = -1.0f;
		m_fRangeScaleMax = 1.0f;
		m_nAxis = GetAxisIndex(pClientDE, m_sTriggerName);
        m_bInvertAxis = LTFALSE;
		m_nDeadZone = 2;
        m_bAnalog = LTFALSE;
		m_nSensitivity = 10;
        m_bCenterOffset = LTFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "Forward", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "Backward", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisForwardBackward", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisForwardBackwardDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CJoystickAxisMove::SaveToConsole(ILTClient *pClientDE)
{
	// call base class to save bindings
	CJoystickAxisBase::SaveToConsole(pClientDE);
}

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/

void CJoystickAxisStrafe::LoadFromConsole(ILTClient *pClientDE)
{
    m_bBindingsRead = LTFALSE;

    m_bAddDigitalBindingsToAnalog = LTTRUE;

	// set up range of sensitivity scaling
	m_fScaleCenter = 1.0f;
	m_fScaleRangeDiv2 = 0.5f;

	if (!m_bBindingsRead)
	{
		// set up the default variables for this axis
		strncpy(m_sTriggerName, "none", INPUTNAME_LEN);
		m_sTriggerName[INPUTNAME_LEN-1] = '\0';
		m_fRangeScaleMin = -1.0f;
		m_fRangeScaleMax = 1.0f;
		m_nAxis = GetAxisIndex(pClientDE, m_sTriggerName);
        m_bInvertAxis = LTFALSE;
		m_nDeadZone = 2;
        m_bAnalog = LTFALSE;
		m_nSensitivity = 10;
        m_bCenterOffset = LTFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "StrafeLeft", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "StrafeRight", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisLeftRight", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisLeftRightDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CJoystickAxisBase::LoadFromConsole(pClientDE);
}

void CJoystickAxisStrafe::SaveToConsole(ILTClient *pClientDE)
{
	// call base class to save bindings
	CJoystickAxisBase::SaveToConsole(pClientDE);
}


LTBOOL	CJoystickAxisBase::IsAxisBound() 
{
	if (!m_bBindingsRead)
		LoadFromConsole(g_pLTClient);
	return (m_nAxis > JOYSTICK_AXIS_NONE);
}