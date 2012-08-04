// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumActivityTypes.h
//
// PURPOSE : Enums and string constants for activities.
//
// CREATED : 6/12/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ACTIVITY_TYPE
	#undef ADD_ACTIVITY_TYPE
#endif
 
#if ACTIVITY_TYPE_AS_ENUM
	#define ADD_ACTIVITY_TYPE(label) kActivity_##label,
#elif ACTIVITY_TYPE_AS_STRING
	#define ADD_ACTIVITY_TYPE(label) #label,
#elif ACTIVITY_TYPE_AS_SWITCH
	#define ADD_ACTIVITY_TYPE(label) case kActivity_##label: extern CAIClassAbstract* AIFactoryCreateCAIActivity##label(); return (CAIActivityAbstract*)AIFactoryCreateCAIActivity##label();
#else
	#error	To use this include file, first define either ACTIVITY_TYPE_AS_ENUM or ACTIVITY_TYPE_AS_STRING, to include the activity as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ACTIVITY_TYPE(x) where 
// x is the name of the enum without the "kActivity_" prefix.
// --------------------------------------------------------------------------

ADD_ACTIVITY_TYPE(AdvanceCover)				// kActivity_AdvanceCover
ADD_ACTIVITY_TYPE(GetToCover)				// kActivity_GetToCover
ADD_ACTIVITY_TYPE(ExchangeWeapons)			// kActivity_ExchangeWeapons
ADD_ACTIVITY_TYPE(OrderlyAdvance)			// kActivity_OrderlyAdvance
ADD_ACTIVITY_TYPE(Search)					// kActivity_Search
