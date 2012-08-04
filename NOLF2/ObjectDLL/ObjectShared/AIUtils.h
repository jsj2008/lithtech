// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_UTILS_H__
#define __AI_UTILS_H__

#include "CVarTrack.h"
#include "UberAssert.h"
#include "ServerUtilities.h"
// Globals

class CAI;

// Helper functions

//----------------------------------------------------------------------------
//              
//	CLASS:		CAIUtils
//              
//	PURPOSE:	Helper class containing general AI tests.  Meant for creating
//				a temp instance to work with when needed
//              
//----------------------------------------------------------------------------
class CAIUtils
{
public:
	// Ctors/Dtors/etc
	CAIUtils(HOBJECT hOwner);
	~CAIUtils();

	// Public members
	LTBOOL FindTrueFloorHeight(float flCheckDist, const LTVector& vDims, const LTVector& vPos, LTFLOAT* pfFloorHeight);
	LTBOOL FindTrueCeilingHeight(float flCheckDist, const LTVector& vDims, const LTVector& vPos, LTFLOAT* pfFloorHeight);

protected:
	// Protected members

private:
	// Private members

	HOBJECT m_hObject;

	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	CAIUtils(const CAIUtils& rhs) {}
	CAIUtils& operator=(const CAIUtils& rhs ) {}
};

inline LTFLOAT FOV2DP(LTFLOAT fFOV)
{
	LTFLOAT fFOVRadians = 90.0f - fFOV/2.0f;
	fFOVRadians = DEG2RAD(fFOVRadians);
	return (LTFLOAT)sin(fFOVRadians);
}

void AIError(const char* szFormat, ...);


LTBOOL FindGrenadeDangerPosition(const LTVector& vPos, LTFLOAT fDangerRadiusSqr, LTVector* pvDangerPos, class CGrenade** ppGrenade);


// GetDifficultyFactor

LTFLOAT GetDifficultyFactor();
inline LTFLOAT RAISE_BY_DIFFICULTY(LTFLOAT fValue) { return fValue*GetDifficultyFactor(); }
inline LTFLOAT LOWER_BY_DIFFICULTY(LTFLOAT fValue) { return fValue/GetDifficultyFactor(); }

inline LTBOOL IsTrueChar(char ch)
{
	return (ch == 't' || ch == 'T' || ch == 'y' || ch == 'Y' || ch == '1');
}

inline LTBOOL IsFalseChar(char ch)
{
	return (ch == 'f' || ch == 'F' || ch == 'n' || ch == 'N' || ch == '0');
}

inline void RoundVector(LTVector& v)
{
	v.x = (LTFLOAT)floor( v.x + 0.5f );
	v.y = (LTFLOAT)floor( v.y + 0.5f );
	v.z = (LTFLOAT)floor( v.z + 0.5f );
}

void GetValueRange(CAI* pAI, const char* szValue, LTFLOAT* pfMin, LTFLOAT* pfMax);


bool GroundFilterFn(HOBJECT hObj, void *pUserData);
bool WorldFilterFn(HOBJECT hObj, void *pUserData);

bool RayIntersectBox(const LTVector& vBoxMin,
					  const LTVector& vBoxMax,
					  const LTVector& vOrigin,
					  const LTVector& vDest,
					  LTVector* pvIntersection);

// IsAIXXXs

LTBOOL IsAI( HOBJECT hObject );
LTBOOL IsAIVolume( HOBJECT hObject );
LTBOOL IsAIRegion( HOBJECT hObject );
LTBOOL IsAINodeUseObject( HOBJECT hObject );
LTBOOL IsAlarm( HOBJECT hObject );

LTBOOL IsAIVolume( ILTBaseClass* pObject );

// Enums

enum Direction
{
	eDirectionRight,
	eDirectionLeft,
	eDirectionForward,
	eDirectionBackward,
};

#define SAVE_DIRECTION(x) SAVE_DWORD(x)
#define LOAD_DIRECTION(x) LOAD_DWORD_CAST(x, Direction)

// Statics

extern int g_cIntersectSegmentCalls;
extern CVarTrack g_vtAIConsoleVar;
extern CVarTrack g_vtMuteAIAssertsVar;
extern CVarTrack g_vtDifficultyFactorEasy;
extern CVarTrack g_vtDifficultyFactorNormal;
extern CVarTrack g_vtDifficultyFactorHard;
extern CVarTrack g_vtDifficultyFactorVeryHard;
extern CVarTrack g_vtDifficultyFactorPlayerIncrease;


// Constants

extern const LTFLOAT c_fFOV180;
extern const LTFLOAT c_fFOV160;
extern const LTFLOAT c_fFOV140;
extern const LTFLOAT c_fFOV120;
extern const LTFLOAT c_fFOV90;
extern const LTFLOAT c_fFOV75;
extern const LTFLOAT c_fFOV60;
extern const LTFLOAT c_fFOV45;
extern const LTFLOAT c_fFOV30;

extern const LTFLOAT c_fUpdateDelta;
extern const LTFLOAT c_fDeactivationTime;

extern const LTFLOAT c_fFacingThreshhold;

extern const char c_szKeyFireWeapon[];
extern const char c_szKeyBodySlump[];
extern const char c_szKeyPickUp[];
extern const char c_szKeyOpen[];
extern const char c_szKeyClose[];
extern const char c_szKeyTurnOn[];
extern const char c_szKeyTurnOff[];
extern const char c_szKeyStopFireWeapon[];

extern const char c_szActivate[];
extern const char c_szAttachmentAnim[];
extern const char c_szKeyFX[];

// Defines

#define AITRACE(var, args) \
	g_vtAIConsoleVar.Init(g_pLTServer, #var, LTNULL, 0.0f); \
	if( g_vtAIConsoleVar.GetFloat() > 0.0f ) { \
		ObjectCPrint args; }

#define ADD_ATTACHMENT(pos, attachment) ADD_STRINGPROP_FLAG(##pos##,	attachment,			PF_GROUP6|PF_STATICLIST)

#endif
