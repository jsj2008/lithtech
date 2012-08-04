// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumActionAbilityTypes.h
//
// PURPOSE : Enums and string constants for actions.
//
// CREATED : 2/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ACTIONABILITY_TYPE
	#undef ADD_ACTIONABILITY_TYPE
#endif
 
#if ACTIONABILITY_TYPE_AS_ENUM
	#define ADD_ACTIONABILITY_TYPE(label) kActionAbility_##label,
#elif ACTIONABILITY_TYPE_AS_STRING
	#define ADD_ACTIONABILITY_TYPE(label) #label,
#elif ACTIONABILITY_TYPE_AS_SWITCH
	#define ADD_ACTIONABILITY_TYPE(label) case kActionAbility_##label: extern CAIClassAbstract* AIFactoryCreateCAIAction##label(); return (CAIActionAbstract*)AIFactoryCreateCAIAction##label();
#else
	#error	To use this include file, first define either ACTIONABILITY_TYPE_AS_ENUM or ACTIONABILITY_TYPE_AS_STRING, to include the actions as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ACTIONABILITY_TYPE(x) where 
// x is the name of the enum without the "kActionAbility_" prefix.
// --------------------------------------------------------------------------

ADD_ACTIONABILITY_TYPE(JumpUp)				// kActionAbility_JumpUp
ADD_ACTIONABILITY_TYPE(UnblockDoor)			// kActionAbility_UnblockDoor
