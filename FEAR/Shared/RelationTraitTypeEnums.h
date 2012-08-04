// ----------------------------------------------------------------------- //
//
// MODULE  : TraitTypeEnums.h
//
// PURPOSE : Enums and string constants for trait types.
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_TRAIT_TYPE
	#undef ADD_TRAIT_TYPE
	#undef SET_NUM_TRAIT_TYPES
#endif
 
#if TRAIT_TYPE_AS_ENUM
	#define ADD_TRAIT_TYPE(label, val) kTrait_##label = val,
	#define SET_NUM_TRAIT_TYPES(val) kTrait_Count = val,
#elif TRAIT_TYPE_AS_STRING
	#define ADD_TRAIT_TYPE(label, val) #label,
	#define SET_NUM_TRAIT_TYPES(val)
#else
	#error ! To use this include file, first define either TRAIT_TYPE_AS_ENUM or TRAIT_TYPE_AS_STRING, to include the trait types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new bitflag enum, just add a ADD_TRAIT_TYPE(x, val) 
// where x is the name of the enum without the "kTrait_" prefix, and val is
// the bit position index.
// Set SET_NUM_TRAIT_TYPES(val), where val is the total number of enums.
// --------------------------------------------------------------------------

ADD_TRAIT_TYPE(Name,			0)		// kTrait_Name
ADD_TRAIT_TYPE(Class,			1)		// ktrait_Class
ADD_TRAIT_TYPE(Alignment,		2)		// kTrait_Alignment

SET_NUM_TRAIT_TYPES(3)
