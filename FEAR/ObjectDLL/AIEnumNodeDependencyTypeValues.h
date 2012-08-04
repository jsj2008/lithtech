// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNodeDependencyTypeValues.h
//
// PURPOSE : Values for Node Dependency types.
//
// CREATED : 08/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_DEPENDENCY_TYPE
#undef ADD_DEPENDENCY_TYPE
#endif

#if DEPENDENCY_TYPE_AS_ENUM
#define ADD_DEPENDENCY_TYPE(label) kDependency_##label,
#elif DEPENDENCY_TYPE_AS_STRING
#define ADD_DEPENDENCY_TYPE(label) #label,
#else
#error	To use this include file, first define either DEPENDENCY_TYPE_AS_ENUM or DEPENDENCY_TYPE_AS_STRING, to include the dependencies as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_DEPENDENCY_TYPE(x) where 
// x is the name of the enum without the "kDependency_" prefix.
// --------------------------------------------------------------------------

ADD_DEPENDENCY_TYPE(Destination)			// kDependency_Destination
ADD_DEPENDENCY_TYPE(Occupied)				// kDependency_Occupied
