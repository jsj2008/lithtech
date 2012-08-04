//----------------------------------------------------------------------
//
//  MODULE   : STDDEFS.H
//
//  PURPOSE  : Definitions file
//
//  CREATED  : 11/14/97 - 8:08:11 PM
//
//----------------------------------------------------------------------

#ifndef __STDDEFS_H_
	#define __STDDEFS_H_

	// Includes....

	#include "Math.h"
	#include "String.h"
	#include "Assert.h"
	
	// Defines....

	#define PI					3.141592736f
	#define ONE_OVER_PI			0.318309888f
	#define TWO_PI				6.283185473f
	#define ANG_TO_RAD			0.017453292f
	#define RAD_TO_ANG			57.29030717f

	#define EPSILON				0.000000001f

	#ifndef BOOL
		#define BOOL			int
	#endif

	#ifndef TRUE
		#define TRUE			1
	#endif

	#ifndef FALSE
		#define FALSE			0
	#endif

	#ifndef ASSERT
		#define ASSERT			assert
	#endif

	#ifndef BYTE
		#define BYTE			unsigned char
	#endif

	#ifndef WORD
		#define WORD			unsigned short
	#endif

	#ifndef DWORD
		#define DWORD			unsigned long
	#endif

	#define	ST_SIZE				32768
	#define ST_MASK				32767
	#define ST_QUARTER			8192

	// Classes
	
	class CMath
	{
		public:
	
			// Constructor
								CMath();
	};

	// External declarations

	extern float				g_SineTable[ST_SIZE];
	extern float				g_RadToSineTableIndex;
	extern float				g_AngToRad;
	extern float				g_RadToAng;
	
	// Trig Functions

	float						Sine(float Angle);
	float						Cosine(float Angle);
	float						RadianToAngle(float r);
	float						AngleToRadian(float a);

	//----------------------------------------------------------
	//
	// FUNCTION : Sine()
	//
	// PURPOSE	: Returns sine
	//
	//----------------------------------------------------------

	inline float Sine(float Angle)
	{
		return g_SineTable[(int)(Angle * g_RadToSineTableIndex) & ST_MASK];
	}

	//----------------------------------------------------------
	//
	// FUNCTION : Cosine()
	//
	// PURPOSE	: Returns cosine
	//
	//----------------------------------------------------------

	inline float Cosine(float Angle)
	{
		return g_SineTable[((int)(Angle * g_RadToSineTableIndex) + ST_QUARTER) & ST_MASK];
	}

	//----------------------------------------------------------
	//
	// FUNCTION : RadianToAngle()
	//
	// PURPOSE	: Converts a radian value to an angle
	//
	//----------------------------------------------------------

	inline float RadianToAngle(float r)
	{
		return r * g_RadToAng;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : AngleToRadian()
	//
	// PURPOSE	: Converts an angle to a radian value
	//
	//----------------------------------------------------------

	inline float AngleToRadian(float a)
	{
		return a * g_AngToRad;
	}

#endif