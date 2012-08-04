// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "AIUtils.h"
#include "AI.h"
#include "AIDB.h"
#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "ProjectileTypes.h"
#include "PlayerObj.h"
#include "AIAssert.h"
#include "AIPathMgrNavMesh.h"
#include "CharacterMgr.h"

// Globals

int g_cIntersectSegmentCalls = 0;
VarTrack g_vtAIConsoleVar;
VarTrack g_vtMuteAIAssertsVar;


// Constants

const float c_fFOV180  = 0.0f;
const float c_fFOV160  = 0.1736481776669f;
const float c_fFOV140  = 0.3420201433257f;
const float c_fFOV120  = 0.5f;
const float c_fFOV90   = 0.70710678118654752440084436210485f;
const float c_fFOV75   = 0.7933533402912351645797769615013f;
const float c_fFOV60   = 0.86602540378443864676372317075294f;
const float c_fFOV45   = 0.92387953251128675612818318939679f;
const float c_fFOV30   = 0.9659258262890682867497431997289f;

const float c_fUpdateDelta         = 0.01f;

const float c_fFacingThreshhold        = .999999f;

const char c_szKeyPickUp[]			= "PICKUP";
const char c_szKeyFireWeapon[]		= "FIRE";
const char c_szKeyStopFireWeapon[]	= "STOPFIRE";
const char c_szKeyThrow[]			= "THROW";
const char c_szKeyTurnOn[]			= "TURNON";
const char c_szKeyTurnOff[]			= "TURNOFF";
const char c_szKeyDraw[]			= "DRAW";
const char c_szKeyHolster[]			= "HOLSTER";

const char c_szKeyFX[]				= "FX";


void GetValueRange(CAI* pAI, const char* szValue, float* pfMin, float* pfMax)
{
	HOBJECT hAI = pAI ? pAI->m_hObject : NULL;
	AIASSERT(szValue[0] == '[' && szValue[LTStrLen(szValue) - 1] == ']', hAI, "GetValueRange: range not in the form [min,max]");
	AIASSERT(LTStrLen(szValue) < 64, hAI, "GetValueRange: range is more than 64 characters");

	char szTemp[64];
	LTStrCpy( szTemp, szValue + 1, LTARRAYSIZE( szTemp ));

	char* pTok = strtok( szTemp, "," );
	*pfMin = (float)atof( pTok );

	pTok = strtok( NULL, "," );
	*pfMax = (float)atof( pTok );
}

//-------------------------------------------------------------------------------------------
// IsAIXXX
//
// Checks if handle is a handle to a CXXX
// Arguments:
//		hObject - handle to object to test
// Return:
//      bool
//-------------------------------------------------------------------------------------------

bool IsAI( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;
	
	HCLASS hTest  = g_pLTServer->GetClass( "CAI" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

bool IsAIRegion( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "AIRegion" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

bool IsAINode( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "AINode" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
	return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

bool IsAINodeSmartObject( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "AINodeSmartObject" );
    HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
    return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

bool IsAICombatOpportunity( HOBJECT hObject )
{
	if ( NULL == hObject )
		return false;

	HCLASS hTest  = g_pLTServer->GetClass( "AICombatOpportunity" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hObject );
	return ( g_pLTServer->IsKindOf( hClass, hTest ));
}

// Dead AI

bool IsDeadAI( HOBJECT hObject )
{
	if( !hObject ) return true;

	if( !IsAI( hObject ) ) return false;

	CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject( hObject );
	return ( pCharacter->GetDestructible()->IsDead() );
}

bool IsDeadCharacter(  HOBJECT hObject )
{
	if( !hObject ) return true;

	if( !IsCharacter( hObject ) ) return false;

	CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject( hObject );
	return ( pCharacter->GetDestructible()->IsDead() );
}

// Awareness.

EnumAIAwareness StringToAwareness( const char* pszAwareness )
{
	if( LTStrIEquals( pszAwareness, "Relaxed" ) )
	{
		return kAware_Relaxed;
	}
	else if( LTStrIEquals( pszAwareness, "Suspicious" ) )
	{
		return kAware_Suspicious;
	}
	else if( LTStrIEquals( pszAwareness, "Alert" ) )
	{
		return kAware_Alert;
	}

	return kAware_Any;
}

// Console Output

void AIError(const char* szFormat, ...)
{
#ifndef _FINAL

	// jeffo 03/03/04:
	// Do NOT require DebugLevel to be 1, or LD will never see their errors!!
	//if (GetConsoleInt("DebugLevel",0) < 1 ) return;

	static char szBuffer[4096];
	va_list val;
	va_start(val, szFormat);
	LTVSNPrintF(szBuffer, LTARRAYSIZE(szBuffer), szFormat, val);
	va_end(val);

	// Check that the pointer is valid before use is attempted,
	// as if this is called in a WorldEdit crash, the LTServer pointer
	// is no where near existing.  This is NOT an error condition, just
	// an unfortunate side effect of WorldEdit and the engine using the same
	// GameServer.dll file for different situations.
	if ( g_pLTServer )
	{
		g_pLTServer->CPrint("AI ERROR: %s", szBuffer);
	}
#endif
}

// GetDifficultyFactor

float GetDifficultyFactor()
{
	if (!g_pGameServerShell)
	{
		if( g_pAIDB )
		{
			return g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorHard;
		}
		else {
			return 1.f;
		}
	}

	float fPlayerMod = 0.0f;
	uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

	if( nPlayersInGame > 1 )
	{
		// Increase the difficulty by an amount per player.  The
		// difficulty grows logrithmically.  2 players will add 100% of
		// fPlayerInc.  3 players adds ~150%.  4 Players adds 200%.
		float fPlayerInc = g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorPlayerIncrease;
		fPlayerMod = fPlayerInc * logf(( float )nPlayersInGame ) / logf( 2.0f );
	}

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_EASY:
			return g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorEasy + fPlayerMod;
		break;

		case GD_NORMAL:
			return g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorNormal + fPlayerMod;
		break;

		case GD_VERYHARD:
			return g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorVeryHard + fPlayerMod;
		break;

		case GD_HARD:
		default :
			return g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorHard + fPlayerMod;


		break;
	}
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
	float fCandidatePlane[kNumDimensions];

	// Find candidate planes.
	uint32 iDim;
	for( iDim=0; iDim < kNumDimensions; ++iDim )
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
	float fMaxT[kNumDimensions];

	// Calculate T distances to candidate planes.

	for( iDim=0; iDim < kNumDimensions; ++iDim )
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

	for( iDim=1; iDim < kNumDimensions; ++iDim )
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

	for( iDim=0; iDim < kNumDimensions; ++iDim )
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TrimLineSegmentByRadius
//
//	PURPOSE:	Trim a line segment by a radius.
//
// ----------------------------------------------------------------------- //

bool TrimLineSegmentByRadius( float fRadius, LTVector* pv0, LTVector* pv1, bool bTrimV0, bool bTrimV1 )
{
	// Sanity check.

	if( !( pv0 && pv1 ) )
	{
		return false;
	}

	// Do not trim anything.

	if( !( bTrimV0 || bTrimV1 ) )
	{
		return false;
	}

	// Line segment is smaller than the minimum radius or diameter.
	// Using the center or endpoint of the segment is the best we can do.

	LTVector vDir = *pv1 - *pv0;
	float fDiameter = fRadius * 2.f;
	float fDistSqr = pv0->DistSqr( *pv1 );

	if( !bTrimV0 )
	{
		if( fDistSqr <= fRadius * fRadius )
		{
			*pv1 = *pv0;
			return true;
		}
	}
	else if( !bTrimV1 )
	{
		if( fDistSqr <= fRadius * fRadius )
		{
			*pv0 = *pv1;
			return true;
		}
	}
	else if( fDistSqr <= fDiameter * fDiameter )
	{
		*pv0 = *pv0 + ( vDir * 0.5f );
		*pv1 = *pv0;
		return true;
	}

	// Trim the line segment.
	// Trimming by epsilon is a hack that keeps the rest of the AI systems running.
	// Otherwise, we end up with duplicate waypoints.

	vDir.Normalize();
	float fEpsilon = 0.01f;

	if( bTrimV0 )
	{
		*pv0 += vDir * fRadius;
	}
	else {
		*pv0 += vDir * fEpsilon;
	}

	if( bTrimV1 )
	{
		*pv1 -= vDir * fRadius;
	}
	else {
		*pv1 -= vDir * fEpsilon;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RayIntersectLineSegment
//
//	PURPOSE:	Find the intersection point between a line and a ray.
//
// ----------------------------------------------------------------------- //

EnumRayIntersectResult RayIntersectLineSegment( const LTVector& l0, const LTVector& l1, const LTVector& r0, const LTVector& r1, bool bSnapToSegment, LTVector* pvPtIntersect )
{
	//
	// Finding the intersection with the following method:
	// We have line a going from P1 to P2:
	//    Pa = P1 + ua( P2 - P1 )
	// and line b going from P3 to P4:
	//    Pb = P3 + ub( P4 - P3 )
	//
	// Solving for Pa = Pb:
	//    x1 + ua( x2 - x1 ) = x3 + ub( x4 - x3 )
	//    y1 + ua( y2 - y1 ) = y3 + ub( y4 - y3 )
	//
	// Solving for ua and ub:
	//    ua = ( ( x4 - x3 )( y1 - y3 ) - ( y4 - y3 )( x1 - x3 ) ) / denom
	//    ub = ( ( x2 - x1 )( y1 - y3 ) - ( y2 - y1 )( x1 - x3 ) ) / denom
	//    denom = ( y4 - y3 )( x2 - x1 ) - ( x4 - x3 )( y2 - y1 )
	//
	// x = x1 + ua( x2 - x1 )
	// y = y1 + ua( y2 - y1 )
	//

	float fDenom = ( ( r1.z - r0.z ) * ( l1.x - l0.x ) ) - ( ( r1.x - r0.x ) * ( l1.z - l0.z ) );

	// Lines are parallel (or coincident).

	if( fDenom == 0.f )
	{
		return kRayIntersect_Failure;
	}

	// Find the point of intersection.

	EnumRayIntersectResult eResult = kRayIntersect_Success;

	float fUa = ( ( ( r1.x - r0.x ) * ( l0.z - r0.z ) ) - ( ( r1.z - r0.z ) * ( l0.x - r0.x ) ) ) / fDenom;
	float fUb = ( ( ( l1.x - l0.x ) * ( l0.z - r0.z ) ) - ( ( l1.z - l0.z ) * ( l0.x - r0.x ) ) ) / fDenom;

	// This epsilon was added in response to a case where the AI was 
	// failing to find an intersection, preventing the straight path
	// test from passing.  This occured when the AI was standing directly
	// on the line segment; fUb was very slightly less than 0, which resulted
	// in a failure.
	// BJL - Increased the epsilon from 0.001 to 0.01 in response to another 
	// failure.  This new value appears to match the value used else where for 
	// determining if a point is in a poly.

	const float kflRayEpsilon = 0.01f;

	if( ( !bSnapToSegment ) &&
		( ( fUa < 0.f ) ||
			( fUa > 1.f ) ||
			( fUb + kflRayEpsilon < 0.f ) ||
			( fUb - kflRayEpsilon > 1.f ) ) )
	{
		return kRayIntersect_Failure;
	}

	// The path intersects the link beyond the boundaries of the link's
	// line segment, so just snap it to one of the boundaries.

	if( fUa < 0.f )
	{
		if( pvPtIntersect )
		{
			*pvPtIntersect = l0;
		}
		eResult = kRayIntersect_SnappedToSegment;
	}
	else if( fUa > 1.f )
	{
		if( pvPtIntersect )
		{
			*pvPtIntersect = l1;
		}
		eResult = kRayIntersect_SnappedToSegment;
	}

	// The path intersects the link within its boundaries.

	else if( pvPtIntersect )
	{
		pvPtIntersect->x = l0.x + fUa * ( l1.x - l0.x );
		pvPtIntersect->y = l0.y + fUa * ( l1.y - l0.y );
		pvPtIntersect->z = l0.z + fUa * ( l1.z - l0.z );
	}

	return eResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FindNearestPointOnLine
//
//	PURPOSE:	Find the nearest point on a line from a specified position.
//
// ----------------------------------------------------------------------- //

bool FindNearestPointOnLine( const LTVector& l0, const LTVector& l1, const LTVector& vPos, LTVector* pvPosNearest )
{
	// Sanity check.

	if( !pvPosNearest )
	{
		return false;
	}

	// Find the line's normal.

	LTVector vUp( 0.f, 1.f, 0.f );
	LTVector vDir = l1 - l0;
	vDir.Normalize();

	LTVector vNormal = vDir.Cross( vUp );
	vNormal.Normalize();

	// Find the nearest intersection point between the point and the line.

	LTVector vRay0 = vPos + ( vNormal * 100000.f );
	LTVector vRay1 = vPos - ( vNormal * 100000.f );

	return ( kRayIntersect_Failure != RayIntersectLineSegment( l0, l1, vRay0, vRay1, true, pvPosNearest ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DidDamage
//
//	PURPOSE:	Returns true if the damage this fact represents in fact
//				damaged the AI.  This is an approximation, as the AIs
//				destructibles state may have changed.
//
// ----------------------------------------------------------------------- //

bool DidDamage( CAI* pAI, CAIWMFact* pFact )
{
	// Sanity checks.

	if ( !pFact || !pAI )
	{
		return false;
	}

	// AI cannot take damage.

	if ( !pAI->GetDestructible()->GetCanDamage() )
	{
		return false;
	}

	// AI isn't damaged by this type.

	DamageType eType = DT_INVALID;
	float flDamageAmount;
	pFact->GetDamage( &eType, &flDamageAmount, NULL );

	if ( pAI->GetDestructible()->IsCantDamageType( eType ) )
	{
		return false;
	}

	// AI was damaged.

	return true;
}

bool IsClearForMovement( CAI* pAI, const LTVector vStart, const LTVector& vEnd)
{
	// No straight path exists.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), vStart, vEnd, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Path is blocked by allies.

	if( g_pCharacterMgr->RayIntersectAI( vStart, vEnd, pAI, NULL, NULL ) )
	{
		return false;
	}

	// Path is clear.

	return true;
}

bool GetAnimationTransform( CAI* pAI, HMODELANIM hAni, LTRigidTransform& rOutTransform )
{
	// Insure the animation is valid.

	if( hAni == INVALID_MODEL_ANIM )
	{
		return false;
	}

	// Number of keyframes.

	uint32 nNumKeyFrames;
	g_pModelLT->GetAnimKeyFrames( pAI->m_hObject, hAni, nNumKeyFrames );

	// Root node.

	HMODELNODE hNode;
	g_pModelLT->GetRootNode( pAI->m_hObject, hNode );

	// Get the transform for the end of the animation.

	LTRigidTransform tTransform;
	if ( LT_OK != g_pModelLT->GetAnimNodeTransform( pAI->m_hObject, hAni, hNode, nNumKeyFrames - 1, tTransform ) )
	{
		return false;
	}

	rOutTransform = tTransform;
	return true;
}

bool AIUtil_PositionShootable(CAI* pAI, const LTVector& vTargetOrigin)
{
	if (NULL == pAI 
		|| NULL == pAI->GetAIWeaponMgr()
		|| NULL == pAI->GetAIWeaponMgr()->GetCurrentWeapon())
	{
		return false;
	}

	LTVector vAIWeaponPosition = pAI->GetWeaponPosition(pAI->GetAIWeaponMgr()->GetCurrentWeapon(), false);

	// Bail if the AIs target is in the same position as the weapon.

	if (vTargetOrigin == vAIWeaponPosition)
	{
		return false;
	}

	// Bail if the combat opportunity is too far above or below the AI
	// (must be within the aiming range). 
	//
	// TODO: Determine what a good FOV without hardcoding this value. The
	// selected FOV fixed out cases, but this is animation driven, there
	// is no guarantee this is the ideal value.

	LTVector vToTargetUnit3D = (vTargetOrigin - vAIWeaponPosition).GetUnit();
	vToTargetUnit3D.y = fabs(vToTargetUnit3D.y);
	float fDotUp = LTVector(0.0f, 1.0f, 0.0f).Dot( vToTargetUnit3D );
	if (fDotUp >= c_fFOV60)
	{
		return false;
	}

	// Bail if the AI has to turn his back on his enemy/target to fire at this
	// position.

	LTVector vDirCombatOp = vTargetOrigin - vAIWeaponPosition;
	vDirCombatOp.y = 0.f;
	vDirCombatOp.Normalize();

	LTVector vDirTargetChar = pAI->GetAIBlackBoard()->GetBBTargetPosition() - vAIWeaponPosition;
	vDirTargetChar.y = 0.f;
	vDirTargetChar.Normalize();

	float fHorizontalDp = vDirCombatOp.Dot( vDirTargetChar );
	if( fHorizontalDp <= c_fFOV140 )
	{
		return false;
	}

	// Position is shootable.

	return true;
}
