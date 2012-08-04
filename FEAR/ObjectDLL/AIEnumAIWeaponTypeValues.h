// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponTypeValues.h
//
// PURPOSE : Values for AI weapon types.
//
// CREATED : 06/15/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AIWEAPON_TYPE
#undef ADD_AIWEAPON_TYPE
#endif

#if AIWEAPON_TYPE_AS_ENUM
#define ADD_AIWEAPON_TYPE(label) kAIWeaponType_##label,
#elif AIWEAPON_TYPE_AS_STRING
#define ADD_AIWEAPON_TYPE(label) #label,
#else
#error	To use this include file, first define either AIWEAPON_TYPE_AS_ENUM or AIWEAPON_TYPE_AS_STRING, to include the AIWEAPONs as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AIWEAPON_TYPE(x) where 
// x is the name of the enum without the "kAIWeapon_" prefix.
// --------------------------------------------------------------------------

ADD_AIWEAPON_TYPE(None)				// kAIWeaponType_None
ADD_AIWEAPON_TYPE(Ranged)			// kAIWeaponType_Ranged
ADD_AIWEAPON_TYPE(Melee)			// kAIWeaponType_Melee
ADD_AIWEAPON_TYPE(Thrown)			// kAIWeaponType_Thrown
ADD_AIWEAPON_TYPE(Kick)				// kAIWeaponType_Kick
ADD_AIWEAPON_TYPE(MeleeDual)		// kAIWeaponType_MeleeDual

