// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseTypeEnums.h
//
// PURPOSE : Enums and string constants for sense types.
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_SENSE_TYPE
	#undef ADD_SENSE_TYPE
	#undef SET_NUM_SENSE_TYPES
#endif
 
#if SENSE_TYPE_AS_ENUM
	#define ADD_SENSE_TYPE(label, val) kSense_##label## = (1 << val),
	#define SET_NUM_SENSE_TYPES(val) kSense_Count = val,
#elif SENSE_TYPE_AS_STRING
	#define ADD_SENSE_TYPE(label, val) #label,
	#define SET_NUM_SENSE_TYPES(val)
#else
	#error ! To use this include file, first define either SENSE_TYPE_AS_ENUM or SENSE_TYPE_AS_STRING, to include the sense types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new bitflag enum, just add a ADD_SENSE_TYPE(x, val) 
// where x is the name of the enum without the "kSense_" prefix, and val is
// the bit position index.
// Set SET_NUM_SENSE_TYPES(val), where val is the total number of enums.
// --------------------------------------------------------------------------

ADD_SENSE_TYPE(SeeEnemy,				0)		// kSense_SeeEnemy
ADD_SENSE_TYPE(SeeEnemyLean,			1)		// kSense_SeeEnemyLean
ADD_SENSE_TYPE(SeeEnemyWeaponImpact,	2)		// kSense_SeeEnemyWeaponImpact
ADD_SENSE_TYPE(SeeEnemyDanger,			3)		// kSense_SeeEnemyDanger
ADD_SENSE_TYPE(SeeEnemyDisturbance,		4)		// kSense_SeeEnemyDisturbance
ADD_SENSE_TYPE(SeeEnemyLightDisturbance,5)		// kSense_SeeEnemyLightDisturbance
ADD_SENSE_TYPE(SeeEnemyFootprint,		6)		// kSense_SeeEnemyFootprint
ADD_SENSE_TYPE(SeeAllyDeath,			7)		// kSense_SeeAllyDeath
ADD_SENSE_TYPE(SeeAllyDisturbance,		8)		// kSense_SeeAllyDisturbance
ADD_SENSE_TYPE(HearEnemyWeaponFire,		9)		// kSense_HearEnemyWeaponFire
ADD_SENSE_TYPE(HearEnemyWeaponImpact,	10)		// kSense_HearEnemyWeaponImpact
ADD_SENSE_TYPE(HearEnemyFootstep,		11)		// kSense_HearEnemyFootstep
ADD_SENSE_TYPE(HearEnemyAlarm,			12)		// kSense_HearEnemyAlarm
ADD_SENSE_TYPE(HearEnemyDisturbance,	13)		// kSense_HearEnemyDisturbance
ADD_SENSE_TYPE(HearAllyDeath,			14)		// kSense_HearAllyDeath
ADD_SENSE_TYPE(HearAllyPain,			15)		// kSense_HearAllyPain
ADD_SENSE_TYPE(HearAllyWeaponFire,		16)		// kSense_HearAllyWeaponFire
ADD_SENSE_TYPE(HearAllyDisturbance,		17)		// kSense_HearAllyDisturbance
ADD_SENSE_TYPE(SeeDangerousProjectile,	18)		// kSense_SeeDangerousProjectile
ADD_SENSE_TYPE(SeeCatchableProjectile,	19)		// kSense_SeeCatchableProjectile
ADD_SENSE_TYPE(SeeAllyDistress,			20)		// kSense_SeeAllyDistress
ADD_SENSE_TYPE(SeeAllySpecialDamage,	21)		// kSense_SeeAllySpecialDamage
ADD_SENSE_TYPE(SeeUndetermined,			22)		// kSense_SeeUndetermined

ADD_SENSE_TYPE(SeeInappropriateBehavior, 23)	// kSense_SeeInappropriateBehavior
ADD_SENSE_TYPE(HearInappropriateBehavior, 24)	// kSense_HearInappropriateBehavior

SET_NUM_SENSE_TYPES(25)
