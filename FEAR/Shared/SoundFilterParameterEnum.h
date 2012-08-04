// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFilterParameterEnum.h
//
// PURPOSE : This file handles creating the SoundFilter parameter enums in
//			a more data drive way; handles mapping between strings and 
//			enums without identifier duplication.
//
// CREATED : 3/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_SOUNDFILTERPARAMETER_TYPE
	#undef ADD_SOUNDFILTERPARAMETER_TYPE
#endif
 
#if SOUNDFILTERPARAMETER_TYPE_AS_ENUM
	#define ADD_SOUNDFILTERPARAMETER_TYPE(label) kSoundFilterParameter_##label,
#elif SOUNDFILTERPARAMETER_TYPE_AS_STRING
	#define ADD_SOUNDFILTERPARAMETER_TYPE(label) #label,
#else
	#error	To use this include file, first define either ACTIONABILITY_TYPE_AS_ENUM or ACTIONABILITY_TYPE_AS_STRING, to include the actions as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_SOUNDFILTERPARAMETER_TYPE(x) where 
// x is the name of the enum without the "kActionAbility_" prefix.
// --------------------------------------------------------------------------

ADD_SOUNDFILTERPARAMETER_TYPE(Room)					// kSoundFilterParameter_Room
ADD_SOUNDFILTERPARAMETER_TYPE(RoomHF)				// kSoundFilterParameter_RoomHF
ADD_SOUNDFILTERPARAMETER_TYPE(RoomRolloffFactor)	// kSoundFilterParameter_RoomRolloffFactor
ADD_SOUNDFILTERPARAMETER_TYPE(DecayTime)			// kSoundFilterParameter_DecayTime
ADD_SOUNDFILTERPARAMETER_TYPE(DecayHFRatio)			// kSoundFilterParameter_DecayHFRatio
ADD_SOUNDFILTERPARAMETER_TYPE(Reflections)			// kSoundFilterParameter_Reflections
ADD_SOUNDFILTERPARAMETER_TYPE(ReflectionsDelay)		// kSoundFilterParameter_ReflectionsDelay
ADD_SOUNDFILTERPARAMETER_TYPE(Reverb)				// kSoundFilterParameter_Reverb
ADD_SOUNDFILTERPARAMETER_TYPE(ReverbDelay)			// kSoundFilterParameter_ReverbDelay
ADD_SOUNDFILTERPARAMETER_TYPE(Diffusion)			// kSoundFilterParameter_Diffusion
ADD_SOUNDFILTERPARAMETER_TYPE(Size)					// kSoundFilterParameter_Size
ADD_SOUNDFILTERPARAMETER_TYPE(AirAbsorptionHF)		// kSoundFilterParameter_AirAbsorptionHF
ADD_SOUNDFILTERPARAMETER_TYPE(Direct)				// kSoundFilterParameter_Direct
ADD_SOUNDFILTERPARAMETER_TYPE(Environment)			// kSoundFilterParameter_Environment
