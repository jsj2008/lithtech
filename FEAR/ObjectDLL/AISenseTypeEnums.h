// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseTypeEnums.h
//
// PURPOSE : Enums and string constants for sense types.
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_SENSE_TYPE
	#undef ADD_SENSE_TYPE
#endif
 
#if SENSE_TYPE_AS_FLAG
	#define ADD_SENSE_TYPE(label, val) kSense_##label = (1 << val),
#elif SENSE_TYPE_AS_STRING
	#define ADD_SENSE_TYPE(label, val) #label,
#elif SENSE_TYPE_AS_ENUM
	#define ADD_SENSE_TYPE(label, val) kSense_DoNotUseThisValueForAutoCountOnly_##label,
#else
	#error ! To use this include file, first define either SENSE_TYPE_AS_ENUM or SENSE_TYPE_AS_STRING, to include the sense types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new bitflag enum, just add a ADD_SENSE_TYPE(x, val) 
// where x is the name of the enum without the "kSense_" prefix, and val is
// the bit position index.
// Set SET_NUM_SENSE_TYPES(val), where val is the total number of enums.
// --------------------------------------------------------------------------

ADD_SENSE_TYPE(Visual,					0)		// kSense_Visual
ADD_SENSE_TYPE(AudioMinor,				1)		// kSense_AudioMinor
ADD_SENSE_TYPE(AudioMajor,				2)		// kSense_AudioMajor
ADD_SENSE_TYPE(Tactile,					3)		// kSense_Tactile
