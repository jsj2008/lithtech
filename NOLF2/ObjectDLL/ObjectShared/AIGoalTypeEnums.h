// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTypeEnums.h
//
// PURPOSE : Enums and string constants for goals.
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_GOAL_TYPE
	#undef ADD_GOAL_TYPE
#endif
 
#if GOAL_TYPE_AS_ENUM
	#define ADD_GOAL_TYPE(label) kGoal_##label##,
#elif GOAL_TYPE_AS_STRING
	#define ADD_GOAL_TYPE(label) #label,
#elif GOAL_TYPE_AS_SWITCH
	#define ADD_GOAL_TYPE(label) case kGoal_##label##: extern CAIClassAbstract* AIFactoryCreateCAIGoal##label##(); return (CAIGoalAbstract*)AIFactoryCreateCAIGoal##label##();
#else
	#error	To use this include file, first define either GOAL_TYPE_AS_ENUM or GOAL_TYPE_AS_STRING, to include the goals as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_GOAL_TYPE(x) where 
// x is the name of the enum without the "kGoal_" prefix.
// --------------------------------------------------------------------------

ADD_GOAL_TYPE(Alarm)					// kGoal_Alarm
ADD_GOAL_TYPE(Animate)					// kGoal_Animate
ADD_GOAL_TYPE(Apprehend)				// kGoal_Apprehend
ADD_GOAL_TYPE(Assassinate)				// kGoal_Assassinate
ADD_GOAL_TYPE(Attack)					// kGoal_Attack
ADD_GOAL_TYPE(AttackFromCover)			// kGoal_AttackFromCover
ADD_GOAL_TYPE(AttackFromRandomVantage)	// kGoal_AttackFromRandomVantage
ADD_GOAL_TYPE(AttackFromRoofVantage)	// kGoal_AttackFromRoofVantage
ADD_GOAL_TYPE(AttackFromVantage)		// kGoal_AttackFromVantage
ADD_GOAL_TYPE(AttackFromView)			// kGoal_AttackFromView
ADD_GOAL_TYPE(AttackMelee)				// kGoal_AttackMelee
ADD_GOAL_TYPE(AttackProne)				// kGoal_AttackProne
ADD_GOAL_TYPE(AttackProp)				// kGoal_AttackProp
ADD_GOAL_TYPE(AttackRanged)				// kGoal_AttackRanged
ADD_GOAL_TYPE(AttackRangedDynamic)		// kGoal_AttackRangedDynamic
ADD_GOAL_TYPE(Catch)					// kGoal_Catch
ADD_GOAL_TYPE(Charge)					// kGoal_Charge
ADD_GOAL_TYPE(Chase)					// kGoal_Chase
ADD_GOAL_TYPE(CheckBody)				// kGoal_CheckBody
ADD_GOAL_TYPE(Cover)					// kGoal_Cover
ADD_GOAL_TYPE(Deflect)					// kGoal_Deflect
ADD_GOAL_TYPE(Destroy)					// kGoal_Destroy
ADD_GOAL_TYPE(DisappearReappear)		// kGoal_DisappearReappear
ADD_GOAL_TYPE(DisappearReappearEvasive)	// kGoal_DisappearReappearEvasive
ADD_GOAL_TYPE(Distress)					// kGoal_Distress
ADD_GOAL_TYPE(DramaDeath)				// kGoal_DramaDeath
ADD_GOAL_TYPE(DrawWeapon)				// kGoal_DrawWeapon
ADD_GOAL_TYPE(EnjoyPoster)				// kGoal_EnjoyPoster
ADD_GOAL_TYPE(ExitLevel)				// kGoal_ExitLevel
ADD_GOAL_TYPE(Flee)						// kGoal_Flee
ADD_GOAL_TYPE(Follow)					// kGoal_Follow
ADD_GOAL_TYPE(FollowFootprint)			// kGoal_FollowFootprint
ADD_GOAL_TYPE(GetBackup)				// kGoal_GetBackup
ADD_GOAL_TYPE(Goto)						// kGoal_Goto
ADD_GOAL_TYPE(Guard)					// kGoal_Guard
ADD_GOAL_TYPE(HolsterWeapon)			// kGoal_HolsterWeapon
ADD_GOAL_TYPE(Investigate)				// kGoal_Investigate
ADD_GOAL_TYPE(LoveKitty)				// kGoal_LoveKitty
ADD_GOAL_TYPE(Lunge)					// kGoal_Lunge
ADD_GOAL_TYPE(Menace)					// kGoal_Menace
ADD_GOAL_TYPE(MountedFlashlight)		// kGoal_MountedFlashlight
ADD_GOAL_TYPE(Obstruct)					// kGoal_Obstruct
ADD_GOAL_TYPE(Patrol)					// kGoal_Patrol
ADD_GOAL_TYPE(PlacePoster)				// kGoal_PlacePoster
ADD_GOAL_TYPE(ProximityCommand)			// kGoal_ProximityCommand
ADD_GOAL_TYPE(PsychoChase)				// kGoal_PsychoChase
ADD_GOAL_TYPE(ReclassifyToEnemy)		// kGoal_ReclassifyToEnemy
ADD_GOAL_TYPE(RespondToAlarm)			// kGoal_RespondToAlarm
ADD_GOAL_TYPE(RespondToBackup)			// kGoal_RespondToBackup
ADD_GOAL_TYPE(Resurrecting)				// kGoal_Resurrecting
ADD_GOAL_TYPE(Retreat)					// kGoal_Retreat
ADD_GOAL_TYPE(Ride)						// kGoal_Ride
ADD_GOAL_TYPE(Search)					// kGoal_Search
ADD_GOAL_TYPE(SentryChallenge)			// kGoal_SentryChallenge
ADD_GOAL_TYPE(SentryMark)				// kGoal_SentryMark
ADD_GOAL_TYPE(SerumDeath)				// kGoal_SerumDeath
ADD_GOAL_TYPE(Sleep)					// kGoal_Sleep		
ADD_GOAL_TYPE(Sniper)					// kGoal_Sniper		
ADD_GOAL_TYPE(SpecialDamage)			// kGoal_SpecialDamage		
ADD_GOAL_TYPE(Tail)						// kGoal_Tail
ADD_GOAL_TYPE(Talk)						// kGoal_Talk
ADD_GOAL_TYPE(Work)						// kGoal_Work
