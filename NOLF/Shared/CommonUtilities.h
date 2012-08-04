// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMON_UTILITIES_H__
#define __COMMON_UTILITIES_H__

#include "ltbasetypes.h"
#include "iltcommon.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "ilttransform.h"

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

#define WAVESTR_LINEAR	"LINEAR"
#define WAVESTR_SINE	"SINE"
#define WAVESTR_SLOWOFF	"SLOWOFF"
#define WAVESTR_SLOWON	"SLOWON"

#define DEFAULT_STAIRSTEP_HEIGHT	32.0

// Externs

extern ILTMath*  g_pMathLT;
extern ILTModel* g_pModelLT;
extern ILTTransform* g_pTransLT;
extern ILTPhysics* g_pPhysicsLT;
extern ILTCSBase* g_pBaseLT;

inline LTBOOL IsMainWorld(HOBJECT hObj)
{
	if (!g_pPhysicsLT) return LTFALSE;

	return (LT_YES == g_pPhysicsLT->IsWorldObject(hObj));
}

// Wave types.
typedef enum
{
	Wave_Linear=0,
	Wave_Sine,
	Wave_SlowOff,
	Wave_SlowOn,
	NUM_WAVETYPES
} WaveType;

typedef float (*WaveFn)(float val);


// Wave functions, pass in a # 0-1 and they return a number 0-1.
float WaveFn_Linear(float val);
float WaveFn_Sine(float val);
float WaveFn_SlowOff(float val);
float WaveFn_SlowOn(float val);

// Guaranteed to return a value function (one of the wave functions).
WaveFn GetWaveFn(WaveType type);

// Should be used by objects allowing the wave type to be specified.
WaveType ParseWaveType(char *pStr);


uint16 Color255VectorToWord( LTVector *pVal );
void Color255WordToVector( uint16 wVal, LTVector *pVal );

int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);

// Compress/decompress a rotation into a single byte.  This only accounts for
// rotation around the Y axis.
uint8 CompressRotationByte(ILTCommon *pCommon, LTRotation *pRotation);
void UncompressRotationByte(ILTCommon *pCommon, uint8 rot, LTRotation *pRotation);

#ifdef _CLIENTBUILD
#include "..\ClientShellDLL\ClientUtilities.h"
#include "..\ClientShellDLL\GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
#else
#include "..\ObjectDLL\ServerUtilities.h"
#include "..\ObjectDLL\GameServerShell.h"
extern CGameServerShell* g_pGameServerShell;
#endif

inline LTBOOL ObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
            return LTFALSE;
		++hList;
	}
    return LTTRUE;
}

inline LTBOOL IsMultiplayerGame()
{
#ifdef _CLIENTBUILD
    return g_pGameClientShell->IsMultiplayerGame();
#else
    return (g_pGameServerShell->GetGameType() != SINGLE);
#endif
}

void ButeToConsoleFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ButeToConsoleString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ConsoleToButeFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ConsoleToButeString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ReadNetHostSettings( );
void WriteNetHostSettings( );


#define DANGER(interface, engineer) \
interface->CPrint("----------------------------"); \
interface->CPrint("                            "); \
interface->CPrint("DDD   A  N   N GGGG EEE RR "); \
interface->CPrint("D  D A A NN  N G    E   R R"); \
interface->CPrint("D  D AAA N N N G GG EEE RR "); \
interface->CPrint("D  D A A N  NN G  G E   R R"); \
interface->CPrint("DDD  A A N   N GGGG EEE R R"); \
interface->CPrint("                            "); \
interface->CPrint("email "#engineer" immediately!!"); \
interface->CPrint("                            "); \
interface->CPrint("----------------------------"); \

#endif // __COMMON_UTILITIES_H__