// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIUtils.h"
#include "AI.h"
#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "ProjectileTypes.h"
#include "PlayerObj.h"
#include "AIAssert.h"
#include "GameBaseLite.h"

// Globals

int g_cIntersectSegmentCalls = 0;
CVarTrack g_vtAIConsoleVar;
CVarTrack g_vtMuteAIAssertsVar;
CVarTrack g_vtDifficultyFactorEasy;
CVarTrack g_vtDifficultyFactorNormal;
CVarTrack g_vtDifficultyFactorHard;
CVarTrack g_vtDifficultyFactorVeryHard;
CVarTrack g_vtDifficultyFactorPlayerIncrease;



// Constants

const LTFLOAT c_fFOV180  = 0.0f;
const LTFLOAT c_fFOV160  = 0.1736481776669f;
const LTFLOAT c_fFOV140  = 0.3420201433257f;
const LTFLOAT c_fFOV120  = 0.5f;
const LTFLOAT c_fFOV90   = 0.70710678118654752440084436210485f;
const LTFLOAT c_fFOV75   = 0.7933533402912351645797769615013f;
const LTFLOAT c_fFOV60   = 0.86602540378443864676372317075294f;
const LTFLOAT c_fFOV45   = 0.92387953251128675612818318939679f;
const LTFLOAT c_fFOV30   = 0.9659258262890682867497431997289f;

const LTFLOAT c_fUpdateDelta         = 0.01f;
const LTFLOAT c_fDeactivationTime    = 10.0f;

const LTFLOAT c_fFacingThreshhold        = .999999f;

const char c_szKeyPickUp[]			= "PICKUP";
const char c_szKeyOpen[]			= "OPEN";
const char c_szKeyClose[]			= "CLOSE";
const char c_szKeyFireWeapon[]		= "FIRE";
const char c_szKeyStopFireWeapon[]	= "STOPFIRE";
const char c_szKeyBodySlump[]		= "NOISE";
const char c_szKeyTurnOn[]			= "TURNON";
const char c_szKeyTurnOff[]			= "TURNOFF";

const char c_szActivate[]			= "ACTIVATE";
const char c_szAttachmentAnim[] = "ATTACHMENTANIM";
const char c_szKeyFX[]				= "FX";


void GetValueRange(CAI* pAI, const char* szValue, LTFLOAT* pfMin, LTFLOAT* pfMax)
{
	AIASSERT(szValue[0] == '[' && szValue[strlen(szValue) - 1] == ']', pAI->m_hObject, "GetValueRange: range not in the form [min,max]");
	AIASSERT(strlen(szValue) < 64, pAI->m_hObject, "GetValueRange: range is more than 64 characters");

	char szTemp[64];
	strcpy( szTemp, szValue + 1 );
	szTemp[strlen(szTemp) - 1] = '\0';

	char* pTok = strtok( szTemp, "," );
	*pfMin = (LTFLOAT)atof( pTok );

	pTok = strtok( LTNULL, "," );
	*pfMax = (LTFLOAT)atof( pTok );
}

//-------------------------------------------------------------------------------------------
// IsAIXXX
//
// Checks if handle is a handle to a CXXX
// Arguments:
//		hObject - handle to object to test
// Return:
//      LTBOOL
//-------------------------------------------------------------------------------------------

LTBOOL IsAI( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "CAI" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

LTBOOL IsAIVolume( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "AIVolume" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

LTBOOL IsAIRegion( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "AIRegion" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

LTBOOL IsAINodeUseObject( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "AINodeUseObject" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

LTBOOL IsAlarm( HOBJECT hObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "Alarm" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}


LTBOOL IsAIVolume( ILTBaseClass* pObject )
{
    static HCLASS hTest  = g_pLTServer->GetClass( "AIVolume" );
	HCLASS hClass;
	if (pObject && !pObject->m_hObject)
		hClass = ((GameBaseLite*)pObject)->GetClass();
	else
		hClass = pObject ? g_pLTServer->GetObjectClass(pObject->m_hObject) : LTNULL;
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}


// Console Output

void AIError(const char* szFormat, ...)
{
#ifndef _FINAL
	static char szBuffer[4096];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);
//	_ASSERT(!szBuffer);

	// Check that the pointer is valid before use is attempted,
	// as if this is called in a DEdit crash, the LTServer pointer
	// is no where near existing.  This is NOT an error condition, just
	// an unfortunate side effect of Dedit and the engine using the same
	// LTO file for different situations.
	if ( g_pLTServer )
	{
		g_pLTServer->CPrint("AI ERROR: %s", szBuffer);
	}
#endif
}


LTBOOL FindGrenadeDangerPosition(const LTVector& vPos, LTFLOAT fDangerRadiusSqr, LTVector* pvDangerPos, CGrenade** ppGrenadeDanger)
{
	_ASSERT(pvDangerPos);

	CGrenade** ppGrenade = g_lstGrenades.GetItem(TLIT_FIRST);
	while ( ppGrenade && *ppGrenade )
	{
		CGrenade* pGrenade = *ppGrenade;

		LTVector vGrenadePosition;
		g_pLTServer->GetObjectPos(pGrenade->m_hObject, &vGrenadePosition);

		if ( vPos.DistSqr(vGrenadePosition) < fDangerRadiusSqr )
		{
			*ppGrenadeDanger = pGrenade;
			*pvDangerPos = vGrenadePosition;
			return LTTRUE;
		}
		
		ppGrenade = g_lstGrenades.GetItem(TLIT_NEXT);
	}

	return LTFALSE;
}

// GetDifficultyFactor

LTFLOAT GetDifficultyFactor()
{
	if (!g_pGameServerShell) return g_vtDifficultyFactorHard.GetFloat();
	
	float fPlayerMod = 0.0f;
	uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

	if( nPlayersInGame > 1 )
	{
		// Increase the difficulty by an amount per player.  The
		// difficulty grows logrithmically.  2 players will add 100% of
		// fPlayerInc.  3 players adds ~150%.  4 Players adds 200%.
		float fPlayerInc = g_vtDifficultyFactorPlayerIncrease.GetFloat();
		fPlayerMod = fPlayerInc * logf(( float )nPlayersInGame ) / logf( 2.0f );
	}

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_EASY:
			return g_vtDifficultyFactorEasy.GetFloat() + fPlayerMod;
		break;

		case GD_NORMAL:
			return g_vtDifficultyFactorNormal.GetFloat() + fPlayerMod;
		break;

		case GD_VERYHARD:
			return g_vtDifficultyFactorVeryHard.GetFloat() + fPlayerMod;
		break;

		case GD_HARD:
		default :
			return g_vtDifficultyFactorHard.GetFloat() + fPlayerMod;


		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WordFilterFn
//
//	PURPOSE:	Filters out everything but the world
//
// ----------------------------------------------------------------------- //

bool WorldFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( IsMainWorld(hObj) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GroundFilterFn
//
//	PURPOSE:	Filters out everything but potential ground candidates
//
// ----------------------------------------------------------------------- //

bool GroundFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)) )
	{
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIUtils::CAIUtils()/~CAIUtils()
//              
//	PURPOSE:	Constructor takes the testing HOBJECT to report errors deeply,
//				and to filter from intersect segments
//              
//----------------------------------------------------------------------------
CAIUtils::CAIUtils(HOBJECT hOwner)
{
	m_hObject = hOwner;
}

CAIUtils::~CAIUtils()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIUtils::FindTrueFloorHeight
//
//	PURPOSE:	Find height of the floor under AI at some position.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIUtils::FindTrueFloorHeight(float flCheckDist, const LTVector& vDims, const LTVector& vPos, LTFLOAT* pfFloorHeight)
{
	AIASSERT( pfFloorHeight, m_hObject, "CAIHuman::FindFloorHeight: fFloorHeight is NULL" );

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = LTVector(vPos.x, vPos.y + vDims.y, vPos.z);
	IQuery.m_To = LTVector(vPos.x, vPos.y - flCheckDist, vPos.z);

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GroundFilterFn;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
	{
		*pfFloorHeight = IInfo.m_Point.y;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIUtils::FindTrueCeilingHeight
//
//	PURPOSE:	Find height of the ceiling above an AI at some position.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIUtils::FindTrueCeilingHeight(float flCheckDist, const LTVector& vDims, const LTVector& vPos, LTFLOAT* pfFloorHeight)
{
	AIASSERT( pfFloorHeight, m_hObject, "CAIHuman::FindFloorHeight: fFloorHeight is NULL" );

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = LTVector(vPos.x, vPos.y - vDims.y, vPos.z);
	IQuery.m_To = LTVector(vPos.x, vPos.y + flCheckDist, vPos.z);

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GroundFilterFn;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
	{
		*pfFloorHeight = IInfo.m_Point.y;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RayIntersectsBox
//
//	PURPOSE:	Returns true of ray intersects box, false if not.
//
// ----------------------------------------------------------------------- //

bool RayIntersectBox(const LTVector& vBoxMin,
					  const LTVector& vBoxMax,
					  const LTVector& vOrigin,
					  const LTVector& vDest,
					  LTVector* pvIntersection)
{
	const uint8 kNumDimensions = 3;

	// Calculate direction of ray.

	LTVector vDir = vDest - vOrigin;
	vDir.Normalize();

	// Algorithm taken from Graphics Gems p.736.

	enum EnumQuadrant
	{
		kQuad_Right,
		kQuad_Left,
		kQuad_Middle,
	};

	bool bInside = true;
	EnumQuadrant eQuad[kNumDimensions];
	LTFLOAT fCandidatePlane[kNumDimensions];

	// Find candidate planes.

	for( uint32 iDim=0; iDim < kNumDimensions; ++iDim )
	{
		if( vOrigin[iDim] < vBoxMin[iDim] )
		{
			if( vDest[iDim] < vBoxMin[iDim] )
			{
				return false;
			}

			eQuad[iDim] = kQuad_Left;
			fCandidatePlane[iDim] = vBoxMin[iDim];
			bInside = false;
		}
		else if( vOrigin[iDim] > vBoxMax[iDim] )
		{
			if( vDest[iDim] > vBoxMax[iDim] )
			{
				return false;
			}

			eQuad[iDim] = kQuad_Right;
			fCandidatePlane[iDim] = vBoxMax[iDim];
			bInside = false;
		}
		else {
			eQuad[iDim] = kQuad_Middle;
		}
	}

	// Ray origin is inside volume.

	if( bInside )
	{
		*pvIntersection = vOrigin;
		return true;
	}

	uint32 nWhichPlane = 0;
	LTFLOAT fMaxT[kNumDimensions];

	// Calculate T distances to candidate planes.

	for(int iDim=0; iDim < kNumDimensions; ++iDim )
	{
		if( ( eQuad[iDim] != kQuad_Middle ) && ( vDir[iDim] != 0.f ) )
		{
			fMaxT[iDim] = ( fCandidatePlane[iDim] - vOrigin[iDim] ) / vDir[iDim];
		}
		else {
			fMaxT[iDim] = -1.f;
		}
	}

	// Get largest of the maxT's for final choice of intersection.

	for(int iDim=1; iDim < kNumDimensions; ++iDim )
	{
		if( fMaxT[nWhichPlane] < fMaxT[iDim] )
		{
			nWhichPlane = iDim;
		}
	}

	// Check final candidate actually inside volume.

	if( fMaxT[nWhichPlane] < 0.f )
	{
		return false;
	}

	for(int iDim=0; iDim < kNumDimensions; ++iDim )
	{
		if( nWhichPlane != iDim )
		{
			(*pvIntersection)[iDim] = vOrigin[iDim] + fMaxT[nWhichPlane] * vDir[iDim];
			if( ((*pvIntersection)[iDim] < vBoxMin[iDim]) ||
				((*pvIntersection)[iDim] > vBoxMax[iDim]) )
			{
				return false;
			}
		}
		else {
			(*pvIntersection)[iDim] = fCandidatePlane[iDim];
		}
	}

	return true;
}
