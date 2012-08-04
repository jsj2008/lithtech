//----------------------------------------------------------------------------
//              
//	MODULE:		AIVolumeTypeEnums.h
//              
//	PURPOSE:	Enums and string constants for AIData.
//              
//	CREATED:	21.02.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	
//              
//----------------------------------------------------------------------------

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_VOLUME_TYPE
	#undef ADD_VOLUME_TYPE
	#undef SET_NUM_VOLUME_TYPES
#endif
 
#if VOLUME_TYPE_AS_ENUM
	#define ADD_VOLUME_TYPE(label, val) kVolumeType_##label## = (1 << val),
	#define SET_NUM_VOLUME_TYPES(val) kVolumeType_Count = val,
#elif VOLUME_TYPE_AS_STRING
	#define ADD_VOLUME_TYPE(label, val) #label,
	#define SET_NUM_VOLUME_TYPES(val)
#else
	#error ! To use this include file, first define either VOLUME_TYPE_AS_ENUM or VOLUME_TYPE_AS_STRING, to include the stimulus types as bitflag enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_VOLUME_TYPE(x) 
// where x is the name of the enum without the "kAIData" prefix.
// --------------------------------------------------------------------------

ADD_VOLUME_TYPE(BaseVolume,		0)			// kVolumeType_BaseVolume
ADD_VOLUME_TYPE(Ladder,			1)			// kVolumeType_Ladder
ADD_VOLUME_TYPE(Stairs,			2)			// kVolumeType_Stairs
ADD_VOLUME_TYPE(Ledge,			3)			// kVolumeType_Ledge
ADD_VOLUME_TYPE(JumpUp,			4)			// kVolumeType_JumpUp
ADD_VOLUME_TYPE(JumpOver,		5) 			// kVolumeType_JumpOver
ADD_VOLUME_TYPE(Junction,		6)			// kVolumeType_Junction
ADD_VOLUME_TYPE(Teleport,		7)			// kVolumeType_Teleport
ADD_VOLUME_TYPE(AmbientLife,	8)			// kVolumeType_AmbientLife

SET_NUM_VOLUME_TYPES(9)