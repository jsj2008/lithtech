// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumAwarenessModifierValues.h
//
// PURPOSE : Values for AwarenessModifiers.
//
// CREATED : 08/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AWARENESSMOD
#undef ADD_AWARENESSMOD
#endif

#if AWARENESSMOD_AS_ENUM
#define ADD_AWARENESSMOD(label) kAwarenessMod_##label,
#elif AWARENESSMOD_AS_STRING
#define ADD_AWARENESSMOD(label) #label,
#else
#error	To use this include file, first define either AWARENESSMOD_AS_ENUM or AWARENESSMOD_AS_STRING, to include the modifiers as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AWARENESSMOD(x) where 
// x is the name of the enum without the "kAwarenessMod_" prefix.
// --------------------------------------------------------------------------

ADD_AWARENESSMOD(ImmediateThreat)	// kAwarenessMod_ImmediateThreat
ADD_AWARENESSMOD(Injured)		// kAwarenessMod_Injured
