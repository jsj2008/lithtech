// ----------------------------------------------------------------------- //
//
// MODULE  : ColorUtilities.cpp
//
// PURPOSE : Implement common color utilities
//
// CREATED : 09/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ColorUtilities.h"


uint32 FadeARGB(uint32 nBaseColor, float fAlpha)
{
	uint8 a,r,g,b;
	GET_ARGB(nBaseColor,a,r,g,b);
	fAlpha = LTCLAMP(fAlpha,0.0f,1.0f);
	a = (uint8)(fAlpha * (float)a);
	return SET_ARGB(a,r,g,b);
};
