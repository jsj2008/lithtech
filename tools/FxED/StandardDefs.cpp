//----------------------------------------------------------------------
//
//  MODULE   : STANDARDDEFS.CPP
//
//  PURPOSE  : Definition file implementation
//
//  CREATED  : 11/14/97 - 8:10:23 PM
//
//----------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "StandardDefs.h"

// Global Variables

float						g_SineTable[ST_SIZE];
CMath						g_Math;
float						g_Step = ST_SIZE / 360.0f;

float						g_AngToRad = TWO_PI / 360.0f;
float						g_RadToAng = 360.0f / TWO_PI;

float						g_RadToSineTableIndex = ST_SIZE / PI;

// Global Functions

//----------------------------------------------------------
//
// FUNCTION : CMath::CMath()
//
// PURPOSE	: Constructor
//
//----------------------------------------------------------

CMath::CMath()
{
	// Generate all tables

	// Generate sine table

	float step   = TWO_PI / ST_SIZE;
	float degree = 0.0f;

	for (int i = 0; i < ST_SIZE; i ++)
	{
		g_SineTable[i] = (float)sin(degree);
		degree += step;
	}
}