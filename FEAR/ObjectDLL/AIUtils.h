// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_UTILS_H__
#define __AI_UTILS_H__

#include "VarTrack.h"
#include "ServerUtilities.h"
#include "AIEnumAIAwareness.h"

// Forward declarations

class CAI;
class CAIWMFact;
class CAnimationProps;

// Helper functions

inline float FOV2DP(float fFOV)
{
	float fFOVRadians = 90.0f - fFOV/2.0f;
	fFOVRadians = DEG2RAD(fFOVRadians);
	return (float)sin(fFOVRadians);
}

void AIError(const char* szFormat, ...);


// GetDifficultyFactor

float GetDifficultyFactor();
inline float RAISE_BY_DIFFICULTY(float fValue) { return fValue*GetDifficultyFactor(); }
inline float LOWER_BY_DIFFICULTY(float fValue) { return fValue/GetDifficultyFactor(); }

inline bool IsTrueChar(char ch)
{
	return (ch == 't' || ch == 'T' || ch == 'y' || ch == 'Y' || ch == '1');
}

inline bool IsFalseChar(char ch)
{
	return (ch == 'f' || ch == 'F' || ch == 'n' || ch == 'N' || ch == '0');
}

void GetValueRange(CAI* pAI, const char* szValue, float* pfMin, float* pfMax);

bool DidDamage( CAI* pAI, CAIWMFact* pFact );


bool RayIntersectBox(const LTVector& vBoxMin,
					  const LTVector& vBoxMax,
					  const LTVector& vOrigin,
					  const LTVector& vDest,
					  LTVector* pvIntersection);

bool TrimLineSegmentByRadius( float fRadius, LTVector* pv0, LTVector* pv1, bool bTrimV0, bool bTrimV1 );

bool FindNearestPointOnLine( const LTVector& l0, const LTVector& l1, const LTVector& vPos, LTVector* pvPosNearest );

enum EnumRayIntersectResult
{
	kRayIntersect_Failure,
	kRayIntersect_Success,
	kRayIntersect_SnappedToSegment,
};
EnumRayIntersectResult RayIntersectLineSegment( const LTVector& l0, const LTVector& l1, const LTVector& r0, const LTVector& r1, bool bSnapToSegment, LTVector* pvPtIntersect );

//
// Movement and animation related utility functions.
//

// Returns true if the linesegment described by vStart and vEnd is unobstructed 
// by nav mesh edges or characters
bool IsClearForMovement( CAI* pAI, const LTVector vStart, const LTVector& vEnd);

// Returns true if successful, false if failed.  If successful, rOutTransform
// contains the transformation of the root node contained in the passed in animation.
bool GetAnimationTransform( CAI* pAI, const HMODELANIM hAnim, LTRigidTransform& rOutTransform );

// Returns true if the AI can shoot at the passed in position.  
// 1) Fails if the AI must turn his back on his target.
// 2) Fails if the target is too far above or below the AI.
bool AIUtil_PositionShootable(CAI* pAI, const LTVector& vTargetOrigin);

// STL containers

typedef std::vector<LTVector, LTAllocator<LTVector, LT_MEM_TYPE_OBJECTSHELL> > VECTOR_LIST;
typedef std::vector<int, LTAllocator<int, LT_MEM_TYPE_OBJECTSHELL> > INT_LIST;


// IsAIXXXs

bool IsAI( HOBJECT hObject );
bool IsAIRegion( HOBJECT hObject );
bool IsAINode( HOBJECT hObject );
bool IsAINodeSmartObject( HOBJECT hObject );
bool IsAICombatOpportunity( HOBJECT hObject );

// Dead AI

bool IsDeadAI( HOBJECT hObject );

// Dead Character

bool IsDeadCharacter(  HOBJECT hObject );

// Awareness

EnumAIAwareness StringToAwareness( const char* pszAwareness );

// Statics

extern int g_cIntersectSegmentCalls;
extern VarTrack g_vtAIConsoleVar;
extern VarTrack g_vtMuteAIAssertsVar;


// Constants

extern const float c_fFOV180;
extern const float c_fFOV160;
extern const float c_fFOV140;
extern const float c_fFOV120;
extern const float c_fFOV90;
extern const float c_fFOV75;
extern const float c_fFOV60;
extern const float c_fFOV45;
extern const float c_fFOV30;

extern const float c_fUpdateDelta;

extern const float c_fFacingThreshhold;

extern const char c_szKeyFireWeapon[];
extern const char c_szKeyThrow[];
extern const char c_szKeyPickUp[];
extern const char c_szKeyTurnOn[];
extern const char c_szKeyTurnOff[];
extern const char c_szKeyStopFireWeapon[];
extern const char c_szKeyDraw[];
extern const char c_szKeyHolster[];

extern const char c_szKeyFX[];

// Defines

// ----------------------------------------------------------------------- //
//
//	MACRO:		AITRACE
//
//	PURPOSE:	Output a debugging message to the game console when the 
// 				console variable "var" is "set" (greater than zero).
// 
// 	USAGE:		In code:
// 					AITRACE( unquoted_symbol, ( HOBJECT /*can be NULL)*/, 
// 							format_string, ... ) )
// 
// 				In the game console:
// 					serv unquoted_symbol [0|1]
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL
	#define AITRACE(var, args) \
		g_vtAIConsoleVar.Init(g_pLTServer, #var, NULL, 0.0f); \
		if( g_vtAIConsoleVar.GetFloat() > 0.0f ) { \
			ObjectCPrint args; }
#else
	#define AITRACE(var, args) (void)0;
#endif


// ----------------------------------------------------------------------- //
//
//	MACRO:		AI_DEBUG_NAME
// 				AI_DEBUG_HANDLE_NAME
//
//	PURPOSE:	Creates a local debug-only variable to easily see the string
// 				name of an AI or handled object during debugging.
// 				Code should never be checked in with these macros called
// 				as the hit on perf can be significant. However, they are
// 				guaranteed to never end up in the final game.
// 
// ----------------------------------------------------------------------- //
#ifndef _FINAL
#define AI_DEBUG_NAME( var, exp )			std::string var = exp;
#define AI_DEBUG_HANDLE_NAME( var, handle )	std::string var = ::ToString(handle);
#else
#define AI_DEBUG_NAME( var, exp )
#define AI_DEBUG_HANDLE_NAME( var, handle )
#endif

// ----------------------------------------------------------------------- //
//
//	MACRO:		AI_CHARACTER_ALIGNMENT
// 				AI_CHARACTER_ALIGNMENT_FROM_HANDLE
//
//	PURPOSE:	Creates a local debug-only variable to easily see the string
// 				name of the alignment of an AI or handled object during debugging.
// 				Code should never be checked in with these macros called
// 				as the hit on perf can be significant. However, they are
// 				guaranteed to never end up in the final game.
// 
// ----------------------------------------------------------------------- //
#ifndef _FINAL
#define AI_CHARACTER_ALIGNMENT_FROM_HANDLE( var, handle )							\
std::string var = "";                                                               \
if (IsCharacter(handle))                                                            \
{                                                                                   \
	CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject( handle );    \
	if( pCharacter )                                                                \
	{                                                                               \
		var = g_pCharacterDB->Alignment2String( pCharacter->GetAlignment() );       \
	}                                                                               \
}
#define AI_CHARACTER_ALIGNMENT( var, ai )	std::string var = g_pCharacterDB->Alignment2String( ai->GetAlignment() );
#else
#define AI_CHARACTER_ALIGNMENT_FROM_HANDLE( var, handle )
#define AI_CHARACTER_ALIGNMENT( var, ai )
#endif


#define ADD_ATTACHMENT(pos, attachment) ADD_STRINGPROP_FLAG(##pos,	attachment,			PF_GROUP6|PF_STATICLIST)

#define CHECK_HOLSTER	true
#define IS_PLANNING		true
#define EVALUATE		true
#define LOOP			true

#define ALL_CHAR_TYPES	0xffffffff
#define FAILURE_IS_ERROR true

#endif
