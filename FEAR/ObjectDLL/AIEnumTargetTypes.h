// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumTargetTypes.h
//
// PURPOSE : Enums and string constants for targets.
//
// CREATED : 2/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_TARGET_TYPE
	#undef ADD_TARGET_CLASS
	#undef ADD_TARGET_TYPE
#endif
 
#if TARGET_TYPE_AS_ENUM
	#define ADD_TARGET_CLASS(label) kTargetSelect_##label,
	#define ADD_TARGET_TYPE(label) kTargetSelect_##label,
#elif TARGET_TYPE_AS_STRING
	#define ADD_TARGET_CLASS(label) #label,
	#define ADD_TARGET_TYPE(label) #label,
#elif TARGET_TYPE_AS_SWITCH
	#define ADD_TARGET_CLASS(label) case kTargetSelect_##label: extern CAIClassAbstract* AIFactoryCreateCAITargetSelect##label(); return (CAITargetSelectAbstract*)AIFactoryCreateCAITargetSelect##label();
	#define ADD_TARGET_TYPE(label)
#else
	#error	To use this include file, first define either TARGET_TYPE_AS_ENUM or TARGET_TYPE_AS_STRING, to include the targets as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_TARGET_TYPE(x) where 
// x is the name of the enum without the "kTargetSelect_" prefix.
// --------------------------------------------------------------------------

// Classes.

ADD_TARGET_CLASS(AllyDisturbance)			// kTargetSelect_AllyDisturbance
ADD_TARGET_CLASS(Berserker)					// kTargetSelect_Berserker
ADD_TARGET_CLASS(Character)					// kTargetSelect_Character
ADD_TARGET_CLASS(CharacterOnlyOne)			// kTargetSelect_CharacterOnlyOne
ADD_TARGET_CLASS(CharacterSquad)			// kTargetSelect_CharacterSquad
ADD_TARGET_CLASS(CharacterUrgent)			// kTargetSelect_CharacterUrgent
ADD_TARGET_CLASS(CharacterUrgentSpatial)	// kTargetSelect_CharacterUrgentSpatial
ADD_TARGET_CLASS(CombatOpportunity)			// kTargetSelect_CombatOpportunity
ADD_TARGET_CLASS(Damager)					// kTargetSelect_Damager
ADD_TARGET_CLASS(Disturbance)				// kTargetSelect_Disturbance
ADD_TARGET_CLASS(DisturbanceBeyondGuard)	// kTargetSelect_DisturbanceBeyondGuard
ADD_TARGET_CLASS(DisturbanceSource)			// kTargetSelect_DisturbanceSource
ADD_TARGET_CLASS(DisturbanceSquad)			// kTargetSelect_DisturbanceSquad
ADD_TARGET_CLASS(DisturbanceUrgent)			// kTargetSelect_DisturbanceUrgent
ADD_TARGET_CLASS(Interest)					// kTargetSelect_Interest
ADD_TARGET_CLASS(Follower)					// kTargetSelect_Follower
ADD_TARGET_CLASS(Leader)					// kTargetSelect_Leader
ADD_TARGET_CLASS(PlayerTurret)				// kTargetSelect_PlayerTurret
ADD_TARGET_CLASS(Scripted)					// kTargetSelect_Scripted
ADD_TARGET_CLASS(Traitor)					// kTargetSelect_Traitor
ADD_TARGET_CLASS(WeaponItem)				// kTargetSelect_WeaponItem

// Types.

