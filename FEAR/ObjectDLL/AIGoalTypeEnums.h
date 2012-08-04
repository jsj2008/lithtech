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
	#undef ADD_GOAL_CLASS
	#undef ADD_GOAL_TYPE
#endif
 
#if GOAL_TYPE_AS_ENUM
	#define ADD_GOAL_CLASS(label) kGoal_##label,
	#define ADD_GOAL_TYPE(label) kGoal_##label,
#elif GOAL_TYPE_AS_STRING
	#define ADD_GOAL_CLASS(label) #label,
	#define ADD_GOAL_TYPE(label) #label,
#elif GOAL_TYPE_AS_SWITCH
	#define ADD_GOAL_CLASS(label) case kGoal_##label: extern CAIClassAbstract* AIFactoryCreateCAIGoal##label(); return (CAIGoalAbstract*)AIFactoryCreateCAIGoal##label();
	#define ADD_GOAL_TYPE(label)
#else
	#error	To use this include file, first define either GOAL_TYPE_AS_ENUM or GOAL_TYPE_AS_STRING, to include the goals as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_GOAL_TYPE(x) where 
// x is the name of the enum without the "kGoal_" prefix.
// --------------------------------------------------------------------------

// Classes.

ADD_GOAL_CLASS(Alert)					// kGoal_Alert
ADD_GOAL_CLASS(Ambush)					// kGoal_Ambush
ADD_GOAL_CLASS(Animate)					// kGoal_Animate
ADD_GOAL_CLASS(AnimateInterrupt)		// kGoal_AnimateInterrupt
ADD_GOAL_CLASS(Berserker)				// kGoal_Berserker
ADD_GOAL_CLASS(BlitzAndUseNode)			// kGoal_BlitzAndUseNode
ADD_GOAL_CLASS(BlitzCharacter)			// kGoal_BlitzCharacter
ADD_GOAL_CLASS(BlitzNode)				// kGoal_BlitzNode
ADD_GOAL_CLASS(ChangePrimaryWeapon)		// kGoal_ChangePrimaryWeapon
ADD_GOAL_CLASS(Charge)					// kGoal_Charge
ADD_GOAL_CLASS(Chase)					// kGoal_Chase
ADD_GOAL_CLASS(ChaseBerserker)			// kGoal_ChaseBerserker
ADD_GOAL_CLASS(CombatOpportunityAttack)	// kGoal_CombatOpportunityAttack
ADD_GOAL_CLASS(CorrectPosition)			// kGoal_CorrectPosition
ADD_GOAL_CLASS(CounterMelee)			// kGoal_CounterMelee
ADD_GOAL_CLASS(Cover)					// kGoal_Cover
ADD_GOAL_CLASS(CircleFlamePot)			// kGoal_CircleFlamePot
ADD_GOAL_CLASS(CritterFlee)				// kGoal_CritterFlee
ADD_GOAL_CLASS(Death)					// kGoal_Death
ADD_GOAL_CLASS(Defeated)				// kGoal_Defeated
ADD_GOAL_CLASS(DismountVehicle)			// kGoal_DismountVehicle
ADD_GOAL_CLASS(Dodge)					// kGoal_Dodge
ADD_GOAL_CLASS(EscapeDanger)			// kGoal_EscapeDanger
ADD_GOAL_CLASS(ExitLink)				// kGoal_ExitLink
ADD_GOAL_CLASS(FallBack)				// kGoal_FallBack
ADD_GOAL_CLASS(FinishBlockFailure)		// kGoal_FinishBlockFailure
ADD_GOAL_CLASS(FinishBlockSuccess)		// kGoal_FinishBlockSuccess
ADD_GOAL_CLASS(Flee)					// kGoal_Flee
ADD_GOAL_CLASS(FlushOut)				// kGoal_FlushOut
ADD_GOAL_CLASS(FlyAway)					// kGoal_FlyAway
ADD_GOAL_CLASS(Follow)					// kGoal_Follow
ADD_GOAL_CLASS(FollowHide)				// kGoal_FollowHide
ADD_GOAL_CLASS(FollowWait)				// kGoal_FollowWait
ADD_GOAL_CLASS(Guard)					// kGoal_Guard
ADD_GOAL_CLASS(Goto)					// kGoal_Goto
ADD_GOAL_CLASS(Idle)					// kGoal_Idle
ADD_GOAL_CLASS(Intro)					// kGoal_Intro
ADD_GOAL_CLASS(Investigate)				// kGoal_Investigate
ADD_GOAL_CLASS(KillEnemy)				// kGoal_KillEnemy
ADD_GOAL_CLASS(Lead)					// kGoal_Lead
ADD_GOAL_CLASS(Menace)					// kGoal_Menace
ADD_GOAL_CLASS(MountVehicle)			// kGoal_MountVehicle
ADD_GOAL_CLASS(Patrol)					// kGoal_Patrol
ADD_GOAL_CLASS(PickupWeapon)			// kGoal_PickupWeapon
ADD_GOAL_CLASS(ReactToBerserkerKick)	// kGoal_ReactToBerserkerKick
ADD_GOAL_CLASS(ReactToBlockedPath)		// kGoal_ReactToBlockedPath
ADD_GOAL_CLASS(ReactToDamage)			// kGoal_ReactToDamage
ADD_GOAL_CLASS(ReactToFollowerFallingBehind) // kGoal_ReactToFollowerFallingBehind
ADD_GOAL_CLASS(ReactToFinishingMove)	// kGoal_ReactToFinishingMove
ADD_GOAL_CLASS(ReactToMeleeBlocked)		// kGoal_ReactToMeleeBlocked
ADD_GOAL_CLASS(ReactToShove)			// kGoal_ReactToShove
ADD_GOAL_CLASS(ReactToWeaponBroke)		// kGoal_ReactToWeaponBroke
ADD_GOAL_CLASS(Retreat)					// kGoal_Retreat
ADD_GOAL_CLASS(RetreatLimited)			// kGoal_RetreatLimited
ADD_GOAL_CLASS(Scan)					// kGoal_Scan
ADD_GOAL_CLASS(Search)					// kGoal_Search
ADD_GOAL_CLASS(SearchFollow)			// kGoal_SearchFollow
ADD_GOAL_CLASS(SearchLostTarget)		// kGoal_SearchLostTarget
ADD_GOAL_CLASS(SelfDefense)				// kGoal_SelfDefense
ADD_GOAL_CLASS(Stalk)					// kGoal_Stalk
ADD_GOAL_CLASS(Stunned)					// kGoal_Stunned
ADD_GOAL_CLASS(SuppressEnemy)			// kGoal_SuppressEnemy
ADD_GOAL_CLASS(SurpriseAttackLaunch)	// kGoal_SurpriseAttackLaunch
ADD_GOAL_CLASS(TraverseLink)			// kGoal_TraverseLink
ADD_GOAL_CLASS(UseArmoredAutonomous)	// kGoal_UseArmoredAutonomous
ADD_GOAL_CLASS(UseSmartObject)			// kGoal_UseSmartObject
ADD_GOAL_CLASS(UseSmartObjectCombat)	// kGoal_UseSmartObjectCombat
ADD_GOAL_CLASS(Work)					// kGoal_Work
ADD_GOAL_CLASS(WorkTargetless)			// kGoal_WorkTargetless

// Types.

ADD_GOAL_TYPE(Alarm)					// kGoal_Alarm
ADD_GOAL_TYPE(AmbushAutonomous)			// kGoal_AmbushAutonomous
ADD_GOAL_TYPE(CombatOpportunityUse)		// kGoal_CombatOpportunityUse
ADD_GOAL_TYPE(DarkChant)				// kGoal_DarkChant
ADD_GOAL_TYPE(DarkWait)					// kGoal_DarkWait
ADD_GOAL_TYPE(DodgeMelee)				// kGoal_DodgeMelee
ADD_GOAL_TYPE(DodgeParanoid)			// kGoal_DodgeParanoid
ADD_GOAL_TYPE(Hide)						// kGoal_Hide
ADD_GOAL_TYPE(IdleCritter)				// kGoal_IdleCritter
ADD_GOAL_TYPE(IdleMelee)				// kGoal_IdleMelee
ADD_GOAL_TYPE(IdleParanoid)				// kGoal_IdleParanoid
ADD_GOAL_TYPE(IdlePoweredArmor)			// kGoal_IdlePoweredArmor
ADD_GOAL_TYPE(IdleSuicide)				// kGoal_IdleSuicide
ADD_GOAL_TYPE(IdleTurret)				// kGoal_IdleTurret
ADD_GOAL_TYPE(KillEnemyNoFOV)			// kGoal_KillEnemyNoFOV
ADD_GOAL_TYPE(KillEnemyNoShootThru)		// kGoal_KillEnemyNoShootThru
ADD_GOAL_TYPE(MenaceNonInterruptable)	// kGoal_MenaceNonInterruptable
ADD_GOAL_TYPE(RetreatCloaked)			// kGoal_RetreatCloaked
ADD_GOAL_TYPE(RetreatDetermined)		// kGoal_RetreatDetermined
ADD_GOAL_TYPE(Safety)					// kGoal_Safety
ADD_GOAL_TYPE(SafetyNodeFirePosition)	// kGoal_SafetyNodeFirePosition
ADD_GOAL_TYPE(SurpriseAttackSetup)		// kGoal_SurpriseAttackSetup
