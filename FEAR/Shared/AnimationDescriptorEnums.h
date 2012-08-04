// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationDescriptorEnums.h
//
// PURPOSE : Enums and string constants for animation descriptors.
//
// CREATED : 10/15/03
//
// (c) 2003-2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ANIM_DESC
	#undef ADD_ANIM_DESC
#endif

#if ANIM_DESC_AS_ENUM
	#define ADD_ANIM_DESC(label) kAD_##label,
#elif ANIM_DESC_AS_STRING
	#define ADD_ANIM_DESC(label) #label,
#else
	#error ! To use this include file, first define either ANIM_DESC_AS_ENUM or ANIM_DESC_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_DESC(x) 
// where x is the name of the enum without the "kAD_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_DESC(None)			// kAD_None

// Movement...
ADD_ANIM_DESC(MOV_Climb)			// kAD_MOV_Climb
ADD_ANIM_DESC(MOV_Encode_G)			// kAD_MOV_Encode_G
ADD_ANIM_DESC(MOV_Encode_GB)		// kAD_MOV_Encode_GB
ADD_ANIM_DESC(MOV_Encode_GL)		// kAD_MOV_Encode_GL
ADD_ANIM_DESC(MOV_Encode_GR)		// kAD_MOV_Encode_GR
ADD_ANIM_DESC(MOV_Encode_NG)		// kAD_MOV_Encode_NG
ADD_ANIM_DESC(MOV_Encode_NGB)		// kAD_MOV_Encode_NGB
ADD_ANIM_DESC(MOV_Encode_NGL)		// kAD_MOV_Encode_NGL
ADD_ANIM_DESC(MOV_Encode_NGR)		// kAD_MOV_Encode_NGR
ADD_ANIM_DESC(MOV_Encode_V)			// kAD_MOV_Encode_V
ADD_ANIM_DESC(MOV_Encode_NP)		// kAD_MOV_Encode_NP
ADD_ANIM_DESC(MOV_Encode_Velocity)	// kAD_MOV_Encode_Velocity
ADD_ANIM_DESC(MOV_Fall)				// kAD_MOV_Fall
ADD_ANIM_DESC(MOV_JumpOver)			// kAD_MOV_JumpOver
ADD_ANIM_DESC(MOV_JumpUp)			// kAD_MOV_JumpUp
ADD_ANIM_DESC(MOV_Run)				// kAD_MOV_Run
ADD_ANIM_DESC(MOV_Set)				// kAD_MOV_Set
ADD_ANIM_DESC(MOV_Swim)				// kAD_MOV_Swim
ADD_ANIM_DESC(MOV_Walk)				// kAD_MOV_Walk

// Trackers....
ADD_ANIM_DESC(TRK_Lower)			// kAD_TRK_Lower
ADD_ANIM_DESC(TRK_Twitch)			// kAD_TRK_Twitch
ADD_ANIM_DESC(TRK_Upper)			// kAD_TRK_Upper
ADD_ANIM_DESC(TRK_Blend)			// kAD_TRK_Blend
ADD_ANIM_DESC(TRK_LowerBlend)		// kAD_TRK_LowerBlend
ADD_ANIM_DESC(TRK_UpperBlend)		// kAD_TRK_UpperBlend
ADD_ANIM_DESC(TRK_Custom)			// kAD_TRK_Custom

// Synchronize...
ADD_ANIM_DESC(SYNC_Lower)			// kAD_SYNC_Lower
ADD_ANIM_DESC(SYNC_Upper)			// kAD_SYNC_Upper

// Camera...
ADD_ANIM_DESC(CAM_Rotation)			// kAD_CAM_Rotation
ADD_ANIM_DESC(CAM_RotationAim)		// kAD_CAM_RotationAim

// Input...
ADD_ANIM_DESC(IN_Locked)			// kAD_IN_Locked

// Condition...
ADD_ANIM_DESC(COND_BentOver)		// kAD_COND_BentOver
ADD_ANIM_DESC(COND_Crouching)		// kAD_COND_Crouching
ADD_ANIM_DESC(COND_Rotating)		// kAD_COND_Rotating

