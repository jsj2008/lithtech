// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponClassTypeValues.h
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

#ifdef ADD_AIWEAPONCLASS_TYPE
#undef ADD_AIWEAPONCLASS_TYPE
#endif

#if AIWEAPONCLASS_TYPE_AS_ENUM
#define ADD_AIWEAPONCLASS_TYPE(label) kAIWeaponClassType_##label,
#elif AIWEAPONCLASS_TYPE_AS_STRING
#define ADD_AIWEAPONCLASS_TYPE(label) #label,
#elif AIWEAPONCLASS_TYPE_AS_SWITCH
#define ADD_AIWEAPONCLASS_TYPE(label) case kAIWeaponClassType_##label: extern CAIClassAbstract* AIFactoryCreateCAIWeapon##label(); return (CAIWeaponAbstract*)AIFactoryCreateCAIWeapon##label();
#else
#error	To use this include file, first define either AIWEAPONCLASS_TYPE_AS_ENUM or AIWEAPONCLASS_TYPE_AS_STRING, to include the AIWEAPONCLASSs as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AIWEAPONCLASS_TYPE(x) where 
// x is the name of the enum without the "kAIWeaponClass_" prefix.
// --------------------------------------------------------------------------

ADD_AIWEAPONCLASS_TYPE(Ranged)			// kAIWeaponClassType_Ranged
ADD_AIWEAPONCLASS_TYPE(Melee)			// kAIWeaponClassType_Melee
ADD_AIWEAPONCLASS_TYPE(Thrown)			// kAIWeaponClassType_Thrown

