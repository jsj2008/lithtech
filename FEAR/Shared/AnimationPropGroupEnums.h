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
	#define ADD_ANIM_PROP_GROUP(label) kAPG_##label,
#elif ANIM_PROP_GROUP_AS_STRING
	#define ADD_ANIM_PROP_GROUP(label) #label,
#else
	#error ! To use this include file, first define either ANIM_PROP_GROUP_AS_ENUM or ANIM_PROP_GROUP_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_PROP_GROUP(x) 
// where x is the name of the enum without the "kAPG_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_PROP_GROUP(Action)			// kAPG_Action
ADD_ANIM_PROP_GROUP(Activity)		// kAPG_Activity
ADD_ANIM_PROP_GROUP(Body)			// kAPG_Body
ADD_ANIM_PROP_GROUP(Damage)			// kAPG_Damage
ADD_ANIM_PROP_GROUP(DamageDir)		// kAPG_DamageDir
ADD_ANIM_PROP_GROUP(Lean)			// kAPG_Lean
ADD_ANIM_PROP_GROUP(LookContext)	// kAPG_LookContext
ADD_ANIM_PROP_GROUP(ConditionalAction)		// kAPG_ConditionalAction
ADD_ANIM_PROP_GROUP(ConditionalSubAction)	// kAPG_ConditionalSubAction
ADD_ANIM_PROP_GROUP(Movement)		// kAPG_Movement
ADD_ANIM_PROP_GROUP(MovementDir)	// kAPG_MovementDir
ADD_ANIM_PROP_GROUP(SpecialFire)	// kAPG_SpecialFire
ADD_ANIM_PROP_GROUP(Posture)		// kAPG_Posture
ADD_ANIM_PROP_GROUP(SonicType)		// kAPG_SonicType
ADD_ANIM_PROP_GROUP(Weapon)			// kAPG_Weapon
ADD_ANIM_PROP_GROUP(WeaponHand)		// kAPG_WeaponHand 	
ADD_ANIM_PROP_GROUP(WeaponPosition)	// kAPG_WeaponPosition
ADD_ANIM_PROP_GROUP(Client)			// kAPG_Client
