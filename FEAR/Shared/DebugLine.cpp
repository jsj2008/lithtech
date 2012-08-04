// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.cpp
//
// PURPOSE : 
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DebugLine.h"

LTRGB DebugLine::null_rgba = {255,255,255,255};

DebugLine::DebugLine(const LTVector & source /*= LTVector(0.0f,0.0f,0.0f)*/,
					 const LTVector & dest /*= LTVector(0.0f,0.0f,0.0f)*/,
					 const LTRGB & my_rgba /*= null_rgba*/ )
	: vSource(source),
		vDest(dest),
		rgba(my_rgba)
{
}

DebugLine::DebugLine(const LTVector & source,
					 const LTVector & dest,
					 const Color & my_color,
					 const uint8 alpha /*= 255*/ )
	: vSource(source),
		vDest(dest)
{
	rgba.r = my_color.r;
	rgba.g = my_color.g;
	rgba.b = my_color.b;
	rgba.a = alpha;
}
