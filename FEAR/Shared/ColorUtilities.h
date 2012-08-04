// ----------------------------------------------------------------------- //
//
// MODULE  : ColorUtilities.h
//
// PURPOSE : Define some common color functions
//
// CREATED : 09/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COLORUTILITIES_H__
#define __COLORUTILITIES_H__


#define SET_ARGB(a,r,g,b) (((uint32)(a) << 24) | (uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b))
#define GET_A(val) (((val) >> 24) & 0xFF)
#define SET_A(val,a) ( (uint32(a) << 24) | ( uint32(val) & 0x00FFFFFF ) )
#define GET_ARGB(val, a, r, g, b) \
{\
	(a) = GET_A(val);\
	(r) = GETR(val);\
	(g) = GETG(val);\
	(b) = GETB(val);\
}

const uint32	argbWhite		= 0xFFFFFFFF;
const uint32	argbGray		= 0xFF606060;
const uint32	argbBlack		= 0xFF000000;
const uint32	argbTransBlack	= 0x00000000;

uint32 FadeARGB(uint32 nBaseColor, float fAlpha);


#endif  // __COLORUTILITIES_H__





