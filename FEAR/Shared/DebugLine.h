// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.h
//
// PURPOSE : 
//
// CREATED : 3/29/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUG_LINE_H__
#define __DEBUG_LINE_H__

struct DebugLine
{
	struct Color
	{
		uint8 r;
		uint8 g;
		uint8 b;

		Color(uint8 my_r, uint8 my_g, uint8 my_b)
			: r(my_r),
			  g(my_g),
			  b(my_b) {}
	};

	static LTRGB null_rgba;

	LTVector vSource;
	LTVector vDest;
	
	LTRGB rgba;
	
	DebugLine( const LTVector & source = LTVector(0.0f,0.0f,0.0f),
			   const LTVector & dest = LTVector(0.0f,0.0f,0.0f),
			   const LTRGB & my_rgba = null_rgba );

	DebugLine( const LTVector & source,
			   const LTVector & dest,
			   const Color & my_color,
			   const uint8 alpha = 255 );
};

#endif //__DEBUG_LINE_H__
