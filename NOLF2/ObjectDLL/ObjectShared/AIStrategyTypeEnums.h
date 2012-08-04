// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyTypeEnums.h
//
// PURPOSE : Enums and string constants for Strategies.
//
// CREATED : 12/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_STRATEGY_TYPE
	#undef ADD_STRATEGY_TYPE
#endif
 
#if STRATEGY_TYPE_AS_ENUM
	#define ADD_STRATEGY_TYPE(aitype,label) kStrat_##aitype##label##,
#elif STRATEGY_TYPE_AS_STRING
	#define ADD_STRATEGY_TYPE(aitype,label) #aitype#label,
#elif STRATEGY_TYPE_AS_SWITCH
	#define ADD_STRATEGY_TYPE(aitype,label) case kStrat_##aitype##label##: extern CAIClassAbstract* AIFactoryCreateCAI##aitype##Strategy##label##(); return (CAI##aitype##Strategy*)AIFactoryCreateCAI##aitype##Strategy##label##();
#else
	#error	To use this include file, first define either STRATEGY_TYPE_AS_ENUM or STRATEGY_TYPE_AS_STRING, to include the Strategies as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_STRATEGY_TYPE(x) where 
// x is the name of the enum without the "kStrat_AITYPE" prefix.
// --------------------------------------------------------------------------

ADD_STRATEGY_TYPE(Human, OneShotAni)		// kStrat_HumanOneShotAni
ADD_STRATEGY_TYPE(Human, FollowPath)		// kStrat_HumanFollowPath
ADD_STRATEGY_TYPE(Human, Dodge)				// kStrat_HumanDodge
ADD_STRATEGY_TYPE(Human, CoverDuck)			// kStrat_HumanCoverDuck
ADD_STRATEGY_TYPE(Human, Cover1WayCorner)	// kStrat_HumanCover1WayCorner
ADD_STRATEGY_TYPE(Human, Cover2WayCorner)	// kStrat_HumanCover2WayCorner
ADD_STRATEGY_TYPE(Human, CoverBlind)		// kStrat_HumanCoverBlind
ADD_STRATEGY_TYPE(Human, GrenadeThrow)		// kStrat_HumanGrenadeThrow
ADD_STRATEGY_TYPE(Human, ShootBurst)		// kStrat_HumanShootBurst
ADD_STRATEGY_TYPE(Human, Flashlight)		// kStrat_HumanFlashlight
ADD_STRATEGY_TYPE(Human, Taunt)				// kStrat_HumanTaunt
ADD_STRATEGY_TYPE(Human, ToggleLights)		// kStrat_HumanToggleLights
ADD_STRATEGY_TYPE(Human, ShootStream)		// kStrat_HumanShootStream