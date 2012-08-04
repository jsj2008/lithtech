// ----------------------------------------------------------------------- //
//
// MODULE  : AIStimulusTypeEnums.h
//
// PURPOSE : Enums and string constants for stimulus types.
//
// CREATED : 5/23/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_STIMULUS_TYPE
	#undef ADD_STIMULUS_TYPE
	#undef SET_NUM_STIMULUS_TYPES
#endif
 
#if STIMULUS_TYPE_AS_ENUM
	#define ADD_STIMULUS_TYPE(label, val) kStim_##label## = (1 << val),
	#define SET_NUM_STIMULUS_TYPES(val) kStim_Count = val,
#elif STIMULUS_TYPE_AS_STRING
	#define ADD_STIMULUS_TYPE(label, val) #label,
	#define SET_NUM_STIMULUS_TYPES(val)
#else
	#error ! To use this include file, first define either STIMULUS_TYPE_AS_ENUM or STIMULUS_TYPE_AS_STRING, to include the stimulus types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new bitflag enum, just add a ADD_STIMULUS_TYPE(x, val) 
// where x is the name of the enum without the "kStim_" prefix, and val is
// the bit position index.
// Set SET_NUM_STIMULUS_TYPES(val), where val is the total number of enums.
// --------------------------------------------------------------------------

ADD_STIMULUS_TYPE(EnemyVisible,					0)	// kStim_EnemyVisible
ADD_STIMULUS_TYPE(EnemyLeanVisible,				1)	// kStim_EnemyLeanVisible
ADD_STIMULUS_TYPE(EnemyFootprintVisible,		2)	// kStim_EnemyFootprintVisible
ADD_STIMULUS_TYPE(EnemyWeaponFireSound,			3)	// kStim_EnemyWeaponFireSound
ADD_STIMULUS_TYPE(EnemyWeaponReloadSound,		4)	// kStim_EnemyWeaponReloadSound
ADD_STIMULUS_TYPE(EnemyWeaponImpactSound,		5)	// kStim_EnemyWeaponImpactSound
ADD_STIMULUS_TYPE(EnemyWeaponImpactVisible,		6)	// kStim_EnemyWeaponImpactVisible
ADD_STIMULUS_TYPE(EnemyFootstepSound,			7)	// kStim_EnemyFootstepSound
ADD_STIMULUS_TYPE(EnemyAlarmSound,				8)	// kStim_EnemyAlarmSound
ADD_STIMULUS_TYPE(EnemyDisturbanceSound,		9)	// kStim_EnemyDisturbanceSound
ADD_STIMULUS_TYPE(EnemyDisturbanceVisible,		10)	// kStim_EnemyDisturbanceVisible
ADD_STIMULUS_TYPE(EnemyLightDisturbanceVisible,	11)	// kStim_EnemyLightDisturbanceVisible
ADD_STIMULUS_TYPE(EnemyDangerVisible,			12)	// kStim_EnemyDangerVisible
ADD_STIMULUS_TYPE(AllyDeathVisible,				13)	// kStim_AllyDeathVisible
ADD_STIMULUS_TYPE(AllyDeathSound,				14)	// kStim_AllyDeathSound
ADD_STIMULUS_TYPE(AllyPainSound,				15)	// kStim_AllyPainSound
ADD_STIMULUS_TYPE(AllyWeaponFireSound,			16)	// kStim_AllyWeaponFireSound
ADD_STIMULUS_TYPE(AllyDisturbanceVisible,		17)	// kStim_AllyDisturbanceVisible
ADD_STIMULUS_TYPE(AllyDisturbanceSound,			18)	// kStim_AllyDisturbanceSound
ADD_STIMULUS_TYPE(ProjectileVisible,			19)	// kStim_ProjectileVisible
ADD_STIMULUS_TYPE(AllyDistressVisible,			20)	// kStim_AllyDistressVisible
ADD_STIMULUS_TYPE(AllySpecialDamageVisible,		21)	// kStim_AllySpecialDamageVisible
ADD_STIMULUS_TYPE(UndeterminedVisible,			22)	// kStim_UndeterminedVisible

ADD_STIMULUS_TYPE(ProhibitedProjectileVisible,		23)	// kStim_ProhibitedProjectileVisible
ADD_STIMULUS_TYPE(ProhibitedWeaponImpactVisible,	24)	// kStim_ProhibitedWeaponImpactVisible
ADD_STIMULUS_TYPE(ProhibitedWeaponImpactSound,		25)	// kStim_ProhibitedWeaponImpactSound
ADD_STIMULUS_TYPE(ProhibitedWeaponFireSound,		26)	// kStim_ProhibitedWeaponFireSound
ADD_STIMULUS_TYPE(ProhibitedWeaponFireVisible,		27)	// kStim_ProhibitedWeaponFireVisible
ADD_STIMULUS_TYPE(CatchableProjectileVisible,		28)	// kStim_CatchableProjectileVisible

ADD_STIMULUS_TYPE(EnemyCoinSound,				29)	// kStim_EnemyCoinSound

SET_NUM_STIMULUS_TYPES(30)
