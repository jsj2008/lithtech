// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdlib.h>
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "VKdefs.h"
#include "ClientServerShared.h"
#include "ClientConnectionMgr.h"
#include "StringEditMgr.h"
#include "lttimeutils.h"
#include "CMoveMgr.h"
#include "PlayerMgr.h"
#include "ClientWeapon.h"
#include "ClientWeaponMgr.h"
#include "PlayerBodyMgr.h"
#include "sys/win/mpstrconv.h"
#include "ltfileoperations.h"

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

extern VarTrack	g_vtShowTimingTrack;
static TLTPrecisionTime s_StartTimer;

void StartTimingCounterClient()
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

	s_StartTimer = LTTimeUtils::GetPrecisionTime();
}

void EndTimingCounterClient(char *pszFormat, ...)
{
    if (!g_pLTClient || g_vtShowTimingTrack.GetFloat() < 1.0f) return;

	double fElapsedMS = LTTimeUtils::GetPrecisionTimeIntervalMS(s_StartTimer, LTTimeUtils::GetPrecisionTime());

	// parse the message

	char szMsg[256];
	va_list marker;
	va_start(marker, pszFormat);
	int nSuccess = LTVSNPrintF(szMsg, LTARRAYSIZE(szMsg), pszFormat, marker);
	va_end(marker);

	if (nSuccess < 0) return;

	g_pLTClient->CPrint("%s : %0.3fms", szMsg, fElapsedMS);
}


char const* GetConsoleString(char* pszKey, char const* pszDefault)
{
	if (g_pLTClient)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(pszKey);
		if (hVar)
		{
			const char* pszValue = g_pLTClient->GetConsoleVariableString(hVar);
			if (pszValue)
			{
				return pszValue;
			}
		}
	}

	return pszDefault;
}

char*  GetConsoleTempString(char const* sKey, char const* sDefault)
{
	static char szTmp[256];
	szTmp[0] = NULL;
	if (g_pLTClient)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(( char* )sKey);
		if (hVar)
		{
			const char* sValue = g_pLTClient->GetConsoleVariableString(hVar);
			if (sValue)
			{
				LTStrCpy(szTmp, sValue, LTARRAYSIZE(szTmp));
				return szTmp;
			}
		}
	}

	LTStrCpy(szTmp, sDefault, LTARRAYSIZE(szTmp));
	return szTmp;
}

int GetConsoleInt(char const* sKey, int nDefault)
{
    if (g_pLTClient)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(( char* )sKey);
		if (hVar)
		{
			float fValue = g_pLTClient->GetConsoleVariableFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}
bool GetConsoleBool( char const* sKey, bool bDefault)
{
    if (g_pLTClient)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(( char* )sKey);
		if (hVar)
		{
			float fValue = g_pLTClient->GetConsoleVariableFloat(hVar);
			return(fValue != 0.0f);
		}
	}

	return(bDefault);
}

float GetConsoleFloat(char const* sKey, float fDefault)
{
    if (g_pLTClient)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(( char* )sKey);
		if (hVar)
		{
			float fValue = g_pLTClient->GetConsoleVariableFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char const* sKey, char const* sValue)
{
    if (g_pLTClient)
	{
		g_pLTClient->SetConsoleVariableString(sKey, sValue);
	}
}

void WriteConsoleInt(char const* sKey, int nValue)
{
    if (g_pLTClient)
	{
		g_pLTClient->SetConsoleVariableFloat(sKey, (float)nValue);
	}
}

void WriteConsoleBool(char const* sKey, bool bValue)
{
    if (g_pLTClient)
	{
		g_pLTClient->SetConsoleVariableFloat(sKey, (bValue) ? 1.0f : 0.0f);
	}
}

void WriteConsoleFloat(char const* sKey, float fValue)
{
    if (g_pLTClient)
	{
		g_pLTClient->SetConsoleVariableFloat(sKey, fValue);
	}
}


static VarTrack s_cvarFirePitchShift;
void PlayWeaponSound(HWEAPON hWeaponID, bool bUseAIData, const LTVector &vPos, PlayerSoundId eSoundId,
					 bool bLocal, HOBJECT hFromObject )
{
	if( !hWeaponID )
		return;
	HWEAPONDATA hWeapon = g_pWeaponDB->GetWeaponData(hWeaponID, bUseAIData);
	if (!hWeapon)
		return;

 	if (!s_cvarFirePitchShift.IsInitted())
	{
		s_cvarFirePitchShift.Init(g_pLTClient, "PitchShiftFire", NULL, -1.0f);
	}

	HATTRIBUTE hWeaponSoundStruct;
	HATTRIBUTE   hWeaponSound = NULL;
	uint32 nValueIndex = 0;
	HRECORD hSR = NULL;
	int32 nMixChannel;

	if (bLocal)
	{
		// play the local one if it's in first person mode...
		hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_LocalSoundInfo);
		nMixChannel = PLAYSOUND_MIX_WEAPONS_PLAYER;
	}
	else
	{
		hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_NonLocalSoundInfo);
		nMixChannel = PLAYSOUND_MIX_WEAPONS_NONPLAYER;
	}

	switch (eSoundId)
	{
		case PSI_FIRE:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireSnd );
		}
		break;

		case PSI_ALT_FIRE:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,  WDB_WEAPON_rAltFireSnd );
		}
		break;

		case PSI_SILENCED_FIRE:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,  WDB_WEAPON_rSilencedFireSnd );
		}
		break;

		case PSI_DRY_FIRE:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,  WDB_WEAPON_rDryFireSnd );
		}
		break;

		case PSI_RELOAD:
		case PSI_RELOAD2:
		case PSI_RELOAD3:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rReloadSnd );
			nValueIndex = eSoundId - PSI_RELOAD;
		}
		break;

		case PSI_SELECT:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );
		}
		break;

		case PSI_DESELECT:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );
		}
		break;

		case PSI_WEAPON_MISC1:
		case PSI_WEAPON_MISC2:
		case PSI_WEAPON_MISC3:
		case PSI_WEAPON_MISC4:
		case PSI_WEAPON_MISC5:
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
			nValueIndex = eSoundId - PSI_WEAPON_MISC1;
		}
		break; 

		case PSI_FIRE_LOOP:
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopSnd );
			break;
		case PSI_FIRE_LOOP_END:
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopEndSnd );
			break;

		case PSI_INVALID:
		default : break;
	}
	if (hWeaponSound != NULL)
	{
		hSR = g_pWeaponDB->GetRecordLink(hWeaponSound,nValueIndex);
	}

	if (hSR)
	{
		uint32 dwFlags = PLAYSOUND_REVERB;
		float fPitchShift = 1.0f;
		if (s_cvarFirePitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
			fPitchShift = s_cvarFirePitchShift.GetFloat();
		}

		if (hFromObject)
		{
			g_pClientSoundMgr->PlayDBSoundFromObject(hFromObject, hSR,
				SMGR_INVALID_RADIUS, SOUNDPRIORITY_PLAYER_HIGH, dwFlags,
				SMGR_INVALID_VOLUME, fPitchShift, SMGR_INVALID_RADIUS, WEAPONS_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER );
		}
 		else if (bLocal)
		{
			g_pClientSoundMgr->PlayDBSoundLocal(hSR, SOUNDPRIORITY_PLAYER_HIGH,
				dwFlags, SMGR_INVALID_VOLUME, fPitchShift, WEAPONS_SOUND_CLASS, nMixChannel );
		}
		else
		{
 			g_pClientSoundMgr->PlayDBSoundFromPos(const_cast<LTVector&>(vPos), hSR,
				SMGR_INVALID_RADIUS, SOUNDPRIORITY_PLAYER_HIGH, dwFlags,
				SMGR_INVALID_VOLUME, fPitchShift, SMGR_INVALID_RADIUS, WEAPONS_SOUND_CLASS, nMixChannel );
		}
	}

}


LTRESULT SendEmptyServerMsg(uint32 nMsgID, uint32 nFlags)
{
	LTRESULT nResult;

	CAutoMessage cMsg;

	cMsg.Writeuint8(nMsgID);

	nResult = g_pLTClient->SendToServer(cMsg.Read(), nFlags);

	return nResult;
}




//void FormatString(int messageCode, wchar_t *outBuf, int outBufLen,  ...)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	va_list marker;
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	*outBuf = '\0';
//
//	if (hModule)
//	{
//		va_start(marker, outBufLen);
//		wchar_t tmpBuffer[kMaxStringBuffer];
//		uint32 nBytes = LoadStringW(hModule, messageCode, tmpBuffer, LTARRAYSIZE(tmpBuffer));
//		if (nBytes)
//		{
//			FormatMessageW(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,outBuf,outBufLen,&marker);
//		}
//		va_end(marker);
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(outBuf, outBufLen, L"Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//}
//
//void LoadString(int messageCode, wchar_t *outBuf, int outBufLen)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	*outBuf = '\0';
//
//	if (hModule)
//	{
//		LoadStringW(hModule, messageCode, outBuf, outBufLen);
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(outBuf, outBufLen, L"Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//}
//
//void LoadString(const char* szStringID, wchar_t *outBuf, int outBufLen)
//{
//	LTASSERT( g_pLTStringEdit, "LoadString" );
//	if( g_pLTStringEdit == NULL )
//	{
//		// Put SOMETHING in the string, at least...
//		LTSNPrintF(outBuf, outBufLen, L"Res#%d", messageCode);
//		return;
//	}
//
//	//g_pLTStringEdit->GetString(  )
//}
//
//
//void FormatString(int messageCode, char *outBuf, int outBufLen,  ...)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	va_list marker;
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	*outBuf = '\0';
//
//	if (hModule)
//	{
//		va_start(marker, outBufLen);
//		char tmpBuffer[kMaxStringBuffer];
//		uint32 nBytes = LoadStringA(hModule, messageCode, tmpBuffer, LTARRAYSIZE(tmpBuffer));
//		if (nBytes)
//		{
//			FormatMessageA(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,outBuf,outBufLen,&marker);
//		}
//		va_end(marker);
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(outBuf, outBufLen, "Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//}
//
//void LoadString(int messageCode, char *outBuf, int outBufLen)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	*outBuf = '\0';
//
//	if (hModule)
//	{
//		LoadStringA(hModule, messageCode, outBuf, outBufLen);
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(outBuf, outBufLen, "Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//}
//
//
//static wchar_t s_wszStringBuffer[kMaxStringBuffer];
//static char s_szStringBuffer[kMaxStringBuffer];
//const wchar_t* FormatTempString(int messageCode, ...)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	va_list marker;
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	s_wszStringBuffer[0] = '\0';
//
//	if (hModule)
//	{
//		va_start(marker, messageCode);
//
//		wchar_t tmpBuffer[kMaxStringBuffer];
//		uint32 nBytes = LoadStringW(hModule, messageCode, tmpBuffer, LTARRAYSIZE(tmpBuffer));
//		if (nBytes)
//		{
//			FormatMessageW(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,s_wszStringBuffer,LTARRAYSIZE(s_szStringBuffer),&marker);
//		}
//
//		va_end(marker);
//
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(s_wszStringBuffer, LTARRAYSIZE(s_wszStringBuffer), L"Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//	return s_wszStringBuffer;
//}
//
//const wchar_t* LoadTempString(int messageCode)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	s_wszStringBuffer[0] = '\0';
//
//	if (hModule)
//	{
//		LoadStringW(hModule, messageCode, s_wszStringBuffer, LTARRAYSIZE(s_szStringBuffer));
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(s_wszStringBuffer, LTARRAYSIZE(s_wszStringBuffer), L"Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//	return s_wszStringBuffer;
//}
//const char* FormatTempCharString(int messageCode, ...)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	va_list marker;
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	s_szStringBuffer[0] = '\0';
//
//	if (hModule)
//	{
//		va_start(marker, messageCode);
//
//		char tmpBuffer[kMaxStringBuffer];
//		uint32 nBytes = LoadStringA(hModule, messageCode, tmpBuffer, LTARRAYSIZE(tmpBuffer));
//		if (nBytes)
//		{
//			FormatMessageA(FORMAT_MESSAGE_FROM_STRING,tmpBuffer,0,0,s_szStringBuffer,LTARRAYSIZE(s_szStringBuffer),&marker);
//		}
//
//		va_end(marker);
//
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(s_szStringBuffer, LTARRAYSIZE(s_szStringBuffer), "Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//	return s_szStringBuffer;
//
//}
//
//const char* LoadTempCharString(int messageCode)
//{
//// XENON: Currently disabled in Xenon builds
//#if !defined(PLATFORM_XENON)
//
//	void* pModule;
//	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
//	HMODULE hModule = (HINSTANCE)pModule;
//
//	s_szStringBuffer[0] = '\0';
//
//	if (hModule)
//	{
//		LoadStringA(hModule, messageCode, s_szStringBuffer, LTARRAYSIZE(s_szStringBuffer));
//	}
//
//#else // PLATFORM_XENON
//
//	// Put SOMETHING in the string, at least...
//	LTSNPrintF(s_szStringBuffer, LTARRAYSIZE(s_szStringBuffer), "Res#%d", messageCode);
//
//#endif // PLATFORM_XENON
//
//	return s_szStringBuffer;
//}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsMultiplayerGameClient()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

bool IsMultiplayerGameClient()
{
	if( !g_pLTClient )
		return false;

	return g_pLTClient->IsMultiplayerExe( );
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool GetIntersectionUnderPoint
//
//  PURPOSE:	Helper for finding the normal and point of intersection beneath a given point
//
// ----------------------------------------------------------------------- //

bool GetIntersectionUnderPoint( LTVector &vInPt, HOBJECT *pFilterList, LTVector &vOutNormal, LTVector &vOutPt )
{
	IntersectQuery	iq;
	IntersectInfo		ii;
	
	vOutNormal.Init(0, 1, 0);

	iq.m_Flags	= IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iq.m_From	= vInPt;
	iq.m_To		= iq.m_From + LTVector( 0, -1, 0) * 256.0f;

	iq.m_FilterFn  = ObjListFilterFn;
	iq.m_pUserData = pFilterList;

	if( g_pLTClient->IntersectSegment( iq, &ii ) )
	{
		if( ii.m_hObject )
		{
			vOutNormal	= ii.m_Plane.m_Normal;
			vOutPt		= ii.m_Point;

			return true;
		}
	}
	
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetContouringNormal
//
//  PURPOSE:	Find the normal of the plane we would like to contour to
//				given a position and dims of an object...
//
// ----------------------------------------------------------------------- //

LTVector GetContouringNormal( LTVector &vPos, LTVector &vDims, LTVector &vForward, LTVector &vRight, HOBJECT *pFilterList )
{
	LTVector	avPt[4];		// points we are casting the rays from
	LTVector	avInterPt[4];	// points of intersection
	LTVector	avNormal[4];	// normals constructed from the points of intersection
	LTVector	avEdge[4];

	// Develop the points we wish to cast rays from...
	// We keep the height from the object incase the vehicle has clipped into the world.

	avPt[0] = vPos + ( vForward ) - ( vRight );	// 0----1
	avPt[1] = vPos + ( vForward ) + ( vRight );	// |    |
	avPt[2] = vPos - ( vForward ) + ( vRight );	// |    |
	avPt[3] = vPos - ( vForward ) - ( vRight );	// 3----2

	// Find the point of intersection that is under the vehicle...
	// If none was found just use the point with the height factored in.
	
	if( !GetIntersectionUnderPoint( avPt[0], pFilterList, avNormal[0], avInterPt[0] ) )
	{
		avInterPt[0] = avPt[0];
		avInterPt[0].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[1], pFilterList, avNormal[1], avInterPt[1] ) )
	{
		avInterPt[1] = avPt[1];
		avInterPt[1].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[2], pFilterList, avNormal[2], avInterPt[2] ) )
	{
		avInterPt[2] = avPt[2];
		avInterPt[2].y -= vDims.y;
	}

	if( !GetIntersectionUnderPoint( avPt[3], pFilterList, avNormal[3], avInterPt[3] ) )
	{
		avInterPt[3] = avPt[3];
		avInterPt[3].y -= vDims.y;
	}

	// Move the points to the origin...

	avInterPt[0] -= vPos;
	avInterPt[1] -= vPos;
	avInterPt[2] -= vPos;
	avInterPt[3] -= vPos;

	// Develop the vectors that will construct the 4 planes...

	avEdge[0] = (avInterPt[1] - avInterPt[0]).GetUnit();
	avEdge[1] = (avInterPt[2] - avInterPt[1]).GetUnit();
	avEdge[2] = (avInterPt[3] - avInterPt[2]).GetUnit();
	avEdge[3] = (avInterPt[0] - avInterPt[3]).GetUnit();

	// Find the normals of the planes...

	avNormal[0] = -avEdge[3].Cross( avEdge[0] );
	avNormal[1] = -avEdge[0].Cross( avEdge[1] );
	avNormal[2] = -avEdge[1].Cross( avEdge[2] );
	avNormal[3] = -avEdge[2].Cross( avEdge[3] );

	avNormal[0].Normalize();
	avNormal[1].Normalize();
	avNormal[2].Normalize();
	avNormal[3].Normalize();

	// Average the normals...
	
	LTVector vNormal = avNormal[0] + avNormal[1] + avNormal[2] + avNormal[3];
	vNormal.Normalize();

	return vNormal;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetContouringInfo
//
//  PURPOSE:	Get the pitch amount and percents to apply for pitch and roll
//				based on the forward direction and plane normal...
//
// ----------------------------------------------------------------------- //

void GetContouringInfo( LTVector &vForward, LTVector &vNormal, 
					   float &fOutAmount, float &fOutPitchPercent, float &fOutRollPercent )
{
	LTVector	vPlaneF = (vNormal.y >= 1.0f) ? vForward : vNormal;
	
	vPlaneF.y = 0.0f;
	vPlaneF.Normalize();

	LTRotation	rPlaneRot( vPlaneF, LTVector(0, 1, 0));
	LTVector	vPlaneR = rPlaneRot.Right();
	
	// Calculate how much pitch and roll we should apply...

	fOutPitchPercent	= vForward.Dot( vPlaneF );
	fOutRollPercent		= vForward.Dot( vPlaneR );

	// Figure out the length of the foward vector projected on the xz plane.  This
	// is needed because Euler angles are calculated cummulatively, not just based
	// on the global coordinate axis.

	float fXZLen = (float)sqrt( 1.0f - vNormal.y * vNormal.y );

	// Subtract the pitch from 90 degrees cause we want to be parallel to the plane

	fOutAmount = MATH_HALFPI - (float)atan2( vNormal.y, fXZLen );
}


//step through the things attached to us and see if we should hide any of them
void UpdateAttachmentVisibility( HOBJECT hParent )
{
	HLOCALOBJ attachList[20];
	uint32 dwListSize = 0;
	uint32 dwNumAttach = 0;

	g_pCommonLT->GetAttachments(hParent, attachList, 20, dwListSize, dwNumAttach);
	uint32 nFlags = 0;
	g_pCommonLT->GetObjectFlags(hParent, OFT_Flags, nFlags);
	bool bVisible = !!( nFlags & FLAG_VISIBLE );
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;
	EEngineLOD eParentLOD = eEngineLOD_Low;
	g_pLTClient->GetObjectShadowLOD(hParent, eParentLOD);

	for (int i=0; i < nNum; i++)
	{
		uint32 dwUsrFlags;
		g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);

		if( !bVisible || (g_pVersionMgr->IsLowViolence() && (dwUsrFlags & USRFLG_GORE)) )
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}
		else if( bVisible )
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}

		g_pLTClient->SetObjectShadowLOD(attachList[i], eParentLOD);

		UpdateAttachmentVisibility( attachList[i] );
	}
}

// Find the specified socket on one of the given objects's attachments.
// Returns true if the socket is found.
bool FindAttachmentSocket(HOBJECT hParent, const char* pszSocket, HOBJECT* pOutObj, HMODELSOCKET* pOutSocket)
{
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;

	// We need to check the weapons individually for the local player object...
	if (hParent == g_pPlayerMgr->GetMoveMgr()->GetObject())
	{
		const CClientWeapon* apWeapons[] =
			{
				g_pClientWeaponMgr->GetCurrentClientWeapon(),
				CPlayerBodyMgr::Instance().GetGrenadeWeapon()
			};

		for (uint32 i=0; i < LTARRAYSIZE(apWeapons); i++)
		{
			const CClientWeapon* pClientWeapon = apWeapons[i];
			if (pClientWeapon)
			{
				HOBJECT hWeaponModel;
				if ((hWeaponModel = pClientWeapon->GetRightHandModel())	// yes, assignment.
					&& g_pModelLT->GetSocket(hWeaponModel, pszSocket, hSocket) == LT_OK
					&& hSocket != INVALID_MODEL_SOCKET)
				{
					if (pOutObj) *pOutObj = hWeaponModel;
					if (pOutSocket) *pOutSocket = hSocket;
					return true;
				}
				else
				if ((hWeaponModel = pClientWeapon->GetLeftHandModel())	// yes, assignment.
					&& g_pModelLT->GetSocket(hWeaponModel, pszSocket, hSocket) == LT_OK
					&& hSocket != INVALID_MODEL_SOCKET)
				{
					if (pOutObj) *pOutObj = hWeaponModel;
					if (pOutSocket) *pOutSocket = hSocket;
					return true;
				}
			}
		}
	}

	// For everything else, just check the attachments ...
	else
	{
		HOBJECT hAttachList[30];
		uint32 dwListSize, dwNumAttachments;

		if (g_pCommonLT->GetAttachments(hParent, hAttachList,
			ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
		{
			LTASSERT(dwNumAttachments < ARRAY_LEN(hAttachList), "Too many attachments!");
			for (uint32 i=0; i < dwListSize; i++)
			{
				if (hAttachList[i]
				&& g_pModelLT->GetSocket(hAttachList[i], pszSocket, hSocket) == LT_OK
					&& hSocket != INVALID_MODEL_SOCKET)
				{
					if (pOutObj) *pOutObj = hAttachList[i];
					if (pOutSocket) *pOutSocket = hSocket;
					return true;
				}
			}
		}
	}

	return false;
}

// create strings
const wchar_t* CreateHelpString(const char *pszStringID)
{
	static std::wstring wsTmp;
	wsTmp = LoadString(pszStringID);

	uint32 nFirst = wsTmp.find_first_of(L"|");
	while (nFirst != std::wstring::npos)
	{
		uint32 nNext = wsTmp.find_first_of(L"|", nFirst+1);

		if (nNext != std::wstring::npos && nNext)
		{
			uint32 nLen = nNext-nFirst;
			std::wstring wsAction;
			if (nLen >= 2) 
			{
				wsAction = wsTmp.substr(nFirst+1,nLen-1);
			}

			wsTmp.erase(nFirst,nLen+1);

			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			int nCommand = GetCommandID( MPW2A(wsAction.c_str()).c_str() );
			wchar_t szTriggerName[32] = L"";
			LTStrCpy(szTriggerName,pProfile->GetTriggerNameFromCommandID(nCommand),LTARRAYSIZE(szTriggerName));
			if (LTStrEmpty(szTriggerName))
			{
				LTStrCpy(szTriggerName,LoadString("Control_Unassigned"),LTARRAYSIZE(szTriggerName));
			}
			wsTmp.insert(nFirst,szTriggerName);

		}

		nFirst = wsTmp.find_first_of(L"|");
	}

	return wsTmp.c_str();
}

// Launch the singleplayer exe.
bool LaunchApplication::LaunchSinglePlayerExe( SwitchToScreen eSwitchToScreen )
{
	return LaunchMainExe( LAUNCHER_EXE_SP, eSwitchToScreen );
}

// Launch the multiplayer exe.
bool LaunchApplication::LaunchMultiPlayerExe( SwitchToScreen eSwitchToScreen )
{
	return LaunchMainExe( LAUNCHER_EXE_MP, eSwitchToScreen );
}

// Utility function to do work.
bool LaunchApplication::LaunchMainExe( char const* pszExeName, SwitchToScreen eSwitchToScreen )
{
#if defined(PLATFORM_WIN32)

	// Rebuild the commandline args without the current process.
	char szCommandLineArgs[MAX_PATH];
	szCommandLineArgs[0] = '\0';
	for( int32 nIndex = 1; nIndex < __argc; nIndex++ ) 
	{
		// Skip any screen arguments that the current process was launched with.
		if( LTStrIEquals( __argv[nIndex], "+screen" ))
		{
			// Skip the screen name parameter.
			nIndex++;
			continue;
		}

		if( !LTStrEmpty( szCommandLineArgs ))
		{
			LTStrCat( szCommandLineArgs, " ", LTARRAYSIZE( szCommandLineArgs ));
		}
		LTStrCat( szCommandLineArgs, __argv[nIndex], LTARRAYSIZE( szCommandLineArgs ));
	}

	// Check if we are switching to a specific screen.
	char const* pszSwitchToScreenName = NULL;
	switch( eSwitchToScreen )
	{
	default:
	case kSwitchToScreen_None:
		break;
	case kSwitchToScreen_Single:
		pszSwitchToScreenName = "single";
		break;
	case kSwitchToScreen_Multi:
		pszSwitchToScreenName = "multi";
		break;
	case kSwitchToScreen_Performance:
		pszSwitchToScreenName = "performance";
		break;
	}

	// Add the screen to the commandline.
	if( !LTStrEmpty( pszSwitchToScreenName ))
	{
		if( !LTStrEmpty( szCommandLineArgs ))
		{
			LTStrCat( szCommandLineArgs, " +screen ", LTARRAYSIZE( szCommandLineArgs ));
		}
		else
		{
			LTStrCat( szCommandLineArgs, "+screen ", LTARRAYSIZE( szCommandLineArgs ));
		}
		// Add any additional commandline.
		LTStrCat( szCommandLineArgs, pszSwitchToScreenName, LTARRAYSIZE( szCommandLineArgs ));
	}

	// Launch the exe.
	return LaunchFromString( pszExeName, szCommandLineArgs, true, true );

#else
	return false;
#endif
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LaunchApplication::LaunchPatchUpdate
//
//	PURPOSE:	Launchers patchupdate given URL to patch file.
//
// --------------------------------------------------------------------------- //
bool LaunchApplication::LaunchPatchUpdate( char const* pszUrl )
{
	static char* pszExe = "FPUpdate.exe";

	// Check inputs.
	if( !pszUrl || !pszUrl[0] )
		return false;

	if( !LaunchFromString( pszExe, pszUrl, false, true ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LaunchApplication::LaunchMOTDLink
//
//	PURPOSE:	Launches Browser with a given MOTD Link
//
// --------------------------------------------------------------------------- //
bool LaunchApplication::LaunchMOTDLink( char const* pszUrl )
{
#if defined(PLATFORM_WIN32)

	// Check inputs.
	if( !pszUrl || !pszUrl[0] )
		return false;


	char szExe[MAX_PATH] = "";
	uint32 nExeSize = LTARRAYSIZE( szExe );
	AssocQueryString(ASSOCF_NOTRUNCATE,
		ASSOCSTR_EXECUTABLE,
		".htm",
		"open",
		szExe,
		( DWORD* )&nExeSize);


	if( !LaunchFromString( szExe, pszUrl, false, false ))
		return false;

	return true;

#else
	return false;
#endif
}

// Launches the end splash screen.
bool LaunchApplication::LaunchEndSplashScreen( )
{
#if defined(PLATFORM_WIN32)

	if( !g_pLTIStringEdit->DoesIDExist(g_pLTDBStringEdit, "EndSplashURL" ))
		return false;

	char szEndSplashURL[MAX_PATH*2] = "";
	LTStrCpy( szEndSplashURL, MPW2A( LoadString( "EndSplashURL" )).c_str(), LTARRAYSIZE( szEndSplashURL ));
	if( LTStrEmpty( szEndSplashURL ))
		return false;

	char szExe[MAX_PATH] = "";
	uint32 nExeSize = LTARRAYSIZE( szExe );
	AssocQueryString(ASSOCF_NOTRUNCATE,
		ASSOCSTR_EXECUTABLE,
		".htm",
		"open",
		szExe,
		( DWORD* )&nExeSize);

	return LaunchFromString( szExe, szEndSplashURL, true, true );
#else
	return false;
#endif
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LaunchApplication::LauncherServerApp()
//
//	PURPOSE:	Launches the serverapp.
//
// --------------------------------------------------------------------------- //
bool LaunchApplication::LaunchServerApp( char const* pszOptionsFile )
{
	// Setup the command line.
	std::string sCmdLine = "-optionsfile ";
	// Enclose the profile name in quotes since we allow spaces in the name...
	sCmdLine += "\"";
	sCmdLine += pszOptionsFile;
	sCmdLine += "\"";

	if( !LaunchFromString( "FEARServer.exe", sCmdLine.c_str( ), false, true ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LaunchApplication::LaunchFromString
//
//	PURPOSE:	Runs a start command.
//
// --------------------------------------------------------------------------- //
bool LaunchApplication::LaunchFromString( char const* pszFile, char const* pszParameters, bool bMaximize, bool bShutdownCurrentApp )
{
#if defined(PLATFORM_WIN32)

	RMode rMode;

	// Check inputs.
	if( LTStrEmpty( pszFile ))
		return false;

	// Check if the input is a file.
	char szFileName[MAX_PATH] = "";
	LTFileOperations::SplitPath( pszFile, NULL, szFileName, NULL );
	if( !LTStrEmpty( szFileName ))
	{
		// Make sure the file exists.
		if( !LTFileOperations::FileExists( pszFile ))
			return false;
	}

	// Save the current render mode.  We'll need to restore it if the serverapp
	// launching fails.
	g_pLTClient->GetRenderMode( &rMode );


	// Shutdown the renderer, minimize it, and hide it...
	if( bShutdownCurrentApp )
	{
		g_pLTClient->ShutdownRender( RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW );
	}
	// Not shutting down, but we'll need to make sure to minimize.  Just executing the new app
	// doesn't always make it foreground.
	else
	{
		extern HWND	g_hMainWnd;
		ShowWindow(g_hMainWnd, SW_MINIMIZE);
	}

	SHELLEXECUTEINFO ShExecInfo = {0}; 
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
	ShExecInfo.fMask = 0;
	ShExecInfo.hwnd = NULL; 
	ShExecInfo.lpVerb = "open";
	ShExecInfo.lpFile = pszFile;
	ShExecInfo.lpParameters = pszParameters;
	ShExecInfo.lpDirectory = NULL; 
	ShExecInfo.nShow = bMaximize ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL; 
	ShExecInfo.hInstApp = NULL;
	if( !ShellExecuteEx(&ShExecInfo))
	{
		// Serverapp failed.  Restore the render mode.
		if( bShutdownCurrentApp )
			g_pLTClient->SetRenderMode( &rMode );
		return false;
	}

	if( bShutdownCurrentApp )
	{
		// We're done with this process.
		g_pLTClient->Shutdown();
	}

	return true;

#elif defined(PLATFORM_XENON)

	// XENON: Not yet implemented
	return false;

#else

	// New platforms must re-implement this function
#error Undefined platform encountered in LaunchFromString
	return false;

#endif
}
