// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "SoundTypes.h"
#include "iltclient.h"
#include "CommonUtilities.h"
#include "WeaponMgr.h"
#include "SoundMgr.h"

extern const int g_kNumCommands;

enum CommandType
{
	COM_MOVE,
	COM_INV,
	COM_VIEW,
	COM_MISC
};

struct CommandID
{
	int		nStringID;
	int		nCommandID;
	int		nActionStringID;
	int		nCommandType;
};

struct DSize
{
	DSize()		{ cx = 0; cy = 0; }

	unsigned long	cx;
	unsigned long	cy;
};

char* CommandName(int nCommand);

HSURFACE CropSurface(HSURFACE hSurf, HLTCOLOR hBorderColor);

int		GetConsoleInt(char* sKey, int nDefault);
void	GetConsoleString(char* sKey, char* sDest, char* sDefault);
void	WriteConsoleString(char* sKey, char* sValue);
void	WriteConsoleInt(char* sKey, int nValue);
LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault);
void	WriteConsoleFloat(char* sKey, LTFLOAT fValue);


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
void StartTimingCounter();
void EndTimingCounter(char *msg, ...);

inline LTBOOL GetAttachmentSocketTransform(HOBJECT hObj, char* pSocketName,
                                          LTVector & vPos, LTRotation & rRot)
{
    if (!hObj || !pSocketName) return LTFALSE;

	HOBJECT hAttachList[30];
    uint32 dwListSize, dwNumAttachments;

    if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList,
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
        for (uint32 i=0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				HMODELSOCKET hSocket;

				if (g_pModelLT->GetSocket(hAttachList[i], pSocketName, hSocket) == LT_OK)
				{
					LTransform transform;
                    if (g_pModelLT->GetSocketTransform(hAttachList[i], hSocket, transform, LTTRUE) == LT_OK)
					{
						g_pTransLT->Get(transform, vPos, rRot);
                        return LTTRUE;
					}
				}
			}
		}
	}

    return LTFALSE;
}

void PlayWeaponSound(WEAPON *pWeapon, LTVector vPos, PlayerSoundId eSoundId,
					 LTBOOL bLocal=LTFALSE);

//return the virtual key code bound to an action, returns -1 if no binding can be found.
int GetCommandKey(int nActionCode);

//get the name of the key bound to an action, "" if no binding can be found.
void GetCommandKeyStr(int nActionCode, char *pBuf, int nBufLen);

#define IsKeyDown(key)		(GetAsyncKeyState(key) & 0x80000000)

#endif // __CLIENT_UTILITIES_H__