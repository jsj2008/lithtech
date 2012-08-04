#include "MainMenus.h"
#include "MenuJoystickAxis.h"
#include "clientres.h"

// The slider offset
#define MENUJOYSTICK_SLIDEROFFSET	100
#define MENUJOYSTICK_ONOFFOFFSET	100

// Slider ranges
#define DEADZONESLIDERLOW 0 
#define DEADZONESLIDERHIGH 9
#define SENSITIVITYSLIDERLOW 0
#define SENSITIVITYSLIDERHIGH 20

// This is just a test function because I wanted to see that available mouse devices BLB TEMP!!!
/*
void FindMouseControls(CClientDE *pClientDE)
{
	DeviceObject* pObjects = pClientDE->GetDeviceObjects(DEVICETYPE_MOUSE);
	DeviceObject* pObj = pObjects;
	DBOOL bFoundIt = DFALSE;

	while (pObj != NULL)
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL))
		{
			pClientDE->CPrint("Mouse DeviceName=%s ObjectName=%s", pObj->m_DeviceName, pObj->m_ObjectName ); // BLB TEMP
		}
		pObj = pObj->m_pNext;
	}

	if (pObjects != NULL) pClientDE->FreeDeviceObjects (pObjects);
}
*/

/**************************************************************************/
// Joystick Axis info class
/**************************************************************************/

void CMenuJoystickAxisInfo::Init(char* sName, DDWORD nType, float fRangeLow, float fRangeHigh)
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
CMenuJoystickAxisBase::CMenuJoystickAxisBase()
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
	m_bBindingsRead = DFALSE;

	m_bAddDigitalBindingsToAnalog = DFALSE;

	// Data members
	m_nAxis=MENUJOYSTICK_AXIS_NONE;
	m_nSensitivity=1;
	m_nDeadZone=1;
	m_bAnalog=DFALSE;
	m_bInvertAxis=DFALSE;
	m_bCenterOffset=DFALSE;

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
	m_nAxisID=IDS_MENU_JOYSTICK_AXIS;
	m_nSensitivityID=IDS_MENU_JOYSTICK_SENSITIVITY;
	m_nDeadZoneID=IDS_MENU_JOYSTICK_DEADZONE;
	m_nAnalogID=IDS_MENU_JOYSTICK_ANALOG;
	m_nInvertAxisID=IDS_MENU_JOYSTICK_INVERTAXIS;
	m_nCenterOffsetID=IDS_MENU_JOYSTICK_CENTEROFFSETCORRECTION;

	// The controls
	m_pAxisCtrl=DNULL;	
	m_pSensitivityCtrl=DNULL;
	m_pAnalogCtrl=DNULL;
	m_pDeadZoneCtrl=DNULL;
	m_pInvertAxisCtrl=DNULL;
	m_pCenterOffsetCtrl=DNULL;	
}

// Build the menu items for this class
void CMenuJoystickAxisBase::Build(CClientDE *pClientDE, CMenuBase *pDestMenu)
{		
	if (!pDestMenu)
	{
		return;
	}

	// Get the main menus pointer
	CMainMenus *pMainMenus=pDestMenu->GetMainMenus();
	if (!pMainMenus)
	{
		return;
	}

	// figure out the name of the joystick device
	SetDeviceName(pClientDE);

	// create the array of possible axis
	CreateAxisArray(pClientDE);

	// just a test BLB TEMP!!!
//	FindMouseControls(pClientDE);

	// The axis option
	m_pAxisCtrl=BuildAxisOption(pClientDE, pDestMenu, m_nAxisID, &m_nAxis);

	// The invert axis control
	m_pInvertAxisCtrl=pDestMenu->AddOnOffOption(m_nInvertAxisID, pMainMenus->GetSmallFont(), MENUJOYSTICK_ONOFFOFFSET,
												&m_bInvertAxis);

	// The dead zone control
	m_pDeadZoneCtrl=pDestMenu->AddSliderOption(m_nDeadZoneID, pMainMenus->GetSmallFont(), MENUJOYSTICK_SLIDEROFFSET,
											   pMainMenus->GetSurfaceSliderBar(), pMainMenus->GetSurfaceSliderTab(),
											   &m_nDeadZone);
	if (m_pDeadZoneCtrl)
	{
		m_pDeadZoneCtrl->SetSliderRange(DEADZONESLIDERLOW, DEADZONESLIDERHIGH);
		m_pDeadZoneCtrl->SetSliderIncrement(1);
	}

	// Analog on/off
	m_pAnalogCtrl=pDestMenu->AddOnOffOption(m_nAnalogID, pMainMenus->GetSmallFont(), MENUJOYSTICK_ONOFFOFFSET, &m_bAnalog);

	// The sensitivity control
	m_pSensitivityCtrl=pDestMenu->AddSliderOption(m_nSensitivityID, pMainMenus->GetSmallFont(), MENUJOYSTICK_SLIDEROFFSET,
												  pMainMenus->GetSurfaceSliderBar(), pMainMenus->GetSurfaceSliderTab(),
												  &m_nSensitivity);
	if (m_pSensitivityCtrl)
	{
		m_pSensitivityCtrl->SetSliderRange(SENSITIVITYSLIDERLOW, SENSITIVITYSLIDERHIGH);
		m_pSensitivityCtrl->SetSliderIncrement(1);
	}
	

	// The center offset axis control
	m_pCenterOffsetCtrl=pDestMenu->AddOnOffOption(m_nCenterOffsetID, pMainMenus->GetSmallFont(), MENUJOYSTICK_ONOFFOFFSET,
												  &m_bCenterOffset);	
}

// Updates the enable/disable status of each control.
// bJoystickOn indicates if the joystick is enabled in the menus.
void CMenuJoystickAxisBase::UpdateEnable(DBOOL bJoystickOn)
{
	// If the joystick is on the enable all of the controls.
	// The analog status is checked down below which may disable some of the controls.
	DBOOL bEnable=bJoystickOn;

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
	if (m_nAxis == MENUJOYSTICK_AXIS_NONE)
	{
		bEnable=DFALSE;
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
		if (m_pSensitivityCtrl)		m_pSensitivityCtrl->Enable(DFALSE);
		if (m_pCenterOffsetCtrl)	m_pCenterOffsetCtrl->Enable(DFALSE);
	}
}

// Build the menu option that allows the user to select the axis
CLTGUITextItemCtrl *CMenuJoystickAxisBase::BuildAxisOption(CClientDE *pClientDE, CMenuBase *pDestMenu, int nMessageID, int *pnDestValue)
{
	// Get the main menus pointer
	CMainMenus *pMainMenus=pDestMenu->GetMainMenus();
	if (!pMainMenus)
	{
		return DNULL;
	}

	// Load the display string from the string resource
	HSTRING hDisplayString=pClientDE->FormatString(nMessageID);
	if (!hDisplayString)
	{
		return DNULL;
	}

	// Create the control
	CLTGUITextItemCtrl *pCtrl=pDestMenu->AddTextItemOption(DNULL, 0, pMainMenus->GetSmallFont(), 1, DTRUE, pnDestValue);		

	// Add the strings to the control
	int i;
	for (i=0; i < m_nNumJoystickAxis; i++)
	{
		// Construct the string for the option (ie "Select axis [axis name]");
		char szBuffer[2048];
		sprintf(szBuffer, "%s [%s]", pClientDE->GetStringData(hDisplayString), m_aryAxisInfo[i].GetName());

		// Add the string to the control
		HSTRING hTemp=pClientDE->CreateString(szBuffer);
		pCtrl->AddString(hTemp);

		// Free the strings
		pClientDE->FreeString(hTemp);
	}

	return pCtrl;
}

// Set the joystick device name
BOOL CMenuJoystickAxisBase::SetDeviceName(CClientDE *pClientDE)
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
	DBOOL bFoundIt = DFALSE;
	while ((pObj != NULL) && (bDeviceNameFound == FALSE))
	{
		if ((pObj->m_ObjectName != NULL) && (pObj->m_DeviceName != NULL) && (pObj->m_DeviceType == DEVICETYPE_JOYSTICK))
		{
			if (stricmp(pObj->m_DeviceName, m_sDeviceName) == 0) bDeviceNameFound = TRUE;
		}
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
void CMenuJoystickAxisBase::CreateAxisArray(CClientDE *pClientDE)
{
	DeviceObject* pObjects = pClientDE->GetDeviceObjects(DEVICETYPE_JOYSTICK);
	DeviceObject* pObj = pObjects;
	DBOOL bFoundIt = DFALSE;

	// the first axis is always the none axis
	HSTRING hAxisName = pClientDE->FormatString(IDS_MENU_JOYSTICK_AXISTYPE_NONE);
	m_nNumJoystickAxis = 0;
	if (hAxisName != NULL) m_aryAxisInfo[m_nNumJoystickAxis].Init(pClientDE->GetStringData(hAxisName), 0, 0.0f, 0.0f);
	else m_aryAxisInfo[m_nNumJoystickAxis].Init("none", 0, 0.0f, 0.0f);
	m_nNumJoystickAxis++;

	// loop through all joystick objects and store the axis ones with our devicename the aryAxisInfo array
	while ((pObj != NULL) && (m_nNumJoystickAxis < MENUJOYSTICK_MAX_AXIS))
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
void CMenuJoystickAxisBase::SaveNewActions(CClientDE *pClientDE)
{
	pClientDE->RunConsoleString("AddAction AxisYaw -10001");
	pClientDE->RunConsoleString("AddAction AxisPitch -10002");
	pClientDE->RunConsoleString("AddAction AxisLeftRight -10003");
	pClientDE->RunConsoleString("AddAction AxisForwardBackward -10004");
}

// get a binding for a joystick trigger
void CMenuJoystickAxisBase::GetJoystickBindingInfo(CClientDE *pClientDE)
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

			DBOOL		bFound = DFALSE;
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
				bFound = DTRUE;

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
				bFound = DTRUE;

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
				m_bBindingsRead = DTRUE;
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
void CMenuJoystickAxisBase::DoRangeBind(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh)
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
void CMenuJoystickAxisBase::ClearRangeBind(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName)
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
void CMenuJoystickAxisBase::DoRangeScale(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset)
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
void CMenuJoystickAxisBase::SetConsoleVar(CClientDE *pClientDE, char *lpszName, float nVal)
{
	assert(lpszName);

	if (lpszName == NULL) return;

	char tempStr[512];

	// Set the rangescale
	sprintf(tempStr, "+%s %f", lpszName, nVal);
	pClientDE->RunConsoleString(tempStr);
}

// find the index into the axis array for the given axis name (return 0 if not found)
int CMenuJoystickAxisBase::GetAxisIndex(CClientDE *pClientDE, char* sAxisName)
{
	// loop through all axis trying to find the matching axis (skip 0 becaus it is the none axis)
	for (int i = 1; i < m_nNumJoystickAxis; i++)
	{
		if (stricmp(sAxisName, m_aryAxisInfo[i].GetName()) == 0) return i;
	}
	return 0;
}

// scales a value from one range to another in a linear fashion
float CMenuJoystickAxisBase::ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax)
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
void CMenuJoystickAxisBase::LoadFromConsole(CClientDE *pClientDE)
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
			if (m_fRangeScaleMin > m_fRangeScaleMax) m_bInvertAxis = DTRUE;
			else m_bInvertAxis = DFALSE;
		}
		else
		{
			if (stricmp(m_sActionName, m_sActionDigitalLow) == 0)
			{
				if (m_fRangeLow < m_fRangeLow2) m_bInvertAxis = DFALSE;
				else m_bInvertAxis = DTRUE;
			}
			else	
			{
				if (m_fRangeLow <= m_fRangeLow2) m_bInvertAxis = DTRUE;
				else m_bInvertAxis = DFALSE;
			}
		}

		// figure out the correct value of the center offset flag from the data read in
		if ((long)m_fRangeScalePreCenterOffset != 100) m_bCenterOffset = DFALSE;
		else m_bCenterOffset = DTRUE;
	}

	// save off the original values
	m_nOrigAxis=m_nAxis;
	m_nOrigSensitivity=m_nSensitivity;
	m_nOrigDeadZone=m_nDeadZone;
	m_bOrigInvertAxis=m_bInvertAxis;
	m_bOrigCenterOffset=m_bCenterOffset;
	m_bOrigAnalog = m_bAnalog;

	// clear the old binding
	if (m_bBindingsRead)
	{
		if (m_nAxis > MENUJOYSTICK_AXIS_NONE)
		{
			strncpy(m_sTriggerName, m_aryAxisInfo[m_nAxis].GetName(), INPUTNAME_LEN);
			m_sTriggerName[INPUTNAME_LEN-1] = '\0';

			ClearRangeBind(pClientDE, m_sDeviceName, m_sTriggerName);
		}
	}
}


// Save an analog binding to the consle
void CMenuJoystickAxisBase::SaveToConsoleAnalog(CClientDE *pClientDE)
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
void CMenuJoystickAxisBase::SaveToConsoleDigital(CClientDE *pClientDE)
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
		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
				 fRightLow, fRightHigh, m_sActionDigitalLow, fLeftLow, fLeftHigh, m_sActionDigitalHigh);
	}
	else
	{
		sprintf (str, "rangebind \"%s\" \"%s\" \"%f\" \"%f\" \"%s\" \"%f\" \"%f\" \"%s\"", m_sDeviceName, m_sTriggerName,
				 fLeftLow, fLeftHigh, m_sActionDigitalLow, fRightLow, fRightHigh, m_sActionDigitalHigh);
	}

	pClientDE->RunConsoleString (str);
	//pClientDE->CPrint("Joystick Digital Binding : %s", str ); // BLB TEMP
}


// Save variables to the console
void CMenuJoystickAxisBase::SaveToConsole(CClientDE *pClientDE)
{
	// figure out the new trigger name
	strncpy(m_sTriggerName, m_aryAxisInfo[m_nAxis].GetName(), INPUTNAME_LEN);
	m_sTriggerName[INPUTNAME_LEN-1] = '\0';

	// set the new dead zone if it has been adjusted or was never read in
	m_fDeadZone = ScaleToRange((float)m_nDeadZone, (float)DEADZONESLIDERLOW, (float)DEADZONESLIDERHIGH, 0.0f, 0.90f);
	if (m_nDeadZone == DEADZONESLIDERLOW) m_fDeadZone = 0.0f;

	// do the analog or digital binding
	if (m_nAxis > MENUJOYSTICK_AXIS_NONE) 
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
}

/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/

// Load from the console
void CMenuJoystickAxisTurn::LoadFromConsole(CClientDE *pClientDE)
{
	m_bBindingsRead = DFALSE;

	m_bAddDigitalBindingsToAnalog = DFALSE;

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
		m_bInvertAxis = DFALSE;	
		m_nDeadZone = 2;
		m_bAnalog = DFALSE;
		m_nSensitivity = 10;		
		m_bCenterOffset = DFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "Left", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "Right", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisYaw", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisYawDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CMenuJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CMenuJoystickAxisTurn::SaveToConsole(CClientDE *pClientDE)
{
	// call base class to save bindings
	CMenuJoystickAxisBase::SaveToConsole(pClientDE);
}

/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

// Constructor
CMenuJoystickAxisLook::CMenuJoystickAxisLook()
{
	CMenuJoystickAxisBase::CMenuJoystickAxisBase();

	// Members
	m_bFixedPosition=DTRUE;

	// String IDs
	m_nFixedPositionID=IDS_MENU_JOYSTICK_LOOKUPDOWNFIXEDBYPOSITION;

	// Controls
	m_pFixedPositionCtrl=DNULL;

}

// Build the menu items
void CMenuJoystickAxisLook::Build(CClientDE *pClientDE, CMenuBase *pDestMenu)
{
	// Get the main menus pointer
	CMainMenus *pMainMenus=pDestMenu->GetMainMenus();
	if (!pMainMenus)
	{
		return;
	}

	// Call the base class
	CMenuJoystickAxisBase::Build(pClientDE, pDestMenu);

	// The center offset axis control
	m_pFixedPositionCtrl=pDestMenu->AddOnOffOption(m_nFixedPositionID, pMainMenus->GetSmallFont(), MENUJOYSTICK_ONOFFOFFSET,
												   &m_bFixedPosition);
}

// Updates the enable/disable status of each control.
// bJoystickOn indicates if the joystick is enabled in the menus.
void CMenuJoystickAxisLook::UpdateEnable(DBOOL bJoystickOn)
{
	// Call the base class
	CMenuJoystickAxisBase::UpdateEnable(bJoystickOn);

	DBOOL bEnable=DTRUE;
	if (bJoystickOn && m_bAnalog)
	{
		bEnable=DTRUE;
	}
	else
	{
		bEnable=DFALSE;
	}

	if (m_pFixedPositionCtrl)
	{
		m_pFixedPositionCtrl->Enable(bEnable);
	}
}

// Load from the console
void CMenuJoystickAxisLook::LoadFromConsole(CClientDE *pClientDE)
{
	m_bBindingsRead = DFALSE;

	m_bAddDigitalBindingsToAnalog = DFALSE;

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
		m_bInvertAxis = DFALSE;	
		m_nDeadZone = 2;
		m_bAnalog = DFALSE;
		m_nSensitivity = 10;		
		m_bCenterOffset = DFALSE;
	}

	// set the string names for this axis
	strncpy(m_sActionDigitalLow, "LookUp", MAX_ACTIONNAME_LEN);
	m_sActionDigitalLow[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionDigitalHigh, "LookDown", MAX_ACTIONNAME_LEN);
	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sActionAnalog, "AxisPitch", MAX_ACTIONNAME_LEN);
	m_sActionAnalog[MAX_ACTIONNAME_LEN-1] = '\0';

	strncpy(m_sDeadZoneConsoleVar, "AxisPitchDeadZone", MAX_ACTIONNAME_LEN+16);
	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16-1] = '\0';

	// call the base code to load everything in and set up the variables
	CMenuJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CMenuJoystickAxisLook::SaveToConsole(CClientDE *pClientDE)
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
	CMenuJoystickAxisBase::SaveToConsole(pClientDE);

	// write out the fixed axis pitch console variable
	if ((m_bFixedPosition) && (m_bAnalog) && (m_nAxis > MENUJOYSTICK_AXIS_NONE)) SetConsoleVar(pClientDE, "FixedAxisPitch", 1.0);
	else SetConsoleVar(pClientDE, "FixedAxisPitch", 0.0);
}

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

// Contructor
CMenuJoystickAxisMove::CMenuJoystickAxisMove()
{
	CMenuJoystickAxisBase::CMenuJoystickAxisBase();
}

// Load from the console
void CMenuJoystickAxisMove::LoadFromConsole(CClientDE *pClientDE)
{
	m_bBindingsRead = DFALSE;

	m_bAddDigitalBindingsToAnalog = DTRUE;

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
		m_bInvertAxis = DFALSE;	
		m_nDeadZone = 2;
		m_bAnalog = DFALSE;
		m_nSensitivity = 10;		
		m_bCenterOffset = DFALSE;
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
	CMenuJoystickAxisBase::LoadFromConsole(pClientDE);
}

// Save to the console
void CMenuJoystickAxisMove::SaveToConsole(CClientDE *pClientDE)
{
	// call base class to save bindings
	CMenuJoystickAxisBase::SaveToConsole(pClientDE);
}

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/

void CMenuJoystickAxisStrafe::LoadFromConsole(CClientDE *pClientDE)
{
	m_bBindingsRead = DFALSE;

	m_bAddDigitalBindingsToAnalog = DTRUE;

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
		m_bInvertAxis = DFALSE;	
		m_nDeadZone = 2;
		m_bAnalog = DFALSE;
		m_nSensitivity = 10;		
		m_bCenterOffset = DFALSE;
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
	CMenuJoystickAxisBase::LoadFromConsole(pClientDE);
}

void CMenuJoystickAxisStrafe::SaveToConsole(CClientDE *pClientDE)
{
	// call base class to save bindings
	CMenuJoystickAxisBase::SaveToConsole(pClientDE);
}
