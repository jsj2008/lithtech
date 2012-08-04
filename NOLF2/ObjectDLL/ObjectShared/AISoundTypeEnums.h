// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundTypeEnums.h
//
// PURPOSE : Enums and string constants for AISounds.
//
// CREATED : 7/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AISOUND_TYPE
	#undef ADD_AISOUND_TYPE
#endif
 
#if AISOUND_TYPE_AS_ENUM
	#define ADD_AISOUND_TYPE(label) kAIS_##label##,
#elif AISOUND_TYPE_AS_STRING
	#define ADD_AISOUND_TYPE(label) #label,
#else
	#error ! To use this include file, first define either AISOUND_TYPE_AS_ENUM or AISOUND_TYPE_AS_STRING, to include the AISound types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AISOUND_TYPE(x, val) 
// where x is the name of the enum without the "kAIS" prefix.
// --------------------------------------------------------------------------

ADD_AISOUND_TYPE(None)						// kAIS_None
ADD_AISOUND_TYPE(Accident)					// kAIS_Accident
ADD_AISOUND_TYPE(Alarm)						// kAIS_Alarm
ADD_AISOUND_TYPE(AlertAllyMinor)			// kAIS_AlertAllyMinor
ADD_AISOUND_TYPE(AlertAllyMajor)			// kAIS_AlertAllyMajor
ADD_AISOUND_TYPE(Apprehend)					// kAIS_Apprehend
ADD_AISOUND_TYPE(AssistTrappedAlly)			// kAIS_AssistTrappedAlly
ADD_AISOUND_TYPE(Attack)					// kAIS_Attack
ADD_AISOUND_TYPE(BackupCryWolf)				// kAIS_BackupCryWolf
ADD_AISOUND_TYPE(BackupFail)				// kAIS_BackupFail
ADD_AISOUND_TYPE(Bleeding)					// kAIS_Bleeding
ADD_AISOUND_TYPE(Burning)					// kAIS_Burning
ADD_AISOUND_TYPE(CatchBreath)				// kAIS_CatchBreath
ADD_AISOUND_TYPE(Charge)					// kAIS_Charge
ADD_AISOUND_TYPE(Chase)						// kAIS_Chase
ADD_AISOUND_TYPE(Choking)					// kAIS_Choking
ADD_AISOUND_TYPE(Cover)						// kAIS_Cover
ADD_AISOUND_TYPE(Crush)						// kAIS_Crush
ADD_AISOUND_TYPE(DamageBearTrap)			// kAIS_DamageBearTrap
ADD_AISOUND_TYPE(DamageGlue)				// kAIS_DamageGlue
ADD_AISOUND_TYPE(DamageLaughing)			// kAIS_DamageLaughing
ADD_AISOUND_TYPE(DamageSleeping)			// kAIS_DamageSleeping
ADD_AISOUND_TYPE(DamageSlippery)			// kAIS_DamageSlippery
ADD_AISOUND_TYPE(DamageStun)				// kAIS_DamageStun
ADD_AISOUND_TYPE(Death)						// kAIS_Death
ADD_AISOUND_TYPE(DeathQuiet)				// kAIS_DeathQuiet
ADD_AISOUND_TYPE(Disappear)					// kAIS_Disappear
ADD_AISOUND_TYPE(Disarmed)					// kAIS_Disarmed
ADD_AISOUND_TYPE(DiscoveryBad)				// kAIS_DiscoveryBad
ADD_AISOUND_TYPE(DiscoveryGood)				// kAIS_DiscoveryGood
ADD_AISOUND_TYPE(DisposeBody)				// kAIS_DisposeBody
ADD_AISOUND_TYPE(Distress)					// kAIS_Distress
ADD_AISOUND_TYPE(DisturbanceSeenUnsure)		// kAIS_DisturbanceSeenUnsure
ADD_AISOUND_TYPE(DisturbanceSeenMinor)		// kAIS_DisturbanceSeenMinor
ADD_AISOUND_TYPE(DisturbanceHeardMinor)		// kAIS_DisturbanceHeardMinor
ADD_AISOUND_TYPE(DisturbanceSeenMajor)		// kAIS_DisturbanceSeenMajor
ADD_AISOUND_TYPE(DisturbanceHeardMajor)		// kAIS_DisturbanceHeardMajor
ADD_AISOUND_TYPE(DisturbanceSeenAlarming)	// kAIS_DisturbanceSeenAlarming
ADD_AISOUND_TYPE(DisturbanceHeardAlarming)	// kAIS_DisturbanceHeardAlarming
ADD_AISOUND_TYPE(Drowsy)					// kAIS_Drowsy
ADD_AISOUND_TYPE(Electrocute)				// kAIS_Electrocute
ADD_AISOUND_TYPE(Explode)					// kAIS_Explode
ADD_AISOUND_TYPE(Fall)						// kAIS_Fall
ADD_AISOUND_TYPE(FallStairs)				// kAIS_FallStairs
ADD_AISOUND_TYPE(FindBackup)				// kAIS_FindBackup
ADD_AISOUND_TYPE(FollowFootprint)			// kAIS_FollowFootprint
ADD_AISOUND_TYPE(Grenade)					// kAIS_Grenade
ADD_AISOUND_TYPE(Idle)						// kAIS_Idle
ADD_AISOUND_TYPE(InvestigateFail)			// kAIS_InvestigateFail
ADD_AISOUND_TYPE(JunctionDilemma)			// kAIS_JunctionDilemma
ADD_AISOUND_TYPE(LightSwitch)				// kAIS_LightSwitch
ADD_AISOUND_TYPE(LightsOn)					// kAIS_LightsOn
ADD_AISOUND_TYPE(LightsOff)					// kAIS_LightsOff
ADD_AISOUND_TYPE(Lunge)						// kAIS_Lunge
ADD_AISOUND_TYPE(OhNo)						// kAIS_OhNo
ADD_AISOUND_TYPE(OrderBackup)				// kAIS_OrderBackup
ADD_AISOUND_TYPE(OrderCrouch)				// kAIS_OrderCrouch
ADD_AISOUND_TYPE(Pain)						// kAIS_Pain
ADD_AISOUND_TYPE(Panic)						// kAIS_Panic
ADD_AISOUND_TYPE(PointOfInterest)			// kAIS_PointOfInterest
ADD_AISOUND_TYPE(Poisoning)					// kAIS_Poisoning
ADD_AISOUND_TYPE(Prone)						// kAIS_Prone
ADD_AISOUND_TYPE(Reappear)					// kAIS_Reappear
ADD_AISOUND_TYPE(Retreat)					// kAIS_Retreat
ADD_AISOUND_TYPE(Search)					// kAIS_Search
ADD_AISOUND_TYPE(SearchFail)				// kAIS_SearchFail
ADD_AISOUND_TYPE(SeeBody)					// kAIS_SeeBody
ADD_AISOUND_TYPE(Snore)						// kAIS_Snore
ADD_AISOUND_TYPE(SplitUp)					// kAIS_SplitUp
ADD_AISOUND_TYPE(Tail)						// kAIS_Tail
ADD_AISOUND_TYPE(Urinal)					// kAIS_Urinal
