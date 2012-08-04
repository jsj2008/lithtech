// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumAIWeaponTypes.h
//
// PURPOSE : Enums and string constants for AI weapon types.
//
// CREATED : 06/15/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_AI_WEAPON_TYPES_H__
#define __AI_ENUM_AI_WEAPON_TYPES_H__

// Macros

#define AIWEAP_CHECK_HOLSTER	true

// ----------------------------------------------------------------------- //

enum ENUM_RangeStatus
{
	kRangeStatus_Invalid,
	kRangeStatus_Ok,
	kRangeStatus_TooClose,
	kRangeStatus_TooFar,
};

// ----------------------------------------------------------------------- //

enum ENUM_AIWeaponType
{
#define AIWEAPON_TYPE_AS_ENUM 1
#include "AIEnumAIWeaponTypeValues.h"
#undef AIWEAPON_TYPE_AS_ENUM

	kAIWeaponType_Count
};

//
// STRINGS: const strings for AIWeapon types.
//
static const char* s_aszAIWeaponTypeNames[] =
{
#define AIWEAPON_TYPE_AS_STRING 1
#include "AIEnumAIWeaponTypeValues.h"
#undef AIWEAPON_TYPE_AS_STRING
};

//
// ENUM: AIWeapon Class identifiers, used in factory creation.
//
enum EnumAIWeaponClassType
{
	kAIWeaponClassType_InvalidType = -1,

#define AIWEAPONCLASS_TYPE_AS_ENUM 1
#include "AIEnumAIWeaponClassTypeValues.h"
#undef AIWEAPONCLASS_TYPE_AS_ENUM

	kAIWeaponClassType_Count
};

//
// STRINGS: const strings for AIWeapon types.
//
static const char* s_aszAIWeaponClassNames[] =
{
#define AIWEAPONCLASS_TYPE_AS_STRING 1
#include "AIEnumAIWeaponClassTypeValues.h"
#undef AIWEAPONCLASS_TYPE_AS_STRING
};

#endif // __AI_ENUM_AI_WEAPON_TYPES_H__
