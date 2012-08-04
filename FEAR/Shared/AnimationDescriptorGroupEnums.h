// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationDescriptorGroupEnums.h
//
// PURPOSE : Enums and string constants for animation descriptor groups.
//
// CREATED : 10/15/03
//
// (c) 2003-2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ANIM_DESC_GROUP
	#undef ADD_ANIM_DESC_GROUP
#endif

#if ANIM_DESC_GROUP_AS_ENUM
	#define ADD_ANIM_DESC_GROUP(label) kADG_##label,
#elif ANIM_DESC_GROUP_AS_STRING
	#define ADD_ANIM_DESC_GROUP(label) #label,
#else
	#error ! To use this include file, first define either ANIM_DESC_GROUP_AS_ENUM or ANIM_DESC_GROUP_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_DESC_GROUP(x) 
// where x is the name of the enum without the "kADG_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_DESC_GROUP(Movement)		// kADG_Movement
ADD_ANIM_DESC_GROUP(Tracker)		// kADG_Tracker
ADD_ANIM_DESC_GROUP(Synchronize)	// kADG_Synchronize
ADD_ANIM_DESC_GROUP(Camera)			// kADG_Camera
ADD_ANIM_DESC_GROUP(Input)			// kADG_Input
ADD_ANIM_DESC_GROUP(Condition)		// kADG_Condition
