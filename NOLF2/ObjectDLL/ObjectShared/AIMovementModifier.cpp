//----------------------------------------------------------------------------
//              
//	MODULE:		AIMovementModifier.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	26.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "AIMovementModifier.h"		
#include "AIAssert.h"
#include "AIUtils.h"

// Forward declarations

// Globals

// Statics



//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::CHoverMovementModifier()
//              
//	PURPOSE:	Static members, constructor, destructor and save/load
//				functionality for the Hover movement modifier
//              
//----------------------------------------------------------------------------
const float CHoverMovementModifier::m_cMaxVerticalDifference = 32.0f;
const float CHoverMovementModifier::m_cCheckDist = 1000.0f;
const float CHoverMovementModifier::m_cMaxRateSpeedScalar = 0.3f;
const float CHoverMovementModifier::m_cStoppedMovementRate = 3.0f;

CHoverMovementModifier::CHoverMovementModifier(){}
CHoverMovementModifier::~CHoverMovementModifier(){}
int CHoverMovementModifier::Save(ILTMessage_Write* pMsg)
{
	return 0;
}
int CHoverMovementModifier::Load(ILTMessage_Read* pMsg)
{
	return 0;
}
void CHoverMovementModifier::Init(){}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::Update()
//              
//	PURPOSE:	Try to smooth the path within limits so that we don't snap on
//				stairs.  Otherwise just 
//              
//----------------------------------------------------------------------------
LTVector CHoverMovementModifier::Update( HOBJECT hObject, const LTVector& vDims, const LTVector& vOldPos, const LTVector& vNewPos, AIVolume* pLastVolume )
{
	// Get the position the AI is really at -- that is, the new X and Z, with
	// the OLD positions Y

	const LTVector vTruePos = LTVector( vNewPos.x, vOldPos.y, vNewPos.z);
	LTVector vFinalPosition = vTruePos;

	// Find the distance down to the new position the AI would be popped to.

	float flLowerBound = GetLowerBound( hObject, m_cCheckDist, vDims, vTruePos );

	// Find the DIFFERENCE between the heights.  If it is less than +-X, then
	// drift in that direction.  If it is greater, then snap to the max
	// distance away that is allowed

	float flDifference = (float)fabs( flLowerBound - vOldPos.y );
	if ( flDifference != 0.0f )
	{
		if ( flDifference <= m_cMaxVerticalDifference )
		{
			// Attempt to make the path a little bit smoother by adjusting
			// the position a bit more slowly
			LTVector vHorizontalMovement = vNewPos - vOldPos;
			vHorizontalMovement.y = 0;
			vFinalPosition.y = Interpolate( vOldPos.y, flLowerBound, vHorizontalMovement.Mag() );
		}
		else
		{
			vFinalPosition.y = Snap( vOldPos.y, flLowerBound, flDifference );
		}
	}

	return vFinalPosition;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::GetLowerBound()
//              
//	PURPOSE:	Returns the lowest point the AI is allowed to move to.  If
//				there is no volume, Min the point.  If there is no floorheight,
//				we can go at least that low.
//              
//----------------------------------------------------------------------------
float CHoverMovementModifier::GetLowerBound( HOBJECT hObject, float flCheckDist, const LTVector& vDims, const LTVector& vPos )
{
	float flFloorHeight;
	CAIUtils Utils(hObject);
	LTVector vStartPosition = LTVector( vPos.x, vPos.y+m_cMaxVerticalDifference, vPos.z);
	if ( LTFALSE == Utils.FindTrueFloorHeight(flCheckDist, vDims, vStartPosition, &flFloorHeight) )
	{
		AIASSERT( 0, hObject, "Floor not found!" );
	}

	return flFloorHeight;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::GetUpperBound()
//              
//	PURPOSE:	Returns the highest point the AI is allowed to move to.  If
//				there is no volume, Max the point.  If there is no ceiling
//				height, assume that we can go x high
//              
//----------------------------------------------------------------------------
float CHoverMovementModifier::GetUpperBound( HOBJECT hObject, float flCheckDist, const LTVector& vDims, const LTVector& vPos )
{
	float flCeilingHeight;
	CAIUtils Utils(hObject);
	if ( LTFALSE == Utils.FindTrueCeilingHeight(flCheckDist, vDims, vPos, &flCeilingHeight) )
	{
		AIASSERT( 0, hObject, "Ceiling not found!" );
	}

	return flCeilingHeight;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::Interpolate()
//              
//	PURPOSE:	Attempt to smooth the vertical movement a bit, so the hovering
//				AI does not snap on stairs.
//              
//----------------------------------------------------------------------------
float CHoverMovementModifier::Interpolate(float OldY, float LowerY, float flSpeed )
{
	if ( fabs(LowerY - OldY) < 1.0f )
	{
		// If the distance to travel is less than 1, just move there so we can
		// stop interpolating so frequently.
		return LowerY;
	}

	// Find the distance we are changing.
	float flAdjustingDiff = (LowerY - OldY);

	float flScalar = (float)fabs(flAdjustingDiff) / m_cMaxVerticalDifference;
	
	// AI wants to go upwards faster when the distance is greater	
	float flPotentialDifference = flAdjustingDiff*flScalar;		 

	// AI cannot go up faster than it is going forward(?)
	float flSpeedDifferenceLimiter;
	if ( flSpeed == 0.0f )
	{
		flSpeedDifferenceLimiter = m_cStoppedMovementRate;
	}
	else
	{
		flSpeedDifferenceLimiter = flSpeed * m_cMaxRateSpeedScalar;
	}

	float flOffset = LTCLAMP( flPotentialDifference, (-flSpeedDifferenceLimiter), flSpeedDifferenceLimiter );

	return ( OldY + flOffset );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHoverMovementModifier::Snap()
//              
//	PURPOSE:	Do a snap -- force the AI to a good position as it is changing
//				height too quickly.  Pick the point closest to the old point
//				which is 'close enough'
//              
//----------------------------------------------------------------------------
float CHoverMovementModifier::Snap( float OldY, float NewY, float flDifference )
{
	float flAdjustment = flDifference - m_cMaxVerticalDifference;
	if ( NewY < OldY )
	{
		return ( OldY - flAdjustment );
	}
	else
	{
		return ( OldY + flAdjustment );
	}
}