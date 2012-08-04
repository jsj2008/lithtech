// ----------------------------------------------------------------------- //
//
// MODULE  : AIDataTypeEnums.h
//
// PURPOSE : Enums and string constants for AIData.
//
// CREATED : 10/15/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AIDATA_TYPE
	#undef ADD_AIDATA_TYPE
#endif
 
#if AIDATA_TYPE_AS_ENUM
	#define ADD_AIDATA_TYPE(label) kAIData_##label##,
#elif AIDATA_TYPE_AS_STRING
	#define ADD_AIDATA_TYPE(label) #label,
#else
	#error ! To use this include file, first define either AIDATA_TYPE_AS_ENUM or AIDATA_TYPE_AS_STRING, to include the AIData types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AIDATA_TYPE(x) 
// where x is the name of the enum without the "kAIData" prefix.
// --------------------------------------------------------------------------

ADD_AIDATA_TYPE(AccidentChance)			// kAIData_AccidentChance
ADD_AIDATA_TYPE(BreaksDoors)			// kAIData_BreaksDoors
ADD_AIDATA_TYPE(CannotPathThruDoors)	// kAIData_CannotPathThruDoors
ADD_AIDATA_TYPE(ChaseEndurance)			// kAIData_ChaseEndurance
ADD_AIDATA_TYPE(DeflectChance)			// kAIData_DeflectChance
ADD_AIDATA_TYPE(DisappearDistMin)		// kAIData_DisappearDistMin
ADD_AIDATA_TYPE(DisappearDistMax)		// kAIData_DisappearDistMin
ADD_AIDATA_TYPE(DisappearFadeTime)		// kAIData_DisappearFadeTime
ADD_AIDATA_TYPE(DisposeLantern)			// kAIData_DisposeLantern
ADD_AIDATA_TYPE(DivergePaths)			// kAIData_DivergePaths
ADD_AIDATA_TYPE(DoNotCloseDoors)		// kAIData_DoNotCloseDoors
ADD_AIDATA_TYPE(DoNotDamageSelf)		// kAIData_DoNotDamageSelf
ADD_AIDATA_TYPE(DoNotExposeHiding)		// kAIData_DoNotExposeHiding
ADD_AIDATA_TYPE(DoNotToggleLights)		// kAIData_DoNotToggleLights
ADD_AIDATA_TYPE(DynMoveTimeMin)			// kAIData_DynMoveTimeMin	
ADD_AIDATA_TYPE(DynMoveTimeMax)			// kAIData_DynMoveTimeMax
ADD_AIDATA_TYPE(DynMoveBackupDist)		// kAIData_DynMoveBackupDist
ADD_AIDATA_TYPE(DynMoveFlankPassDist)	// kAIData_DynMoveFlankPassDist
ADD_AIDATA_TYPE(DynMoveFlankWidthDist)	// kAIData_DynMoveFlankWidthDist
ADD_AIDATA_TYPE(IgnoreJunctions)		// kAIData_IgnoreJunctions
ADD_AIDATA_TYPE(LethalSpecialDamage)	// kAIData_LethalSpecialDamage
ADD_AIDATA_TYPE(LongJumpHeightMin)		// kAIData_LongJumpHeightMin
ADD_AIDATA_TYPE(LongJumpHeightMax)		// kAIData_LongJumpHeightMax
ADD_AIDATA_TYPE(LungeDistMin)			// kAIData_LungeDistMin
ADD_AIDATA_TYPE(LungeDistMax)			// kAIData_LungeDistMax
ADD_AIDATA_TYPE(LungeSpeed)				// kAIData_LungeSpeed
ADD_AIDATA_TYPE(LungeStopDistance)		// kAIData_LungeStopDistance
ADD_AIDATA_TYPE(GoalProximityDist)		// kAIData_GoalProximityDist
ADD_AIDATA_TYPE(MeleeKnockBackDist)		// kAIData_MeleeKnockBackDist
ADD_AIDATA_TYPE(MusicMoodMin)			// kAIData_MusicMoodMin
ADD_AIDATA_TYPE(MusicMoodMax)			// kAIData_MusicMoodMax
ADD_AIDATA_TYPE(NoDynamicPathfinding)	// kAIData_NoDynamicPathfinding
ADD_AIDATA_TYPE(PrimaryWeaponOnLeft)	// kAIData_PrimaryWeaponOnLeft
ADD_AIDATA_TYPE(ProneDistMin)			// kAIData_ProneDistMin
ADD_AIDATA_TYPE(ProneSlideDist)			// kAIData_ProneSlideDist
ADD_AIDATA_TYPE(ProneTime)				// kAIData_ProneTime
ADD_AIDATA_TYPE(ReappearDist)			// kAIData_ReappearDist
ADD_AIDATA_TYPE(Resurrecting)			// kAIData_Resurrecting
ADD_AIDATA_TYPE(RetreatTriggerDist)		// kAIData_RetreatTriggerDist
ADD_AIDATA_TYPE(RetreatJumpDist)		// kAIData_RetreatJumpDist
ADD_AIDATA_TYPE(RetreatSpeed)			// kAIData_RetreatSpeed
ADD_AIDATA_TYPE(SleepTimeMin)			// kAIData_SleepTimeMin
ADD_AIDATA_TYPE(SleepTimeMax)			// kAIData_SleepTimeMax
ADD_AIDATA_TYPE(MaxDeflectDuration)		// kAIData_MaxDeflectDuration
ADD_AIDATA_TYPE(MaxCatchDuration)		// kAIData_MaxCatchDuration
ADD_AIDATA_TYPE(MinPathWeight)			// kAIData_MinPathWeight
ADD_AIDATA_TYPE(OneAnimCover)			// kAIData_OneAnimCover
ADD_AIDATA_TYPE(SprintDist)				// kAIData_SprintDist
ADD_AIDATA_TYPE(HoverMaxSpeed)			// kAIData_HoverMaxSpeed
ADD_AIDATA_TYPE(HoverMinSpeed)			// kAIData_HoverMinSpeed
ADD_AIDATA_TYPE(HoverAccelerationRate)	// kAIData_HoverAccelerationRate
ADD_AIDATA_TYPE(HoverResetSpeedTime)	// kAIData_HoverResetSpeedTime