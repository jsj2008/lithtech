// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumStimulusTypeValues.h
//
// PURPOSE : Values for AI stimulus types.
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

#if STIMULUS_TYPE_AS_FLAG
	#define ADD_STIMULUS_TYPE(label, val) kStim_##label = (1 << val),
#elif STIMULUS_TYPE_AS_STRING
	#define ADD_STIMULUS_TYPE(label, val) #label,
#elif STIMULUS_TYPE_AS_ENUM
	#define ADD_STIMULUS_TYPE(label, flag) kStim_DoNotUseThisValueForAutoCountOnly_##label,
#else
	#error ! To use this include file, first define either STIMULUS_TYPE_AS_ENUM or STIMULUS_TYPE_AS_STRING, to include the stimulus types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new bitflag enum, just add a ADD_STIMULUS_TYPE(x, val) 
// where x is the name of the enum without the "kStim_" prefix, and val is
// the bit position index.
// Set SET_NUM_STIMULUS_TYPES(val), where val is the total number of enums.
// --------------------------------------------------------------------------

ADD_STIMULUS_TYPE(CharacterVisible,			0)	// kStim_CharacterVisible
ADD_STIMULUS_TYPE(LeanVisible,				1)	// kStim_LeanVisible
ADD_STIMULUS_TYPE(FootprintVisible,			2)	// kStim_FootprintVisible
ADD_STIMULUS_TYPE(WeaponFireSound,			3)	// kStim_WeaponFireSound
ADD_STIMULUS_TYPE(WeaponReloadSound,		4)	// kStim_WeaponReloadSound
ADD_STIMULUS_TYPE(WeaponImpactSound,		5)	// kStim_WeaponImpactSound
ADD_STIMULUS_TYPE(WeaponImpactVisible,		6)	// kStim_WeaponImpactVisible
ADD_STIMULUS_TYPE(FootstepSound,			7)	// kStim_FootstepSound
ADD_STIMULUS_TYPE(AlarmSound,				8)	// kStim_AlarmSound
ADD_STIMULUS_TYPE(DisturbanceSound,			9)	// kStim_DisturbanceSound
ADD_STIMULUS_TYPE(DisturbanceVisible,		10)	// kStim_DisturbanceVisible
ADD_STIMULUS_TYPE(LightVisible,				11)	// kStim_LightVisible
ADD_STIMULUS_TYPE(DangerVisible,			12)	// kStim_DangerVisible
ADD_STIMULUS_TYPE(DeathVisible,				13)	// kStim_DeathVisible
ADD_STIMULUS_TYPE(DeathSound,				14)	// kStim_DeathSound
ADD_STIMULUS_TYPE(PainSound,				15)	// kStim_PainSound
ADD_STIMULUS_TYPE(ProjectileVisible,		16)	// kStim_ProjectileVisible
ADD_STIMULUS_TYPE(DistressVisible,			17)	// kStim_DistressVisible
ADD_STIMULUS_TYPE(SpecialDamageVisible,		18)	// kStim_SpecialDamageVisible
ADD_STIMULUS_TYPE(FlashlightBeamVisible,	19)	// kStim_FlashlightBeamVisible
ADD_STIMULUS_TYPE(DamageBullet,				20)	// kStim_DamageBullet
ADD_STIMULUS_TYPE(DamageExplode,			21)	// kStim_DamageExplode
ADD_STIMULUS_TYPE(DamageMelee,				22)	// kStim_DamageMelee
ADD_STIMULUS_TYPE(DamageStun,				23)	// kStim_DamageStun
ADD_STIMULUS_TYPE(CombatOpportunity,		24)	// kStim_CombatOpportunity
ADD_STIMULUS_TYPE(WeaponItem,				25)	// kStim_WeaponItem
ADD_STIMULUS_TYPE(Touched,					26)	// kStim_Touched
ADD_STIMULUS_TYPE(SquadCommunicationThreat,	27)	// kStim_SquadCommunicationThreat
ADD_STIMULUS_TYPE(SquadCommunicationDisturbance, 28)	// kStim_SquadCommunicationDisturbance
ADD_STIMULUS_TYPE(Shoved,					29)	// kStim_Shoved
