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
#include "GameButeMgr.h"
#include <bitset>

#define DTMGR_DEFAULT_FILE		"Attributes\\DamageTypes.txt"

typedef uint64 DamageFlags;

struct DTINFO;
extern DTINFO DTInfoArray[];

// IMPORTANT NOTE:  If you add a new DamageType, make sure you update
// the DTInfoArray in DamageTypes.cpp!!!

enum DamageType
{
	DT_INVALID = -1,
#define INCLUDE_AS_ENUM
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_ENUM
	kNumDamageTypes
};

static const char* pDTNames[kNumDamageTypes] =
{
#define INCLUDE_AS_STRING
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_STRING
};

struct DTINFO
{
    DTINFO(DamageType type, const char* name, const char* pszAltName)
	{
		eType		= type;
		nDamageFlag	= 0;
		pName		= name;
		pAltName	= pszAltName;
		bJarCamera	= LTFALSE;
		bGadget		= LTFALSE;
		bAccuracy	= LTFALSE;
		fSlowMove	= 0.0f;
		bScoreTag	= LTFALSE;
		bGrief		= LTFALSE;
		bHurtAnim	= false;
	}

	DamageType	eType;
	DamageFlags	nDamageFlag;
	const char*	pName;

	// Used for backwards compatibility with unprocessed levels.
	const char* pAltName; 
	LTBOOL		bJarCamera;
	LTBOOL		bGadget;
	LTBOOL		bAccuracy;
	LTFLOAT		fSlowMove;
	LTBOOL		bScoreTag;
	LTBOOL		bGrief;
	bool		bHurtAnim;
	StringArray saPlayerDamageSounds;
};


class CDTButeMgr : public CGameButeMgr
{
public :

	CDTButeMgr();
	~CDTButeMgr();

    LTBOOL      Init(const char* szAttributeFile=DTMGR_DEFAULT_FILE);
	void		Term();

};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline bool SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj) return false;

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
//	ROUTINE:	IsScoreTagType()
//
//	PURPOSE:	Does this damage type score a tag in deathmatch
//
// ----------------------------------------------------------------------- //

LTBOOL IsScoreTagType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGriefType()
//
//	PURPOSE:	Should the player be protected against multiple instances of 
//					this type in multiplayer.
//
// ----------------------------------------------------------------------- //

LTBOOL IsGriefType(DamageType eType);


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
//	ROUTINE:	SlowMovementDuration()
//
//	PURPOSE:	For how long does this damage type slow movement
//
// ----------------------------------------------------------------------- //

LTFLOAT SlowMovementDuration(DamageType eType);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageFlagToType()
//
//	PURPOSE:	Convert the damage flag to its type equivillent
//
// ----------------------------------------------------------------------- //

DamageType DamageFlagToType(DamageFlags nDamageFlag);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

DamageFlags DamageTypeToFlag(DamageType eType);

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDTINFO()
//
//	PURPOSE:	Gets the damagetype structure.
//
// ----------------------------------------------------------------------- //

DTINFO const* GetDTINFO( DamageType eType );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDamageFlagsWithHurtAnim
//
//	PURPOSE:	Get the list of damageflags that have the hurt flag set.
//
// ----------------------------------------------------------------------- //

DamageFlags GetDamageFlagsWithHurtAnim( );


#endif // __DAMAGE_TYPES_H__