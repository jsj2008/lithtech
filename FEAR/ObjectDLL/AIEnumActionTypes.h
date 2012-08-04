// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumActionTypes.h
//
// PURPOSE : Enums and string constants for actions.
//
// CREATED : 2/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ACTION_TYPE
	#undef ADD_ACTION_CLASS
	#undef ADD_ACTION_TYPE
#endif
 
#if ACTION_TYPE_AS_ENUM
	#define ADD_ACTION_CLASS(label) kAct_##label,
	#define ADD_ACTION_TYPE(label) kAct_##label,
#elif ACTION_TYPE_AS_STRING
	#define ADD_ACTION_CLASS(label) #label,
	#define ADD_ACTION_TYPE(label) #label,
#elif ACTION_TYPE_AS_SWITCH
	#define ADD_ACTION_CLASS(label) case kAct_##label: extern CAIClassAbstract* AIFactoryCreateCAIAction##label(); return (CAIActionAbstract*)AIFactoryCreateCAIAction##label();
	#define ADD_ACTION_TYPE(label)
#else
	#error	To use this include file, first define either ACTION_TYPE_AS_ENUM or ACTION_TYPE_AS_STRING, to include the actions as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ACTION_TYPE(x) where 
// x is the name of the enum without the "kAct_" prefix.
// --------------------------------------------------------------------------

// Classes.

ADD_ACTION_CLASS(Animate)						// kAct_Animate
ADD_ACTION_CLASS(Attack)						// kAct_Attack
ADD_ACTION_CLASS(AttackBurstLimited)			// kAct_AttackBurstLimited
ADD_ACTION_CLASS(AttackCrouch)					// kAct_AttackCrouch
ADD_ACTION_CLASS(AttackFromAmbush)				// kAct_AttackFromAmbush
ADD_ACTION_CLASS(AttackFromCover)				// kAct_AttackFromCover
ADD_ACTION_CLASS(AttackFromNode)				// kAct_AttackFromNode
ADD_ACTION_CLASS(AttackFromNodeBounded)			// kAct_AttackFromNodeBounded
ADD_ACTION_CLASS(AttackFromSafetyNodeFirePosition)	// kAct_AttackFromSafetyNodeFirePosition
ADD_ACTION_CLASS(AttackFromVehicle)				// kAct_AttackFromVehicle
ADD_ACTION_CLASS(AttackFromVehicleMelee)		// kAct_AttackFromVehicleMelee
ADD_ACTION_CLASS(AttackFromView)				// kAct_AttackFromView
ADD_ACTION_CLASS(AttackGrenade)					// kAct_AttackGrenade
ADD_ACTION_CLASS(AttackGrenadeFromCover)		// kAct_AttackGrenadeFromCover
ADD_ACTION_CLASS(AttackLunge3D)					// kAct_AttackLunge3D
ADD_ACTION_CLASS(AttackLungeMelee)				// kAct_AttackLungeMelee
ADD_ACTION_CLASS(AttackLungeSuicide)			// kAct_AttackLungeSuicide
ADD_ACTION_CLASS(AttackLungeUncloaked)			// kAct_AttackLungeUncloaked
ADD_ACTION_CLASS(AttackMelee)					// kAct_AttackMelee
ADD_ACTION_CLASS(AttackMeleeFromDistance)		// kAct_AttackMeleeFromDistance
ADD_ACTION_CLASS(AttackMeleeFromRun)			// kAct_AttackMeleeFromRun
ADD_ACTION_CLASS(AttackMeleeKick)				// kAct_AttackMeleeKick
ADD_ACTION_CLASS(AttackMeleeLong)				// kAct_AttackMeleeLong
ADD_ACTION_CLASS(AttackMeleeUncloaked)			// kAct_AttackMeleeUncloaked
ADD_ACTION_CLASS(AttackReady)					// kAct_AttackReady
ADD_ACTION_CLASS(AttackTurret)					// kAct_AttackTurret
ADD_ACTION_CLASS(AttackTurretCeiling)			// kAct_AttackTurretCeiling
ADD_ACTION_CLASS(Berserker)						// kAct_Berserker
ADD_ACTION_CLASS(BerserkerRecoil)				// kAct_BerserkerRecoil
ADD_ACTION_CLASS(BerserkerRecoilDismount)		// kAct_BerserkerRecoilDismount
ADD_ACTION_CLASS(BlindFireFromCover)			// kAct_BlindFireFromCover
ADD_ACTION_CLASS(ChangePrimaryWeapon)			// kAct_ChangePrimaryWeapon
ADD_ACTION_CLASS(CounterMeleeBlock)				// kAct_CounterMeleeBlock
ADD_ACTION_CLASS(CounterMeleeDodge)				// kAct_CounterMeleeDodge
ADD_ACTION_CLASS(DeathAnimated)					// kAct_DeathAnimated
ADD_ACTION_CLASS(DeathOnVehicle)				// kAct_DeathOnVehicle
ADD_ACTION_CLASS(DeathOnVehicleUnanimated)		// kAct_DeathOnVehicleUnanimated
ADD_ACTION_CLASS(DefeatedRecoil)				// kAct_DefeatedRecoil
ADD_ACTION_CLASS(DefeatedTaserRecoil)			// kAct_DefeatedTaserRecoil
ADD_ACTION_CLASS(DismountNode)					// kAct_DismountNode
ADD_ACTION_CLASS(DismountNodeAggressive)		// kAct_DismountNodeAggressive
ADD_ACTION_CLASS(DismountNodeUncloaked)			// kAct_DismountNodeUncloaked
ADD_ACTION_CLASS(DismountPlayer)				// kAct_DismountPlayer
ADD_ACTION_CLASS(DismountVehicle)				// kAct_DismountVehicle
ADD_ACTION_CLASS(DodgeBackpedal)				// kAct_DodgeBackpedal
ADD_ACTION_CLASS(DodgeCovered)					// kAct_DodgeCovered
ADD_ACTION_CLASS(DodgeOnVehicle)				// kAct_DodgeOnVehicle
ADD_ACTION_CLASS(DodgeRoll)						// kAct_DodgeRoll
ADD_ACTION_CLASS(DodgeRollParanoid)				// kAct_DodgeRollParanoid
ADD_ACTION_CLASS(DodgeShuffle)					// kAct_DodgeShuffle
ADD_ACTION_CLASS(DrawWeapon)					// kAct_DrawWeapon
ADD_ACTION_CLASS(DropWeapon)					// kAct_DropWeapon
ADD_ACTION_CLASS(EscapeDanger)					// kAct_EscapeDanger
ADD_ACTION_CLASS(FaceNode)						// kAct_FaceNode
ADD_ACTION_CLASS(FinishBlockAttack)				// kAct_FinishBlockAttack
ADD_ACTION_CLASS(FinishBlockNormal)				// kAct_FinishBlockNormal
ADD_ACTION_CLASS(FinishingMoveReaction)			// kAct_FinishingMoveReaction
ADD_ACTION_CLASS(FlushOutWithGrenade)			// kAct_FlushOutWithGrenade
ADD_ACTION_CLASS(Follow)						// kAct_Follow
ADD_ACTION_CLASS(FollowHeavyArmor)				// kAct_FollowHeavyArmor
ADD_ACTION_CLASS(FollowPlayer)					// kAct_FollowPlayer
ADD_ACTION_CLASS(FollowWaitAtNode)				// kAct_FollowWaitAtNode
ADD_ACTION_CLASS(GetOutOfTheWay)				// kAct_GetOutOfTheWay
ADD_ACTION_CLASS(GotoNode)						// kAct_GotoNode
ADD_ACTION_CLASS(GotoNode3D)					// kAct_GotoNode3D
ADD_ACTION_CLASS(GotoNodeCovered)				// kAct_GotoNodeCovered
ADD_ACTION_CLASS(GotoNodeDirect)				// kAct_GotoNodeDirect
ADD_ACTION_CLASS(GotoNodeOfType)				// kAct_GotoNodeOfType
ADD_ACTION_CLASS(GotoTarget)					// kAct_GotoTarget
ADD_ACTION_CLASS(GotoTarget3D)					// kAct_GotoTarget3D
ADD_ACTION_CLASS(GotoTargetLost)				// kAct_GotoTargetLost
ADD_ACTION_CLASS(GotoFlamePotPosition)			// kAct_GotoFlamePotPosition
ADD_ACTION_CLASS(GotoValidPosition)				// kAct_GotoValidPosition
ADD_ACTION_CLASS(HolsterWeapon)					// kAct_HolsterWeapon
ADD_ACTION_CLASS(Idle)							// kAct_Idle
ADD_ACTION_CLASS(IdleCombat)					// kAct_IdleCombat
ADD_ACTION_CLASS(IdleFidget)					// kAct_IdleFidget
ADD_ACTION_CLASS(IdleOnVehicle)					// kAct_IdleOnVehicle
ADD_ACTION_CLASS(IdleTurret)					// kAct_IdleTurret
ADD_ACTION_CLASS(InspectDisturbance)			// kAct_InspectDisturbance
ADD_ACTION_CLASS(InstantDeath)					// kAct_InstantDeath
ADD_ACTION_CLASS(InstantDeathKnockedDown)		// kAct_InstantDeathKnockedDown
ADD_ACTION_CLASS(LookAtDisturbance)				// kAct_LookAtDisturbance
ADD_ACTION_CLASS(LookAtDisturbanceFromView)		// kAct_LookAtDisturbanceFromView
ADD_ACTION_CLASS(LongRecoil)					// kAct_LongRecoil
ADD_ACTION_CLASS(LongRecoilHelmetPiercing)		// kAct_LongRecoilHelmetPiercing
ADD_ACTION_CLASS(LopeToTargetUncloaked)			// kAct_LopeToTargetUncloaked
ADD_ACTION_CLASS(MeleeBlocked)					// kAct_MeleeBlocked
ADD_ACTION_CLASS(MountNode)						// kAct_MountNode
ADD_ACTION_CLASS(MountNodeUncloaked)			// kAct_MountNodeUncloaked
ADD_ACTION_CLASS(MountPlayer)					// kAct_MountPlayer
ADD_ACTION_CLASS(MountVehicle)					// kAct_MountVehicle
ADD_ACTION_CLASS(PickupWeaponInWorld)			// kAct_PickupWeaponInWorld
ADD_ACTION_CLASS(ReactToDanger)					// kAct_ReactToDanger
ADD_ACTION_CLASS(ReactToShove)					// kAct_ReactToShove
ADD_ACTION_CLASS(ReactToWeaponBroke)			// kAct_ReactToWeaponBroke
ADD_ACTION_CLASS(Reload)						// kAct_Reload
ADD_ACTION_CLASS(ReloadFromSafetyNode)			// kAct_ReloadFromSafetyNode
ADD_ACTION_CLASS(ReloadCovered)					// kAct_ReloadCovered
ADD_ACTION_CLASS(ReloadCrouch)					// kAct_ReloadCrouch
ADD_ACTION_CLASS(ShortRecoil)					// kAct_ShortRecoil
ADD_ACTION_CLASS(ShortRecoilOnVehicle)			// kAct_ShortRecoilOnVehicle
ADD_ACTION_CLASS(ShovePathBlocker)				// kAct_ShovePathBlocker
ADD_ACTION_CLASS(Stunned)						// kAct_Stunned
ADD_ACTION_CLASS(SuppressionFire)				// kAct_SuppressionFire
ADD_ACTION_CLASS(SuppressionFireFromCover)		// kAct_SuppressionFireFromCover
ADD_ACTION_CLASS(SurpriseAttackLaunch)			// kAct_SurpriseAttackLaunch
ADD_ACTION_CLASS(Surprised)						// kAct_Surprised
ADD_ACTION_CLASS(SurveyArea)					// kAct_SurveyArea
ADD_ACTION_CLASS(TaseredLevel1)					// kAct_TaseredLevel1
ADD_ACTION_CLASS(TaseredLevel2)					// kAct_TaseredLevel2
ADD_ACTION_CLASS(TaseredRecovery)				// kAct_TaseredRecovery
ADD_ACTION_CLASS(TraverseBlockedDoor)			// kAct_TraverseBlockedDoor
ADD_ACTION_CLASS(TraverseLink)					// kAct_TraverseLink
ADD_ACTION_CLASS(TraverseLinkUncloaked)			// kAct_TraverseLinkUncloaked
ADD_ACTION_CLASS(Uncover)						// kAct_Uncover
ADD_ACTION_CLASS(UseSmartObjectNode)			// kAct_UseSmartObjectNode
ADD_ACTION_CLASS(UseSmartObjectNodeMounted)		// kAct_UseSmartObjectNodeMounted
ADD_ACTION_CLASS(WaitForBlockedPath)			// kAct_WaitForBlockedPath
ADD_ACTION_CLASS(WaitForFollowerToCatchUp)		// kAct_WaitForFollowerToCatchUp

// Types.

ADD_ACTION_TYPE(AttackFromArmored)				// kAct_AttackFromArmored
ADD_ACTION_TYPE(AttackFromArmoredBounded)		// kAct_AttackFromArmoredBounded
ADD_ACTION_TYPE(AttackFromViewCombatOpportunity)// kAct_AttackFromViewCombatOpportunity
ADD_ACTION_TYPE(Charge)							// kAct_Charge
ADD_ACTION_TYPE(FollowPlayerFidget)				// kAct_FollowPlayerFidget
ADD_ACTION_TYPE(GotoTargetBerserker)			// kAct_GotoTargetBerserker
ADD_ACTION_TYPE(KnockDown)						// kAct_KnockDown
ADD_ACTION_TYPE(KnockDownBullet)				// kAct_KnockDownBullet
ADD_ACTION_TYPE(KnockDownExplosive)				// kAct_KnockDownExplosive
ADD_ACTION_TYPE(KnockDownMelee)					// kAct_KnockDownMelee
ADD_ACTION_TYPE(LongRecoilBullet)				// kAct_LongRecoilBullet
ADD_ACTION_TYPE(LongRecoilExplosive)			// kAct_LongRecoilExplosive
ADD_ACTION_TYPE(LongRecoilMelee)				// kAct_LongRecoilMelee
ADD_ACTION_TYPE(MountPlayerFailedRecoil)		// kAct_MountPlayerFailedRecoil
ADD_ACTION_TYPE(ShortRecoilMelee)				// kAct_ShortRecoilMelee
