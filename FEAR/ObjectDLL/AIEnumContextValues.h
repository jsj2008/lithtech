// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumContextValues.h
//
// PURPOSE : Values for AI Contexts.
//
// CREATED : 08/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_CONTEXT
#undef ADD_CONTEXT
#endif

#if CONTEXT_AS_ENUM
#define ADD_CONTEXT(label) kContext_##label,
#elif CONTEXT_AS_STRING
#define ADD_CONTEXT(label) #label,
#else
#error	To use this include file, first define either CONTEXT_AS_ENUM or CONTEXT_AS_STRING, to include the contexts as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_CONTEXT(x) where 
// x is the name of the enum without the "kContext_" prefix.
// --------------------------------------------------------------------------

ADD_CONTEXT(Advancing)		// kContext_Advancing
ADD_CONTEXT(Blitzing)		// kContext_Blitzing
ADD_CONTEXT(Goto)			// kContext_Goto
ADD_CONTEXT(Investigating)	// kContext_Investigating
ADD_CONTEXT(Menacing)		// kContext_Menacing
ADD_CONTEXT(None)			// kContext_None
ADD_CONTEXT(Patrolling)		// kContext_Patrolling
ADD_CONTEXT(Retreating)		// kContext_Retreating
ADD_CONTEXT(Searching)		// kContext_Searching

