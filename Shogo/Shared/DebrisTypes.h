// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisTypes.h
//
// PURPOSE : Definition of debris types
//
// CREATED : 7/2/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_TYPES_H__
#define __DEBRIS_TYPES_H__

#include "clientheaders.h"
#include "SurfaceTypes.h"

enum DebrisType 
{
	DBT_FIRST = 0,
	DBT_GENERIC	= 0,	// Generic debris
	DBT_GENERIC_FLAT,	// Generic flat debris
	DBT_BOARDS,			// Wood boards
	DBT_BRANCHES_BIG,	// Big Tree/bush branches 
	DBT_BRANCHES_SMALL,	// Small Tree/bush branches
	DBT_WOODCHIPS,		// Small wood chips
	DBT_PLASTIC,		// Plastic debris
	DBT_GLASS_BIG,		// Big glass debris
	DBT_GLASS_SMALL,	// Small glass debris
	DBT_FEATHERS,		// Feathers (for furniture)
	DBT_STONE_BIG,		// Big stone debris
	DBT_STONE_SMALL,	// Small stone debris
	DBT_METAL_BIG,		// Big metal debris
	DBT_METAL_SMALL,	// Small metal debris
	DBT_CAR_PARTS,		// Car parts
	DBT_MECHA_PARTS,	// Mecha parts
	DBT_VEHICLE_PARTS,	// Vehicle parts
	DBT_HUMAN_PARTS,	// Human parts
	DBT_LAST
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSurfaceType
//
//	PURPOSE:	Return the surface type of the debris
//
// ----------------------------------------------------------------------- //

SurfaceType GetDebrisSurfaceType(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisType
//
//	PURPOSE:	Return the debris type of the surface
//
// ----------------------------------------------------------------------- //

DebrisType GetDebrisType(SurfaceType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetVectorDebrisType
//
//	PURPOSE:	Return the debris type of the surface (vector weapons)
//
// ----------------------------------------------------------------------- //

DebrisType GetVectorDebrisType(SurfaceType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsSmallDebris
//
//	PURPOSE:	Return  true if the debris type is small
//
// ----------------------------------------------------------------------- //

LTBOOL IsSmallDebris(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisModels
//
//	PURPOSE:	Return the number of debris models associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisModels(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisModel
//
//	PURPOSE:	Return the model associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisModel(DebrisType eType, LTVector & vScale, int nIndex=-1);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSkin
//
//	PURPOSE:	Return the skin associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisSkin(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisBounceSounds
//
//	PURPOSE:	Return the number of debris bounce sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisBounceSounds(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisBounceSound
//
//	PURPOSE:	Return the bounce sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisBounceSound(DebrisType eType, int nIndex=-1);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisExplodeSounds
//
//	PURPOSE:	Return the number of debris explode sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisExplodeSounds(DebrisType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisExplodeSound
//
//	PURPOSE:	Return the explode sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisExplodeSound(DebrisType eType, int nIndex=-1);

#endif // __DEBRIS_TYPES_H__