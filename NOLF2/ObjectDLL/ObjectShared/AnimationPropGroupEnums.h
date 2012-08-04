// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationPropGroupEnums.h
//
// PURPOSE : Enums and string constants for animation property groups.
//
// CREATED : 6/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ANIM_PROP_GROUP
	#undef ADD_ANIM_PROP_GROUP
#endif
 
#if ANIM_PROP_GROUP_AS_ENUM
	#define ADD_ANIM_PROP_GROUP(label) kAPG_##label##,
#elif ANIM_PROP_GROUP_AS_STRING
	#define ADD_ANIM_PROP_GROUP(label) #label,
#else
	#error ! To use this include file, first define either ANIM_PROP_GROUP_AS_ENUM or ANIM_PROP_GROUP_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_PROP_GROUP(x) 
// where x is the name of the enum without the "kAPG_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_PROP_GROUP(Posture)		// kAPG_Posture
ADD_ANIM_PROP_GROUP(WeaponAction)	// kAPG_WeaponAction
ADD_ANIM_PROP_GROUP(WeaponPosition)	// kAPG_WeaponPosition	
ADD_ANIM_PROP_GROUP(Movement)		// kAPG_Movement
ADD_ANIM_PROP_GROUP(Weapon)			// kAPG_Weapon
ADD_ANIM_PROP_GROUP(Mood)			// kAPG_Mood
ADD_ANIM_PROP_GROUP(Evasive)		// kAPG_Evasive
ADD_ANIM_PROP_GROUP(Awareness)		// kAPG_Awareness
ADD_ANIM_PROP_GROUP(Action)			// kAPG_Action
