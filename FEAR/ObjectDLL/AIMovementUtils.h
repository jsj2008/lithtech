// ----------------------------------------------------------------------- //
//
// MODULE  : AIMovementUtils.h
//
// PURPOSE : This file contains utility functions shared by AIMovement 
//			 code and AI behavior code.  This insures AI behavior code 
//			 can make accurate predictions about how the movement code
//			 will behave.
//
// CREATED : 11/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIMOVEMENTUTILS_H_
#define _AIMOVEMENTUTILS_H_

class DebugLineSystem;

class AIMovementUtils
{
public:
	// This is a utility function for drawing a characters cylinder for
	// visualizing collision more easily.

	static void DrawCollisionCylinder( DebugLineSystem& sys, const LTVector& vCenter, const LTVector& vDims, float flRadius, int nSegments );

	// This function is called once a collision has been found; this means the Y 
	// does not need to be tested in this function.

	static bool BlocksPosition2D(const LTVector& vCharacterPos, float flCharacterRadiusSqr, float flAIRadiusSqr, const LTVector& vDest );

	// This function determines if a collision occurs in 3D.

	static bool Collides(const LTVector& vCharacterPos, const LTVector& vCharacterDims, float flCharacterRadiusSqr,
						const LTVector& vTesterPos, const LTVector& vTesterDims, float flTesterRadiusSqr,
						float flBufferRadius );

	// Returns the radius buffer used for various collision operations.

	static float GetRadiusBuffer( HOBJECT hTarget );
};


#endif // _AIMOVEMENTUTILS_H_
