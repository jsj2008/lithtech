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


class ILTMessage_Read;
class ILTMessage_Write;

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
			   const LTRGB & my_rgba = null_rgba )
		: vSource(source),
		  vDest(dest),
		  rgba(my_rgba) {}

	DebugLine( const LTVector & source,
			   const LTVector & dest,
			   const Color & my_color,
			   const uint8 alpha = 255 )
		: vSource(source),
		  vDest(dest)
	{
		rgba.r = my_color.r;
		rgba.g = my_color.g;
		rgba.b = my_color.b;
		rgba.a = alpha;
	}

};


ILTMessage_Write & operator<<(ILTMessage_Write & out, const DebugLine & line_to_save);
ILTMessage_Read & operator>>(ILTMessage_Read & in, DebugLine & line_to_load);

#endif //__DEBUG_LINE_H__
