// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceDefs.h
//
// PURPOSE : Definition of surface related stuff
//
// CREATED : 12/09/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_DEFS_H__
#define __SURFACE_DEFS_H__

// Special pre-defined surface types...

enum SurfaceType
{
	ST_UNKNOWN				= 0,	// Unknown value
	ST_AIR					= 1,	// Not a surface, but not the sky either
	ST_FLESH				= 2,	// Human flesh
	ST_ARMOR				= 3,	// Armored Human
	ST_SKY					= 110,	// Sky poly
	ST_LADDER				= 200,	// Ladder volume brush
	ST_LIQUID				= 201,	// Liquid volume brush
	ST_INVISIBLE			= 202,	// Invisible surface
};


#endif // __SURFACE_DEFS_H__
