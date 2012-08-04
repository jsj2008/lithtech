//------------------------------------------------------------------
//
//	FILE	  : GameMath.cpp
//
//	PURPOSE	  : Math function implementations.
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Monolith Productions Inc. 1996-99
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "gamemath.h"
#include "ltbasetypes.h"


// Globals....
static uint8	g_OctantLookup[2][2][2];


// Angle to index
#define ANGLE_TO_INDEX(x)	(uint16)( ((LTFLOAT)ANGLE_RES/MATH_CIRCLE) * (x) )


Math	g_Math;

Math::Math()
{
	// Setup the octant table for the ATan2 function.
	// (g_OctantLookup[x>0][y>0][x>y]).
	g_OctantLookup[LTTRUE][LTTRUE][LTTRUE] 		= 0;
	g_OctantLookup[LTTRUE][LTTRUE][LTFALSE] 	= 1;
	g_OctantLookup[LTFALSE][LTTRUE][LTFALSE]	= 2;
	g_OctantLookup[LTFALSE][LTTRUE][LTTRUE] 	= 3;
	g_OctantLookup[LTFALSE][LTFALSE][LTTRUE] 	= 4;
	g_OctantLookup[LTFALSE][LTFALSE][LTFALSE]	= 5;
	g_OctantLookup[LTTRUE][LTFALSE][LTFALSE] 	= 6;
	g_OctantLookup[LTTRUE][LTFALSE][LTTRUE] 	= 7;
}


Math::~Math()
{
}


LTFLOAT Math::ATan2( LTFLOAT x, LTFLOAT y )
{
	LTFLOAT	answer;
	uint8	octant;


	// Check for these here to avoid divide-by-zeros.
	if( x == 0 )
	{
		if( y == 0 )
			return 0.0f;
		else if( y > 0 )
			return MATH_HALFPI;
		else
			return MATH_PI + MATH_HALFPI;
	}
	
	if( y == 0 )
	{
		if( x > 0 )
			return 0.0f;
		else
			return MATH_PI;
	}

	octant = g_OctantLookup[x>0][y>0][LTABS(x) > LTABS(y)];
	switch( octant )
	{
		case 0:
			answer = y/x;
			break;

		case 1:
			answer = 1 - x/y;
			break;

		case 2:
			answer = LTABS(x) / y;
			break;

		case 3:
			answer = 1 - y / LTABS(x);
			break;

		case 4:
			answer = y / x;
			break;

		case 5:
			answer = 1 - x / y;
			break;

		case 6:
			answer = x / LTABS(y);
			break;

		default:
			answer = 1 - LTABS(y) / x;
			break;
	}

	// Remap the answer to be between 0 and PI.
	answer += octant;
	answer = (answer * MATH_CIRCLE) / 8;

	return answer;
}


