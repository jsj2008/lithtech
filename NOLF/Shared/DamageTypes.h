// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.h
//
// PURPOSE : Definition of damage types
//
// CREATED : 11/26/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_TYPES_H__
#define __DAMAGE_TYPES_H__

#include "ltbasedefs.h"

struct DTINFO;
extern DTINFO DTInfoArray[];
extern const int c_nDTInfoArraySize;

// IMPORTANT NOTE:  If you add a new DamageType, make sure you update
// the DTInfoArray in DamageTypes.cpp!!!

enum DamageType
{
	DT_INVALID=-1,			// Not a valid damage type
	DT_UNSPECIFIED=0,		// Unknown type (self-damage)
	DT_BLEEDING=1,			// (loss of blood)
	DT_BULLET=2,			// (bullets)
	DT_BURN=3,				// (fire, corrosives)
	DT_CHOKE=4,				// (water, hostile atmosphere)
	DT_CRUSH=5,				// (falling, crushing, collision)
	DT_ELECTROCUTE=6,		// (electricity)
	DT_EXPLODE=7,			// (explosions)
	DT_FREEZE=8,			// (freezing air or fluid)
	DT_POISON=9,			// (poison gas/dart)
	DT_ENDLESS_FALL=10,		// Falling and can never get up
	DT_SLEEPING=11,			// Sleeping
	DT_STUN=12,				// Stunned
	DT_MELEE=13,			// Melee

	// Special gadget damage types...

	DT_GADGET_CAMERA_DISABLER=21,	// Camera disabler gadget
	DT_GADGET_CODE_DECIPHERER=22,	// Code decipherer gadget
	DT_GADGET_POODLE=23,			// Poodle gadget
	DT_GADGET_LOCK_PICK=24,			// Lock pick gadget
	DT_GADGET_WELDER=25,			// Welder gadget
	DT_GADGET_LIGHTER=26,			// Lighter gadget
	DT_GADGET_CAMERA=27,			// Camera gadget
	DT_GADGET_MINE_DETECTOR=28,		// Mine detector gadget
	DT_GADGET_INFRA_RED=29,			// Infra red gadget
	DT_GADGET_ZIPCORD=30,			// Zipcord gadget
	DT_GADGET_DECAYPOWDER=31		// Decay powder gadget
};

struct DTINFO
{
    DTINFO(DamageType e=DT_INVALID, uint32 d=0, 
		const char* p="INVALID", LTBOOL bJar=LTFALSE, LTBOOL bGad=LTFALSE,
		LTBOOL bAcc=LTFALSE)
	{
		eType		= e;
		dwFlag		= d;
		pName		= p;
		bJarCamera	= bJar;
		bGadget		= bGad;
		bAccuracy	= bAcc;
	}

	DamageType	eType;
    uint32      dwFlag;
	const char*	pName;
	LTBOOL		bJarCamera;
	LTBOOL		bGadget;
	LTBOOL		bAccuracy;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline LTBOOL SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj) return LTFALSE;

	return (hObj != (HOBJECT)pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsJarCameraType()
//
//	PURPOSE:	Does this damage type cause the camera to be "jarred"
//
// ----------------------------------------------------------------------- //

LTBOOL IsJarCameraType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGadgetType()
//
//	PURPOSE:	Does this damage type a gadget damage type?
//
// ----------------------------------------------------------------------- //

LTBOOL IsGadgetType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAccuracyType()
//
//	PURPOSE:	Does this damage type count towards player accuracy
//
// ----------------------------------------------------------------------- //

LTBOOL IsAccuracyType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

uint32 DamageTypeToFlag(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToString(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StringToDamageType()
//
//	PURPOSE:	Convert the string to its damage type equivillent
//
// ----------------------------------------------------------------------- //

DamageType StringToDamageType(const char* pDTName);


#endif // __DAMAGE_TYPES_H__