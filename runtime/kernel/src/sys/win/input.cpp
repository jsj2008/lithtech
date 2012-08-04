//------------------------------------------------------------------
//
//	FILE	  : Input.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : May 17 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "dinput.h"
#include "concommand.h"
#include "console.h"
#include "ltpvalue.h"
#include "input.h"

/*

	LithTech supports:

		3 axis -- x,y (mouse/joystick), and z
		toggle buttons - trigger actions
		push buttons - trigger actions

		Now, an actioncode of -10000 or less uses a console variable with the trigger name!
*/

extern int32 g_CV_InputRate;
extern int32 g_CV_JoystickDisable;
extern int32 g_CV_InputDebug;


class CJoystickEffect
{
	public:

		CJoystickEffect()
		{
			memset (m_EffectName, 0, 256);
			m_pEffect = LTNULL;
			m_pNext = LTNULL;
		}

		TCHAR					m_EffectName[256];
		LPDIRECTINPUTEFFECT		m_pEffect;

		CJoystickEffect*		m_pNext;
};


enum InputType
{
	Input_PushButton=0,
	Input_ToggleButton=1,
	Input_AbsAxis=2,
	Input_RelAxis=3,
	Input_Pov=4
};


struct TrackObjectInfo
{
	GUID		guidType;
	uint32		dwOfs;
	uint32		dwType;
	TCHAR		tszName[MAX_PATH];

};

// moved to basedefs_de.h
//#define INPUTNAME_LEN		100

#define MAX_OBJECT_BINDINGS	150
#define INPUT_BUFFER_SIZE	64

// --------------------------------------------------------------------- //
// Action definitions
// --------------------------------------------------------------------- //

// moved to basedefs_de.h
//#define MAX_ACTIONNAME_LEN	30

class ActionDef
{
	public:

				ActionDef()
				{
					m_pPrev = m_pNext = this;
				}

		int		m_ActionCode;
		char	m_ActionName[MAX_ACTIONNAME_LEN];

		ActionDef	*m_pPrev, *m_pNext;

};

ActionDef g_ActionDefHead;


static ConsoleState		*g_pInputConsoleState;


// --------------------------------------------------------------------- //
// Device-to-action mappings
// --------------------------------------------------------------------- //

class CTriggerActionConsoleVar;

// list of all the console variables that are used in the TriggerAction
static CTriggerActionConsoleVar*	g_lstTriggerActionConsoleVariables = LTNULL;

class TriggerAction
{
	public:

						TriggerAction()
						{
							m_pConsoleString = LTNULL;
							m_pAction = LTNULL;
							m_pNext = LTNULL;
							m_pConsoleVar = LTNULL;
						}

						inline ~TriggerAction();

		// Both zero if it's not using range.
		float			m_RangeLow, m_RangeHigh;

		// If this is non-LTNULL, then the action is a console string instead of an ActionDef.
		char			*m_pConsoleString; 
		
		ActionDef		*m_pAction;
		TriggerAction	*m_pNext;

		// Contains a pointer to a trigger console var structure that needs to be updated
		// this currently only works for axis types
		// Set to LTNULL if there is no console variable to update.
		CTriggerActionConsoleVar*	m_pConsoleVar;

};

class DeviceDef;
class TriggerObject : public CGLLNode
{
	public:

						TriggerObject()
						{
							m_State = 0.0f;
							m_PrevState = 0.0f;
							m_BaseState = 0.0f;
							m_bJustWentDown = true;
							m_dwUpdateTime = 0;
							m_dwPrevUpdateTime = 0;
							m_dwBaseUpdateTime = 0;
							m_Scale = 1.0f;
						}

						~TriggerObject()
						{
							TriggerAction	*pCur, *pNext;

							pCur = m_pActionHead;
							while( pCur )
							{
								pNext = pCur->m_pNext;
								delete pCur;
								pCur = pNext;
							}
						}

		// Info on the trigger (key, mouse axis, or mouse button...)
		TCHAR			m_TriggerName[INPUTNAME_LEN];
		TCHAR			m_RealName[INPUTNAME_LEN];	// Alternate name for it (like if m_TriggerName is ##21, this could be 'Space bar').
		uint32			m_diType;
		GUID			m_diGuid;
		InputType		m_InputType;	// What DirectEngine sees it as.
		uint32			m_StateIndex;	// index in the object state data array

		float			m_State;		// The current state of the trigger
		float			m_PrevState;	// Previous state of the trigger
		float			m_BaseState;
		
		uint32			m_dwUpdateTime;	// The time of the most recent update.
		uint32			m_dwPrevUpdateTime; // The time of the previous update
		uint32			m_dwBaseUpdateTime;
		
		bool			m_bJustWentDown;

		// If it's a relative axis trigger, this scales the value.
		float			m_Scale;

		// scale an axis value to a range if these are not both 0.0
		float			m_fRangeScaleMin;
		float			m_fRangeScaleMax;
		float			m_fRangeScaleMultiplier;
		float			m_fRangeScaleOffset;
		float			m_fRangeScaleMultiplierLo;
		float			m_fRangeScaleOffsetLo;
		float			m_fRangeScaleMultiplierHi;
		float			m_fRangeScaleOffsetHi;
		float			m_fRangeScalePreCenterOffset;
		float			m_fRangeScalePreCenter;

		// Which device it comes from.
		DeviceDef		*m_pDevice;

		// The actions it will trigger.
		TriggerAction	*m_pActionHead;

		// Data range.
		float		m_DataMin;
		float		m_DataMax;

};


// Trigger objects that have been created.
static TriggerObject	g_TriggerHead;

// class that holds records for storing trigger actions in a console variable
class CTriggerActionConsoleVar
{
	public:
					CTriggerActionConsoleVar()
					{
						m_pTriggerAction = LTNULL;
						m_pCommandVar = LTNULL;
					}

					~CTriggerActionConsoleVar()
					{
						m_pTriggerAction = LTNULL;
						m_pCommandVar = LTNULL;

						// remove from global list
						if (g_lstTriggerActionConsoleVariables == this)
						{
							g_lstTriggerActionConsoleVariables = this->m_pNext;
						}
						else
						{
							CTriggerActionConsoleVar* pConVar = g_lstTriggerActionConsoleVariables;
							while (pConVar != LTNULL)
							{
								if (pConVar->m_pNext == this)
								{
									pConVar->m_pNext = this->m_pNext;
									pConVar = LTNULL;
								}
								else pConVar = pConVar->m_pNext;
							}
						}
					}

					void InitCommandVar()
					{
						m_pCommandVar = cc_FindConsoleVar(g_pInputConsoleState, GetCommandName());
					}

					char* GetCommandName()
					{
						if (m_pTriggerAction == LTNULL) return LTNULL;
						return m_pTriggerAction->m_pAction->m_ActionName;
					}

					void SetFloat(float nVal)
					{
						char sConValue[100];

						if (m_pTriggerAction != LTNULL)
						{
							LTSNPrintF(sConValue, sizeof(sConValue),"%f", nVal);
							cc_SetConsoleVariable(g_pInputConsoleState, GetCommandName(), sConValue);
						}
					}

					float GetFloat()
					{
						if (m_pCommandVar == LTNULL) return 0.0;
						else return m_pCommandVar->floatVal;
					}

		// trigger action that contains this console var
		TriggerAction*				m_pTriggerAction;

		// pointer to data structure that holds information about the console variable
		LTCommandVar*				m_pCommandVar;

		// pointer to next item in the list
		CTriggerActionConsoleVar*	m_pNext;
};


class DeviceDef
{
	public:

					DeviceDef()
					{
						m_pDevice = LTNULL;
						m_bTracking = false;
						m_bTrackingOnly = false;
						m_dwLastTime = GetTickCount( );
						m_bSysMouse = false;
						m_pTrackObjects = LTNULL;
						m_nTrackObjects = 0;
					}

					~DeviceDef()
					{
						GPOS pos;

						for(pos=m_Triggers; pos; )
						{
							delete m_Triggers.GetNext(pos);
						}

						m_Triggers.RemoveAll();
					}

		bool		IsEnabled()		{ return !!m_pDevice; }
		bool		IsJoystick()	{ return DIEFT_GETTYPE(m_DeviceType) == DI8DEVTYPE_JOYSTICK; }
		bool		IsGamepad()	{ return DIEFT_GETTYPE(m_DeviceType) == DI8DEVTYPE_GAMEPAD; }

		// LTNULL if this device hasn't been enabled.
		LPDIRECTINPUTDEVICE8	m_pDevice;

		// If it's referenced by one of the special names (##mouse or ##keyboard),
		// then it's stored here and should be saved in the bindings as such.
		char		*m_pSpecialName;

		// How many bytes used to read a state (GetDeviceState()).
		uint32		m_StateReadSize;

		TCHAR		m_InstanceName[INPUTNAME_LEN];
		GUID		m_InstanceGuid;
		uint32		m_DeviceType;

		CGLinkedList<TriggerObject*>	m_Triggers;

		// Trigger object pointers (indexed by device data offset / 4)
		TriggerObject*	m_TriggerTable[MAX_OBJECT_BINDINGS];

		// Are we tracking this device?
		bool		m_bTracking;

		// Is this device enabled solely for tracking purposes?
		bool		m_bTrackingOnly;

		// Info on all objects for this device
		TrackObjectInfo*	m_pTrackObjects;
		uint32				m_nTrackObjects;

		uint32		m_dwLastTime;
		
		bool			m_bSysMouse;
		DeviceDef		*m_pNext;

};

// DirectInput stuff.

	// The window input is attached to.
	static HWND				g_InputHWND=LTNULL;

	// The main DirectInput object.
	static LPDIRECTINPUT8	g_pDirectInput=LTNULL;

	// Devices we've enumerated.
	static DeviceDef		*g_pDeviceHead=LTNULL;

	// Force-Feedback effects
	static CJoystickEffect	*g_pJoystickEffects=LTNULL;

	// Useful for remembering current device when enumerating device objects
	static DeviceDef		*g_pCurrentEnumDevice=LTNULL;



TriggerAction::~TriggerAction()
{
	if(m_pConsoleString)
		delete m_pConsoleString;

	// remove the console var
	if (m_pConsoleVar != LTNULL)
	{
		delete m_pConsoleVar;
		m_pConsoleVar = LTNULL;
	}
}

// --------------------------------------------------------------------- //
// Helpers.
// --------------------------------------------------------------------- //

static ActionDef* input_FindAction( const char *pActionName )
{
	ActionDef	*pCur;

	pCur = g_ActionDefHead.m_pNext;
	while(pCur != &g_ActionDefHead)
	{
		if( CHelpers::UpperStrcmp(pActionName, pCur->m_ActionName) )
			return pCur;

		pCur = pCur->m_pNext;
	}

	return LTNULL;
}


static DeviceDef* input_FindDeviceByType( uint32 nType )
{
	// Go through the first time looking for non-HID versions (on Win9x there are two types)...
	DeviceDef* pDev = g_pDeviceHead;
	while (pDev) {
		if (DIEFT_GETTYPE(pDev->m_DeviceType) == nType) {
			// Make sure it's not HID (usb) ... even if it is HID,
			// there will be a non-HID version as well and we need to use that one.
			if (!(pDev->m_DeviceType & DIDEVTYPE_HID)) {
				return pDev; } }
		pDev = pDev->m_pNext; }

 	// Do second pass for device, checking only HID devices.  This is a Win2K fix as all devices
 	//	on Win2K will be HID devices due to Win2ks plug and play manager.
 	//	This may also fix some Win9x issues as well.
 	pDev = g_pDeviceHead;
 	while (pDev) {
 		if (DIEFT_GETTYPE( pDev->m_DeviceType ) == nType) {
 			// check for HID devices only... if we had a non-HID device, we would not have gotten here
 			if ((pDev->m_DeviceType & DIDEVTYPE_HID)) {
				return pDev; } }
  		pDev = pDev->m_pNext; }
 
	return LTNULL;
}

static DeviceDef* FindDeviceByTypeWithSkip( uint32 nType, int nSkipNum = 0 )
{
	DeviceDef	*pDev;

	pDev = g_pDeviceHead;
	while( pDev )
	{
		if( DIEFT_GETTYPE( pDev->m_DeviceType ) == nType )
		{
			if (nSkipNum <= 0) return pDev;
			else nSkipNum--;
		}

		pDev = pDev->m_pNext;
	}
	
	return LTNULL;
}

static DeviceDef* input_FindDeviceByFlags( uint32 flags )
{
	DeviceDef	*pDev;

	pDev = g_pDeviceHead;
	while( pDev )
	{
		if( pDev->m_DeviceType & flags )
			return pDev;

		pDev = pDev->m_pNext;
	}

	return LTNULL;
}


static DeviceDef* input_FindDeviceByName( const TCHAR *pName )
{
	DeviceDef *pDev;

	// Look for it with the normal name.
	pDev = g_pDeviceHead;
	while( pDev )
	{
		if( CHelpers::UpperStrcmp(pName, pDev->m_InstanceName) )
			return pDev;
	
		pDev = pDev->m_pNext;
	}

	// Look for it by its special name.
	if(stricmp(pName, "##mouse") == 0)
	{
		pDev = input_FindDeviceByType(DI8DEVTYPE_MOUSE);
		if(pDev)
			pDev->m_pSpecialName = "##mouse";
		
		return pDev;
	}
	else if(stricmp(pName, "##keyboard") == 0)
	{
		pDev = input_FindDeviceByType(DI8DEVTYPE_KEYBOARD);
		if(pDev)
			pDev->m_pSpecialName = "##keyboard";
		return pDev;
	}
	// KEF - 1/4/00 - Evil hack to handle Win98 SE not defining "Joystick 1"
	else if(strnicmp(pName, "Joystick 1", 8) == 0)
	{
		pDev = input_FindDeviceByType(DI8DEVTYPE_JOYSTICK);
		if(pDev)
			pDev->m_pSpecialName = "Joystick 1";

		return pDev;
	}

	return LTNULL;
}


static TriggerObject* input_FindTrigger(DeviceDef *pDef, const char *pName)
{
	TriggerObject *pCur;
	GPOS pos;

	// Look for it the normal way..
	for(pos=pDef->m_Triggers; pos; )
	{
		pCur = pDef->m_Triggers.GetNext(pos);

		if(stricmp(pCur->m_TriggerName, pName)==0 || stricmp(pCur->m_RealName, pName)==0)
			return pCur;
	}

	return LTNULL;
}

// Forward declarations
LTRESULT input_GetManager(InputMgr **pMgr);


struct DeviceObjectEnum_Context
{
	bool m_bObjectFound;
	bool m_bLookingForSpecial;
	uint32 m_nSpecialType;
	uint32 m_nSpecialOffset;
	GUID m_SpecialGuid;
	bool m_bSpecialGuid;
	char const* m_pszLookingFor;
	uint32 m_nType;
};

static BOOL CALLBACK DeviceObjectEnumCallback( LPCDIDEVICEOBJECTINSTANCE pObj, LPVOID pvRef )
{
	DeviceObjectEnum_Context* pContext = ( DeviceObjectEnum_Context* )pvRef;

	if( pContext->m_bLookingForSpecial )
	{
		if( pContext->m_bSpecialGuid )
		{
			if( pObj->guidType == pContext->m_SpecialGuid )
			{
				// Make sure we have the right offset.
				if( pContext->m_nSpecialOffset != 0xFFFFFFFF)
				{
					if(pObj->dwOfs != pContext->m_nSpecialOffset )
					{
						return DIENUM_CONTINUE;
					}
				}

				pContext->m_bObjectFound = true;
				pContext->m_nType = pObj->dwType;
				return DIENUM_STOP;
			}
		}
		else if( pContext->m_nSpecialType != 0xFFFFFFFF)
		{
			if(DIDFT_GETINSTANCE(pObj->dwType) == pContext->m_nSpecialType )
			{
				pContext->m_bObjectFound = true;
				pContext->m_nType = pObj->dwType;
				return DIENUM_STOP;
			}
		}
		else if( pContext->m_nSpecialOffset != 0xFFFFFFFF)
		{
			if( pObj->dwOfs == pContext->m_nSpecialOffset )
			{
				pContext->m_bObjectFound = true;
				pContext->m_nType = pObj->dwType;
				return DIENUM_STOP;
			}
		}
	}
	else
	{
		if(stricmp( pObj->tszName, pContext->m_pszLookingFor ) == 0)
		{
			pContext->m_bObjectFound = true;
			pContext->m_nType = pObj->dwType;
			return DIENUM_STOP;
		}
	}

	return DIENUM_CONTINUE;		
}

 
static TriggerObject* input_AddTrigger(DeviceDef *pDevice, const char *pTriggerName)
{
	HRESULT hResult;
	TriggerObject *pTrigger;
	int inputType;
	DIPROPRANGE dipr;
	DIDEVICEOBJECTINSTANCE deviceEnumFindings;
	DeviceObjectEnum_Context context;
	memset( &context, 0, sizeof( context ));

	
	ASSERT( pDevice->IsEnabled() );

	context.m_bLookingForSpecial = false;
	context.m_pszLookingFor = pTriggerName;

	// Setup for special types..
	if(pTriggerName[0] != 0 && pTriggerName[1] != 0)
	{
		if(pTriggerName[0] == '#' && pTriggerName[1] == '#')
		{
			context.m_bLookingForSpecial = true;
			context.m_nSpecialType = context.m_nSpecialOffset = 0xFFFFFFFF;
			context.m_bSpecialGuid = false;
			
			if(stricmp(&pTriggerName[2], "x-axis") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_XAxis;
			}
			else if(stricmp(&pTriggerName[2], "y-axis") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_YAxis;
			}
			else if(stricmp(&pTriggerName[2], "z-axis") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_ZAxis;
			}
			else if(stricmp(&pTriggerName[2], "Button 0") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_Button;
				context.m_nSpecialOffset = DIMOFS_BUTTON0;
			}
			else if(stricmp(&pTriggerName[2], "Button 1") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_Button;
				context.m_nSpecialOffset = DIMOFS_BUTTON1;
			}
			else if(stricmp(&pTriggerName[2], "Button 2") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_Button;
				context.m_nSpecialOffset = DIMOFS_BUTTON2;
			}
			else if(stricmp(&pTriggerName[2], "Slider 0") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_Slider;
				context.m_nSpecialOffset = DIJOFS_SLIDER(0);
			}
			else if(stricmp(&pTriggerName[2], "Slider 1") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_Slider;
				context.m_nSpecialOffset = DIJOFS_SLIDER(1);
			}
			else if(stricmp(&pTriggerName[2], "POV 0") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_POV;
				context.m_nSpecialOffset = DIJOFS_POV(0);
			}
			else if(stricmp(&pTriggerName[2], "POV 1") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_POV;
				context.m_nSpecialOffset = DIJOFS_POV(1);
			}
			else if(stricmp(&pTriggerName[2], "POV 2") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_POV;
				context.m_nSpecialOffset = DIJOFS_POV(2);
			}
			else if(stricmp(&pTriggerName[2], "POV 3") == 0)
			{
				context.m_bSpecialGuid = true;
				context.m_SpecialGuid = GUID_POV;
				context.m_nSpecialOffset = DIJOFS_POV(3);
			}
			else if(isalnum(pTriggerName[2]))
			{
				context.m_nSpecialType = (uint32)atoi(&pTriggerName[2]);
			}
		}
	}
	
	// Enumerate objects on this device.
	context.m_bObjectFound = false;
	hResult = pDevice->m_pDevice->EnumObjects( DeviceObjectEnumCallback, (void*)&context, DIDFT_ALL );
	if( hResult != DI_OK || !context.m_bObjectFound )
		return LTNULL;

	// Get the object found.  We don't trust the name found during enumeration.  It was
	// causing bugs on WinXP.  This seems more reliable.
	memset( &deviceEnumFindings, 0, sizeof( deviceEnumFindings ));
	deviceEnumFindings.dwSize = sizeof( deviceEnumFindings );
	hResult = pDevice->m_pDevice->GetObjectInfo( &deviceEnumFindings, context.m_nType, DIPH_BYID );
	if( hResult != DI_OK )
		return LTNULL;

	// Found an object with that name.  Make sure DirectEngine supports its type of input.
	inputType = -1;
	if( deviceEnumFindings.dwType & DIDFT_PSHBUTTON )
		inputType = Input_PushButton;
	else if( deviceEnumFindings.dwType & DIDFT_TGLBUTTON )
		inputType = Input_ToggleButton;
	else if( deviceEnumFindings.dwType & DIDFT_RELAXIS )
		inputType = Input_RelAxis;
	else if( deviceEnumFindings.dwType & DIDFT_ABSAXIS )
		inputType = Input_AbsAxis;
	else if( deviceEnumFindings.dwType & DIDFT_POV )
		inputType = Input_Pov;
	
	if( inputType == -1 )
		return LTNULL;
	
	// Setup a TriggerObject for it.
	LT_MEM_TRACK_ALLOC(pTrigger = new TriggerObject,LT_MEM_TYPE_INPUT);
	memset( pTrigger, 0, sizeof(TriggerObject) );

	pDevice->m_Triggers.AddHead(pTrigger);
	
	LTStrCpy(pTrigger->m_RealName, pTriggerName, INPUTNAME_LEN);
	LTStrCpy(pTrigger->m_TriggerName, deviceEnumFindings.tszName, INPUTNAME_LEN);

	pTrigger->m_InputType = (InputType)inputType;
	memcpy( &pTrigger->m_diGuid, &deviceEnumFindings.guidType, sizeof(GUID) );
	
	pTrigger->m_diType = deviceEnumFindings.dwType;

	pTrigger->m_pDevice = pDevice;
	pTrigger->m_Scale = 1.0f;
	pTrigger->m_fRangeScaleMin = 0.0f;
	pTrigger->m_fRangeScaleMax = 0.0f;
	pTrigger->m_fRangeScaleMultiplier = 1.0f;
	pTrigger->m_fRangeScaleOffset = 0.0f;
	pTrigger->m_fRangeScaleMultiplierHi = 1.0f;
	pTrigger->m_fRangeScaleOffsetHi = 0.0f;
	pTrigger->m_fRangeScaleMultiplierLo = 1.0f;
	pTrigger->m_fRangeScaleOffsetLo = 0.0f;
	pTrigger->m_fRangeScalePreCenterOffset = 0.0f;
	pTrigger->m_fRangeScalePreCenter = 0.0f;

	// Get the trigger max range.
	pTrigger->m_DataMin = ( float )DIPROPRANGE_NOMIN;
	pTrigger->m_DataMax = ( float )DIPROPRANGE_NOMAX;
	if (pDevice->m_pDevice && ( inputType == Input_AbsAxis || inputType == Input_Pov ))
	{
		dipr.diph.dwSize = sizeof( DIPROPRANGE );
		dipr.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		dipr.diph.dwObj = deviceEnumFindings.dwType;
		dipr.diph.dwHow = DIPH_BYID;
		hResult = pDevice->m_pDevice->GetProperty( DIPROP_RANGE, &dipr.diph );
		if( hResult == DI_OK )
		{
			pTrigger->m_DataMin = ( float )dipr.lMin;
			pTrigger->m_DataMax = ( float )dipr.lMax;
		}
	}

	return pTrigger;
}



// --------------------------------------------------------------------- //
// The main function that sets up how we'll communicate with DirectInput
// about the device objects.
// --------------------------------------------------------------------- //

static bool input_SetupDeviceFormats( DeviceDef *pDevice )
{
	DIOBJECTDATAFORMAT objectFormats[MAX_OBJECT_BINDINGS];
	DIDATAFORMAT format;
	DIPROPDWORD prop;
	TriggerObject *pTrigger;
	int iTrigger;
	HRESULT hResult;
	uint32 curOffset;
	uint32 dwStates[MAX_OBJECT_BINDINGS];
	bool bDuplicate;
	TriggerObject *ptr;
	GPOS pos, pos2;

	
	ASSERT( pDevice->IsEnabled() );


	// Unacquire it.
	pDevice->m_pDevice->Unacquire();

	// Reset the TriggerTable
	memset( pDevice->m_TriggerTable, 0, sizeof(TriggerObject*) * MAX_OBJECT_BINDINGS );
	
	// Setup the object formats.
	memset( objectFormats, 0, sizeof(DIOBJECTDATAFORMAT)*MAX_OBJECT_BINDINGS );
	
	iTrigger = 0;
	curOffset = 0;
	
	for(pos=pDevice->m_Triggers; pos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(pos);

		if( iTrigger >= MAX_OBJECT_BINDINGS )
			break;

		// make sure we haven't already created on object data instance for this object
		bDuplicate = false;
		for(pos2=pDevice->m_Triggers; pos2; )
		{
			ptr = pDevice->m_Triggers.GetNext(pos2);

			if( ptr == pTrigger ) 
				break;
			
			if(stricmp(ptr->m_TriggerName, pTrigger->m_TriggerName) == 0 ||
				stricmp(ptr->m_RealName, pTrigger->m_RealName) == 0 )
			{
				pTrigger->m_StateIndex = ptr->m_StateIndex;
				bDuplicate = true;
				break;
			}
		}
		
		if( bDuplicate )
		{
			continue;
		}

		// create the object data instance
		objectFormats[iTrigger].pguid = &pTrigger->m_diGuid;
		objectFormats[iTrigger].dwOfs = curOffset;
		objectFormats[iTrigger].dwType = pTrigger->m_diType;
		pTrigger->m_StateIndex = iTrigger;
		
		// put the pointer to this trigger in the TriggerTable
		pDevice->m_TriggerTable[curOffset >> 2] = pTrigger;

		++iTrigger;
		curOffset += 4;
	}

	
	// Set format.
	memset( &format, 0, sizeof(format) );
	format.dwSize = sizeof(DIDATAFORMAT);
	format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
	format.rgodf = objectFormats;
	format.dwNumObjs = iTrigger;
	format.dwFlags = (pDevice->IsJoystick() || pDevice->IsGamepad() )? DIDF_ABSAXIS : DIDF_RELAXIS;
	format.dwDataSize = curOffset;

	hResult = pDevice->m_pDevice->SetDataFormat( &format );
	if( hResult != DI_OK )
		return false;

	pDevice->m_StateReadSize = curOffset;

	// Set the device properties
	prop.diph.dwSize = sizeof (DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof (DIPROPHEADER);
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = INPUT_BUFFER_SIZE;

	hResult = pDevice->m_pDevice->SetProperty (DIPROP_BUFFERSIZE, &prop.diph);
	hResult = pDevice->m_pDevice->GetProperty (DIPROP_BUFFERSIZE, &prop.diph);
	if( prop.dwData == 0 )
	{
		return false;
	}


	// Init some data in case we can't read the current state.
	for(pos=pDevice->m_Triggers; pos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(pos);

		if ((pTrigger->m_InputType == Input_RelAxis) || (pTrigger->m_InputType == Input_AbsAxis))
		{
			pTrigger->m_State = ((pTrigger->m_DataMax - pTrigger->m_DataMin) / 2.0f) + pTrigger->m_fRangeScalePreCenterOffset;
			pTrigger->m_PrevState = pTrigger->m_State;
			pTrigger->m_BaseState = pTrigger->m_PrevState;
		}
		else 
		{
			pTrigger->m_State = 0.0f;
			pTrigger->m_PrevState = 0.0f;
			pTrigger->m_BaseState = 0.0f;
		}
	}

	
	// Re-acquire it.
	hResult = pDevice->m_pDevice->Acquire();
	if(hResult != DI_OK)
	{
		if(hResult == DIERR_OTHERAPPHASPRIO)
		{
			// Not a big deal, we just don't have focus.
			return true;
		}
		else
		{
			// Some bad error..
			return false;
		}
	}

	// Init the states of all the triggers for this device
	hResult = pDevice->m_pDevice->GetDeviceState( pDevice->m_StateReadSize, dwStates );
	if( hResult == DI_OK )
	{
//		dsi_ConsolePrint("SetupDeviceFormats GetDeviceState Succeeded for %s", pDevice->m_InstanceName); // BLB TEMP
		for(pos=pDevice->m_Triggers; pos; )
		{
			pTrigger = pDevice->m_Triggers.GetNext(pos);
			if ((pTrigger->m_InputType == Input_RelAxis) || (pTrigger->m_InputType == Input_AbsAxis))
			{
				pTrigger->m_State = (float)dwStates[pTrigger->m_StateIndex];
				pTrigger->m_PrevState = pTrigger->m_State;
				pTrigger->m_BaseState = pTrigger->m_State;
			}
			else 
			{
				pTrigger->m_State = (float)dwStates[pTrigger->m_StateIndex];
				pTrigger->m_PrevState = pTrigger->m_State;
				pTrigger->m_BaseState = pTrigger->m_State;
			}
		}
	}

	// BLB TEMP
//	for(pos=pDevice->m_Triggers; pos; )
//	{
//		pTrigger = pDevice->m_Triggers.GetNext(pos);
//		dsi_ConsolePrint("SetupDeviceFormats m_RealName=%s m_State=%20.6f", pTrigger->m_RealName, (double)pTrigger->m_State); // BLB TEMP
//	}

	return true;
}


static BOOL CALLBACK DeviceEnumCallback( LPCDIDEVICEINSTANCE pDevice, LPVOID pvRef )
{
	DeviceDef *pDef;

	LT_MEM_TRACK_ALLOC(pDef = new DeviceDef,LT_MEM_TYPE_INPUT);
	memset( pDef, 0, sizeof(DeviceDef) );
	LTStrCpy( pDef->m_InstanceName, pDevice->tszInstanceName, INPUTNAME_LEN );
	memcpy( &pDef->m_InstanceGuid, &pDevice->guidInstance, sizeof(GUID) );
	pDef->m_DeviceType = pDevice->dwDevType;

	pDef->m_pNext = g_pDeviceHead;
	g_pDeviceHead = pDef;

	return DIENUM_CONTINUE;
}


#define TraceIf(x)	\
	if(hResult==x && g_DebugLevel > 13) \
		con_Printf(CONRGB(255,255,255), 1, #x);

static void input_TraceDInputError( HRESULT hResult )
{
	TraceIf(DIERR_INPUTLOST);
	TraceIf(DIERR_INVALIDPARAM);
	TraceIf(DIERR_NOTACQUIRED);
	TraceIf(E_PENDING);
	TraceIf(DI_POLLEDDEVICE);
	TraceIf(DI_DOWNLOADSKIPPED);
	TraceIf(DI_EFFECTRESTARTED);
	TraceIf(DI_TRUNCATED);
	TraceIf(DI_TRUNCATEDANDRESTARTED);
	TraceIf(DIERR_OLDDIRECTINPUTVERSION);
	TraceIf(DIERR_BETADIRECTINPUTVERSION);
	TraceIf(DIERR_BADDRIVERVER);
	TraceIf(DIERR_DEVICENOTREG);
	TraceIf(DIERR_NOTFOUND);
	TraceIf(DIERR_OBJECTNOTFOUND);
	TraceIf(DIERR_NOINTERFACE);
	TraceIf(DIERR_GENERIC);
	TraceIf(DIERR_OUTOFMEMORY);
	TraceIf(DIERR_UNSUPPORTED);
	TraceIf(DIERR_NOTINITIALIZED);
	TraceIf(DIERR_ALREADYINITIALIZED);
	TraceIf(DIERR_NOAGGREGATION);
	TraceIf(DIERR_OTHERAPPHASPRIO);
	TraceIf(DIERR_INPUTLOST);
	TraceIf(DIERR_NOTACQUIRED);
	TraceIf(DIERR_READONLY);
	TraceIf(DIERR_HANDLEEXISTS);
	TraceIf(DIERR_INSUFFICIENTPRIVS);
	TraceIf(DIERR_DEVICEFULL);
	TraceIf(DIERR_MOREDATA);
	TraceIf(DIERR_NOTDOWNLOADED);
	TraceIf(DIERR_HASEFFECTS);
	TraceIf(DIERR_NOTEXCLUSIVEACQUIRED);
	TraceIf(DIERR_INCOMPLETEEFFECT);
	TraceIf(DIERR_NOTBUFFERED);
}


static char* GetDeviceObjectTypeString(LPCDIDEVICEOBJECTINSTANCE pObj)
{
	if(pObj->guidType == GUID_XAxis)
		return "X axis";
	
	else if(pObj->guidType == GUID_YAxis)
		return "Y axis";

	else if(pObj->guidType == GUID_ZAxis)
		return "Z axis";

	else if(pObj->guidType == GUID_RxAxis)
		return "X axis rotation";

	else if(pObj->guidType == GUID_RyAxis)
		return "Y axis rotation";

	else if(pObj->guidType == GUID_RzAxis)
		return "Z axis rotation";

	else if(pObj->guidType == GUID_Slider)
		return "slider";

	else if(pObj->guidType == GUID_Button)
		return "button";

	else if(pObj->guidType == GUID_Key)
		return "key";

	else if(pObj->guidType == GUID_POV)
		return "POV hat";

	else
		return "unknown";
}


static BOOL CALLBACK ListDeviceObjectsEnumCallback(LPCDIDEVICEOBJECTINSTANCE pObj, LPVOID pvRef)
{
	dsi_ConsolePrint("-    Object %s (type: %s)", pObj->tszName, GetDeviceObjectTypeString(pObj));
	return DIENUM_CONTINUE;		
}


static BOOL CALLBACK ListDevicesEnumCallback(LPCDIDEVICEINSTANCE pDevice, LPVOID pvRef)
{
	HRESULT hResult;
	LPDIRECTINPUTDEVICE8 pDIDevice;

	// Try to create it.
	hResult = g_pDirectInput->CreateDevice(pDevice->guidInstance, &pDIDevice, LTNULL);
	if(hResult == DI_OK)
	{
		dsi_ConsolePrint("- Device %s", pDevice->tszInstanceName);

		hResult = pDIDevice->EnumObjects(ListDeviceObjectsEnumCallback, LTNULL, DIDFT_ALL);

		pDIDevice->Release();
	}
	else
	{
		dsi_ConsolePrint("- Device %s (unable to create)", pDevice->tszInstanceName);
	}

	return DIENUM_CONTINUE;
}



// --------------------------------------------------------------------- //
// Term..
// --------------------------------------------------------------------- //

void input_Term(InputMgr *pMgr)
{
	DeviceDef		*pDef, *pNext;
	ActionDef		*pCurAction, *pNextAction;

	g_pInputConsoleState = LTNULL;
	
	// Clear the device defs.
	pDef = g_pDeviceHead;
	while( pDef )
	{
		pNext = pDef->m_pNext;
		
		if( pDef->m_pDevice )
		{
			pDef->m_pDevice->Unacquire();
			pDef->m_pDevice->Release();
		}

		delete pDef;
		
		pDef = pNext;
	}
	g_pDeviceHead = LTNULL;

	// clear the effects
	CJoystickEffect* pEffect = g_pJoystickEffects;
	CJoystickEffect* pPrev = LTNULL;
	while (pEffect)
	{
		if (pEffect->m_pEffect)
		{
			pEffect->m_pEffect->Unload();
			pEffect->m_pEffect->Release();
		}
		pEffect->m_pEffect = LTNULL;
		pPrev = pEffect;
		pEffect = pEffect->m_pNext;
		delete pPrev;
	}
	g_pJoystickEffects = LTNULL; 
	
	
	// Free DirectInput stuff.
	if( g_pDirectInput )
	{
		g_pDirectInput->Release();
		g_pDirectInput = LTNULL;
	}

	
	// Free all action defs.
	pCurAction = g_ActionDefHead.m_pNext;
	while(pCurAction != &g_ActionDefHead)
	{
		pNextAction = pCurAction->m_pNext;
		delete pCurAction;
		pCurAction = pNextAction;
	}

	g_ActionDefHead.m_pPrev = g_ActionDefHead.m_pNext = &g_ActionDefHead;
}


// --------------------------------------------------------------------- //
// Init input.
// --------------------------------------------------------------------- //

bool input_Init(InputMgr *pMgr, ConsoleState *pState)
{
	HRESULT		hResult;

	g_InputHWND = (HWND)dsi_GetMainWindow();
	g_pInputConsoleState = pState;

	// Create the DirectInput device.
	hResult = DirectInput8Create( (HINSTANCE)dsi_GetInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&g_pDirectInput, LTNULL );
	if( hResult != DI_OK )
	{
		return false;
	}

	// Enumerate devices so we know what's there.
	g_pDirectInput->EnumDevices( 0, DeviceEnumCallback, LTNULL, DIEDFL_ATTACHEDONLY );

	return true;
}


bool input_IsInitted(InputMgr *pMgr)
{
	return g_pDirectInput != 0;
}


void input_ListDevices(InputMgr *pMgr)
{
	if(!g_pDirectInput)
	{
		dsi_ConsolePrint("Input not initialized");
	}

	dsi_ConsolePrint("------------ Input devices ------------");
	g_pDirectInput->EnumDevices(0, ListDevicesEnumCallback, LTNULL, 0);
}


// --------------------------------------------------------------------- //
// Plays a force feedback effect if loaded
// --------------------------------------------------------------------- //

HRESULT	input_PlayJoystickEffect( InputMgr *pMgr, const char* strEffectName, float x, float y )
{
/*
	// find the joystick device
	DeviceDef* pDev = g_pDeviceHead;
	while( pDev )
	{
		if( DIEFT_GETTYPE( pDev->m_DeviceType ) == DI8DEVTYPE_JOYSTICK && pDev->m_pDevice2 )
		{
			// find the effect
			CJoystickEffect* pEffect = g_pJoystickEffects;
			while( pEffect )
			{
				if( CHelpers::UpperStrcmp(strEffectName, pEffect->m_EffectName) )
				{
					break;
				}

				pEffect = pEffect->m_pNext;
			}

			// if we found it, play it
			if( pEffect )
			{
				if (x == 0.0f && y == 0.0f)
				{
					LONG rglDirection[2];
					rglDirection[0] = DEFAULT_VFX_EFFECT_DIRECTION;
					rglDirection[1] = 0;

					DIEFFECT die;
					memset (&die, 0, sizeof(DIEFFECT));
					die.dwSize = sizeof(DIEFFECT);
					die.dwFlags = DIEFF_CARTESIAN;
					die.cAxes = 2;
					die.rglDirection = rglDirection;

					HRESULT hResult = pEffect->m_pEffect->SetParameters (&die, DIEP_DIRECTION);
				}
				else
				{
					LONG rglDirection[2];
					rglDirection[0] = (LONG) (x*100.0f);
					rglDirection[1] = (LONG) (y*100.0f);

					DIEFFECT die;
					memset (&die, 0, sizeof(DIEFFECT));
					die.dwSize = sizeof(DIEFFECT);
					die.dwFlags = DIEFF_CARTESIAN;
					die.cAxes = 2;
					die.rglDirection = rglDirection;

					HRESULT hResult = pEffect->m_pEffect->SetParameters (&die, DIEP_DIRECTION);
				}
				
				return pEffect->m_pEffect->Start( 1, 0 );
			}
		}

		pDev = pDev->m_pNext;
	}

	return DIERR_INVALIDPARAM;
*/
	return DI_OK;
}

// --------------------------------------------------------------------- //
// Enables input from a particular device.
// --------------------------------------------------------------------- //

bool input_EnableDevice( InputMgr *pMgr, const char *pDeviceName )
{
	DeviceDef *pDevice;
	HRESULT	hResult;
	DIDATAFORMAT format;
	bool bKeyboard, bMouse;
	GUID theGuid;


	bKeyboard = bMouse = false;
	
	if(!input_IsInitted(pMgr))
		return false;
	

	// Find the device.
	pDevice = input_FindDeviceByName(pDeviceName);
	if( !pDevice )
		return false;
	
	// Create the device if it hasn't been created yet.
	if( !pDevice->m_pDevice )
	{
		// Do special stuff if it's the mouse (fixes the USB mouse lockups).
		theGuid = pDevice->m_InstanceGuid;
		pDevice->m_bSysMouse = false;
		if(stricmp(pDeviceName, "##mouse")==0)
		{
			pDevice->m_bSysMouse = true;
			theGuid = GUID_SysMouse;
		}
		else if(stricmp(pDeviceName, "##keyboard")==0)
		{
			theGuid = GUID_SysKeyboard;
		}

		hResult = g_pDirectInput->CreateDevice( theGuid, &pDevice->m_pDevice, LTNULL );
		if( hResult != DI_OK )
			return false;

		// Set cooperative level.
		if(pDevice->IsJoystick() || pDevice->IsGamepad())
		{
			hResult = pDevice->m_pDevice->SetCooperativeLevel( g_InputHWND, DISCL_EXCLUSIVE|DISCL_FOREGROUND );
		}
		else
		{
			hResult = pDevice->m_pDevice->SetCooperativeLevel( g_InputHWND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );
		}
		
		if( hResult != DI_OK )
		{
			pDevice->m_pDevice->Release();
			pDevice->m_pDevice = LTNULL;
			return false;
		}

		// Set the data format.  It starts with no device objects used so this part is easy!
		memset( &format, 0, sizeof(format) );
		format.dwSize = sizeof(DIDATAFORMAT);
		format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
		format.dwFlags = DIDF_RELAXIS;	// We like relative axis.

		if(pDevice->IsJoystick())
		{
			hResult = pDevice->m_pDevice->SetDataFormat( &c_dfDIJoystick );
		}
		else if(pDevice->IsGamepad())
		{
			hResult = pDevice->m_pDevice->SetDataFormat( &c_dfDIJoystick2 );
		}
		else
		{
			hResult = pDevice->m_pDevice->SetDataFormat( &format );
		}
		
		if( hResult != DI_OK )
		{
			pDevice->m_pDevice->Release();
			pDevice->m_pDevice = LTNULL;
			return false;
		}
	
		// Normally, we would acquire it at this point, but since nothing is bound to
		// anything on this device, we'll wait to acquire it.

		// BB: Actually, we're going to acquire it here so we can create some forces...
		// BB: Removed the forces, but still acquiring it here because it's been done for so long (don't
		//     want to anything up...)
		hResult = pDevice->m_pDevice->Acquire();
	}
	
	return true;
}


// --------------------------------------------------------------------- //
// This is the main routine that reads input from all devices and sets
// any actions that are on.
// --------------------------------------------------------------------- //
void input_ReadInput( InputMgr *pMgr, BYTE *pActionsOn, float axisOffsets[3] )
{
	DeviceDef *pDevice;
	TriggerObject *pTrigger;
	TriggerAction *pAction;
	DIDEVICEOBJECTDATA data[INPUT_BUFFER_SIZE];
	HRESULT hResult;
	uint32 i;
	unsigned long nEvents;
	int actionCode;
	float addAmt;
	uint32 dwTickCount;
	GPOS pos;
	CTriggerActionConsoleVar* pTrigConVar;

	// clear the axis array
	axisOffsets[0] = axisOffsets[1] = axisOffsets[2] = 0.0f;

	// clear all of the axis console variables 
	pTrigConVar = g_lstTriggerActionConsoleVariables;
	while(pTrigConVar != LTNULL)
	{
		pTrigConVar->SetFloat(0.0);
		pTrigConVar->InitCommandVar();
		pTrigConVar = pTrigConVar->m_pNext;
	}
	
	// Get the time stamp for this frame...
	dwTickCount = GetTickCount( );
	
	// Query each device.
	pDevice = g_pDeviceHead;
	while( pDevice )
	{
		while( pDevice->IsEnabled() )
		{
			// Check for JoystickDisable..
			if(LOBYTE(pDevice->m_DeviceType) == DI8DEVTYPE_JOYSTICK && g_CV_JoystickDisable)
				break;

			// Poll the device if necessary
			if(pDevice->IsJoystick() || pDevice->IsGamepad())
			{
				hResult = pDevice->m_pDevice->Poll();
			}

			memset (data, 0, sizeof(data));
			
			if(pDevice->m_bSysMouse)
			{
				// This fixes the USB mouse lockups.
				nEvents = 1;
			}
			else
			{
				nEvents = INPUT_BUFFER_SIZE;
			}

			hResult = pDevice->m_pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), data, &nEvents, 0 );
			//hResult = pDevice->m_pDevice->GetDeviceState( pDevice->m_StateReadSize, dwStates );

			if( hResult != DI_OK )
			{
				if( hResult == DI_BUFFEROVERFLOW )
				{
					dsi_ConsolePrint ("Input buffer overflow on %s", pDevice->m_InstanceName);
				}
				else
				{
					// Attempt to acquire it and retry .. maybe it got lost.
					hResult = pDevice->m_pDevice->Acquire();
					
					//hResult = pDevice->m_pDevice->GetDeviceState( pDevice->m_StateReadSize, dwStates );
					hResult = pDevice->m_pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), data, &nEvents, 0 );
				}
			}
	
			if(hResult != DI_OK && hResult != DI_BUFFEROVERFLOW)
				break;

			// Go through each event and update the associated triggers
			// (unless it's a key up event - handle those at the end in
			//  case there was a down-up combination in the same frame)
			for(i = 0; i < nEvents; i++)
			{
				if((data[i].dwOfs>>2) >= pDevice->m_Triggers.GetSize())
					continue;

				// get the trigger associated with this event
				pTrigger = pDevice->m_TriggerTable[data[i].dwOfs >> 2];

				if(g_CV_InputDebug)
				{
					dsi_ConsolePrint("%s (%s) generated %d", pTrigger->m_TriggerName, pTrigger->m_RealName, data[i].dwData);
				}

				// See if the trigger is a button...
				if ( pTrigger->m_InputType == Input_PushButton )
				{
					// If the button was up, then worry about it later, otherwise save the data...
					if( data[i].dwData != 0 )
					{
						// Did it just go down?
						if(!pTrigger->m_State)
						{
							// Maybe trigger a console command..
							pTrigger->m_bJustWentDown = true;
						}

						pTrigger->m_State = (float)data[i].dwData;
					}

					continue;
				}

				// See if the trigger is a hat...
				if ( pTrigger->m_InputType == Input_Pov )
				{
					pTrigger->m_State = (float)data[i].dwData;
					continue;
				}

				// Process all the non-buttons...
				pTrigger->m_dwBaseUpdateTime = pTrigger->m_dwPrevUpdateTime;
				pTrigger->m_dwPrevUpdateTime = pTrigger->m_dwUpdateTime;
				pTrigger->m_dwUpdateTime = dwTickCount;

				// set the trigger's state
				pTrigger->m_BaseState = pTrigger->m_PrevState;
				pTrigger->m_PrevState = pTrigger->m_State;
				pTrigger->m_State = (float)((int32)(data[i].dwData));
			}


			// Go thru each trigger and check the state.
			for(pos=pDevice->m_Triggers; pos; )
			{
				pTrigger = pDevice->m_Triggers.GetNext(pos);

				// Interpolate the non-pushbuttons...
				if(pTrigger->m_InputType == Input_PushButton)
				{
					// Scale the state...
					addAmt = pTrigger->m_State * pTrigger->m_Scale;
				}
				else
				{
					float fCurTriggerState = pTrigger->m_State;

					if( pTrigger->m_InputType != Input_Pov )
					{
						fCurTriggerState = (pTrigger->m_BaseState + pTrigger->m_PrevState + pTrigger->m_State) / 3.0f;
						float fInterpolant = (float)g_CV_InputRate / 100.0f;
						fCurTriggerState = LTLERP(pTrigger->m_State, fCurTriggerState, fInterpolant);

						// Zero out relative axes based on the input rate
						if (pTrigger->m_InputType == Input_RelAxis)
						{
							// Simulate a zero sample based on the input rate
							if ((dwTickCount - pTrigger->m_dwUpdateTime) > ((uint32)g_CV_InputRate / 10))
							{
								pTrigger->m_BaseState = pTrigger->m_PrevState;
								pTrigger->m_PrevState = pTrigger->m_State;
								pTrigger->m_State = 0.0f;

								pTrigger->m_dwBaseUpdateTime = pTrigger->m_dwPrevUpdateTime;
								pTrigger->m_dwPrevUpdateTime = pTrigger->m_dwUpdateTime;
								pTrigger->m_dwUpdateTime = dwTickCount;
							}
						}
					}

					// Scale the state...
					addAmt = LTCLAMP(fCurTriggerState, pTrigger->m_DataMin, pTrigger->m_DataMax );

					if (pTrigger->m_fRangeScalePreCenterOffset != 0.0)
					{
						if (addAmt <= pTrigger->m_fRangeScalePreCenter)
						{
							addAmt = pTrigger->m_fRangeScaleOffsetLo + (pTrigger->m_fRangeScaleMultiplierLo * addAmt);
						}
						else
						{
							addAmt = pTrigger->m_fRangeScaleOffsetHi + (pTrigger->m_fRangeScaleMultiplierHi * addAmt);
						}
					}
					else
					{
						addAmt = pTrigger->m_fRangeScaleOffset + (pTrigger->m_fRangeScaleMultiplier * addAmt);
					}

					// NOTE : the RangeScale stuff will not work well if there are mutiple addAmt's
					addAmt = addAmt * pTrigger->m_Scale;
				}

	
				// Hit all its actions.
				pAction = pTrigger->m_pActionHead;
				while( pAction )
				{
					if(pAction->m_pConsoleString && pTrigger->m_bJustWentDown && g_pInputConsoleState)
					{
						cc_HandleCommand(g_pInputConsoleState, pAction->m_pConsoleString);
					}
					else if(pAction->m_pAction)
					{					
						// Map the trigger state to the action!
						actionCode = pAction->m_pAction->m_ActionCode;
						if(pTrigger->m_InputType == Input_RelAxis && actionCode < 0)
						{
							if( actionCode == -1 )
								axisOffsets[0] += addAmt;
							else if( actionCode == -2 )
								axisOffsets[1] += addAmt;
							else if( actionCode == -3 )
								axisOffsets[2] += addAmt;
							else if( pAction->m_pConsoleVar != LTNULL)
							{
								pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);
								if (g_CV_InputDebug) dsi_ConsolePrint("rel Trigger %s set console var %s to %22.12f", pTrigger->m_TriggerName, pAction->m_pConsoleVar->GetCommandName(), (double)(pAction->m_pConsoleVar->GetFloat()));
							}
						}
						else if(pTrigger->m_InputType == Input_AbsAxis )
						{
							if( actionCode < 0 )
							{
								if( actionCode == -1 )
									axisOffsets[0] += addAmt;
								else if( actionCode == -2 )
									axisOffsets[1] += addAmt;
								else if( actionCode == -3 )
									axisOffsets[2] += addAmt;
								else if( pAction->m_pConsoleVar != LTNULL)
								{
									pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);
									if (g_CV_InputDebug) dsi_ConsolePrint("abs Trigger %s set console var %s to %22.12f", pTrigger->m_TriggerName, pAction->m_pConsoleVar->GetCommandName(), (double)(pAction->m_pConsoleVar->GetFloat()));
								}
							}
							else if( addAmt >= pAction->m_RangeLow && addAmt <= pAction->m_RangeHigh )
							{
								ASSERT( actionCode < 256 );
								pActionsOn[actionCode] |= 1;
							}
						}
						else if ((pTrigger->m_InputType == Input_Pov) && (pAction->m_pConsoleVar != LTNULL))
						{
							if (pAction->m_pConsoleVar != LTNULL)
							{
								pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);
							}
						}
						else if( pTrigger->m_InputType == Input_Pov && actionCode < 0 && addAmt != 65536.0f )
						{
							if( addAmt >= 4500 && addAmt <= 13500 )
							{
								axisOffsets[0] += 30;
							}
							else if( addAmt >= 22500 && addAmt <= 31500 )
							{
								axisOffsets[0] -= 30;
							}

							if( (addAmt >= 31500 && addAmt <= 36000) || (addAmt <= 4500 && addAmt >= 0) )
							{
								axisOffsets[1] -= 30;
							}

							if( addAmt >= 13500 && addAmt <= 22500 )
							{
								axisOffsets[1] += 30;
							}
						}
						else if(pAction->m_RangeLow != 0.0f || pAction->m_RangeHigh != 0.0f)
						{
							// dsi_ConsolePrint("Checking range low=%f high=%f amt=%f",(double)pAction->m_RangeLow, (double)pAction->m_RangeHigh, (double)addAmt);  // BLB TEMP
							if( addAmt >= pAction->m_RangeLow && addAmt <= pAction->m_RangeHigh )
							{
//								dsi_ConsolePrint("Checking range low=%f high=%f amt=%f",(double)pAction->m_RangeLow, (double)pAction->m_RangeHigh, (double)addAmt);  // BLB TEMP
								// dsi_ConsolePrint("Setting action from range check actionCode=%i",(int)actionCode);  // BLB TEMP
								ASSERT( actionCode < 256 );
								pActionsOn[actionCode] |= 1;
							}
						}
						else if( actionCode >= 0 )
						{
							ASSERT( actionCode < 256 );
							pActionsOn[actionCode] |= pTrigger->m_State != 0.0f;
						}
					}
					
					pAction = pAction->m_pNext;
				}
				
				// Clear this..
				pTrigger->m_bJustWentDown = false;
			}

			// Go through any key up events and update the associated triggers
			// Also clear any relative axis data...
			for( i = 0; i < nEvents; i++ )
			{
				if((data[i].dwOfs>>2) >= pDevice->m_Triggers.GetSize())
					continue;

				// get the trigger associated with this event
				pTrigger = pDevice->m_TriggerTable[data[i].dwOfs >> 2];

				// see if the trigger is a button and if it's state has changed to 'up'
				if( pTrigger->m_InputType == Input_PushButton && data[i].dwData == 0 )
				{
					pTrigger->m_State = (float)data[i].dwData;
				}
			}

			if(nEvents == 0 || !pDevice->m_bSysMouse)
				break;
		}
	
		// Update the last time...
		pDevice->m_dwLastTime = dwTickCount;

		pDevice = pDevice->m_pNext;
	}
}


// --------------------------------------------------------------------- //
// Flush the DirectInput buffers
// --------------------------------------------------------------------- //

bool input_FlushInputBuffers(InputMgr *pMgr)
{
	bool bSuccess = true;

	DeviceDef* pDef = g_pDeviceHead;
	while( pDef )
	{
		if ( pDef->IsEnabled() )
		{
			unsigned long dwItems = INFINITE;
			HRESULT hResult = pDef->m_pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), LTNULL, &dwItems, 0 );

			if( hResult != DI_OK && hResult != DI_BUFFEROVERFLOW )
			{
				bSuccess = false;
			}
		}

		pDef = pDef->m_pNext;
	}

	return bSuccess;
}

static LTRESULT input_ClearInput()
{
	DeviceDef *pDevice;

	pDevice = g_pDeviceHead;
	while( pDevice )
	{
		if( pDevice->IsEnabled() )
		{
			input_SetupDeviceFormats(pDevice);
		}
		pDevice = pDevice->m_pNext;
	}
	return LT_OK;
}


// --------------------------------------------------------------------- //
// You want to add action defs first so there will be something to bind to!
// --------------------------------------------------------------------- //

void input_AddAction(InputMgr *pMgr, const char *pActionName, int code)
{
	ActionDef	*pDef;

	if(code >= 256)
		return;

	// If there's already an action with that name, change its code.
	pDef = input_FindAction(pActionName);
	if(pDef)
	{
		pDef->m_ActionCode = code;
		return;
	}

	// Ok, add a new one.
	LT_MEM_TRACK_ALLOC(pDef = new ActionDef,LT_MEM_TYPE_INPUT);
	pDef->m_pPrev = g_ActionDefHead.m_pPrev;
	pDef->m_pNext = &g_ActionDefHead;
	pDef->m_pPrev->m_pNext = pDef->m_pNext->m_pPrev = pDef;
	LTStrCpy(pDef->m_ActionName, pActionName, MAX_ACTIONNAME_LEN);
	pDef->m_ActionCode = code;
}


// --------------------------------------------------------------------- //
// Clear bindings for an input trigger.
// Note:  It would probably be better to have this routine remove the trigger completely,
//        but it's not too big a deal right now.
// --------------------------------------------------------------------- //

bool input_ClearBindings( InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName )
{
	DeviceDef *pDef = input_FindDeviceByName(pDeviceName);
	if(!pDef)
		return false;

	if(!pDef->IsEnabled())
		return false;

	TriggerObject *pTrigger = input_FindTrigger(pDef, pTriggerName);
	if(!pTrigger)
		return false;

	// Delete the trigger and remove it from the list.
	pDef->m_Triggers.RemoveAt(pTrigger);
	delete pTrigger;

	if(g_DebugLevel > 10)
		dsi_ConsolePrint("Cleared bindings for %s (on %s)", pTriggerName, pDeviceName);

	// Re-setup the device data formats / possibly acquire the device.
	return input_SetupDeviceFormats(pDef);
}


// --------------------------------------------------------------------- //
// Add a binding for a device.
// --------------------------------------------------------------------- //

bool input_AddBinding(InputMgr *pMgr, 
	const char *pDeviceName, const char *pTriggerName, const char *pActionName,
	float rangeLow, float rangeHigh)
{
	TriggerObject				*pTrigger;
	TriggerAction				*pAction;
	CTriggerActionConsoleVar	*pConVar = LTNULL;
	ActionDef					*pActionDef;
	DeviceDef					*pDevice;
	float temp;


	if(rangeLow > rangeHigh)
	{
		temp = rangeLow;
		rangeLow = rangeHigh;
		rangeHigh = temp;
	}

	if (stricmp(pDeviceName, "Joystick 1") == 0)
	{
//		dsi_ConsolePrint("AddBinding called for Joystick");  // BLB TEMP
	}


	pDevice = input_FindDeviceByName(pDeviceName);
	if( !pDevice )
		return false;

	if( !pDevice->IsEnabled() )
		return false;

	pTrigger = input_FindTrigger(pDevice, pTriggerName);
	if(!pTrigger)
	{
		pTrigger = input_AddTrigger(pDevice, pTriggerName);
		if(!pTrigger)
			return false;
	}

	if(strlen(pActionName) > 1 && pActionName[0] == '*')
	{
		pActionDef = LTNULL;
	}
	else
	{
		// find the action
		pActionDef = input_FindAction(pActionName);
		if( !pActionDef )
			return false;

	    // If we are using a console variable to send input data to the game set it up
		if (pActionDef->m_ActionCode <= -10000)
		{
			// make a new console var
			LT_MEM_TRACK_ALLOC(pConVar = new CTriggerActionConsoleVar,LT_MEM_TYPE_INPUT);
			
			// add to list of console vars for trigger actions
			pConVar->m_pNext = g_lstTriggerActionConsoleVariables;
			g_lstTriggerActionConsoleVariables = pConVar;
		}
	}

	// Add the action.
	LT_MEM_TRACK_ALLOC(pAction = new TriggerAction,LT_MEM_TYPE_INPUT);
	pAction->m_pNext = pTrigger->m_pActionHead;
	pAction->m_RangeLow = rangeLow;
	pAction->m_RangeHigh = rangeHigh;
	pAction->m_pConsoleVar = pConVar;

    if (pConVar != LTNULL) pConVar->m_pTriggerAction = pAction;

	// If pActionDef is LTNULL then it's a console string.
	pAction->m_pAction = pActionDef;
	if(!pActionDef)
	{
		LT_MEM_TRACK_ALLOC(pAction->m_pConsoleString = new char[strlen(pActionName)],LT_MEM_TYPE_INPUT);
		if(!pAction->m_pConsoleString)
		{
			delete pAction;
			return false;
		}
		strcpy(pAction->m_pConsoleString, &pActionName[1]);
	}

	pTrigger->m_pActionHead = pAction;

	// Re-setup the device data formats / possibly acquire the device.
	if(!input_SetupDeviceFormats(pDevice))
	{
		// Get rid of the trigger we just tried to add.
		if(pTrigger->m_pActionHead == pAction)
		{
			pTrigger->m_pActionHead = pAction->m_pNext;
			delete pAction;
		}

		input_SetupDeviceFormats(pDevice);
		return false;
	}

	if(g_DebugLevel > 10)
		dsi_ConsolePrint("Bound %s (on %s) to action %s", pTriggerName, pDeviceName, pActionName);

	return true;
}

// --------------------------------------------------------------------- //
// Sets the trigger's scale.
// --------------------------------------------------------------------- //

bool input_ScaleTrigger( InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName, float scale, float fRangeScaleMin, float fRangeScaleMax, float fRangeScalePreCenterOffset )
{
	DeviceDef		*pDevice;
	TriggerObject	*pTrigger;

	scale = LTCLAMP(scale, -500.0f, 500.0f);

	pDevice = input_FindDeviceByName(pDeviceName);
	if( !pDevice )
		return false;

	pTrigger = input_FindTrigger(pDevice, pTriggerName);
	if( !pTrigger )
		return false;

	pTrigger->m_Scale = scale;

	if ((fRangeScaleMin != 0.0f) || (fRangeScaleMax != 0.0f))
	{
		pTrigger->m_fRangeScaleMin = fRangeScaleMin;
		pTrigger->m_fRangeScaleMax = fRangeScaleMax;

		float fDataMinMinusMax = pTrigger->m_DataMin - pTrigger->m_DataMax;
		if (fDataMinMinusMax != 0.0f) pTrigger->m_fRangeScaleMultiplier = (fRangeScaleMin - fRangeScaleMax) / fDataMinMinusMax;
		else pTrigger->m_fRangeScaleMultiplier = 1.0f;
		pTrigger->m_fRangeScaleOffset = fRangeScaleMin - (pTrigger->m_fRangeScaleMultiplier * pTrigger->m_DataMin);

		pTrigger->m_fRangeScalePreCenterOffset = fRangeScalePreCenterOffset;
		if (fRangeScalePreCenterOffset != 0.0f)
		{
			float fDataCenter = ((pTrigger->m_DataMax - pTrigger->m_DataMin) / 2.0f) + pTrigger->m_DataMin + fRangeScalePreCenterOffset;
			float fRangeScaleCenter = ((fRangeScaleMax - fRangeScaleMin) / 2.0f) + fRangeScaleMin;

			float fDataMinMinusMaxHi = (fDataCenter - pTrigger->m_DataMax);
			if (fDataMinMinusMaxHi != 0.0f) pTrigger->m_fRangeScaleMultiplierHi = (fRangeScaleCenter - fRangeScaleMax) / fDataMinMinusMaxHi;
			else pTrigger->m_fRangeScaleMultiplierHi = 1.0f;
			pTrigger->m_fRangeScaleOffsetHi = fRangeScaleCenter - (pTrigger->m_fRangeScaleMultiplierHi * fDataCenter);

			float fDataMinMinusMaxLo = (pTrigger->m_DataMin - fDataCenter);
			if (fDataMinMinusMaxLo != 0.0f) pTrigger->m_fRangeScaleMultiplierLo = (fRangeScaleMin - fRangeScaleCenter) / fDataMinMinusMaxLo;
			else pTrigger->m_fRangeScaleMultiplierLo = 1.0f;
			pTrigger->m_fRangeScaleOffsetLo = fRangeScaleMin - (pTrigger->m_fRangeScaleMultiplierLo * pTrigger->m_DataMin);

			pTrigger->m_fRangeScalePreCenter = fDataCenter;
		}
	}

	return true;
}


// --------------------------------------------------------------------- //
// Saves the state of all the input bindings.
// --------------------------------------------------------------------- //

void input_SaveBindings( FILE *fp )
{
	DeviceDef *pDevice;
	TriggerObject *pTrigger;
	TriggerAction *pAction;
	ActionDef *pCurActionDef;
	char str[1024], str2[1024];
	char *pDeviceName;
	GPOS triggerPos;


	// Save all action defs.
	pCurActionDef = g_ActionDefHead.m_pNext;
	while(pCurActionDef != &g_ActionDefHead)
	{
		fprintf(fp, "AddAction %s %d\n", pCurActionDef->m_ActionName, pCurActionDef->m_ActionCode);
		pCurActionDef = pCurActionDef->m_pNext;
	}


	fprintf(fp, "\n");
	
	
	// Save all the active devices.
	pDevice = g_pDeviceHead;
	while( pDevice )
	{
		if(pDevice->IsEnabled() && pDevice->m_Triggers.GetSize() > 0)
		{
			pDeviceName = pDevice->m_pSpecialName ? pDevice->m_pSpecialName : pDevice->m_InstanceName;
			fprintf( fp, "enabledevice \"%s\"\n", pDeviceName );
		
			// Save all the triggers.
			for(triggerPos=pDevice->m_Triggers; triggerPos; )
			{
				pTrigger = pDevice->m_Triggers.GetNext(triggerPos);

				LTSNPrintF(str, sizeof(str), "rangebind \"%s\" \"%s\" ", pDeviceName, pTrigger->m_RealName);

				pAction = pTrigger->m_pActionHead;
				while( pAction )
				{
					if(pAction->m_pAction)
						LTSNPrintF( str2, sizeof(str2), "%f %f \"%s\" ", pAction->m_RangeLow, pAction->m_RangeHigh, pAction->m_pAction->m_ActionName );
					else if(pAction->m_pConsoleString)
						LTSNPrintF( str2, sizeof(str2), "%f %f \"*%s\" ", pAction->m_RangeLow, pAction->m_RangeHigh, pAction->m_pConsoleString );
					else
						str2[0] = 0;
					
					LTStrCat( str, str2, sizeof(str) );
					pAction = pAction->m_pNext;
				}

				LTStrCat( str, "\n", sizeof(str) );
				fprintf( fp, str );

				if( (pTrigger->m_fRangeScaleMin != 0.0f) || (pTrigger->m_fRangeScaleMax != 0.0f)  || (pTrigger->m_fRangeScalePreCenterOffset != 0.0f))
					fprintf( fp, "rangescale \"%s\" \"%s\" %f %f %f %f\n", pDeviceName, pTrigger->m_RealName, pTrigger->m_Scale, pTrigger->m_fRangeScaleMin, pTrigger->m_fRangeScaleMax, pTrigger->m_fRangeScalePreCenterOffset );
				else if( pTrigger->m_Scale != 1.0f )
					fprintf( fp, "scale \"%s\" \"%s\" %f\n", pDeviceName, pTrigger->m_RealName, pTrigger->m_Scale );

			}
		}
		
		pDevice = pDevice->m_pNext;
	}
}

// --------------------------------------------------------------------- //
// Device Binding Retrieval.
// --------------------------------------------------------------------- //

static void input_FreeDeviceBindings ( DeviceBinding* pBindings )
{
	DeviceBinding* pBinding = pBindings;
	while( pBinding )
	{
		GameAction* pAction = pBinding->pActionHead;
		while( pAction )
		{
			GameAction* pNext = pAction->pNext;
			delete pAction;
			pAction = pNext;
		}

		DeviceBinding* pNext = pBinding->pNext;
		delete pBinding;
		pBinding = pNext;			
	}
}

static DeviceBinding* input_GetDeviceBindings ( uint32 nDevice )
{
	DeviceDef* pDevice;
	DeviceBinding* pBindingsHead = LTNULL;
	TriggerObject* pTrigger;
	GPOS triggerPos;

	
	pDevice = LTNULL;
	pTrigger = LTNULL;

	// get the device they are looking for
	if( nDevice == DEVICETYPE_KEYBOARD )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_KEYBOARD );
	}
	else if( nDevice & DEVICETYPE_MOUSE )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_MOUSE );
	}
	else if( nDevice & DEVICETYPE_JOYSTICK )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_JOYSTICK );
	}
	else if( nDevice & DEVICETYPE_GAMEPAD )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_GAMEPAD );
	}
	else if( nDevice & DEVICETYPE_UNKNOWN )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_DEVICE );
	}

	if( !pDevice ) 
		return LTNULL;

	// go through each trigger, building a list of DeviceBindings to return
	for(triggerPos=pDevice->m_Triggers; triggerPos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(triggerPos);

		DeviceBinding* pBinding;
		LT_MEM_TRACK_ALLOC(pBinding = new DeviceBinding,LT_MEM_TYPE_INPUT);
		if( !pBinding ) 
		{
			input_FreeDeviceBindings(pBindingsHead);
			return LTNULL;
		}
		
		memset( pBinding, 0, sizeof(DeviceBinding) );

		SAFE_STRCPY( pBinding->strDeviceName, pDevice->m_InstanceName );
		SAFE_STRCPY( pBinding->strTriggerName, pTrigger->m_TriggerName );
		SAFE_STRCPY( pBinding->strRealName, pTrigger->m_RealName );
		pBinding->m_nObjectId = pTrigger->m_diType;

		// store all the scale information for the trigger
		pBinding->nScale = pTrigger->m_Scale;
		pBinding->nRangeScaleMin = pTrigger->m_fRangeScaleMin;
		pBinding->nRangeScaleMax = pTrigger->m_fRangeScaleMax;
		pBinding->nRangeScalePreCenterOffset = pTrigger->m_fRangeScalePreCenterOffset;

		// go through the actions, adding them to the trigger
		GameAction* pActionHead = LTNULL;
		TriggerAction* pTriggerAction = pTrigger->m_pActionHead;
		while( pTriggerAction )
		{
			if(pTriggerAction->m_pAction)
			{
				GameAction* pNewAction;
				LT_MEM_TRACK_ALLOC(pNewAction = new GameAction,LT_MEM_TYPE_INPUT);
				if( !pNewAction ) 
				{
					input_FreeDeviceBindings(pBindingsHead);
					return LTNULL;
				}
				
				memset( pNewAction, 0, sizeof(GameAction) );

				pNewAction->nActionCode = pTriggerAction->m_pAction->m_ActionCode;
				SAFE_STRCPY(pNewAction->strActionName, pTriggerAction->m_pAction->m_ActionName);
				pNewAction->nRangeLow = pTriggerAction->m_RangeLow;
				pNewAction->nRangeHigh = pTriggerAction->m_RangeHigh;
				pNewAction->pNext = pActionHead;
				pActionHead = pNewAction;
			}

			pTriggerAction = pTriggerAction->m_pNext;
		}
		
		if( pTriggerAction ) 
		{
			input_FreeDeviceBindings(pBindingsHead);
			return LTNULL;
		}

		pBinding->pActionHead = pActionHead;

		pBinding->pNext = pBindingsHead;
		pBindingsHead = pBinding;
	}

	return pBindingsHead;
}


// --------------------------------------------------------------------- //
// Device Tracking.
// --------------------------------------------------------------------- //

static BOOL CALLBACK DeviceObjectCountProc ( LPCDIDEVICEOBJECTINSTANCE pObj, LPVOID pvRef )
{
	uint32* pCount = (uint32*) pvRef;
	(*pCount)++;
	
	return DIENUM_CONTINUE;		
}

static BOOL CALLBACK EnumTrackObjectProc ( LPCDIDEVICEOBJECTINSTANCE pObj, LPVOID pvRef )
{
	TrackObjectInfo** ppInfo = (TrackObjectInfo**)pvRef;

	memcpy( &(*ppInfo)->guidType, &pObj->guidType, sizeof(GUID) );
	(*ppInfo)->dwType = pObj->dwType;
	SAFE_STRCPY( (*ppInfo)->tszName, pObj->tszName );

	(*ppInfo)++;

	return DIENUM_CONTINUE;
}

static bool input_StartDeviceTrack( InputMgr *pMgr, uint32 nDevices, uint32 nBufferSize )
{
	if( nBufferSize > MAX_INPUT_BUFFER_SIZE ) return false;

	// this will track the first device found of the requested type

	int i;
	for( i = 0; i < 5; i++ )
	{
		DeviceDef*			pDevice = LTNULL;
		HRESULT				hResult = DI_OK;
		uint32				nObjects = 0;
		TrackObjectInfo*	pInfo = LTNULL;
		DIOBJECTDATAFORMAT* pObjectDataFormats = LTNULL;
		DIDATAFORMAT		deviceDataFormat;
		DIPROPDWORD			prop;

		// check for each possible type of device
		if( i == 0 )
		{
			if( nDevices & DEVICETYPE_KEYBOARD )
			{
				pDevice = input_FindDeviceByType( DI8DEVTYPE_KEYBOARD );
			}
			else
			{
				continue;
			}
		}
		else if( i == 1 )
		{
			if( nDevices & DEVICETYPE_MOUSE )
			{
				pDevice = input_FindDeviceByType( DI8DEVTYPE_MOUSE );
			}
			else
			{
				continue;
			}
		}
		else if( i == 2 )
		{
			if( nDevices & DEVICETYPE_JOYSTICK )
			{
				pDevice = input_FindDeviceByType( DI8DEVTYPE_JOYSTICK );
			}
			else
			{
				continue;
			}
		}
		else if( i == 3 )
		{
			if( nDevices & DEVICETYPE_GAMEPAD )
			{
				pDevice = input_FindDeviceByType( DI8DEVTYPE_GAMEPAD );
			}
			else
			{
				continue;
			}
		}
		else if( i == 4 )
		{
			if( nDevices & DEVICETYPE_UNKNOWN )
			{
				pDevice = input_FindDeviceByType( DI8DEVTYPE_DEVICE );
			}
			else
			{
				continue;
			}
		}

		if( !pDevice ) continue;

		// we now have a device - make sure it's been created
		if( !pDevice->IsEnabled() )
		{
			input_EnableDevice( pMgr, pDevice->m_InstanceName );
			if( !pDevice->IsEnabled() ) break;
			pDevice->m_bTrackingOnly = true;
		}

		// unacquire the device
		pDevice->m_pDevice->Unacquire();

		// JeffR 9/19/2000 This line used to be requesting DIDFT_ALL.  This was causing a crash in 
		//		SetDataFormat() with Win2k joysticks as all joysticks under win2k are HID devices.
 		//		Using DIDFT_ALL, in addition to the normal input objects, you end up with all the 
 		//		HID features the device supports. Most of the HID specific objects don't really 
 		//		have properties (zero size objects as placeholders).  These cause DInput to crash
 		//		if you try to manipulate them. The same problem exist with USB devices in Win9x.

		// count how many objects are on this device
		hResult = pDevice->m_pDevice->EnumObjects( DeviceObjectCountProc, &nObjects, DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV );
		if( hResult != DI_OK || nObjects == 0 ) break;

		// now enumerate through all objects on the device
		LT_MEM_TRACK_ALLOC(pDevice->m_pTrackObjects = new TrackObjectInfo [nObjects],LT_MEM_TYPE_INPUT);
		if( !pDevice->m_pTrackObjects ) break;

		pInfo = pDevice->m_pTrackObjects;
		hResult = pDevice->m_pDevice->EnumObjects( EnumTrackObjectProc, &pInfo, DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV );
		if( hResult != DI_OK ) 
		{
			delete [] pDevice->m_pTrackObjects;
			pDevice->m_pTrackObjects = LTNULL;
			break;
		}

		// set up the object formats for each object
		LT_MEM_TRACK_ALLOC(pObjectDataFormats = new DIOBJECTDATAFORMAT [nObjects],LT_MEM_TYPE_INPUT);
		if( !pObjectDataFormats )
		{
			delete [] pDevice->m_pTrackObjects;
			pDevice->m_pTrackObjects = LTNULL;
			break;
		}
		for( uint32 i = 0; i < nObjects; i++ )
		{
			pObjectDataFormats[i].pguid = &pDevice->m_pTrackObjects[i].guidType;
			pObjectDataFormats[i].dwOfs = i << 2;
			pObjectDataFormats[i].dwType = pDevice->m_pTrackObjects[i].dwType;
			pObjectDataFormats[i].dwFlags = 0;
		}

		// set the format for the device
		deviceDataFormat.dwSize = sizeof(DIDATAFORMAT);
		deviceDataFormat.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
		deviceDataFormat.dwFlags = (pDevice->IsJoystick() || pDevice->IsGamepad() )? DIDF_ABSAXIS : DIDF_RELAXIS;
		deviceDataFormat.dwDataSize = nObjects << 2;
		deviceDataFormat.dwNumObjs = nObjects;
		deviceDataFormat.rgodf = pObjectDataFormats;
		hResult = pDevice->m_pDevice->SetDataFormat (&deviceDataFormat);

		delete [] pObjectDataFormats;
		if( hResult != DI_OK )
		{
			delete [] pDevice->m_pTrackObjects;
			pDevice->m_pTrackObjects = LTNULL;
			break;
		}

		// set the buffer size
		prop.diph.dwSize = sizeof (DIPROPDWORD);
		prop.diph.dwHeaderSize = sizeof (DIPROPHEADER);
		prop.diph.dwObj = 0;
		prop.diph.dwHow = DIPH_DEVICE;
		prop.dwData = 8; // We don't need a buffer very big.

		hResult = pDevice->m_pDevice->SetProperty (DIPROP_BUFFERSIZE, &prop.diph);
		/* Joysticks don't like this call.. doesn't seem to matter if it fails.
		if( hResult != DI_OK && hResult != DI_PROPNOEFFECT )
		{
			input_TraceDInputError(hResult);
			delete [] pDevice->m_pTrackObjects;
			pDevice->m_pTrackObjects = LTNULL;
			break;
		}
		*/

		// set the tracking flag
		pDevice->m_bTracking = true;
		pDevice->m_nTrackObjects = nObjects;
	
		// acquire the device again
		hResult = pDevice->m_pDevice->Acquire();
	}

	if( i < 4 ) return false;

	return true;
}

static bool input_TrackDevice( DeviceInput *pInputArray, uint32 *pnInOut )
{
	HRESULT				hResult;
	DIDEVICEOBJECTDATA	data[MAX_INPUT_BUFFER_SIZE];
	unsigned long				nEvents;
	uint32				nArraySize;

	nArraySize = *pnInOut;
	*pnInOut = 0;
	
	DeviceDef* pDevice = g_pDeviceHead;
	while( pDevice )
	{
		if( pDevice->m_bTracking && pDevice->m_pTrackObjects )
		{
			// Poll the device if necessary
			if(pDevice->IsJoystick() || pDevice->IsGamepad())
			{
				hResult = pDevice->m_pDevice->Poll();
			}

			memset (data, 0, sizeof(DIDEVICEOBJECTDATA) * MAX_INPUT_BUFFER_SIZE );
			nEvents = MAX_INPUT_BUFFER_SIZE;
			hResult = pDevice->m_pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), data, &nEvents, 0 );
			if( hResult != DI_OK )
			{
				if( hResult == DI_BUFFEROVERFLOW )
				{
					dsi_ConsolePrint ("Input buffer overflow on %s", pDevice->m_InstanceName);
				}
				else
				{
					// Attempt to acquire it and retry .. maybe it got lost.
					hResult = pDevice->m_pDevice->Acquire();
					hResult = pDevice->m_pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), data, &nEvents, 0 );
				}
			}

			if( hResult == DI_OK || hResult == DI_BUFFEROVERFLOW )
			{
				// go through events and add them to input array
				for( uint32 i = 0; i < nEvents; i++ )
				{
					if( *pnInOut == nArraySize ) return true;

					switch( DIEFT_GETTYPE( pDevice->m_DeviceType ) )
					{
						case DI8DEVTYPE_KEYBOARD:	pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_KEYBOARD; break;
						case DI8DEVTYPE_MOUSE:		pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_MOUSE; break;
						case DI8DEVTYPE_JOYSTICK:	pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_JOYSTICK; break;
						case DI8DEVTYPE_GAMEPAD:	pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_GAMEPAD; break;
						case DI8DEVTYPE_DEVICE:		pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_UNKNOWN; break;
					}
					LTStrCpy( pInputArray[*pnInOut].m_DeviceName, pDevice->m_InstanceName, sizeof(pInputArray[*pnInOut].m_DeviceName) );
					
					uint32 nOffset = data[i].dwOfs >> 2;
					if(nOffset < pDevice->m_nTrackObjects)
					{
						if(	pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_XAxis )			pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_XAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_YAxis )		pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_YAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_ZAxis )		pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_ZAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_RxAxis )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RXAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_RyAxis )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RYAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_RzAxis )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RZAXIS;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_Slider )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_SLIDER;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_Button )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_BUTTON;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_Key )		pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_KEY;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_POV )		pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_POV;
						else if( pDevice->m_pTrackObjects[ nOffset ].guidType == GUID_Unknown )	pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_UNKNOWN;

						LTStrCpy( pInputArray[*pnInOut].m_ControlName, pDevice->m_pTrackObjects[ nOffset ].tszName, sizeof(pInputArray[*pnInOut].m_ControlName) );

						uint16 objInstance = (DIDFT_GETINSTANCE(pDevice->m_pTrackObjects[ nOffset ].dwType));
						pInputArray[*pnInOut].m_ControlCode = objInstance;
						pInputArray[*pnInOut].m_nObjectId = pDevice->m_pTrackObjects[ nOffset ].dwType;
						pInputArray[*pnInOut].m_InputValue = data[i].dwData;



						(*pnInOut)++;
					}
				}
			}
		}

		pDevice = pDevice->m_pNext;
	}

	return true;
}

static bool input_EndDeviceTrack()
{
	DeviceDef* pDef = g_pDeviceHead;
	while( pDef )
	{
		if( pDef->IsEnabled() )
		{
			pDef->m_bTracking = false;

			// disable any device enabled specifically for device tracking
			if( pDef->m_bTrackingOnly )
			{
				if( pDef->m_pDevice )
				{
					pDef->m_pDevice->Unacquire();
					pDef->m_pDevice->Release();
					pDef->m_pDevice = LTNULL;
				}
				
				// reset the tracking only flag
				pDef->m_bTrackingOnly = false;

				// remove any tracking object structures
				if( pDef->m_pTrackObjects )
				{
					delete [] pDef->m_pTrackObjects;
					pDef->m_pTrackObjects = LTNULL;
				}
			}
			else
			{
				// re-setup the device
				input_SetupDeviceFormats( pDef );
			}
		}

		pDef = pDef->m_pNext;
	}

	return true;
}


// --------------------------------------------------------------------- //
// Device Object List Retrieval.
// --------------------------------------------------------------------- //

static BOOL CALLBACK DeviceObjectListProc ( LPCDIDEVICEOBJECTINSTANCE pObj, LPVOID pvRef )
{
	HRESULT hResult;
	DIPROPRANGE dipr;

	DeviceObject*** pppList = (DeviceObject***) pvRef;

	LT_MEM_TRACK_ALLOC(**pppList = new DeviceObject,LT_MEM_TYPE_INPUT);
	memset( **pppList, 0, sizeof(DeviceObject) );

	if( pObj->guidType == GUID_XAxis )			(**pppList)->m_ObjectType = CONTROLTYPE_XAXIS;
	else if( pObj->guidType == GUID_YAxis )		(**pppList)->m_ObjectType = CONTROLTYPE_YAXIS;
	else if( pObj->guidType == GUID_ZAxis )		(**pppList)->m_ObjectType = CONTROLTYPE_ZAXIS;
	else if( pObj->guidType == GUID_RxAxis )	(**pppList)->m_ObjectType = CONTROLTYPE_RXAXIS;
	else if( pObj->guidType == GUID_RyAxis )	(**pppList)->m_ObjectType = CONTROLTYPE_RYAXIS;
	else if( pObj->guidType == GUID_RzAxis )	(**pppList)->m_ObjectType = CONTROLTYPE_RZAXIS;
	else if( pObj->guidType == GUID_Slider )	(**pppList)->m_ObjectType = CONTROLTYPE_SLIDER;
	else if( pObj->guidType == GUID_Button )	(**pppList)->m_ObjectType = CONTROLTYPE_BUTTON;
	else if( pObj->guidType == GUID_Key )		(**pppList)->m_ObjectType = CONTROLTYPE_KEY;
	else if( pObj->guidType == GUID_POV )		(**pppList)->m_ObjectType = CONTROLTYPE_POV;
	else										(**pppList)->m_ObjectType = CONTROLTYPE_UNKNOWN;
	
	LTStrCpy( (**pppList)->m_ObjectName, pObj->tszName, sizeof((**pppList)->m_ObjectName) );
	(**pppList)->m_nObjectId = pObj->dwType;

	if( g_pCurrentEnumDevice )
	{
		dipr.diph.dwSize = sizeof( DIPROPRANGE );
		dipr.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		dipr.diph.dwObj = pObj->dwType;
		dipr.diph.dwHow = DIPH_BYID;
		// Keys & Buttons don't have a range, so don't try to query it...
		// Note : This check avoids the debug output spam associated with all the keys
		// and buttons not having a range.  This should probably check the device for 
		// not having any axes and base it on that, but then you have to query the 
		// device caps for each individual key on the keyboard, which is quite slow.
		if ((pObj->guidType != GUID_Key) && (pObj->guidType != GUID_Button))
			hResult = g_pCurrentEnumDevice->m_pDevice->GetProperty( DIPROP_RANGE, &dipr.diph );
		else
			hResult = DIERR_OBJECTNOTFOUND;
		if( hResult == DI_OK )
		{
			(**pppList)->m_RangeLow = (float) dipr.lMin;
			(**pppList)->m_RangeHigh = (float) dipr.lMax;
		}
	}

	*pppList = &((**pppList)->m_pNext);

	return DIENUM_CONTINUE;		
}

static DeviceObject* input_GetDeviceObjects (uint32 nDeviceFlags)
{
	HRESULT			hResult = DI_OK;
	uint32			nCurrentDeviceType = 0;
	DeviceDef*		pDevice = LTNULL;
	DeviceObject*	pList = LTNULL;
	DeviceObject*	pListPtr = LTNULL;
	int				nJoysticksFound = 0;
	int				nGamepadsFound = 0;


	// keep looping until no more devices were found (pDevice will be LTNULL)
	for(;;)
	{
		pDevice = LTNULL;
		
		// get the device they are looking for
		if( nDeviceFlags & DEVICETYPE_KEYBOARD )
		{
			nDeviceFlags &= ~DEVICETYPE_KEYBOARD;
			nCurrentDeviceType = DEVICETYPE_KEYBOARD;
			pDevice = input_FindDeviceByType( DI8DEVTYPE_KEYBOARD );
		}
		else if( nDeviceFlags & DEVICETYPE_MOUSE )
		{
			nDeviceFlags &= ~DEVICETYPE_MOUSE;
			nCurrentDeviceType = DEVICETYPE_MOUSE;
			pDevice = input_FindDeviceByType( DI8DEVTYPE_MOUSE );
		}
		else if( nDeviceFlags & DEVICETYPE_JOYSTICK )
		{
			nCurrentDeviceType = DEVICETYPE_JOYSTICK;
			pDevice = FindDeviceByTypeWithSkip( DI8DEVTYPE_JOYSTICK, nJoysticksFound );
			if (pDevice != LTNULL) nJoysticksFound++;
			else nDeviceFlags &= ~DEVICETYPE_JOYSTICK;
		}
		else if( nDeviceFlags & DEVICETYPE_GAMEPAD )
		{
			nCurrentDeviceType = DEVICETYPE_GAMEPAD;
			pDevice = FindDeviceByTypeWithSkip( DI8DEVTYPE_GAMEPAD, nGamepadsFound );
			if (pDevice != LTNULL) nGamepadsFound++;
			else nDeviceFlags &= ~DEVICETYPE_GAMEPAD;
		}
		else if( nDeviceFlags & DEVICETYPE_UNKNOWN )
		{
			nDeviceFlags &= ~DEVICETYPE_UNKNOWN;
			nCurrentDeviceType = DEVICETYPE_UNKNOWN;
			pDevice = input_FindDeviceByType( DI8DEVTYPE_DEVICE );
		}

		if( !pDevice ) return pList;

		// we now have a device - make sure it's been created
		if( !pDevice->IsEnabled() )
		{
			InputMgr *pMgr;
			input_GetManager( &pMgr );
			input_EnableDevice( pMgr, pDevice->m_InstanceName );
			if( !pDevice->IsEnabled() ) continue;
		}

		// enumerate the objects on the device
		if( pDevice->m_pDevice )
		{
			g_pCurrentEnumDevice = pDevice;
			DeviceObject** ppList = &pList;
			hResult = pDevice->m_pDevice->EnumObjects( DeviceObjectListProc, &ppList, DIDFT_ALL );
			g_pCurrentEnumDevice = LTNULL;
		}

		// go through the list and set the device info for each new object we just found
		pListPtr = pList;
		while( pListPtr )
		{
			if( pListPtr->m_DeviceType == 0 )
			{
				pListPtr->m_DeviceType = nCurrentDeviceType;
				LTStrCpy( pListPtr->m_DeviceName, pDevice->m_InstanceName, sizeof(pListPtr->m_DeviceName) );
			}
			pListPtr = pListPtr->m_pNext;
		}
	}

	return pList;
}

static void input_FreeDeviceObjects (DeviceObject* pObjectList)
{
	DeviceObject* pObject = pObjectList;
	while( pObject )
	{
		DeviceObject* pNext = pObject->m_pNext;
		delete pObject;
		pObject = pNext;
	}
}



// --------------------------------------------------------------------- //
// Device Helper Functions.
// --------------------------------------------------------------------- //

static bool input_GetDeviceName ( uint32 nDeviceType, char* pStrBuffer, uint32 nBufferSize )
{
	if( !pStrBuffer ) return false;

	DeviceDef* pDevice = LTNULL;

	// get the device they are looking for
	if( nDeviceType == DEVICETYPE_KEYBOARD )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_KEYBOARD );
	}
	else if( nDeviceType & DEVICETYPE_MOUSE )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_MOUSE );
	}
	else if( nDeviceType & DEVICETYPE_JOYSTICK )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_JOYSTICK );
	}
	else if( nDeviceType & DEVICETYPE_GAMEPAD )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_GAMEPAD );
	}
	else if( nDeviceType & DEVICETYPE_UNKNOWN )
	{
		pDevice = input_FindDeviceByType( DI8DEVTYPE_DEVICE );
	}

	if( !pDevice ) return false;
	
	LTStrCpy( pStrBuffer, pDevice->m_InstanceName, nBufferSize );

	return true;
}

static bool input_IsDeviceEnabled ( const char* pDeviceName )
{
	DeviceDef* pDev = input_FindDeviceByName( pDeviceName );
	if( !pDev ) return false;
	
	return pDev->IsEnabled();
}

static bool input_GetDeviceObjectName( char const* pszDeviceName, uint32 nDeviceObjectId, 
									  char* pszDeviceObjectName, uint32 nDeviceObjectNameLen )
{
	DeviceDef* pDev = input_FindDeviceByName(pszDeviceName);
	if( !pDev )
		return false;

	// Get the name using the objectid.
	DIDEVICEOBJECTINSTANCE deviceEnumFindings;
	memset( &deviceEnumFindings, 0, sizeof( deviceEnumFindings ));
	deviceEnumFindings.dwSize = sizeof( deviceEnumFindings );
	HRESULT hResult = pDev->m_pDevice->GetObjectInfo( &deviceEnumFindings, nDeviceObjectId, DIPH_BYID );
	if( hResult != DI_OK )
		return false;

	strncpy( pszDeviceObjectName, deviceEnumFindings.tszName, nDeviceObjectNameLen );
	return true;
}

// --------------------------------------------------------------------- //
// Print out the available controls for a device
// --------------------------------------------------------------------- //

static bool input_ShowDeviceObjects(const char* sDeviceName)
{
	HRESULT			hResult = DI_OK;
	DeviceDef*		pDevice = LTNULL;
	DeviceObject*	pList = LTNULL;
	DeviceObject*	pListPtr = LTNULL;

//	dsi_ConsolePrint("ShowDeviceObjects : sDeviceName = %s", sDeviceName); // BLB Temp

	// make sure a name was passed in
	if ( sDeviceName == LTNULL) return false;

	// get the device with this name
	pDevice = input_FindDeviceByName( sDeviceName );

	// check if our device was found
	if( pDevice == LTNULL ) return false;
	if( pDevice->m_pDevice == LTNULL ) return false;

	// enumerate the objects on the device
	g_pCurrentEnumDevice = pDevice;
	DeviceObject** ppList = &pList;
	hResult = pDevice->m_pDevice->EnumObjects( DeviceObjectListProc, &ppList, DIDFT_ALL );
	if (hResult != DI_OK) return false;
	g_pCurrentEnumDevice = LTNULL;

	// print out device information for each object
	pListPtr = pList;
	while( pListPtr != LTNULL )
	{
//		if( pListPtr->m_DeviceType == 0 )
		{
			dsi_ConsolePrint("Device = %s  Control = %s Type = %i", pDevice->m_InstanceName, pListPtr->m_ObjectName, (int)pListPtr->m_ObjectType);
		}
		pListPtr = pListPtr->m_pNext;
	}

	// remove all the objects from the list
	DeviceObject* pObject = pList;
	while( pObject != LTNULL )
	{
		DeviceObject* pNext = pObject->m_pNext;
		delete pObject;
		pObject = pNext;
	}

	return true;
}


// --------------------------------------------------------------------- //
// Print out the available input devices to the console
// --------------------------------------------------------------------- //

static bool input_ShowInputDevices()
{
	DeviceDef *pDev;

//	dsi_ConsolePrint("ShowInputDevices"); // BLB Temp

	// Look for it with the normal name.
	pDev = g_pDeviceHead;
	while( pDev )
	{
		dsi_ConsolePrint("Device = %s", pDev->m_InstanceName);
	
		pDev = pDev->m_pNext;
	}

	return true;
}


// Input managers.
InputMgr g_MainInputMgr =
{
	input_Init,
	input_Term,
	input_IsInitted,
	input_ListDevices,
	input_PlayJoystickEffect,
	input_ReadInput,
	input_FlushInputBuffers,
	input_ClearInput,
	input_AddAction,
	input_EnableDevice,
	input_ClearBindings,
	input_AddBinding,
	input_ScaleTrigger,
	input_GetDeviceBindings,
	input_FreeDeviceBindings,
	input_StartDeviceTrack,
	input_TrackDevice,
	input_EndDeviceTrack,
	input_GetDeviceObjects,
	input_FreeDeviceObjects,
	input_GetDeviceName,
	input_GetDeviceObjectName,
	input_IsDeviceEnabled,
	input_ShowDeviceObjects,
	input_ShowInputDevices
};
#ifdef _DEBUG
    extern InputMgr g_InputSimMgr; // From InputSim.cpp.

    //command line argument mgr
    #include "icommandlineargs.h"

    //command line arg mgr interface pointer
    static ICommandLineArgs *command_line_args;
    define_holder(ICommandLineArgs, command_line_args);
#endif

LTRESULT input_GetManager(InputMgr **pMgr)
{
	*pMgr = &g_MainInputMgr;

	// Possibly use the input sim.
	#ifdef _DEBUG
		if(command_line_args->FindArgDash("InputSim"))
			*pMgr = &g_InputSimMgr;
	#endif
	
	return LT_OK;
}


