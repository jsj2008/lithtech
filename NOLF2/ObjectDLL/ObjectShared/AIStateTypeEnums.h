// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateTypeEnums.h
//
// PURPOSE : Enums and string constants for states.
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
#endif
 
#if STATE_TYPE_AS_ENUM
	#define ADD_STATE_TYPE(aitype,label) kState_##aitype##label##,
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label) ADD_STATE_TYPE(Prop,label)
	#define ADD_STATE_SMARTOBJ(label) ADD_STATE_TYPE(SmartObject,label)
#elif STATE_TYPE_AS_STRING
	#define ADD_STATE_TYPE(aitype,label) #aitype#label,
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label) ADD_STATE_TYPE(Prop,label)
	#define ADD_STATE_SMARTOBJ(label) ADD_STATE_TYPE(SmartObject,label)
#elif STATE_TYPE_AS_SWITCH_HUMAN
	#define ADD_STATE_TYPE(aitype,label) case kState_##aitype##label##: extern CAIClassAbstract* AIFactoryCreateCAI##aitype##State##label##(); return (CAI##aitype##State*)AIFactoryCreateCAI##aitype##State##label##();
	#define ADD_STATE_HUMAN(label) ADD_STATE_TYPE(Human,label)
	#define ADD_STATE_BODY(label)
	#define ADD_STATE_PROP(label)
	#define ADD_STATE_SMARTOBJ(label)
#elif STATE_TYPE_AS_SWITCH_BODY
	#define ADD_STATE_TYPE(aitype,label) case kState_##aitype##label##: extern CAIClassAbstract* AIFactoryCreateC##aitype##State##label##(); return (C##aitype##State*)AIFactoryCreateC##aitype##State##label##();
	#define ADD_STATE_HUMAN(label)
	#define ADD_STATE_BODY(label) ADD_STATE_TYPE(Body,label)
	#define ADD_STATE_PROP(label)
	#define ADD_STATE_SMARTOBJ(label)
#else
	#error ! To use this include file, first define either STATE_TYPE_AS_ENUM or STATE_TYPE_AS_STRING, to include the state types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_STATE_TYPE(x) 
// where x is the name of the enum without the "kState_" prefix.
// --------------------------------------------------------------------------

ADD_STATE_HUMAN(Idle)				// kState_HumanIdle
ADD_STATE_HUMAN(Aware)				// kState_HumanAware
ADD_STATE_HUMAN(LookAt)				// kState_HumanLookAt
ADD_STATE_HUMAN(Assassinate)		// kState_HumanAssassinate
ADD_STATE_HUMAN(Draw)				// kState_HumanDraw
ADD_STATE_HUMAN(Holster)			// kState_HumanHolster
ADD_STATE_HUMAN(Attack)				// kState_HumanAttack
ADD_STATE_HUMAN(AttackFromCover)	// kState_HumanAttackFromCover
ADD_STATE_HUMAN(AttackFromVantage)	// kState_HumanAttackFromVantage
ADD_STATE_HUMAN(AttackFromView)		// kState_HumanAttackFromView
ADD_STATE_HUMAN(AttackProp)			// kState_HumanAttackProp
ADD_STATE_HUMAN(Cover)				// kState_HumanCover
ADD_STATE_HUMAN(Panic)				// kState_HumanPanic
ADD_STATE_HUMAN(Distress)			// kState_HumanDistress
ADD_STATE_HUMAN(Patrol)				// kState_HumanPatrol
ADD_STATE_HUMAN(Goto)				// kState_HumanGoto
ADD_STATE_HUMAN(Flee)				// kState_HumanFlee
ADD_STATE_HUMAN(Search)				// kState_HumanSearch
ADD_STATE_HUMAN(Chase)				// kState_HumanChase
ADD_STATE_HUMAN(Tail)				// kState_HumanTail
ADD_STATE_HUMAN(FollowFootprint)	// kState_HumanFollowFootprint	
ADD_STATE_HUMAN(Investigate)		// kState_HumanInvestigate
ADD_STATE_HUMAN(CheckBody)			// kState_HumanCheckBody
ADD_STATE_HUMAN(UseObject)			// kState_HumanUseObject
ADD_STATE_HUMAN(Talk)				// kState_HumanTalk
ADD_STATE_HUMAN(GetBackup)			// kState_HumanGetBackup
ADD_STATE_HUMAN(Charge)				// kState_HumanCharge
ADD_STATE_HUMAN(Animate)			// kState_HumanAnimate
ADD_STATE_HUMAN(Follow)				// kState_HumanFollow
ADD_STATE_HUMAN(LongJump)			// kState_HumanLongJump
ADD_STATE_HUMAN(DisappearReappear)	// kState_HumanDisappearReappear
ADD_STATE_HUMAN(Deflect)			// kState_HumanDeflect
ADD_STATE_HUMAN(Catch)				// kState_HumanCatch
ADD_STATE_HUMAN(SentryChallenge)	// kState_HumanSentryChallenge
ADD_STATE_HUMAN(Obstruct)			// kState_HumanObstruct
ADD_STATE_HUMAN(Apprehend)			// kState_HumanApprehend
ADD_STATE_HUMAN(Resurrecting)		// kState_HumanResurrecting
ADD_STATE_HUMAN(Launch)				// kState_HumanLaunch
ADD_STATE_HUMAN(Sniper)				// kState_HumanSniper
ADD_STATE_HUMAN(AttackMove)			// kState_HumanAttackMove
ADD_STATE_HUMAN(AttackProne)		// kState_HumanAttackProne

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
ADD_STATE_BODY(Carried)				// kState_BodyCarried
ADD_STATE_BODY(Dropped)				// kState_BodyDropped

ADD_STATE_PROP(Default)				// kState_PropDefault
ADD_STATE_PROP(Touching)			// kState_PropTouching
ADD_STATE_PROP(Knocked)				// kState_PropKnocked
ADD_STATE_PROP(Destroyed)			// kState_PropDestroyed
ADD_STATE_PROP(Hit)					// kState_PropHit

ADD_STATE_SMARTOBJ(Begin)			// kState_SmartObjectBegin
ADD_STATE_SMARTOBJ(Default)			// kState_SmartObjectDefault
ADD_STATE_SMARTOBJ(Disturbed)		// kState_SmartObjectDisturbed
ADD_STATE_SMARTOBJ(End)				// kState_SmartObjectEnd
