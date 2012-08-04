// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "SoundTypes.h"
#include "iltclient.h"
#include "CommonUtilities.h"
#include "SoundMgr.h"
#include "WeaponDB.h"
#include "StringUtilities.h"

#if defined(PLATFORM_WIN32)
#include <shellapi.h> // For ShellExecuteEx (LaunchFromString)
#include <shlwapi.h> // For AssocQueryString
#endif // PLATFORM_WIN32

struct DSize
{
	DSize()		{ cx = 0; cy = 0; }

	unsigned long	cx;
	unsigned long	cy;
};

bool	GetConsoleBool(char const* sKey, bool bDefault);
int		GetConsoleInt(char const* sKey, int nDefault);
char const* GetConsoleString(char* pszKey, char const* pszDefault);
char*	GetConsoleTempString(char const* sKey, char const* sDefault);
float GetConsoleFloat(char const* sKey, float fDefault);
void	WriteConsoleBool(char const* sKey, bool bValue);
void	WriteConsoleInt(char const* sKey, int nValue);
void	WriteConsoleString(char const* sKey, char const* sValue);
void	WriteConsoleFloat(char const* sKey, float fValue);


// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounterClient();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounterClient("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., showtiming 1)
void StartTimingCounterClient();
void EndTimingCounterClient(char *msg, ...);

inline bool GetAttachmentSocketTransform(HOBJECT hObj, char* pSocketName,
                                          LTVector & vPos, LTRotation & rRot)
{
    if( !hObj || !pSocketName )
		return false;

	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;

	// Check the actual object first for the socket before traversing the attachments...
	if( g_pModelLT->GetSocket( hObj, pSocketName, hSocket ) == LT_OK)
	{
		LTTransform transform;
		if( g_pModelLT->GetSocketTransform( hObj, hSocket, transform, true ) == LT_OK)
		{
			vPos = transform.m_vPos;
			rRot = transform.m_rRot;
			return true;
		}
	}

	// Not found on the actual object, search the attachments...
	HOBJECT hAttachList[30];
    uint32 dwListSize, dwNumAttachments;

    if (g_pCommonLT->GetAttachments(hObj, hAttachList,
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
        for (uint32 i=0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				if (g_pModelLT->GetSocket(hAttachList[i], pSocketName, hSocket) == LT_OK)
				{
					LTTransform transform;
                    if (g_pModelLT->GetSocketTransform(hAttachList[i], hSocket, transform, true) == LT_OK)
					{
						vPos = transform.m_vPos;
						rRot = transform.m_rRot;
                        return true;
					}
				}
			}
		}
	}

    return false;
}

struct WEAPON;
enum PlayerSoundId;

void PlayWeaponSound( HWEAPON hWeaponID, bool bUseAIData, const LTVector &vPos, PlayerSoundId eSoundId,
					 bool bLocal=false, HOBJECT hFromObject=NULL );

#define IsKeyDown(key)		(GetAsyncKeyState(key) & 0x80000000)

// Send an empty message to the server
LTRESULT SendEmptyServerMsg(uint32 nMsgID, uint32 nFlags = MESSAGE_GUARANTEED);

bool IsMultiplayerGameClient();

// Helper for finding the normal and point of intersection beneath a given point
bool GetIntersectionUnderPoint( LTVector &vInPt, HOBJECT *pFilterList, LTVector &vOutNormal, LTVector &vOutPt );

// Find the normal of the plane we would like to contour to given a position and dims of an object.
LTVector GetContouringNormal( LTVector &vPos, LTVector &vDims, LTVector &vForward, LTVector &vRight, HOBJECT *pFilterList );

// Get the pitch amount and percents to apply for pitch and roll based on the forward direction and plane normal.
void GetContouringInfo( LTVector &vForward, LTVector &vNormal, float &fOutAmount, float &fOutPitchPercent, float &fOutRollPercent );

// Limits the angle to a -PI to PI range.
inline float LimitToPosNegPi( float fAngle )
{
	// Copy the angle and make sure it's under 2 pi.
	float fNewAngle = ( float )fmod( fAngle, MATH_CIRCLE );

	if( fNewAngle > MATH_PI )
		fNewAngle = fNewAngle - MATH_CIRCLE;
	else if( fNewAngle < -MATH_PI )
		fNewAngle = fNewAngle + MATH_CIRCLE;
	
	return fNewAngle;
}

void UpdateAttachmentVisibility( HOBJECT hParent );


// Find the specified socket on one of the given objects's attachments.
// Returns true if the socket is found.
bool FindAttachmentSocket(HOBJECT hParent, const char* pszSocket, HOBJECT* pOutObj=NULL, HMODELSOCKET* pOutSocket=NULL);

// parse a help string and replace commands with the appropriate bound control
//		e.g. replace '|Activate|' with 'F'
const wchar_t* CreateHelpString(const char *pszStringID);


// Launches application
class LaunchApplication
{
public:

	// Used to switch to a specific screen after launching.
	enum SwitchToScreen
	{
		kSwitchToScreen_None,
		kSwitchToScreen_Single,
		kSwitchToScreen_Multi,
		kSwitchToScreen_Performance,
	};

	// Launch the singleplayer exe.
	static bool LaunchSinglePlayerExe( SwitchToScreen eSwitchToScreen );
	// Launch the multiplayer exe.
	static bool LaunchMultiPlayerExe( SwitchToScreen eSwitchToScreen );

	// Launchers patchupdate given URL to patch file.
	static bool	 LaunchPatchUpdate( char const* pszUrl );

	// Launches Browser with a given MOTD Link
	static bool	LaunchMOTDLink( char const* pszUrl );

	// Launches the end splash screen.
	static bool LaunchEndSplashScreen( );

	// Launches the serverapp.
	static bool LaunchServerApp( char const* pszOptionsFile );

	// Runs a start command.
	static bool	LaunchFromString( char const* pszFile, char const* pszParameters, bool bMaximizeNewApp, bool bShutdownCurrentApp );

private:

	// Utility function to launch main executable.
	static bool LaunchMainExe( char const* pszExeName, SwitchToScreen eSwitchToScreen );
};

#endif // __CLIENT_UTILITIES_H__