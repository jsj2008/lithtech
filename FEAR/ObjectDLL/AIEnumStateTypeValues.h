// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumStateTypeValues.h
//
// PURPOSE : Values for AI state types.
//
// CREATED : 6/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_STATE_TYPE
	#undef ADD_STATE_TYPE
	#undef ADD_STATE_HUMAN
	#undef ADD_STATE_BODY
	#undef ADD_STATE_PROP
	#undef ADD_STATE_SMARTOBJ
	#undef ADD_STATE
#endif
 
#if STATE_TYPE_AS_ENUM
	#define ADD_STATE_TYPE(aitype,label) kState_##aitype##label,
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label) ADD_STATE_TYPE(Prop,label)
	#define ADD_STATE_SMARTOBJ(label) ADD_STATE_TYPE(SmartObject,label)
	#define ADD_STATE(label) kState_##label,
#elif STATE_TYPE_AS_STRING
	#define ADD_STATE_TYPE(aitype,label) #aitype#label,
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label) ADD_STATE_TYPE(Prop,label)
	#define ADD_STATE_SMARTOBJ(label) ADD_STATE_TYPE(SmartObject,label)
	#define ADD_STATE(label) #label,
#elif STATE_TYPE_AS_SWITCH_HUMAN
	#define ADD_STATE_TYPE(aitype,label) case kState_##aitype##label: extern CAIClassAbstract* AIFactoryCreateCAI##aitype##State##label(); return (CAI##aitype##State*)AIFactoryCreateCAI##aitype##State##label();
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label)
	#define ADD_STATE_PROP(label)
	#define ADD_STATE_SMARTOBJ(label)
	#define ADD_STATE(label) case kState_##label: extern CAIClassAbstract* AIFactoryCreateCAI##State##label(); return (CAI##State*)AIFactoryCreateCAI##State##label();
#elif STATE_TYPE_AS_SWITCH_BODY
	#define ADD_STATE_TYPE(aitype,label) case kState_##aitype##label: extern CAIClassAbstract* AIFactoryCreateC##aitype##State##label(); return (C##aitype##State*)AIFactoryCreateC##aitype##State##label();
	#define ADD_STATE_HUMAN(label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label)
	#define ADD_STATE_SMARTOBJ(label)
	#define ADD_STATE(label)
#else
	#error ! To use this include file, first define either STATE_TYPE_AS_ENUM or STATE_TYPE_AS_STRING, to include the state types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_STATE_TYPE(x) 
// where x is the name of the enum without the "kState_" prefix.
// --------------------------------------------------------------------------

ADD_STATE(Animate)					// kState_Animate
ADD_STATE(Goto)						// kState_Goto
ADD_STATE(UseSmartObject)			// kState_UseSmartObject

ADD_STATE_BODY(Normal)				// kState_BodyNormal
ADD_STATE_BODY(Explode)				// kState_BodyExplode
ADD_STATE_BODY(Crush)				// kState_BodyCrush
ADD_STATE_BODY(Laser)				// kState_BodyLaser
ADD_STATE_BODY(Decay)				// kState_BodyDecay
ADD_STATE_BODY(Stairs)				// kState_BodyStairs
ADD_STATE_BODY(Ledge)				// kState_BodyLedge
ADD_STATE_BODY(Ladder)				// kState_BodyLadder
ADD_STATE_BODY(Underwater)			// kState_BodyUnderwater
ADD_STATE_BODY(Chair)				// kState_BodyChair
ADD_STATE_BODY(Poison)				// kState_BodyPoison
ADD_STATE_BODY(Acid)				// kState_BodyAcid
ADD_STATE_BODY(Arrow)				// kState_BodyArrow
ADD_STATE_BODY(Fade)				// kState_BodyFade

ADD_STATE_PROP(Default)				// kState_PropDefault
ADD_STATE_PROP(Touching)			// kState_PropTouching
ADD_STATE_PROP(Knocked)				// kState_PropKnocked
ADD_STATE_PROP(Destroyed)			// kState_PropDestroyed
ADD_STATE_PROP(Hit)					// kState_PropHit

ADD_STATE_SMARTOBJ(Begin)			// kState_SmartObjectBegin
ADD_STATE_SMARTOBJ(Default)			// kState_SmartObjectDefault
ADD_STATE_SMARTOBJ(Disturbed)		// kState_SmartObjectDisturbed
ADD_STATE_SMARTOBJ(End)				// kState_SmartObjectEnd
