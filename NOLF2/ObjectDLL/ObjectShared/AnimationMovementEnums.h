// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationMovementEnums.h
//
// PURPOSE : Enums and string constants for animation movement.
//
// CREATED : 9/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ANIM_MOVEMENT
	#undef ADD_ANIM_MOVEMENT
#endif
 
#if ANIM_MOVEMENT_AS_ENUM
	#define ADD_ANIM_MOVEMENT(label) kAM_##label##,
#elif ANIM_MOVEMENT_AS_STRING
	#define ADD_ANIM_MOVEMENT(label) #label,
#else
	#error ! To use this include file, first define either ANIM_MOVEMENT_AS_ENUM or ANIM_MOVEMENT_AS_STRING, to include the movement types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_MOVEMENT(x) 
// where x is the name of the enum without the "kAM_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_MOVEMENT(None)			// kAM_None
ADD_ANIM_MOVEMENT(Set)			// kAM_Set
ADD_ANIM_MOVEMENT(Walk)			// kAM_Walk
ADD_ANIM_MOVEMENT(Run)			// kAM_Run
ADD_ANIM_MOVEMENT(JumpUp)		// kAM_JumpUp
ADD_ANIM_MOVEMENT(JumpOver)		// kAM_JumpOver
ADD_ANIM_MOVEMENT(Fall)			// kAM_Fall
ADD_ANIM_MOVEMENT(Swim)			// kAM_Swim
ADD_ANIM_MOVEMENT(Hover)		// kAM_Hover
ADD_ANIM_MOVEMENT(Climb)		// kAM_Climb
ADD_ANIM_MOVEMENT(Encode_NG)	// kAM_Encode_NG (no gravity)
ADD_ANIM_MOVEMENT(Encode_G)		// kAM_Encode_G  (gravity)
ADD_ANIM_MOVEMENT(Encode_GB)	// kAM_Encode_G  (gravity, backwards)
ADD_ANIM_MOVEMENT(Encode_V)		// kAM_Encode_V  (vertical)
