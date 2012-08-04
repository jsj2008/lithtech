//------------------------------------------------------------------
//
//  FILE      : GameMath.h
//
//  PURPOSE   : Defines the Math class, which contains tons of
//              useful math routines!
//
//  CREATED   : 1st May 1996
//
//  COPYRIGHT : Monolith Productions Inc. 1996-99
//
//------------------------------------------------------------------

#ifndef __GAMEMATH_H__
 
	#define __GAMEMATH_H__
	
#ifndef __LTSYSOPTIM_H__
	#include "ltsysoptim.h"
#endif


 
	// Includes....


class Math
{
    public:

        // Constructor
        Math();

        // Destructor
        ~Math();


        // Member functions

 
        // Square root.
        static LTFLOAT      Sqrt(LTFLOAT x)       { return (LTFLOAT)sqrt(x); }


 
			static LTFLOAT		Cos( LTFLOAT angle )	{ return ltcosf(angle); }
			static LTFLOAT		Sin( LTFLOAT angle )	{ return ltsinf(angle); }


 
			static LTFLOAT		ACos( LTFLOAT x )		{ return ltacosf(x); }


        // Returns from 0 to CIRCLE (NOT -PI to PI).
        static LTFLOAT      ATan2(LTFLOAT x, LTFLOAT y);

};


#endif




