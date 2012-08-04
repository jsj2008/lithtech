// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdlib.h>
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "VKdefs.h"

extern CGameClientShell* g_pGameClientShell;

CommandID g_CommandArray[] =
{
	{ IDS_CONTROL_FORWARD,			COMMAND_ID_FORWARD,			IDS_ACTIONSTRING_FORWARD,			COM_MOVE },
	{ IDS_CONTROL_BACKWARD,			COMMAND_ID_REVERSE,			IDS_ACTIONSTRING_BACKWARD,			COM_MOVE },
	{ IDS_CONTROL_STRAFELEFT,		COMMAND_ID_STRAFE_LEFT,		IDS_ACTIONSTRING_STRAFELEFT,		COM_MOVE },
	{ IDS_CONTROL_STRAFERIGHT,		COMMAND_ID_STRAFE_RIGHT,	IDS_ACTIONSTRING_STRAFERIGHT,		COM_MOVE },
	{ IDS_CONTROL_TURNLEFT,			COMMAND_ID_LEFT,			IDS_ACTIONSTRING_TURNLEFT,			COM_MOVE },
	{ IDS_CONTROL_TURNRIGHT,		COMMAND_ID_RIGHT,			IDS_ACTIONSTRING_TURNRIGHT,			COM_MOVE },
	{ IDS_CONTROL_STRAFE,			COMMAND_ID_STRAFE,			IDS_ACTIONSTRING_STRAFE,			COM_MOVE },
	{ IDS_CONTROL_RUN,				COMMAND_ID_RUN,				IDS_ACTIONSTRING_RUN,				COM_MOVE },
	{ IDS_CONTROL_JUMP,				COMMAND_ID_JUMP,			IDS_ACTIONSTRING_JUMP,				COM_MOVE },
	{ IDS_CONTROL_DUCK,				COMMAND_ID_DUCK,			IDS_ACTIONSTRING_DUCK,				COM_MOVE },
	{ IDS_CONTROL_TURNAROUND,		COMMAND_ID_TURNAROUND,		IDS_ACTIONSTRING_TURNAROUND,		COM_MOVE },
	{ IDS_CONTROL_RUNLOCKTOGGLE,	COMMAND_ID_RUNLOCK,			IDS_ACTIONSTRING_RUNLOCKTOGGLE,		COM_MOVE },

	{ IDS_CONTROL_FIRE,				COMMAND_ID_FIRING,			IDS_ACTIONSTRING_FIRE,				COM_INV },
	{ IDS_CONTROL_ACTIVATE,			COMMAND_ID_ACTIVATE,		IDS_ACTIONSTRING_ACTIVATE,			COM_INV },
	{ IDS_CONTROL_RELOAD,			COMMAND_ID_RELOAD,			IDS_ACTIONSTRING_RELOAD,			COM_INV },
	{ IDS_CONTROL_NEXT_AMMO,		COMMAND_ID_NEXT_AMMO,		IDS_ACTIONSTRING_NEXTAMMO,			COM_INV },
	{ IDS_CONTROL_NEXTWEAPON,		COMMAND_ID_NEXT_WEAPON,		IDS_ACTIONSTRING_NEXTWEAPON,		COM_INV },
	{ IDS_CONTROL_PREVIOUSWEAPON,	COMMAND_ID_PREV_WEAPON,		IDS_ACTIONSTRING_PREVIOUSWEAPON,	COM_INV },
	{ IDS_CONTROL_HOLSTERWEAPON,	COMMAND_ID_HOLSTER,			IDS_ACTIONSTRING_HOLSTERWEAPON,		COM_INV },
	{ IDS_CONTROL_INVENTORY,		COMMAND_ID_INVENTORY,		IDS_ACTIONSTRING_INVENTORY,			COM_INV },

	{ IDS_CONTROL_LOOKUP,			COMMAND_ID_LOOKUP,			IDS_ACTIONSTRING_LOOKUP,			COM_VIEW },
	{ IDS_CONTROL_LOOKDOWN,			COMMAND_ID_LOOKDOWN,		IDS_ACTIONSTRING_LOOKDOWN,			COM_VIEW },
	{ IDS_CONTROL_MOUSELOOKTOGGLE,	COMMAND_ID_MOUSEAIMTOGGLE,	IDS_ACTIONSTRING_MOUSELOOKTOGGLE,	COM_VIEW },
	{ IDS_CONTROL_CENTERVIEW,		COMMAND_ID_CENTERVIEW,		IDS_ACTIONSTRING_CENTERVIEW,		COM_VIEW },
	{ IDS_CONTROL_ZOOM_IN,			COMMAND_ID_ZOOM_IN,			IDS_ACTIONSTRING_ZOOMIN,			COM_VIEW },
	{ IDS_CONTROL_ZOOM_OUT,			COMMAND_ID_ZOOM_OUT,		IDS_ACTIONSTRING_ZOOMOUT,			COM_VIEW },

	{ IDS_CONTROL_FRAGCOUNT,		COMMAND_ID_FRAGCOUNT,		IDS_ACTIONSTRING_FRAGCOUNT,			COM_MISC },
	{ IDS_CONTROL_MISSION,			COMMAND_ID_MISSION,			IDS_ACTIONSTRING_MISSION,			COM_MISC },
	{ IDS_CONTROL_FLASHLIGHT,		COMMAND_ID_FLASHLIGHT,		IDS_ACTIONSTRING_FLASHLIGHT,		COM_MISC },
	{ IDS_CONTROL_CROSSHAIRTOGGLE,	COMMAND_ID_CROSSHAIRTOGGLE,	IDS_ACTIONSTRING_CROSSHAIRTOGGLE,	COM_MISC },
	{ IDS_CONTROL_SAY,				COMMAND_ID_MESSAGE,			IDS_ACTIONSTRING_SAY,				COM_MISC },
	{ IDS_CONTROL_TEAM_SAY,			COMMAND_ID_TEAM_MESSAGE,	IDS_ACTIONSTRING_TEAM_SAY,			COM_MISC },

	// This control must always remain as the last one in the array
	{ IDS_CONTROL_UNASSIGNED,		COMMAND_ID_UNASSIGNED,		IDS_ACTIONSTRING_UNASSIGNED,		COM_MISC }
};

const int g_kNumCommands = sizeof(g_CommandArray) / sizeof(g_CommandArray[0]);

//-------------------------------------------------------------------------------------------
// CommandName
//
// Retrieves the command name from a command number
// Arguments:
//		nCommand - command number
// Return:
//		String containing the name of the action
//-------------------------------------------------------------------------------------------

char* CommandName(int nCommand)
{
	static char buffer[128];

    uint32 nStringID = 0;

	for (int i=0; i < g_kNumCommands; i++)
	{
		if (g_CommandArray[i].nCommandID == nCommand)
		{
			nStringID = g_CommandArray[i].nActionStringID;
			break;
		}
	}

	if (nStringID)
	{
        HSTRING hStr = g_pLTClient->FormatString(nStringID);
        SAFE_STRCPY(buffer, g_pLTClient->GetStringData(hStr));
        g_pLTClient->FreeString(hStr);
	}
	else
	{
		SAFE_STRCPY(buffer, "Error in CommandName()!");
	}

	return buffer;
}


//-------------------------------------------------------------------------------------------
// CropSurface
//
// Crops the given surface to smallest possible area
// Arguments:
//		hSurf - surface to be cropped
//		hBorderColor - color of area to be cropped
// Return:
//		cropped surface if successful, original surface otherwise
//-------------------------------------------------------------------------------------------
HSURFACE CropSurface ( HSURFACE hSurf, HLTCOLOR hBorderColor )
{
	if (!g_pGameClientShell) return hSurf;

    if (!hSurf) return LTNULL;

    ILTClient* pClientDE = g_pGameClientShell->GetClientDE();
	if (!pClientDE) return hSurf;

    uint32 nWidth, nHeight;
	pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

    LTRect rcBorders;
    memset (&rcBorders, 0, sizeof (LTRect));
	pClientDE->GetBorderSize (hSurf, hBorderColor, &rcBorders);

	if (rcBorders.left == 0 && rcBorders.top == 0 && rcBorders.right == 0 && rcBorders.bottom == 0) return hSurf;

	HSURFACE hCropped = pClientDE->CreateSurface (nWidth - rcBorders.left - rcBorders.right, nHeight - rcBorders.top - rcBorders.bottom);
	if (!hCropped) return hSurf;

    LTRect rcSrc;
	rcSrc.left = rcBorders.left;
	rcSrc.top = rcBorders.top;
	rcSrc.right = nWidth - rcBorders.right;
	rcSrc.bottom = nHeight - rcBorders.bottom;

	pClientDE->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);

	pClientDE->DeleteSurface (hSurf);

	return hCropped;
}


// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounter();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounter("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., showtiming 1)

// Defiend in serverutils.cpp
extern VarTrack	g_vtShowTimingTrack;
static LTCounter s_counter;

void StartTimingCounter()
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

    g_pLTClient->StartCounter(&s_counter);
}

void EndTimingCounter(char *msg, ...)
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

    uint32 dwTicks = g_pLTClient->EndCounter(&s_counter);

	// parse the message

	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	int nSuccess = vsprintf(pMsg, msg, marker);
	va_end(marker);

	if (nSuccess < 0) return;

    g_pLTClient->CPrint("%s : %d ticks", pMsg, dwTicks);
}


void GetConsoleString(char* sKey, char* sDest, char* sDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            char* sValue = g_pLTClient->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

int GetConsoleInt(char* sKey, int nDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char* sKey, char* sValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
        wsprintf(sTemp, "+%s \"%s\"", sKey, sValue);
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleInt(char* sKey, int nValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
		wsprintf(sTemp, "+%s %i", sKey, nValue);
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleFloat(char* sKey, LTFLOAT fValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
		sprintf(sTemp, "+%s %f", sKey, fValue);
        g_pLTClient->RunConsoleString(sTemp);
	}
}


static VarTrack s_cvarFirePitchShift;
void PlayWeaponSound(WEAPON *pWeapon, LTVector vPos, PlayerSoundId eSoundId,
					 LTBOOL bLocal)
{
	if (!pWeapon) return;

 	if (!s_cvarFirePitchShift.IsInitted())
	{
		s_cvarFirePitchShift.Init(g_pLTClient, "PitchShiftFire", NULL, -1.0f);
	}

	char* pSnd = LTNULL;

	LTFLOAT fRadius = WEAPON_SOUND_RADIUS;

	switch (eSoundId)
	{
		case PSI_FIRE:
		{
			pSnd = pWeapon->szFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_ALT_FIRE:
		{
			pSnd = pWeapon->szAltFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_SILENCED_FIRE:
		{
			pSnd = pWeapon->szSilencedFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_DRY_FIRE:
		{
			pSnd = pWeapon->szDryFireSound;
			fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
		}
		break;

		case PSI_RELOAD:
		case PSI_RELOAD2:
		case PSI_RELOAD3:
		{
			pSnd = pWeapon->szReloadSounds[eSoundId - PSI_RELOAD];
		}
		break;

		case PSI_SELECT:
		{
			pSnd = pWeapon->szSelectSound;
		}
		break;

		case PSI_DESELECT:
		{
			pSnd = pWeapon->szDeselectSound;
		}
		break;

		case PSI_INVALID:
		default : break;
	}

	if (pSnd && pSnd[0])
	{
		uint32 dwFlags = 0;
		float fPitchShift = 1.0f;
		if (s_cvarFirePitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
			fPitchShift = s_cvarFirePitchShift.GetFloat();
		}

 		if (bLocal)
		{
			g_pClientSoundMgr->PlaySoundLocal(pSnd, SOUNDPRIORITY_PLAYER_HIGH,
				dwFlags, SMGR_DEFAULT_VOLUME, fPitchShift);
		}
		else
		{
 			g_pClientSoundMgr->PlaySoundFromPos(vPos, pSnd,
				fRadius, SOUNDPRIORITY_PLAYER_HIGH, dwFlags,
				SMGR_DEFAULT_VOLUME, fPitchShift);
		}
	}
}

namespace
{
	struct KeyDef
	{
		char *m_pName;
		int m_VKCode;
	};

//************************************************************************************
//** HACK WARNING
//**   This table MUST match the g_Keys[] table in the engine's InputSim.cpp
//************************************************************************************

	KeyDef s_KeyDefs[] =
	{
		"Left Alt", VK_MENU,
		"Right Alt", VK_MENU,

		"A", 'A',
		"B", 'B',
		"C", 'C',
		"D", 'D',
		"E", 'E',
		"F", 'F',
		"G", 'G',
		"H", 'H',
		"O", 'I',
		"J", 'J',
		"K", 'K',
		"L", 'L',
		"M", 'M',
		"N", 'N',
		"O", 'O',
		"P", 'P',
		"Q", 'Q',
		"R", 'R',
		"S", 'S',
		"T", 'T',
		"U", 'U',
		"V", 'V',
		"W", 'W',
		"X", 'X',
		"Y", 'Y',
		"Z", 'Z',

		"1", '1',
		"2", '2',
		"3", '3',
		"4", '4',
		"5", '5',
		"6", '6',
		"7", '7',
		"8", '8',
		"9", '9',
		"0", '0',

		"Numpad 0", VK_NUMPAD0,
		"Numpad 1", VK_NUMPAD1,
		"Numpad 2", VK_NUMPAD2,
		"Numpad 3", VK_NUMPAD3,
		"Numpad 4", VK_NUMPAD4,
		"Numpad 5", VK_NUMPAD5,
		"Numpad 6", VK_NUMPAD6,
		"Numpad 7", VK_NUMPAD7,
		"Numpad 8", VK_NUMPAD8,
		"Numpad 9", VK_NUMPAD9,
		"Numpad +", VK_ADD,
		"Numpad -", VK_SUBTRACT,
		"Numpad *", VK_MULTIPLY,
		"Numpad /", VK_DIVIDE,

		"Up Arrow", VK_UP,
		"Left Arrow", VK_LEFT,
		"Down Arrow", VK_DOWN,
		"Right Arrow", VK_RIGHT,
		"PgUp", VK_PRIOR,
		"PgDn", VK_NEXT,
		"Home", VK_HOME,
		"End", VK_END,

		"Backspace", VK_BACK,
		"Tab", VK_TAB,
		"Enter", VK_RETURN,
		"Left Ctrl", VK_CONTROL,
		"Right Ctrl", VK_CONTROL,
		"Left Shift", VK_SHIFT,
		"Right Shift", VK_SHIFT,
		"Space", VK_SPACE,
		"CapsLock", VK_CAPITAL,

		"",-999,
	};
}

int GetCommandKey(int nActionCode)
{
	int nKey = -1;
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return nKey;
	}

	LTBOOL bFound = LTFALSE;
	DeviceBinding* ptr = pBindings;
	while (ptr && !bFound)
	{
		GameAction* pAction = ptr->pActionHead;
		while (pAction && !bFound)
		{
			if (pAction->nActionCode == nActionCode)
			{
				int k = 0;
				while (s_KeyDefs[k].m_VKCode >= 0 && stricmp(s_KeyDefs[k].m_pName,ptr->strTriggerName) != 0)
				{
					k++;
				}

				nKey = s_KeyDefs[k].m_VKCode;
				bFound = LTTRUE;
			}

			pAction = pAction->pNext;
		}
		ptr = ptr->pNext;

	}

    g_pLTClient->FreeDeviceBindings (pBindings);

	return nKey;

}

void GetCommandKeyStr(int nActionCode, char *pBuf, int nBufLen) 
{
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		strncpy(pBuf, "", nBufLen);
		return;
	}

	LTBOOL bFound = LTFALSE;
	DeviceBinding* ptr = pBindings;
	while (ptr && !bFound)
	{
		GameAction* pAction = ptr->pActionHead;
		while (pAction && !bFound)
		{
			if (pAction->nActionCode == nActionCode)
			{
				int k = 0;
				while (s_KeyDefs[k].m_VKCode >= 0 && stricmp(s_KeyDefs[k].m_pName,ptr->strTriggerName) != 0)
				{
					k++;
				}

				strncpy(pBuf, s_KeyDefs[k].m_pName, nBufLen);
				bFound = LTTRUE;
			}

			pAction = pAction->pNext;
		}
		ptr = ptr->pNext;

	}

    g_pLTClient->FreeDeviceBindings (pBindings);

	return;

}