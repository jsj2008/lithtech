
// This is a hacked up input manager to get around the problems with DInput that make it hard
// to debug on a local machine.  This just uses Windows calls so it won't screw up the input
// while debugging.  This input manager does NOT save bindings.

#include "bdefs.h"
#include "input.h"
#include "dsys_interface.h"
#include "dinput.h"


#ifdef _DEBUG


#define SPECIAL_MOUSEX	-50000
#define SPECIAL_MOUSEY	-50001


struct ISKey
{
	char *m_pName;
	int m_VKCode, m_DIKCode;
	float m_Scale;
};

struct ISAction
{
	char m_Name[100];
	int m_Code;
	ISAction *m_pNext;
};

struct ISBinding
{
	ISKey *m_pKey;
	ISAction *m_pAction;
	ISBinding *m_pNext;
};


ISBinding *g_Bindings;
ISAction *g_Actions;

bool g_bFirstCursorRead;
POINT g_LastCursorPos;


ISKey g_Keys[] = 
{
	"X-axis", SPECIAL_MOUSEX, -1, 1.0f, 
	"Y-axis", SPECIAL_MOUSEY, -1, 1.0f, 

	"Button 0", VK_LBUTTON, -1, 1.0f, 
	"Button 1", VK_RBUTTON, -1, 1.0f, 

	"Left Alt", VK_MENU, -1, 1.0f,
	"Right Alt", VK_MENU, -1, 1.0f,

	"A", 'A', DIK_A, 1.0f, 
	"B", 'B', DIK_B, 1.0f, 
	"C", 'C', DIK_C, 1.0f, 
	"D", 'D', DIK_D, 1.0f, 
	"E", 'E', DIK_E, 1.0f, 
	"F", 'F', DIK_F, 1.0f, 
	"G", 'G', DIK_G, 1.0f, 
	"H", 'H', DIK_H, 1.0f, 
	"O", 'I', DIK_I, 1.0f, 
	"J", 'J', DIK_J, 1.0f, 
	"K", 'K', DIK_K, 1.0f, 
	"L", 'L', DIK_L, 1.0f, 
	"M", 'M', DIK_M, 1.0f, 
	"N", 'N', DIK_N, 1.0f, 
	"O", 'O', DIK_O, 1.0f, 
	"P", 'P', DIK_P, 1.0f, 
	"Q", 'Q', DIK_Q, 1.0f, 
	"R", 'R', DIK_R, 1.0f, 
	"S", 'S', DIK_S, 1.0f, 
	"T", 'T', DIK_T, 1.0f, 
	"U", 'U', DIK_U, 1.0f, 
	"V", 'V', DIK_V, 1.0f, 
	"W", 'W', DIK_W, 1.0f, 
	"X", 'X', DIK_X, 1.0f, 
	"Y", 'Y', DIK_Y, 1.0f, 
	"Z", 'Z', DIK_Z, 1.0f, 

	"1", '1', DIK_1, 1.0f, 
	"2", '2', DIK_2, 1.0f, 
	"3", '3', DIK_3, 1.0f, 
	"4", '4', DIK_4, 1.0f, 
	"5", '5', DIK_5, 1.0f, 
	"6", '6', DIK_6, 1.0f, 
	"7", '7', DIK_7, 1.0f, 
	"8", '8', DIK_8, 1.0f, 
	"9", '9', DIK_9, 1.0f, 
	"0", '0', DIK_0, 1.0f, 

	"F1", VK_F1, DIK_F1, 1.0f, 
	"F2", VK_F2, DIK_F2, 1.0f, 
	"F3", VK_F3, DIK_F3, 1.0f, 
	"F4", VK_F4, DIK_F4, 1.0f, 
	"F5", VK_F5, DIK_F5, 1.0f, 
	"F6", VK_F6, DIK_F6, 1.0f, 
	"F7", VK_F7, DIK_F7, 1.0f, 
	"F8", VK_F8, DIK_F8, 1.0f, 
	"F9", VK_F9, DIK_F9, 1.0f, 
	"F10", VK_F10, DIK_F10, 1.0f, 
	"F11", VK_F11, DIK_F11, 1.0f, 
	"F12", VK_F12, DIK_F12, 1.0f, 
	
	"Numpad 1", VK_NUMPAD1, DIK_NUMPAD1, 1.0f, 
	"Numpad 2", VK_NUMPAD2, DIK_NUMPAD2, 1.0f, 
	"Numpad 3", VK_NUMPAD3, DIK_NUMPAD3, 1.0f, 
	"Numpad 4", VK_NUMPAD4, DIK_NUMPAD4, 1.0f, 
	"Numpad 5", VK_NUMPAD5, DIK_NUMPAD5, 1.0f, 
	"Numpad 6", VK_NUMPAD6, DIK_NUMPAD6, 1.0f, 
	"Numpad 7", VK_NUMPAD7, DIK_NUMPAD7, 1.0f, 
	"Numpad 8", VK_NUMPAD8, DIK_NUMPAD8, 1.0f, 
	"Numpad 9", VK_NUMPAD9, DIK_NUMPAD9, 1.0f, 
	"Numpad +", VK_ADD, DIK_ADD, 1.0f, 
	"Numpad -", VK_SUBTRACT, DIK_SUBTRACT, 1.0f, 
	"Numpad *", VK_MULTIPLY, DIK_MULTIPLY, 1.0f, 
	"Numpad /", VK_DIVIDE, DIK_DIVIDE, 1.0f, 
	
	"Up Arrow", VK_UP, DIK_UPARROW, 1.0f, 
	"Left Arrow", VK_LEFT, DIK_LEFTARROW, 1.0f, 
	"Down Arrow", VK_DOWN, DIK_DOWNARROW, 1.0f, 
	"Right Arrow", VK_RIGHT, DIK_RIGHTARROW, 1.0f, 
	"PgUp", VK_PRIOR, DIK_PRIOR, 1.0f, 
	"PgDn", VK_NEXT, DIK_NEXT, 1.0f, 
	"Home", VK_HOME, DIK_HOME, 1.0f, 
	"End", VK_END, DIK_END, 1.0f, 

	"Backspace", VK_BACK, DIK_BACK, 1.0f, 
	"Tab", VK_TAB, DIK_TAB, 1.0f, 
	"Enter", VK_RETURN, DIK_RETURN, 1.0f, 
	"Left Ctrl", VK_CONTROL, DIK_LCONTROL, 1.0f, 
	"Right Ctrl", VK_CONTROL, DIK_RCONTROL, 1.0f, 
	"Left Shift", VK_SHIFT, DIK_LSHIFT, 1.0f, 
	"Right Shift", VK_SHIFT, DIK_RSHIFT, 1.0f, 
	"Space", VK_SPACE, DIK_SPACE, 1.0f, 
	"CapsLock", VK_CAPITAL, DIK_CAPITAL, 1.0f 
};
#define NUM_KEYS (sizeof(g_Keys)/sizeof(g_Keys[0]))

ISKey* isim_FindKey(const char *pName)
{
	uint32 i;
	int testCode;

	if(strlen(pName) > 2 && pName[0] == '#' && pName[1] == '#')
	{
		pName += 2;

		testCode = atoi(pName);
		for(i=0; i < NUM_KEYS; i++)
		{
			if(g_Keys[i].m_DIKCode == testCode)
			{
				return &g_Keys[i];
			}
		}
		
		// Fall below where it looks without the ##.
	}

	for(i=0; i < NUM_KEYS; i++)
	{
		if(stricmp(pName, g_Keys[i].m_pName) == 0)
		{
			return &g_Keys[i];
		}
	}

	return LTNULL;
}

ISAction* isim_FindAction(const char *pName)
{
	ISAction *pCur;

	for(pCur=g_Actions; pCur; pCur=pCur->m_pNext)
	{
		if(stricmp(pCur->m_Name, pName) == 0)
			return pCur;
	}
	
	return LTNULL;
}


bool isim_Init(InputMgr *pMgr, ConsoleState *pState)
{
	g_Bindings = LTNULL;
	g_Actions = LTNULL;
	g_bFirstCursorRead = true;
	return true;
}

void isim_Term(InputMgr *pMgr)
{
	ISBinding *pBinding, *pNextBinding;
	ISAction *pAction, *pNextAction;

	for(pBinding=g_Bindings; pBinding; )
	{
		pNextBinding = pBinding->m_pNext;
		dfree(pBinding);
		pBinding = pNextBinding;
	}
	g_Bindings = LTNULL;

	for(pAction=g_Actions; pAction; )
	{
		pNextAction = pAction->m_pNext;
		dfree(pAction);
		pAction = pNextAction;
	}
	g_Actions = LTNULL;
}

bool isim_IsInitted(InputMgr *pMgr)
{
	return true;
}

void isim_ListDevices(InputMgr *pMgr)
{
	dsi_ConsolePrint("SIMULATED INPUT ------------------");
	dsi_ConsolePrint("Only devices are keyboard and mouse");
}

long isim_PlayJoystickEffect(InputMgr *pMgr, const char* strEffectName, float x, float y)
{
	return 0;
}

void isim_ReadInput(InputMgr *pMgr, uint8 *pActionsOn, float axisOffsets[3])
{
	ISBinding *pBinding;
	float value;
	POINT cursorPos;


	axisOffsets[0] = axisOffsets[1] = axisOffsets[2] = 0.0f;

	if(g_bFirstCursorRead)
	{
		GetCursorPos(&g_LastCursorPos);
		g_bFirstCursorRead = false;
	}

	GetCursorPos(&cursorPos);

	for(pBinding=g_Bindings; pBinding; pBinding=pBinding->m_pNext)
	{
		value = 0.0f;

		if(pBinding->m_pKey->m_VKCode == SPECIAL_MOUSEX)
		{
			value = (float)(cursorPos.x - g_LastCursorPos.x) * pBinding->m_pKey->m_Scale;
		}
		else if(pBinding->m_pKey->m_VKCode == SPECIAL_MOUSEY)
		{
			value = (float)(cursorPos.y - g_LastCursorPos.y) * pBinding->m_pKey->m_Scale;
		}
		else
		{
			if(GetAsyncKeyState(pBinding->m_pKey->m_VKCode) & 0x8000)
			{
				value = 1.0f;
			}
		}

		switch(pBinding->m_pAction->m_Code)
		{
			case -1:
			{
				axisOffsets[0] += value;
			}
			break;

			case -2:
			{
				axisOffsets[1] += value;
			}
			break;

			case -3:
			{
				axisOffsets[2] += value;
			}
			break;

			default:
			{
				pActionsOn[pBinding->m_pAction->m_Code] |= (uint8)value;
			}
			break;
		}
	}

	memcpy(&g_LastCursorPos, &cursorPos, sizeof(cursorPos));
}

bool isim_FlushInputBuffers(InputMgr *pMgr)
{
	return true;
}

LTRESULT isim_ClearInput()
{
	return LT_OK;
}

void isim_AddAction(InputMgr *pMgr, const char *pActionName, int actionCode)
{
	ISAction *pAction = isim_FindAction(pActionName);

	if(pAction)
		return;

	LT_MEM_TRACK_ALLOC(pAction = (ISAction*)dalloc(sizeof(ISAction)),LT_MEM_TYPE_INPUT);
	LTStrCpy(pAction->m_Name, pActionName, sizeof(pAction->m_Name));
	pAction->m_Code = actionCode;
	pAction->m_pNext = g_Actions;
	g_Actions = pAction;
}

bool isim_EnableDevice(InputMgr *pMgr, const char *pDeviceName)
{
	return true;
}

bool isim_ClearBindings(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName)
{
	return true;
}

bool isim_AddBinding(InputMgr *pMgr, 
	const char *pDeviceName, const char *pTriggerName, const char *pActionName,
	float rangeLow, float rangeHigh)
{
	ISBinding *pBinding;
	
	ISAction *pAction = isim_FindAction(pActionName);
	if(!pAction)
		return false;

	ISKey *pKey = isim_FindKey(pTriggerName);
	if(!pKey)
		return false;

	LT_MEM_TRACK_ALLOC(pBinding = (ISBinding*)dalloc(sizeof(ISBinding)),LT_MEM_TYPE_INPUT);
	pBinding->m_pKey = pKey;
	pBinding->m_pAction = pAction;
	pBinding->m_pNext = g_Bindings;
	g_Bindings = pBinding;		
	return true;
}

bool isim_ScaleTrigger(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName, float scale, float fRangeScaleMin, float fRangeScaleMax, float fRangeScalePreCenterOffset )
{
	ISKey *pKey = isim_FindKey(pTriggerName);

	if(pKey)
	{
		pKey->m_Scale = scale;
		return true;
	}
	else
	{
		return false;
	}
}

DeviceBinding* isim_GetDeviceBindings ( uint32 nDevice )
{
	DeviceBinding* pBindingsHead = LTNULL;
	ISBinding *pISBinding;

	pISBinding = g_Bindings;
	while(pISBinding)
	{
		DeviceBinding* pBinding;
		LT_MEM_TRACK_ALLOC(pBinding = new DeviceBinding,LT_MEM_TYPE_INPUT);
		if( !pBinding ) break;
		memset( pBinding, 0, sizeof(DeviceBinding) );

		LTStrCpy( pBinding->strDeviceName, "Keyboard", sizeof(pBinding->strDeviceName) );
		LTStrCpy( pBinding->strTriggerName, pISBinding->m_pKey->m_pName, sizeof(pBinding->strTriggerName) );

		// go through the actions, adding them to the trigger
		GameAction* pActionHead = LTNULL;
		
		GameAction* pNewAction;
		LT_MEM_TRACK_ALLOC(pNewAction = new GameAction,LT_MEM_TYPE_INPUT);
		memset( pNewAction, 0, sizeof(GameAction) );

		pNewAction->nActionCode = pISBinding->m_pAction->m_Code;
		strcpy (pNewAction->strActionName, pISBinding->m_pAction->m_Name);
		pNewAction->pNext = pActionHead;
		pActionHead = pNewAction;

		pBinding->pActionHead = pActionHead;

		pBinding->pNext = pBindingsHead;
		pBindingsHead = pBinding;

		pISBinding = pISBinding->m_pNext;
	}

	return pBindingsHead;
}

void isim_FreeDeviceBindings ( DeviceBinding* pBindings )
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

bool isim_StartDeviceTrack(InputMgr *pMgr, uint32 nDeviceFlags, uint32 nBufferSize)
{
	return true;
}

bool isim_TrackDevice(DeviceInput *pInputArray, uint32 *pnInOut)
{
	*pnInOut = 0;
	return true;
}

bool isim_EndDeviceTrack()
{
	return true;
}

DeviceObject* isim_GetDeviceObjects ( uint32 nDeviceFlags )
{
	return LTNULL;
}

void isim_FreeDeviceObjects ( DeviceObject* pList )
{
}

bool isim_GetDeviceName ( uint32 nDeviceType, char* pStrBuffer, uint32 nBufferSize )
{
	return true;
}

bool isim_IsDeviceEnabled ( const char* pDeviceName )
{
	return true;
}

static bool isim_GetDeviceObjectName( char const* pszDeviceName, uint32 nDeviceObjectId, 
									  char* pszDeviceObjectName, uint32 nDeviceObjectNameLen )
{
	return true;
}

InputMgr g_InputSimMgr =
{
	isim_Init,
	isim_Term,
	isim_IsInitted,
	isim_ListDevices,
	isim_PlayJoystickEffect,
	isim_ReadInput,
	isim_FlushInputBuffers,
	isim_ClearInput,
	isim_AddAction,
	isim_EnableDevice,
	isim_ClearBindings,
	isim_AddBinding,
	isim_ScaleTrigger,
	isim_GetDeviceBindings,
	isim_FreeDeviceBindings,
	isim_StartDeviceTrack,
	isim_TrackDevice,
	isim_EndDeviceTrack,
	isim_GetDeviceObjects,
	isim_FreeDeviceObjects,
	isim_GetDeviceName,
	isim_GetDeviceObjectName,
	isim_IsDeviceEnabled
};


#endif // _DEBUG

