###############################################################
#
# Makefile for Game_ServerShell.
#
# (c) 2006 Monolith Productions, Inc. All Rights Reserved.
#
###############################################################

# Build tools
ifeq "$(CPP_TOOL)" "" 
CPP_TOOL=g++
endif
ifeq "$(CC_TOOL)" ""
CC_TOOL=gcc
endif
ifeq "$(LIB_TOOL)" ""
LIB_TOOL=ar
endif
ifeq "$(EXE_TOOL)" ""
EXE_TOOL=g++
endif

# Project name
ProjectName=Game_ServerShell

# Set default configuration
ifeq "$(ConfigurationName)" ""
ConfigurationName = Debug
endif

# Platform-specific variables
LINUX_DEFINES=-D_LINUX -DDISABLE_LT_MEM_TRACKING -D_GLIBCPP_USE_WCHAR_T
HAVOK_INCLUDE_PATH=../../../libs/Havok231Linux/include
HAVOK_DEBUG_LIB_PATH=../../../libs/Havok231Linux/lib/unix-gcc/debug
HAVOK_RELEASE_LIB_PATH=../../../libs/Havok231Linux/lib/unix-gcc/release

# SourceFiles
SOURCEFILES=../Shared/ActivateTypeHandler.cpp \
		./ActiveWorldModel.cpp \
		./AI.cpp \
		./AIActionAbstract.cpp \
		./AIActionAnimate.cpp \
		./AIActionAttack.cpp \
		./AIActionAttackBurstLimited.cpp \
		./AIActionAttackCrouch.cpp \
		./AIActionAttackFromAmbush.cpp \
		./AIActionAttackFromCover.cpp \
		./AIActionAttackFromNode.cpp \
		./AIActionAttackFromNodeBounded.cpp \
		./AIActionAttackFromSafetyNodeFirePosition.cpp \
		./AIActionAttackFromVehicle.cpp \
		./AIActionAttackFromVehicleMelee.cpp \
		./AIActionAttackFromView.cpp \
		./AIActionAttackGrenade.cpp \
		./AIActionAttackGrenadeFromCover.cpp \
		./AIActionAttackLunge3D.cpp \
		./AIActionAttackLungeMelee.cpp \
		./AIActionAttackLungeSuicide.cpp \
		./AIActionAttackLungeUncloaked.cpp \
		./AIActionAttackMelee.cpp \
		./AIActionAttackMeleeFromDistance.cpp \
		./AIActionAttackMeleeFromRun.cpp \
		./AIActionAttackMeleeKick.cpp \
		./AIActionAttackMeleeLong.cpp \
		./AIActionAttackMeleeUncloaked.cpp \
		./AIActionAttackReady.cpp \
		./AIActionAttackTurret.cpp \
		./AIActionAttackTurretCeiling.cpp \
		./AIActionBerserker.cpp \
		./AIActionBerserkerRecoil.cpp \
		./AIActionBerserkerRecoilDismount.cpp \
		./AIActionBlindFireFromCover.cpp \
		./AIActionChangePrimaryWeapon.cpp \
		./AIActionCounterMeleeBlock.cpp \
		./AIActionCounterMeleeDodge.cpp \
		./AIActionDeathAnimated.cpp \
		./AIActionDeathOnVehicle.cpp \
		./AIActionDeathOnVehicleUnanimated.cpp \
		./AIActionDefeatedRecoil.cpp \
		./AIActionDefeatedTaserRecoil.cpp \
		./AIActionDismountNode.cpp \
		./AIActionDismountNodeAggressive.cpp \
		./AIActionDismountNodeUncloaked.cpp \
		./AIActionDismountPlayer.cpp \
		./AIActionDismountVehicle.cpp \
		./AIActionDodgeBackpedal.cpp \
		./AIActionDodgeCovered.cpp \
		./AIActionDodgeOnVehicle.cpp \
		./AIActionDodgeRoll.cpp \
		./AIActionDodgeRollParanoid.cpp \
		./AIActionDodgeShuffle.cpp \
		./AIActionDrawWeapon.cpp \
		./AIActionDropWeapon.cpp \
		./AIActionEscapeDanger.cpp \
		./AIActionFaceNode.cpp \
		./AIActionFinishBlockAttack.cpp \
		./AIActionFinishBlockNormal.cpp \
		./AIActionFinishingMoveReaction.cpp \
		./AIActionFlushOutWithGrenade.cpp \
		./AIActionFollow.cpp \
		./AIActionFollowHeavyArmor.cpp \
		./AIActionFollowPlayer.cpp \
		./AIActionFollowWaitAtNode.cpp \
		./AIActionGetOutOfTheWay.cpp \
		./AIActionGotoAbstract.cpp \
		./AIActionGotoFlamePotPosition.cpp \
		./AIActionGotoNode.cpp \
		./AIActionGotoNode3D.cpp \
		./AIActionGotoNodeCovered.cpp \
		./AIActionGotoNodeDirect.cpp \
		./AIActionGotoNodeOfType.cpp \
		./AIActionGotoTarget.cpp \
		./AIActionGotoTarget3D.cpp \
		./AIActionGotoTargetLost.cpp \
		./AIActionGotoValidPosition.cpp \
		./AIActionHolsterWeapon.cpp \
		./AIActionIdle.cpp \
		./AIActionIdleCombat.cpp \
		./AIActionIdleFidget.cpp \
		./AIActionIdleOnVehicle.cpp \
		./AIActionIdleTurret.cpp \
		./AIActionInspectDisturbance.cpp \
		./AIActionInstantDeath.cpp \
		./AIActionInstantDeathKnockedDown.cpp \
		./AIActionLongRecoil.cpp \
		./AIActionLongRecoilHelmetPiercing.cpp \
		./AIActionLookAtDisturbance.cpp \
		./AIActionLookAtDisturbanceFromView.cpp \
		./AIActionLopeToTargetUncloaked.cpp \
		./AIActionMeleeBlocked.cpp \
		./AIActionMgr.cpp \
		./AIActionMountNode.cpp \
		./AIActionMountNodeUncloaked.cpp \
		./AIActionMountPlayer.cpp \
		./AIActionMountVehicle.cpp \
		./AIActionPickupWeaponInWorld.cpp \
		./AIActionReactToDanger.cpp \
		./AIActionReactToShove.cpp \
		./AIActionReactToWeaponBroke.cpp \
		./AIActionReload.cpp \
		./AIActionReloadCovered.cpp \
		./AIActionReloadCrouch.cpp \
		./AIActionReloadFromSafetyNode.cpp \
		./AIActionShortRecoil.cpp \
		./AIActionShortRecoilOnVehicle.cpp \
		./AIActionShovePathBlocker.cpp \
		./AIActionStunned.cpp \
		./AIActionSuppressionFire.cpp \
		./AIActionSuppressionFireFromCover.cpp \
		./AIActionSurpriseAttackLaunch.cpp \
		./AIActionSurprised.cpp \
		./AIActionSurveyArea.cpp \
		./AIActionTaseredLevel1.cpp \
		./AIActionTaseredLevel2.cpp \
		./AIActionTaseredRecovery.cpp \
		./AIActionTraverseBlockedDoor.cpp \
		./AIActionTraverseLink.cpp \
		./AIActionTraverseLinkUncloaked.cpp \
		./AIActionUncover.cpp \
		./AIActionUseSmartObjectNode.cpp \
		./AIActionUseSmartObjectNodeMounted.cpp \
		./AIActionWaitForBlockedPath.cpp \
		./AIActionWaitForFollowerToCatchUp.cpp \
		./AIActivityAbstract.cpp \
		./AIActivityAdvanceCover.cpp \
		./AIActivityExchangeWeapons.cpp \
		./AIActivityGetToCover.cpp \
		./AIActivityOrderlyAdvance.cpp \
		./AIActivitySearch.cpp \
		./AIAssert.cpp \
		./AIAStarMachine.cpp \
		./AIAStarNavMesh.cpp \
		./AIAStarNavMeshEscape.cpp \
		./AIAStarNavMeshSafe.cpp \
		./AIAStarPlanner.cpp \
		./AIAStarStorageLinkedList.cpp \
		./AIBlackBoard.cpp \
		./AIBrain.cpp \
		./AICharacterTypeRestrictions.cpp \
		./AIClassFactory.cpp \
		./AICombatOpportunity.cpp \
		./AICombatOpportunityRadius.cpp \
		./AICommandMgr.cpp \
		./AIConfig.cpp \
		./AICoordinator.cpp \
		./AIDB.cpp \
		./AIEnumContexts.cpp \
		./AIGoalAbstract.cpp \
		./AIGoalAlert.cpp \
		./AIGoalAmbush.cpp \
		./AIGoalAnimate.cpp \
		./AIGoalAnimateInterrupt.cpp \
		./AIGoalBerserker.cpp \
		./AIGoalBlitzAndUseNode.cpp \
		./AIGoalBlitzCharacter.cpp \
		./AIGoalBlitzNode.cpp \
		./AIGoalChangePrimaryWeapon.cpp \
		./AIGoalCharge.cpp \
		./AIGoalChase.cpp \
		./AIGoalChaseBerserker.cpp \
		./AIGoalCircleFlamePot.cpp \
		./AIGoalCombatOpportunityAttack.cpp \
		./AIGoalCorrectPosition.cpp \
		./AIGoalCounterMelee.cpp \
		./AIGoalCover.cpp \
		./AIGoalCritterFlee.cpp \
		./AIGoalDeath.cpp \
		./AIGoalDefeated.cpp \
		./AIGoalDismountVehicle.cpp \
		./AIGoalDodge.cpp \
		./AIGoalEscapeDanger.cpp \
		./AIGoalExitLink.cpp \
		./AIGoalFallBack.cpp \
		./AIGoalFinishBlockFailure.cpp \
		./AIGoalFinishBlockSuccess.cpp \
		./AIGoalFlee.cpp \
		./AIGoalFlushOut.cpp \
		./AIGoalFlyAway.cpp \
		./AIGoalFollow.cpp \
		./AIGoalFollowHide.cpp \
		./AIGoalFollowWait.cpp \
		./AIGoalGoto.cpp \
		./AIGoalGuard.cpp \
		./AIGoalIdle.cpp \
		./AIGoalIntro.cpp \
		./AIGoalInvestigate.cpp \
		./AIGoalKillEnemy.cpp \
		./AIGoalLead.cpp \
		./AIGoalMenace.cpp \
		./AIGoalMgr.cpp \
		./AIGoalMountVehicle.cpp \
		./AIGoalPatrol.cpp \
		./AIGoalPickupWeapon.cpp \
		./AIGoalReactToBerserkerKick.cpp \
		./AIGoalReactToBlockedPath.cpp \
		./AIGoalReactToDamage.cpp \
		./AIGoalReactToFinishingMove.cpp \
		./AIGoalReactToFollowerFallingBehind.cpp \
		./AIGoalReactToMeleeBlocked.cpp \
		./AIGoalReactToShove.cpp \
		./AIGoalReactToWeaponBroke.cpp \
		./AIGoalRetreat.cpp \
		./AIGoalRetreatLimited.cpp \
		./AIGoalScan.cpp \
		./AIGoalSearch.cpp \
		./AIGoalSearchFollow.cpp \
		./AIGoalSearchLostTarget.cpp \
		./AIGoalSelfDefense.cpp \
		./AIGoalStalk.cpp \
		./AIGoalStunned.cpp \
		./AIGoalSuppressEnemy.cpp \
		./AIGoalSurpriseAttackLaunch.cpp \
		./AIGoalTraverseLink.cpp \
		./AIGoalUseArmoredAutonomous.cpp \
		./AIGoalUseSmartObject.cpp \
		./AIGoalUseSmartObjectCombat.cpp \
		./AIGoalWork.cpp \
		./AIGoalWorkTargetless.cpp \
		./AIMgr.cpp \
		./AIMovement.cpp \
		./AIMovementUtils.cpp \
		./AINavigationMgr.cpp \
		./AINavMesh.cpp \
		./AINavMeshGen.cpp \
		./AINavMeshGenCarver.cpp \
		./AINavMeshGenImportExportGame.cpp \
		./AINavMeshGenPoly.cpp \
		./AINavMeshGenQuadTree.cpp \
		./AINavMeshGenTypes.cpp \
		./AINavMeshLink.cpp \
		./AINavMeshLinkAbstract.cpp \
		./AINavMeshLinkAbstractOneAnim.cpp \
		./AINavMeshLinkClimb.cpp \
		./AINavMeshLinkCrawl.cpp \
		./AINavMeshLinkDiveThru.cpp \
		./AINavMeshLinkDoor.cpp \
		./AINavMeshLinkDuckRun.cpp \
		./AINavMeshLinkDuckUnder.cpp \
		./AINavMeshLinkExit.cpp \
		./AINavMeshLinkFlamePot.cpp \
		./AINavMeshLinkFlyThru.cpp \
		./AINavMeshLinkJump.cpp \
		./AINavMeshLinkJumpOver.cpp \
		./AINavMeshLinkLoseTarget.cpp \
		./AINavMeshLinkPlayer.cpp \
		./AINavMeshLinkStairs.cpp \
		./AINode.cpp \
		./AINodeArmored.cpp \
		./AINodeCluster.cpp \
		./AINodeCombatOpportunity.cpp \
		./AINodeCombatOpportunityView.cpp \
		./AINodeDarkChant.cpp \
		./AINodeDarkWait.cpp \
		./AINodeFallBack.cpp \
		./AINodeFollow.cpp \
		./AINodeGuard.cpp \
		./AINodeHide.cpp \
		./AINodeInterest.cpp \
		./AINodeIntro.cpp \
		./AINodeLead.cpp \
		./AINodeMgr.cpp \
		./AINodePickupWeapon.cpp \
		./AINodeSafety.cpp \
		./AINodeSafetyFirePosition.cpp \
		./AINodeScanner.cpp \
		./AINodeStalk.cpp \
		./AINodeStimulus.cpp \
		./AINodeSurprise.cpp \
		./AINodeTrackerContext.cpp \
		./AINodeTypes.cpp \
		./AINodeValidators.cpp \
		./AIPathKnowledgeMgr.cpp \
		./AIPathMgrNavMesh.cpp \
		./AIPlanner.cpp \
		./AIQuadTree.cpp \
		./AIRegion.cpp \
		./AISensorAbstract.cpp \
		./AISensorAbstractStimulatable.cpp \
		./AISensorAwarenessMod.cpp \
		./AISensorBerserker.cpp \
		./AISensorCloak.cpp \
		./AISensorCombatOpportunity.cpp \
		./AISensorCoolMove.cpp \
		./AISensorCounterMelee.cpp \
		./AISensorCritter.cpp \
		./AISensorDamage.cpp \
		./AISensorDamageFX.cpp \
		./AISensorDamageTurret.cpp \
		./AISensorEnemyDamaged.cpp \
		./AISensorEnemyDamagedSuicide.cpp \
		./AISensorFlamePot.cpp \
		./AISensorFlashlight.cpp \
		./AISensorFlyThru.cpp \
		./AISensorGuard.cpp \
		./AISensorHearDisturbance.cpp \
		./AISensorInstantDeath.cpp \
		./AISensorLocation.cpp \
		./AISensorLostTarget.cpp \
		./AISensorLunge.cpp \
		./AISensorMgr.cpp \
		./AISensorNode.cpp \
		./AISensorNodeCombat.cpp \
		./AISensorNodeFollow.cpp \
		./AISensorNodeGuarded.cpp \
		./AISensorNodeInterest.cpp \
		./AISensorNodeSafetyFirePosition.cpp \
		./AISensorNodeSearch.cpp \
		./AISensorNodeStalk.cpp \
		./AISensorPassTarget.cpp \
		./AISensorPatrol.cpp \
		./AISensorPersonalBubble.cpp \
		./AISensorPlayerTooClose.cpp \
		./AISensorPulse.cpp \
		./AISensorScanner.cpp \
		./AISensorSeeDanger.cpp \
		./AISensorSeeDisturbance.cpp \
		./AISensorSeeEnemy.cpp \
		./AISensorSeeEnemyNoFOV.cpp \
		./AISensorSeeEnemyNoShootThru.cpp \
		./AISensorSeeFlashlightBeam.cpp \
		./AISensorSeekEnemy.cpp \
		./AISensorShoved.cpp \
		./AISensorSlowMo.cpp \
		./AISensorSquadCommunicationThreat.cpp \
		./AISensorStatusCheck.cpp \
		./AISensorTargetIsAimingAtMe.cpp \
		./AISensorTargetIsAimingAtMeMelee.cpp \
		./AISensorTargetIsAimingAtMeParanoid.cpp \
		./AISensorTargetIsAwareOfMyPosition.cpp \
		./AISensorTouch.cpp \
		./AISensorTraitor.cpp \
		./AISensorValidPosition.cpp \
		./AISensorView.cpp \
		./AISensorWeaponItem.cpp \
		./AISoundDB.cpp \
		./AISoundMgr.cpp \
		./AISounds.cpp \
		./AISpawner.cpp \
		./AISquad.cpp \
		./AIState.cpp \
		./AIStateAnimate.cpp \
		./AIStateGoto.cpp \
		./AIStateUseSmartObject.cpp \
		./AIStimulusMgr.cpp \
		./AIStreamSim.cpp \
		./AITarget.cpp \
		./AITargetSelectAbstract.cpp \
		./AITargetSelectAllyDisturbance.cpp \
		./AITargetSelectBerserker.cpp \
		./AITargetSelectCharacter.cpp \
		./AITargetSelectCharacterOnlyOne.cpp \
		./AITargetSelectCharacterSquad.cpp \
		./AITargetSelectCharacterUrgent.cpp \
		./AITargetSelectCharacterUrgentSpatial.cpp \
		./AITargetSelectCombatOpportunity.cpp \
		./AITargetSelectDamager.cpp \
		./AITargetSelectDisturbance.cpp \
		./AITargetSelectDisturbanceBeyondGuard.cpp \
		./AITargetSelectDisturbanceSource.cpp \
		./AITargetSelectDisturbanceSquad.cpp \
		./AITargetSelectDisturbanceUrgent.cpp \
		./AITargetSelectFollower.cpp \
		./AITargetSelectInterest.cpp \
		./AITargetSelectLeader.cpp \
		./AITargetSelectMgr.cpp \
		./AITargetSelectPlayerTurret.cpp \
		./AITargetSelectScripted.cpp \
		./AITargetSelectTraitor.cpp \
		./AITargetSelectWeaponItem.cpp \
		./AIUtils.cpp \
		./AIWeaponAbstract.cpp \
		./AIWeaponMelee.cpp \
		./AIWeaponMgr.cpp \
		./AIWeaponRanged.cpp \
		./AIWeaponThrown.cpp \
		./AIWeaponUtils.cpp \
		./AIWorkingMemory.cpp \
		./AIWorkingMemoryCentral.cpp \
		./AIWorldState.cpp \
		./Alarm.cpp \
		./AmmoBox.cpp \
		./AnimatorPlayer.cpp \
		./Arsenal.cpp \
		../Shared/AttachmentDB.cpp \
		./Attachments.cpp \
		./AutoAimMagnet.cpp \
		./BanIPMgr.cpp \
		./BanUserMgr.cpp \
		./Camera.cpp \
		./Character.cpp \
		../Shared/CharacterAlignment.cpp \
		./CharacterHitBox.cpp \
		./CharacterMgr.cpp \
		./ClientWeaponSFX.cpp \
		./CommandDB.cpp \
		./CommandMgr.cpp \
		./CommandObject.cpp \
		./ConstraintBase.cpp \
		./ConstraintHinge.cpp \
		./ConstraintLimitedHinge.cpp \
		./ConstraintPoint.cpp \
		./ConstraintPrismatic.cpp \
		./ConstraintRagdoll.cpp \
		./ConstraintStiffSpring.cpp \
		./ConstraintWheel.cpp \
		./Controller.cpp \
		./ControlPoint.cpp \
		./CTFFlag.cpp \
		./CTFFlagBase.cpp \
		./DamageTracker.cpp \
		../Shared/DebugLine.cpp \
		./DebugLineSystem.cpp \
		./DecisionObject.cpp \
		./DEditHook.cpp \
		./Destructible.cpp \
		./DestructibleModel.cpp \
		./DestructibleWorldModel.cpp \
		./Dialogue.cpp \
		./DisplayMeter.cpp \
		./DisplayTimer.cpp \
		./Door.cpp \
		./DynamicSectorVolume.cpp \
		./EntryToolLock.cpp \
		./EventCounter.cpp \
		./Explosion.cpp \
		./Flicker.cpp \
		../Shared/ForceVolume.cpp \
		./ForensicObject.cpp \
		../Shared/GameAlloc.cpp \
		./GameBase.cpp \
		./GameServerShell.cpp \
		./GameStartPoint.cpp \
		./GameStartPointMgr.cpp \
		./GameWorldEditImpl.cpp \
		./GameWorldPackerImpl.cpp \
		./GearItems.cpp \
		./GlobalServerMgr.cpp \
		./GlobalsInit.cpp \
		./Group.cpp \
		./GunMount.cpp \
		./HHWeaponModel.cpp \
		./JumpVolume.cpp \
		./Key.cpp \
		./KeyFramer.cpp \
		./KeyframeToRigidBody.cpp \
		./Ladder.cpp \
		./LightBase.cpp \
		./LightCube.cpp \
		./LightDirectional.cpp \
		./LightEditor.cpp \
		./LightPoint.cpp \
		./LightPointFill.cpp \
		./LightSpot.cpp \
		./LookAtTrigger.cpp \
		../../Engine/sdk/inc/ltengineobjects.cpp \
		../../Engine/sdk/inc/ltmemhook.cpp \
		../../Engine/sdk/inc/ltmodule.cpp \
		../../Engine/sdk/inc/ltobjref.cpp \
		../Shared/MissionDB.cpp \
		./NamedObjectList.cpp \
		./NavMarker.cpp \
		./NoPlayerTrigger.cpp \
		./ObjectRemover.cpp \
		./ObjectTemplateMgr.cpp \
		./ObjectTransformHistory.cpp \
		../../Engine/sdk/inc/performancemonitorhook.cpp \
		./PhysicsCollisionSystem.cpp \
		./PhysicsImpulseDirectional.cpp \
		./PhysicsImpulseRadial.cpp \
		./PickupItem.cpp \
		./PlayerInventory.cpp \
		./PlayerLeash.cpp \
		./PlayerLure.cpp \
		./PlayerNodeGoto.cpp \
		./PlayerObj.cpp \
		./PlayerTracker.cpp \
		./PlayerTrigger.cpp \
		./Point.cpp \
		./PolyGrid.cpp \
		./PolyGridModifier.cpp \
		./PositionalDamage.cpp \
		./PrefetchUtilities.cpp \
		./Projectile.cpp \
		./ProjectileTypes.cpp \
		./Prop.cpp \
		./RefillStation.cpp \
		./RemoteTurret.cpp \
		./RenderTarget.cpp \
		./RenderTargetGroup.cpp \
		./RotatingDoor.cpp \
		./RotatingSwitch.cpp \
		./RotatingWorldModel.cpp \
		./ScaleAlpha.cpp \
		./ScatterVolume.cpp \
		./ScmdConsoleDriver_PunkBuster.cpp \
		./ScmdServer.cpp \
		./ScreenEffect.cpp \
		./ServerConnectionMgr.cpp \
		./ServerDB.cpp \
		./ServerMeleeCollisionController.cpp \
		./ServerMissionMgr.cpp \
		./ServerNodeTrackerContext.cpp \
		./ServerNonPointSound.cpp \
		./ServerPhysicsCollisionMgr.cpp \
		./ServerSaveLoadMgr.cpp \
		./ServerSoundFX.cpp \
		./ServerSoundMgr.cpp \
		./ServerSoundZoneVolume.cpp \
		./ServerSpecialFX.cpp \
		./ServerUtilities.cpp \
		./ServerVoteMgr.cpp \
		./SlidingDoor.cpp \
		./SlidingSwitch.cpp \
		./SlidingWorldModel.cpp \
		./SnowVolume.cpp \
		./SonicIntoneHandlers.cpp \
		./SoundButeFX.cpp \
		../Shared/SoundDB.cpp \
		../Shared/SoundFilterDB.cpp \
		../Shared/SoundMgr.cpp \
		../Shared/SoundOcclusionDB.cpp \
		./Spawner.cpp \
		./Speaker.cpp \
		./SpecialMove.cpp \
		./SpinningWorldModel.cpp \
		./StairVolume.cpp \
		./StartupCommand.cpp \
		./Stdafx.cpp \
		./StoryMode.cpp \
		./StringEditMgrPlugin.cpp \
		../Shared/SurfaceDB.cpp \
		../Shared/SurfaceFunctions.cpp \
		./Switch.cpp \
		./TeamBalancer.cpp \
		./TeamClientFX.cpp \
		./TeleportPoint.cpp \
		./TransitionAggregate.cpp \
		./TransitionArea.cpp \
		./TransitionMgr.cpp \
		./Trigger.cpp \
		../Shared/TriggerTypeDB.cpp \
		./Turret.cpp \
		./VolumeBrush.cpp \
		./VolumeBrushTypes.cpp \
		./VolumeEffect.cpp \
		./Weapon.cpp \
		./WeaponItems.cpp \
		./WorldModel.cpp \
		./WorldProperties.cpp \
		
# Object files
OBJECTFILES=$(IntDir)/ActivateTypeHandler.o \
		$(IntDir)/ActiveWorldModel.o \
		$(IntDir)/AI.o \
		$(IntDir)/AIActionAbstract.o \
		$(IntDir)/AIActionAnimate.o \
		$(IntDir)/AIActionAttack.o \
		$(IntDir)/AIActionAttackBurstLimited.o \
		$(IntDir)/AIActionAttackCrouch.o \
		$(IntDir)/AIActionAttackFromAmbush.o \
		$(IntDir)/AIActionAttackFromCover.o \
		$(IntDir)/AIActionAttackFromNode.o \
		$(IntDir)/AIActionAttackFromNodeBounded.o \
		$(IntDir)/AIActionAttackFromSafetyNodeFirePosition.o \
		$(IntDir)/AIActionAttackFromVehicle.o \
		$(IntDir)/AIActionAttackFromVehicleMelee.o \
		$(IntDir)/AIActionAttackFromView.o \
		$(IntDir)/AIActionAttackGrenade.o \
		$(IntDir)/AIActionAttackGrenadeFromCover.o \
		$(IntDir)/AIActionAttackLunge3D.o \
		$(IntDir)/AIActionAttackLungeMelee.o \
		$(IntDir)/AIActionAttackLungeSuicide.o \
		$(IntDir)/AIActionAttackLungeUncloaked.o \
		$(IntDir)/AIActionAttackMelee.o \
		$(IntDir)/AIActionAttackMeleeFromDistance.o \
		$(IntDir)/AIActionAttackMeleeFromRun.o \
		$(IntDir)/AIActionAttackMeleeKick.o \
		$(IntDir)/AIActionAttackMeleeLong.o \
		$(IntDir)/AIActionAttackMeleeUncloaked.o \
		$(IntDir)/AIActionAttackReady.o \
		$(IntDir)/AIActionAttackTurret.o \
		$(IntDir)/AIActionAttackTurretCeiling.o \
		$(IntDir)/AIActionBerserker.o \
		$(IntDir)/AIActionBerserkerRecoil.o \
		$(IntDir)/AIActionBerserkerRecoilDismount.o \
		$(IntDir)/AIActionBlindFireFromCover.o \
		$(IntDir)/AIActionChangePrimaryWeapon.o \
		$(IntDir)/AIActionCounterMeleeBlock.o \
		$(IntDir)/AIActionCounterMeleeDodge.o \
		$(IntDir)/AIActionDeathAnimated.o \
		$(IntDir)/AIActionDeathOnVehicle.o \
		$(IntDir)/AIActionDeathOnVehicleUnanimated.o \
		$(IntDir)/AIActionDefeatedRecoil.o \
		$(IntDir)/AIActionDefeatedTaserRecoil.o \
		$(IntDir)/AIActionDismountNode.o \
		$(IntDir)/AIActionDismountNodeAggressive.o \
		$(IntDir)/AIActionDismountNodeUncloaked.o \
		$(IntDir)/AIActionDismountPlayer.o \
		$(IntDir)/AIActionDismountVehicle.o \
		$(IntDir)/AIActionDodgeBackpedal.o \
		$(IntDir)/AIActionDodgeCovered.o \
		$(IntDir)/AIActionDodgeOnVehicle.o \
		$(IntDir)/AIActionDodgeRoll.o \
		$(IntDir)/AIActionDodgeRollParanoid.o \
		$(IntDir)/AIActionDodgeShuffle.o \
		$(IntDir)/AIActionDrawWeapon.o \
		$(IntDir)/AIActionDropWeapon.o \
		$(IntDir)/AIActionEscapeDanger.o \
		$(IntDir)/AIActionFaceNode.o \
		$(IntDir)/AIActionFinishBlockAttack.o \
		$(IntDir)/AIActionFinishBlockNormal.o \
		$(IntDir)/AIActionFinishingMoveReaction.o \
		$(IntDir)/AIActionFlushOutWithGrenade.o \
		$(IntDir)/AIActionFollow.o \
		$(IntDir)/AIActionFollowHeavyArmor.o \
		$(IntDir)/AIActionFollowPlayer.o \
		$(IntDir)/AIActionFollowWaitAtNode.o \
		$(IntDir)/AIActionGetOutOfTheWay.o \
		$(IntDir)/AIActionGotoAbstract.o \
		$(IntDir)/AIActionGotoFlamePotPosition.o \
		$(IntDir)/AIActionGotoNode.o \
		$(IntDir)/AIActionGotoNode3D.o \
		$(IntDir)/AIActionGotoNodeCovered.o \
		$(IntDir)/AIActionGotoNodeDirect.o \
		$(IntDir)/AIActionGotoNodeOfType.o \
		$(IntDir)/AIActionGotoTarget.o \
		$(IntDir)/AIActionGotoTarget3D.o \
		$(IntDir)/AIActionGotoTargetLost.o \
		$(IntDir)/AIActionGotoValidPosition.o \
		$(IntDir)/AIActionHolsterWeapon.o \
		$(IntDir)/AIActionIdle.o \
		$(IntDir)/AIActionIdleCombat.o \
		$(IntDir)/AIActionIdleFidget.o \
		$(IntDir)/AIActionIdleOnVehicle.o \
		$(IntDir)/AIActionIdleTurret.o \
		$(IntDir)/AIActionInspectDisturbance.o \
		$(IntDir)/AIActionInstantDeath.o \
		$(IntDir)/AIActionInstantDeathKnockedDown.o \
		$(IntDir)/AIActionLongRecoil.o \
		$(IntDir)/AIActionLongRecoilHelmetPiercing.o \
		$(IntDir)/AIActionLookAtDisturbance.o \
		$(IntDir)/AIActionLookAtDisturbanceFromView.o \
		$(IntDir)/AIActionLopeToTargetUncloaked.o \
		$(IntDir)/AIActionMeleeBlocked.o \
		$(IntDir)/AIActionMgr.o \
		$(IntDir)/AIActionMountNode.o \
		$(IntDir)/AIActionMountNodeUncloaked.o \
		$(IntDir)/AIActionMountPlayer.o \
		$(IntDir)/AIActionMountVehicle.o \
		$(IntDir)/AIActionPickupWeaponInWorld.o \
		$(IntDir)/AIActionReactToDanger.o \
		$(IntDir)/AIActionReactToShove.o \
		$(IntDir)/AIActionReactToWeaponBroke.o \
		$(IntDir)/AIActionReload.o \
		$(IntDir)/AIActionReloadCovered.o \
		$(IntDir)/AIActionReloadCrouch.o \
		$(IntDir)/AIActionReloadFromSafetyNode.o \
		$(IntDir)/AIActionShortRecoil.o \
		$(IntDir)/AIActionShortRecoilOnVehicle.o \
		$(IntDir)/AIActionShovePathBlocker.o \
		$(IntDir)/AIActionStunned.o \
		$(IntDir)/AIActionSuppressionFire.o \
		$(IntDir)/AIActionSuppressionFireFromCover.o \
		$(IntDir)/AIActionSurpriseAttackLaunch.o \
		$(IntDir)/AIActionSurprised.o \
		$(IntDir)/AIActionSurveyArea.o \
		$(IntDir)/AIActionTaseredLevel1.o \
		$(IntDir)/AIActionTaseredLevel2.o \
		$(IntDir)/AIActionTaseredRecovery.o \
		$(IntDir)/AIActionTraverseBlockedDoor.o \
		$(IntDir)/AIActionTraverseLink.o \
		$(IntDir)/AIActionTraverseLinkUncloaked.o \
		$(IntDir)/AIActionUncover.o \
		$(IntDir)/AIActionUseSmartObjectNode.o \
		$(IntDir)/AIActionUseSmartObjectNodeMounted.o \
		$(IntDir)/AIActionWaitForBlockedPath.o \
		$(IntDir)/AIActionWaitForFollowerToCatchUp.o \
		$(IntDir)/AIActivityAbstract.o \
		$(IntDir)/AIActivityAdvanceCover.o \
		$(IntDir)/AIActivityExchangeWeapons.o \
		$(IntDir)/AIActivityGetToCover.o \
		$(IntDir)/AIActivityOrderlyAdvance.o \
		$(IntDir)/AIActivitySearch.o \
		$(IntDir)/AIAssert.o \
		$(IntDir)/AIAStarMachine.o \
		$(IntDir)/AIAStarNavMesh.o \
		$(IntDir)/AIAStarNavMeshEscape.o \
		$(IntDir)/AIAStarNavMeshSafe.o \
		$(IntDir)/AIAStarPlanner.o \
		$(IntDir)/AIAStarStorageLinkedList.o \
		$(IntDir)/AIBlackBoard.o \
		$(IntDir)/AIBrain.o \
		$(IntDir)/AICharacterTypeRestrictions.o \
		$(IntDir)/AIClassFactory.o \
		$(IntDir)/AICombatOpportunity.o \
		$(IntDir)/AICombatOpportunityRadius.o \
		$(IntDir)/AICommandMgr.o \
		$(IntDir)/AIConfig.o \
		$(IntDir)/AICoordinator.o \
		$(IntDir)/AIDB.o \
		$(IntDir)/AIEnumContexts.o \
		$(IntDir)/AIGoalAbstract.o \
		$(IntDir)/AIGoalAlert.o \
		$(IntDir)/AIGoalAmbush.o \
		$(IntDir)/AIGoalAnimate.o \
		$(IntDir)/AIGoalAnimateInterrupt.o \
		$(IntDir)/AIGoalBerserker.o \
		$(IntDir)/AIGoalBlitzAndUseNode.o \
		$(IntDir)/AIGoalBlitzCharacter.o \
		$(IntDir)/AIGoalBlitzNode.o \
		$(IntDir)/AIGoalChangePrimaryWeapon.o \
		$(IntDir)/AIGoalCharge.o \
		$(IntDir)/AIGoalChase.o \
		$(IntDir)/AIGoalChaseBerserker.o \
		$(IntDir)/AIGoalCircleFlamePot.o \
		$(IntDir)/AIGoalCombatOpportunityAttack.o \
		$(IntDir)/AIGoalCorrectPosition.o \
		$(IntDir)/AIGoalCounterMelee.o \
		$(IntDir)/AIGoalCover.o \
		$(IntDir)/AIGoalCritterFlee.o \
		$(IntDir)/AIGoalDeath.o \
		$(IntDir)/AIGoalDefeated.o \
		$(IntDir)/AIGoalDismountVehicle.o \
		$(IntDir)/AIGoalDodge.o \
		$(IntDir)/AIGoalEscapeDanger.o \
		$(IntDir)/AIGoalExitLink.o \
		$(IntDir)/AIGoalFallBack.o \
		$(IntDir)/AIGoalFinishBlockFailure.o \
		$(IntDir)/AIGoalFinishBlockSuccess.o \
		$(IntDir)/AIGoalFlee.o \
		$(IntDir)/AIGoalFlushOut.o \
		$(IntDir)/AIGoalFlyAway.o \
		$(IntDir)/AIGoalFollow.o \
		$(IntDir)/AIGoalFollowHide.o \
		$(IntDir)/AIGoalFollowWait.o \
		$(IntDir)/AIGoalGoto.o \
		$(IntDir)/AIGoalGuard.o \
		$(IntDir)/AIGoalIdle.o \
		$(IntDir)/AIGoalIntro.o \
		$(IntDir)/AIGoalInvestigate.o \
		$(IntDir)/AIGoalKillEnemy.o \
		$(IntDir)/AIGoalLead.o \
		$(IntDir)/AIGoalMenace.o \
		$(IntDir)/AIGoalMgr.o \
		$(IntDir)/AIGoalMountVehicle.o \
		$(IntDir)/AIGoalPatrol.o \
		$(IntDir)/AIGoalPickupWeapon.o \
		$(IntDir)/AIGoalReactToBerserkerKick.o \
		$(IntDir)/AIGoalReactToBlockedPath.o \
		$(IntDir)/AIGoalReactToDamage.o \
		$(IntDir)/AIGoalReactToFinishingMove.o \
		$(IntDir)/AIGoalReactToFollowerFallingBehind.o \
		$(IntDir)/AIGoalReactToMeleeBlocked.o \
		$(IntDir)/AIGoalReactToShove.o \
		$(IntDir)/AIGoalReactToWeaponBroke.o \
		$(IntDir)/AIGoalRetreat.o \
		$(IntDir)/AIGoalRetreatLimited.o \
		$(IntDir)/AIGoalScan.o \
		$(IntDir)/AIGoalSearch.o \
		$(IntDir)/AIGoalSearchFollow.o \
		$(IntDir)/AIGoalSearchLostTarget.o \
		$(IntDir)/AIGoalSelfDefense.o \
		$(IntDir)/AIGoalStalk.o \
		$(IntDir)/AIGoalStunned.o \
		$(IntDir)/AIGoalSuppressEnemy.o \
		$(IntDir)/AIGoalSurpriseAttackLaunch.o \
		$(IntDir)/AIGoalTraverseLink.o \
		$(IntDir)/AIGoalUseArmoredAutonomous.o \
		$(IntDir)/AIGoalUseSmartObject.o \
		$(IntDir)/AIGoalUseSmartObjectCombat.o \
		$(IntDir)/AIGoalWork.o \
		$(IntDir)/AIGoalWorkTargetless.o \
		$(IntDir)/AIMgr.o \
		$(IntDir)/AIMovement.o \
		$(IntDir)/AIMovementUtils.o \
		$(IntDir)/AINavigationMgr.o \
		$(IntDir)/AINavMesh.o \
		$(IntDir)/AINavMeshGen.o \
		$(IntDir)/AINavMeshGenCarver.o \
		$(IntDir)/AINavMeshGenImportExportGame.o \
		$(IntDir)/AINavMeshGenPoly.o \
		$(IntDir)/AINavMeshGenQuadTree.o \
		$(IntDir)/AINavMeshGenTypes.o \
		$(IntDir)/AINavMeshLink.o \
		$(IntDir)/AINavMeshLinkAbstract.o \
		$(IntDir)/AINavMeshLinkAbstractOneAnim.o \
		$(IntDir)/AINavMeshLinkClimb.o \
		$(IntDir)/AINavMeshLinkCrawl.o \
		$(IntDir)/AINavMeshLinkDiveThru.o \
		$(IntDir)/AINavMeshLinkDoor.o \
		$(IntDir)/AINavMeshLinkDuckRun.o \
		$(IntDir)/AINavMeshLinkDuckUnder.o \
		$(IntDir)/AINavMeshLinkExit.o \
		$(IntDir)/AINavMeshLinkFlamePot.o \
		$(IntDir)/AINavMeshLinkFlyThru.o \
		$(IntDir)/AINavMeshLinkJump.o \
		$(IntDir)/AINavMeshLinkJumpOver.o \
		$(IntDir)/AINavMeshLinkLoseTarget.o \
		$(IntDir)/AINavMeshLinkPlayer.o \
		$(IntDir)/AINavMeshLinkStairs.o \
		$(IntDir)/AINode.o \
		$(IntDir)/AINodeArmored.o \
		$(IntDir)/AINodeCluster.o \
		$(IntDir)/AINodeCombatOpportunity.o \
		$(IntDir)/AINodeCombatOpportunityView.o \
		$(IntDir)/AINodeDarkChant.o \
		$(IntDir)/AINodeDarkWait.o \
		$(IntDir)/AINodeFallBack.o \
		$(IntDir)/AINodeFollow.o \
		$(IntDir)/AINodeGuard.o \
		$(IntDir)/AINodeHide.o \
		$(IntDir)/AINodeInterest.o \
		$(IntDir)/AINodeIntro.o \
		$(IntDir)/AINodeLead.o \
		$(IntDir)/AINodeMgr.o \
		$(IntDir)/AINodePickupWeapon.o \
		$(IntDir)/AINodeSafety.o \
		$(IntDir)/AINodeSafetyFirePosition.o \
		$(IntDir)/AINodeScanner.o \
		$(IntDir)/AINodeStalk.o \
		$(IntDir)/AINodeStimulus.o \
		$(IntDir)/AINodeSurprise.o \
		$(IntDir)/AINodeTrackerContext.o \
		$(IntDir)/AINodeTypes.o \
		$(IntDir)/AINodeValidators.o \
		$(IntDir)/AIPathKnowledgeMgr.o \
		$(IntDir)/AIPathMgrNavMesh.o \
		$(IntDir)/AIPlanner.o \
		$(IntDir)/AIQuadTree.o \
		$(IntDir)/AIRegion.o \
		$(IntDir)/AISensorAbstract.o \
		$(IntDir)/AISensorAbstractStimulatable.o \
		$(IntDir)/AISensorAwarenessMod.o \
		$(IntDir)/AISensorBerserker.o \
		$(IntDir)/AISensorCloak.o \
		$(IntDir)/AISensorCombatOpportunity.o \
		$(IntDir)/AISensorCoolMove.o \
		$(IntDir)/AISensorCounterMelee.o \
		$(IntDir)/AISensorCritter.o \
		$(IntDir)/AISensorDamage.o \
		$(IntDir)/AISensorDamageFX.o \
		$(IntDir)/AISensorDamageTurret.o \
		$(IntDir)/AISensorEnemyDamaged.o \
		$(IntDir)/AISensorEnemyDamagedSuicide.o \
		$(IntDir)/AISensorFlamePot.o \
		$(IntDir)/AISensorFlashlight.o \
		$(IntDir)/AISensorFlyThru.o \
		$(IntDir)/AISensorGuard.o \
		$(IntDir)/AISensorHearDisturbance.o \
		$(IntDir)/AISensorInstantDeath.o \
		$(IntDir)/AISensorLocation.o \
		$(IntDir)/AISensorLostTarget.o \
		$(IntDir)/AISensorLunge.o \
		$(IntDir)/AISensorMgr.o \
		$(IntDir)/AISensorNode.o \
		$(IntDir)/AISensorNodeCombat.o \
		$(IntDir)/AISensorNodeFollow.o \
		$(IntDir)/AISensorNodeGuarded.o \
		$(IntDir)/AISensorNodeInterest.o \
		$(IntDir)/AISensorNodeSafetyFirePosition.o \
		$(IntDir)/AISensorNodeSearch.o \
		$(IntDir)/AISensorNodeStalk.o \
		$(IntDir)/AISensorPassTarget.o \
		$(IntDir)/AISensorPatrol.o \
		$(IntDir)/AISensorPersonalBubble.o \
		$(IntDir)/AISensorPlayerTooClose.o \
		$(IntDir)/AISensorPulse.o \
		$(IntDir)/AISensorScanner.o \
		$(IntDir)/AISensorSeeDanger.o \
		$(IntDir)/AISensorSeeDisturbance.o \
		$(IntDir)/AISensorSeeEnemy.o \
		$(IntDir)/AISensorSeeEnemyNoFOV.o \
		$(IntDir)/AISensorSeeEnemyNoShootThru.o \
		$(IntDir)/AISensorSeeFlashlightBeam.o \
		$(IntDir)/AISensorSeekEnemy.o \
		$(IntDir)/AISensorShoved.o \
		$(IntDir)/AISensorSlowMo.o \
		$(IntDir)/AISensorSquadCommunicationThreat.o \
		$(IntDir)/AISensorStatusCheck.o \
		$(IntDir)/AISensorTargetIsAimingAtMe.o \
		$(IntDir)/AISensorTargetIsAimingAtMeMelee.o \
		$(IntDir)/AISensorTargetIsAimingAtMeParanoid.o \
		$(IntDir)/AISensorTargetIsAwareOfMyPosition.o \
		$(IntDir)/AISensorTouch.o \
		$(IntDir)/AISensorTraitor.o \
		$(IntDir)/AISensorValidPosition.o \
		$(IntDir)/AISensorView.o \
		$(IntDir)/AISensorWeaponItem.o \
		$(IntDir)/AISoundDB.o \
		$(IntDir)/AISoundMgr.o \
		$(IntDir)/AISounds.o \
		$(IntDir)/AISpawner.o \
		$(IntDir)/AISquad.o \
		$(IntDir)/AIState.o \
		$(IntDir)/AIStateAnimate.o \
		$(IntDir)/AIStateGoto.o \
		$(IntDir)/AIStateUseSmartObject.o \
		$(IntDir)/AIStimulusMgr.o \
		$(IntDir)/AIStreamSim.o \
		$(IntDir)/AITarget.o \
		$(IntDir)/AITargetSelectAbstract.o \
		$(IntDir)/AITargetSelectAllyDisturbance.o \
		$(IntDir)/AITargetSelectBerserker.o \
		$(IntDir)/AITargetSelectCharacter.o \
		$(IntDir)/AITargetSelectCharacterOnlyOne.o \
		$(IntDir)/AITargetSelectCharacterSquad.o \
		$(IntDir)/AITargetSelectCharacterUrgent.o \
		$(IntDir)/AITargetSelectCharacterUrgentSpatial.o \
		$(IntDir)/AITargetSelectCombatOpportunity.o \
		$(IntDir)/AITargetSelectDamager.o \
		$(IntDir)/AITargetSelectDisturbance.o \
		$(IntDir)/AITargetSelectDisturbanceBeyondGuard.o \
		$(IntDir)/AITargetSelectDisturbanceSource.o \
		$(IntDir)/AITargetSelectDisturbanceSquad.o \
		$(IntDir)/AITargetSelectDisturbanceUrgent.o \
		$(IntDir)/AITargetSelectFollower.o \
		$(IntDir)/AITargetSelectInterest.o \
		$(IntDir)/AITargetSelectLeader.o \
		$(IntDir)/AITargetSelectMgr.o \
		$(IntDir)/AITargetSelectPlayerTurret.o \
		$(IntDir)/AITargetSelectScripted.o \
		$(IntDir)/AITargetSelectTraitor.o \
		$(IntDir)/AITargetSelectWeaponItem.o \
		$(IntDir)/AIUtils.o \
		$(IntDir)/AIWeaponAbstract.o \
		$(IntDir)/AIWeaponMelee.o \
		$(IntDir)/AIWeaponMgr.o \
		$(IntDir)/AIWeaponRanged.o \
		$(IntDir)/AIWeaponThrown.o \
		$(IntDir)/AIWeaponUtils.o \
		$(IntDir)/AIWorkingMemory.o \
		$(IntDir)/AIWorkingMemoryCentral.o \
		$(IntDir)/AIWorldState.o \
		$(IntDir)/Alarm.o \
		$(IntDir)/AmmoBox.o \
		$(IntDir)/AnimatorPlayer.o \
		$(IntDir)/Arsenal.o \
		$(IntDir)/AttachmentDB.o \
		$(IntDir)/Attachments.o \
		$(IntDir)/AutoAimMagnet.o \
		$(IntDir)/BanIPMgr.o \
		$(IntDir)/BanUserMgr.o \
		$(IntDir)/Camera.o \
		$(IntDir)/Character.o \
		$(IntDir)/CharacterAlignment.o \
		$(IntDir)/CharacterHitBox.o \
		$(IntDir)/CharacterMgr.o \
		$(IntDir)/ClientWeaponSFX.o \
		$(IntDir)/CommandDB.o \
		$(IntDir)/CommandMgr.o \
		$(IntDir)/CommandObject.o \
		$(IntDir)/ConstraintBase.o \
		$(IntDir)/ConstraintHinge.o \
		$(IntDir)/ConstraintLimitedHinge.o \
		$(IntDir)/ConstraintPoint.o \
		$(IntDir)/ConstraintPrismatic.o \
		$(IntDir)/ConstraintRagdoll.o \
		$(IntDir)/ConstraintStiffSpring.o \
		$(IntDir)/ConstraintWheel.o \
		$(IntDir)/Controller.o \
		$(IntDir)/ControlPoint.o \
		$(IntDir)/CTFFlag.o \
		$(IntDir)/CTFFlagBase.o \
		$(IntDir)/DamageTracker.o \
		$(IntDir)/DebugLine.o \
		$(IntDir)/DebugLineSystem.o \
		$(IntDir)/DecisionObject.o \
		$(IntDir)/DEditHook.o \
		$(IntDir)/Destructible.o \
		$(IntDir)/DestructibleModel.o \
		$(IntDir)/DestructibleWorldModel.o \
		$(IntDir)/Dialogue.o \
		$(IntDir)/DisplayMeter.o \
		$(IntDir)/DisplayTimer.o \
		$(IntDir)/Door.o \
		$(IntDir)/DynamicSectorVolume.o \
		$(IntDir)/EntryToolLock.o \
		$(IntDir)/EventCounter.o \
		$(IntDir)/Explosion.o \
		$(IntDir)/Flicker.o \
		$(IntDir)/ForceVolume.o \
		$(IntDir)/ForensicObject.o \
		$(IntDir)/GameAlloc.o \
		$(IntDir)/GameBase.o \
		$(IntDir)/GameServerShell.o \
		$(IntDir)/GameStartPoint.o \
		$(IntDir)/GameStartPointMgr.o \
		$(IntDir)/GameWorldEditImpl.o \
		$(IntDir)/GameWorldPackerImpl.o \
		$(IntDir)/GearItems.o \
		$(IntDir)/GlobalServerMgr.o \
		$(IntDir)/GlobalsInit.o \
		$(IntDir)/Group.o \
		$(IntDir)/GunMount.o \
		$(IntDir)/HHWeaponModel.o \
		$(IntDir)/JumpVolume.o \
		$(IntDir)/Key.o \
		$(IntDir)/KeyFramer.o \
		$(IntDir)/KeyframeToRigidBody.o \
		$(IntDir)/Ladder.o \
		$(IntDir)/LightBase.o \
		$(IntDir)/LightCube.o \
		$(IntDir)/LightDirectional.o \
		$(IntDir)/LightEditor.o \
		$(IntDir)/LightPoint.o \
		$(IntDir)/LightPointFill.o \
		$(IntDir)/LightSpot.o \
		$(IntDir)/LookAtTrigger.o \
		$(IntDir)/ltengineobjects.o \
		$(IntDir)/ltmemhook.o \
		$(IntDir)/ltmodule.o \
		$(IntDir)/ltobjref.o \
		$(IntDir)/MissionDB.o \
		$(IntDir)/NamedObjectList.o \
		$(IntDir)/NavMarker.o \
		$(IntDir)/NoPlayerTrigger.o \
		$(IntDir)/ObjectRemover.o \
		$(IntDir)/ObjectTemplateMgr.o \
		$(IntDir)/ObjectTransformHistory.o \
		$(IntDir)/performancemonitorhook.o \
		$(IntDir)/PhysicsCollisionSystem.o \
		$(IntDir)/PhysicsImpulseDirectional.o \
		$(IntDir)/PhysicsImpulseRadial.o \
		$(IntDir)/PickupItem.o \
		$(IntDir)/PlayerInventory.o \
		$(IntDir)/PlayerLeash.o \
		$(IntDir)/PlayerLure.o \
		$(IntDir)/PlayerNodeGoto.o \
		$(IntDir)/PlayerObj.o \
		$(IntDir)/PlayerTracker.o \
		$(IntDir)/PlayerTrigger.o \
		$(IntDir)/Point.o \
		$(IntDir)/PolyGrid.o \
		$(IntDir)/PolyGridModifier.o \
		$(IntDir)/PositionalDamage.o \
		$(IntDir)/PrefetchUtilities.o \
		$(IntDir)/Projectile.o \
		$(IntDir)/ProjectileTypes.o \
		$(IntDir)/Prop.o \
		$(IntDir)/RefillStation.o \
		$(IntDir)/RemoteTurret.o \
		$(IntDir)/RenderTarget.o \
		$(IntDir)/RenderTargetGroup.o \
		$(IntDir)/RotatingDoor.o \
		$(IntDir)/RotatingSwitch.o \
		$(IntDir)/RotatingWorldModel.o \
		$(IntDir)/ScaleAlpha.o \
		$(IntDir)/ScatterVolume.o \
		$(IntDir)/ScmdConsoleDriver_PunkBuster.o \
		$(IntDir)/ScmdServer.o \
		$(IntDir)/ScreenEffect.o \
		$(IntDir)/ServerConnectionMgr.o \
		$(IntDir)/ServerDB.o \
		$(IntDir)/ServerMeleeCollisionController.o \
		$(IntDir)/ServerMissionMgr.o \
		$(IntDir)/ServerNodeTrackerContext.o \
		$(IntDir)/ServerNonPointSound.o \
		$(IntDir)/ServerPhysicsCollisionMgr.o \
		$(IntDir)/ServerSaveLoadMgr.o \
		$(IntDir)/ServerSoundFX.o \
		$(IntDir)/ServerSoundMgr.o \
		$(IntDir)/ServerSoundZoneVolume.o \
		$(IntDir)/ServerSpecialFX.o \
		$(IntDir)/ServerUtilities.o \
		$(IntDir)/ServerVoteMgr.o \
		$(IntDir)/SlidingDoor.o \
		$(IntDir)/SlidingSwitch.o \
		$(IntDir)/SlidingWorldModel.o \
		$(IntDir)/SnowVolume.o \
		$(IntDir)/SonicIntoneHandlers.o \
		$(IntDir)/SoundButeFX.o \
		$(IntDir)/SoundDB.o \
		$(IntDir)/SoundFilterDB.o \
		$(IntDir)/SoundMgr.o \
		$(IntDir)/SoundOcclusionDB.o \
		$(IntDir)/Spawner.o \
		$(IntDir)/Speaker.o \
		$(IntDir)/SpecialMove.o \
		$(IntDir)/SpinningWorldModel.o \
		$(IntDir)/StairVolume.o \
		$(IntDir)/StartupCommand.o \
		$(IntDir)/Stdafx.o \
		$(IntDir)/StoryMode.o \
		$(IntDir)/StringEditMgrPlugin.o \
		$(IntDir)/SurfaceDB.o \
		$(IntDir)/SurfaceFunctions.o \
		$(IntDir)/Switch.o \
		$(IntDir)/TeamBalancer.o \
		$(IntDir)/TeamClientFX.o \
		$(IntDir)/TeleportPoint.o \
		$(IntDir)/TransitionAggregate.o \
		$(IntDir)/TransitionArea.o \
		$(IntDir)/TransitionMgr.o \
		$(IntDir)/Trigger.o \
		$(IntDir)/TriggerTypeDB.o \
		$(IntDir)/Turret.o \
		$(IntDir)/VolumeBrush.o \
		$(IntDir)/VolumeBrushTypes.o \
		$(IntDir)/VolumeEffect.o \
		$(IntDir)/Weapon.o \
		$(IntDir)/WeaponItems.o \
		$(IntDir)/WorldModel.o \
		$(IntDir)/WorldProperties.o \
		
###############################################################
# Configuration: Release
###############################################################
ifeq "$(ConfigurationName)" "Release"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -DNDEBUG -DUSE_INTEL_COMPILER -D_SERVERBUILD -DMEMTRACK_SERVER
INCLUDES=-I.  -I../Shared -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../../libs/platform
LIBRARYPATHS= -L$(OutDir) -L../../Engine/sdk/lib/win -L../../Engine/sdk/lib/linux/$(ConfigurationName)
LIBRARIES= -lGame_Shared_Server -lShared_CRC -lShared_Assert

# Compile options
DEBUG_OPTION=-g
OPTIMIZATION_OPTION=-O2

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/libGameServer.so
EXPORTS_FILE=$(wildcard $(ProjectName)_LinuxExports.txt)
ifneq "$(EXPORTS_FILE)" ""
EXPORTS_OPTION=-Wl,--version-script $(ProjectName)_LinuxExports.txt
endif
LINK=$(EXE_TOOL) $(EXPORTS_OPTION) -rdynamic -pthread $(DEBUG_OPTION) -o "$(TargetPath)" -shared $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

	
###############################################################
# Configuration: Debug
###############################################################
ifeq "$(ConfigurationName)" "Debug"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -D_DEBUG -D_SERVERBUILD -DMEMTRACK_SERVER
INCLUDES=-I.  -I../Shared -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../../libs/platform
LIBRARYPATHS= -L$(OutDir) -L../../Engine/sdk/lib/win -L../../Engine/sdk/lib/linux/$(ConfigurationName)
LIBRARIES= -lGame_Shared_Server -lShared_CRC -lShared_Assert

# Compile options
DEBUG_OPTION=-g
	

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/libGameServer.so
EXPORTS_FILE=$(wildcard $(ProjectName)_LinuxExports.txt)
ifneq "$(EXPORTS_FILE)" ""
EXPORTS_OPTION=-Wl,--version-script $(ProjectName)_LinuxExports.txt
endif
LINK=$(EXE_TOOL) $(EXPORTS_OPTION) -rdynamic -pthread $(DEBUG_OPTION) -o "$(TargetPath)" -shared $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

###############################################################
# Configuration: Final
###############################################################
ifeq "$(ConfigurationName)" "Final"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -DNDEBUG -D_SERVERBUILD -D_FINAL -DMEMTRACK_SERVER
INCLUDES=-I.  -I../Shared -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../../libs/platform
LIBRARYPATHS= -L$(OutDir) -L../../Engine/sdk/lib/win -L../../Engine/sdk/lib/linux/$(ConfigurationName)
LIBRARIES= -lGame_Shared_Server -lShared_CRC -lShared_Assert

# Compile options
OPTIMIZATION_OPTION=-O2
	

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/libGameServer.so
EXPORTS_FILE=$(wildcard $(ProjectName)_LinuxExports.txt)
ifneq "$(EXPORTS_FILE)" ""
EXPORTS_OPTION=-Wl,--version-script $(ProjectName)_LinuxExports.txt
endif
LINK=$(EXE_TOOL) $(EXPORTS_OPTION) -rdynamic -pthread $(DEBUG_OPTION) -o "$(TargetPath)" -shared $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

	
