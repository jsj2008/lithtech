// ----------------------------------------------------------------------- //
//
// MODULE  : AIMovementUtils.cpp
//
// PURPOSE : 
//
// CREATED : 11/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIMovementUtils.h"
#include "DebugLineSystem.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIMovementUtils::DrawCylinder
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void AIMovementUtils::DrawCollisionCylinder( DebugLineSystem& sys, 
											const LTVector& vCenter, const LTVector& vDims, 
											float flRadius, int nSegments )
{
	float flHalfHeight = vDims.y;

	LTVector vCenterTop = vCenter + LTVector(0.f, flHalfHeight, 0.f);
	LTVector vCenterBottom = vCenter - LTVector(0.f, flHalfHeight, 0.f);
	LTVector vLastOffset;
	for ( int iSeg = 0; iSeg < nSegments + 1; ++iSeg )
	{
		float flArcSegment = iSeg * ( MATH_CIRCLE / nSegments );
		LTVector vOffset( sinf( flArcSegment ) * flRadius, 0.f, cosf( flArcSegment ) * flRadius );

		if ( iSeg != 0 )
		{
			// Draw the upper line
			sys.AddLine( vCenterTop + vOffset, vCenterTop + vLastOffset );

			// Draw the lower line
			sys.AddLine( vCenterBottom + vOffset, vCenterBottom + vLastOffset );

			// Draw the vertical connector
			sys.AddLine( vCenterTop + vOffset, vCenterBottom + vOffset );
		}

		vLastOffset = vOffset;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIMovementUtils::BlocksPosition2D
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool AIMovementUtils::BlocksPosition2D( const LTVector& vCharacterPos, float flCharacterRadiusSqr, 
									   float flAIRadiusSqr, const LTVector& vDest )
{
	LTVector vSeparation2D = vDest - vCharacterPos;
	vSeparation2D.y = 0;
	return ( vSeparation2D.MagSqr() <  ( flCharacterRadiusSqr + flAIRadiusSqr ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIMovementUtils::Collides
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool AIMovementUtils::Collides( const LTVector& vCharacterPos, const LTVector& vCharacterDims, float flCharacterRadiusSqr,
							   const LTVector& vTesterPos, const LTVector& vTesterDims, float flTesterRadiusSqr,
							   float flBufferRadius )
{
	// Determine if a collision occured in Y

	float flCharacterYMax = vCharacterPos.y + vCharacterDims.y;
	float flCharacterYMin = vCharacterPos.y - vCharacterDims.y;

	float flTesterYMax = vTesterPos.y + vTesterDims.y;
	float flTesterYMin = vTesterPos.y - vTesterDims.y;

	if ( flCharacterYMin > flTesterYMax || flCharacterYMax < flTesterYMin )
	{
		return false;
	}

	// Determine if a collision occured in XZ

	LTVector vDistance2D = vCharacterPos - vTesterPos;
	vDistance2D.y = 0.0f;
	float flDistanceToCharacter2D = vDistance2D.MagSqr();

	if ( flDistanceToCharacter2D > 
		( flBufferRadius*flCharacterRadiusSqr + flBufferRadius*flTesterRadiusSqr) )
	{
		return false;
	}

	// Collision!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIMovementUtils::GetRadiusBuffer
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

float AIMovementUtils::GetRadiusBuffer( HOBJECT hTarget )
{
	if ( !IsPlayer( hTarget ) )
	{
		return 1.0f;
	}

	return g_pAIDB->GetAIConstantsRecord()->fAntiPenetrationRadiusBuffer;
}
