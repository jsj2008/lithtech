// ----------------------------------------------------------------------- //
//
// MODULE  : AIDB.h
//
// PURPOSE : AI database definition
//
// CREATED : 03/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_DATABASE_H__
#define __AI_DATABASE_H__

//
// Includes...
//

	#include "GameDatabaseMgr.h"
	#include "AIActionAbstract.h"
	#include "AIActionMgr.h"
	#include "AIActivityAbstract.h"
	#include "AISensorAbstract.h"
	#include "AIGoalAbstract.h"
	#include "CharacterAlignment.h"
	#include "AnimationContext.h"
	#include "AINodeTypes.h"
	#include "AIEnumStimulusTypes.h"
	#include "AISensorMgr.h"
	#include "AIEnumAIWeaponTypes.h"
	#include "AIEnumContexts.h"
	#include "AIEnumAwarenessModifiers.h"
	#include "AIEnumNodeDependencyTypes.h"
	#include "AIEnumAIAwareness.h"
	#include "AITargetSelectMgr.h"

//
// Defines...
//

	// The main system category were all AI related data goes...
	#define AIDB_CATEGORY										"AI/"

	#define AIDB_MISC_CATEGORY									AIDB_CATEGORY "Misc"

	#define AIDB_ACTION_CATEGORY								AIDB_CATEGORY "Actions"

		#define AIDB_ACTION_sClass								"Class"
		#define AIDB_ACTION_fCost								"Cost"
		#define AIDB_ACTION_fPrecedence							"Precedence"
		#define AIDB_ACTION_bInterruptible						"Interruptable"
		#define AIDB_ACTION_sSmartObject						"SmartObject"
		#define AIDB_ACTION_fProbability						"Probability"
		#define AIDB_ACTION_sActionAbility						"ActionAbility"
        #define AIDB_ACTION_sNodeType                           "NodeType"
        #define AIDB_ACTION_sAwareness                          "Awareness"

	#define AIDB_ACTIONSET_CATEGORY								AIDB_CATEGORY "ActionSets"

		#define AIDB_ACTIONSET_sAction							"Action"
		#define AIDB_ACTIONSET_sInclude							"Include"

	#define AIDB_ACTIVITYSET_CATEGORY							AIDB_CATEGORY "ActivitySets"

		#define AIDB_ACTIVITYSET_sActivity						"Activity"

	#define AIDB_DAMAGEMASK_CATEGORY							"Damage/Masks"

		#define AIDB_DAMAGEMASK_sDamage							"Damage"

	#define AIDB_BRAIN_CATEGORY									AIDB_CATEGORY "Brains"

		#define AIDB_BRAIN_fDodgeVectorShuffleDist				"DodgeVectorShuffleDist"
		#define AIDB_BRAIN_fDodgeVectorRollDist					"DodgeVectorRollDist"
		#define AIDB_BRAIN_fAttackPoseCrouchTime				"AttackPoseCrouchTime"
		#define AIDB_BRAIN_fAttackGrenadeThrowTimeRange			"AttackGrenadeThrowTimeRange"
		#define AIDB_BRAIN_nMajorAlarmThreshold					"MajorAlarmThreshold"
		#define AIDB_BRAIN_nImmediateAlarmThreshold				"ImmediateAlarmThreshold"
		#define AIDB_BRAIN_bCanLipSync							"CanLipSync"

	#define AIDB_CONSTANTS_CATEGORY								AIDB_CATEGORY "Constants"

		#define AIDB_CONSTANTS_fNoFOVDistance					"NoFOVDistance"
		#define AIDB_CONSTANTS_fInstantSeeDistance				"InstantSeeDistance"
		#define AIDB_CONSTANTS_fAlertImmediateThreatInstantSeeDistance	"AlertImmediateThreatInstantSeeDistance"
		#define AIDB_CONSTANTS_fPersonalBubbleDistance			"PersonalBubbleDistance"
		#define AIDB_CONSTANTS_fPersonalBubbleStimulusRateMod	"PersonalBubbleStimulusRateMod"
		#define AIDB_CONSTANTS_fThreatTooCloseDistance			"ThreatTooCloseDistance"
		#define AIDB_CONSTANTS_fThreatUnseenTime				"ThreatUnseenTime"
		#define AIDB_CONSTANTS_fFOVAlertImmediateThreat			"FOVAlertImmediateThreat"
		#define AIDB_CONSTANTS_fFOVAlert						"FOVAlert"
		#define AIDB_CONSTANTS_fFOVSuspicious					"FOVSuspicious"
		#define AIDB_CONSTANTS_fFOVRelaxed						"FOVRelaxed"
		#define AIDB_CONSTANTS_fHoldPositionTime				"HoldPositionTime"
		#define AIDB_CONSTANTS_fHoldPositionDistance			"HoldPositionDistance"
		#define AIDB_CONSTANTS_fDamagedAtNodeAvoidanceTime		"DamagedAtNodeAvoidanceTime"
		#define AIDB_CONSTANTS_fBlockedDoorAvoidanceTime		"BlockedDoorAvoidanceTime"
		#define AIDB_CONSTANTS_fBlockedDoorWeightMultiplier		"BlockedDoorWeightMultiplier"
		#define AIDB_CONSTANTS_fAISoundFrequencyChatter			"AISoundFrequencyChatter"
		#define AIDB_CONSTANTS_fAISoundFrequencyEvent			"AISoundFrequencyEvent"
		#define AIDB_CONSTANTS_fAISoundFrequencyLocation		"AISoundFrequencyLocation"
		#define AIDB_CONSTANTS_fAISoundFrequencyMelee			"AISoundFrequencyMelee"
		#define AIDB_CONSTANTS_fDifficultyFactorEasy			"DifficultyFactorEasy"
		#define AIDB_CONSTANTS_fDifficultyFactorNormal			"DifficultyFactorNormal"
		#define AIDB_CONSTANTS_fDifficultyFactorHard			"DifficultyFactorHard"
		#define AIDB_CONSTANTS_fDifficultyFactorVeryHard		"DifficultyFactorVeryHard"
		#define AIDB_CONSTANTS_fDifficultyFactorPlayerIncrease	"DifficultyFactorPlayerIncrease"
		#define AIDB_CONSTANTS_fCycleCutOffForward				"CycleCutOffForward"
		#define AIDB_CONSTANTS_fCycleCutOffLeft					"CycleCutOffLeft"
		#define AIDB_CONSTANTS_fCycleCutOffBackward				"CycleCutOffBackward"
		#define AIDB_CONSTANTS_fCycleCutOffRight				"CycleCutOffRight"
		#define AIDB_CONSTANTS_sObjectAlignmentName				"ObjectAlignmentName"
		#define AIDB_CONSTANTS_fTargetIsAimingAtMeFOV			"TargetIsAimingAtMeFOV"
		#define AIDB_CONSTANTS_fTargetIsLookingAtMeFOV			"TargetIsLookingAtMeFOV"
		#define AIDB_CONSTANTS_fMinDirectionalRunChangeDistance	"MinDirectionalRunChangeDistance"
		#define AIDB_CONSTANTS_sAwarenessMods					"AwarenessMods"
		#define AIDB_CONSTANTS_bAllowRecoilsToInterruptRecoils	"AllowRecoilsToInterruptRecoils"
		#define AIDB_CONSTANTS_bWhenDroppedWeaponsInheritSocketVelocities	"WhenDroppedWeaponsInheritSocketVelocities"
		#define AIDB_CONSTANTS_nStringPullingMaxIterations		"StringPullingMaxIterations"
		#define AIDB_CONSTANTS_fAntiPenetrationRadiusBuffer		"AntiPenetrationRadiusBuffer"
		#define AIDB_CONSTANTS_sUnarmedWeaponProp				"UnarmedWeaponProp"
		#define AIDB_CONSTANTS_fCombatOpportunityFrequency		"CombatOpportunityFrequency"
		#define AIDB_CONSTANTS_fNavMeshLinkDeathDelay			"NavMeshLinkDeathDelay"
		#define AIDB_CONSTANTS_bLoopInvestigateIfCantSearch		"LoopInvestigateIfCantSearch"
		#define AIDB_CONSTANTS_bDisturbancesCauseTargetSelection "DisturbancesCauseTargetSelection"

	#define AIDB_ATTRIBUTES_CATEGORY							AIDB_CATEGORY "Attributes"

		#define AIDB_ATTRIBUTES_sName							"Name"
		#define AIDB_ATTRIBUTES_sAlignment						"Alignment"
		#define AIDB_ATTRIBUTES_sNavMeshType					"NavMeshType"
		#define AIDB_ATTRIBUTES_sLimpLimits						"LimpLimits"
		#define AIDB_ATTRIBUTES_sAIActionSet					"AIActionSet"
		#define AIDB_ATTRIBUTES_sTargetSelectSet				"TargetSelectSet"
		#define AIDB_ATTRIBUTES_sAIActivitySet					"AIActivitySet"
		#define AIDB_ATTRIBUTES_bCanLipSync						"CanLipSync"
		#define AIDB_ATTRIBUTES_bCanJoinSquads					"CanJoinSquads"
		#define AIDB_ATTRIBUTES_sAIMovementSet					"AIMovementSet"
		#define AIDB_ATTRIBUTES_sAIWeaponOverrideSet			"AIWeaponOverridesSet"
		#define AIDB_ATTRIBUTES_sDroppedItems					"DroppedItems"
		#define AIDB_ATTRIBUTES_sDamageMask						"DamageMask"
		#define AIDB_ATTRIBUTES_fRunSpeed						"RunSpeed"
		#define AIDB_ATTRIBUTES_fJumpSpeed						"JumpSpeed"
		#define AIDB_ATTRIBUTES_fJumpOverSpeed					"JumpOverSpeed"
		#define AIDB_ATTRIBUTES_fFallSpeed						"FallSpeed"
		#define AIDB_ATTRIBUTES_fWalkSpeed						"WalkSpeed"
		#define AIDB_ATTRIBUTES_fSwimSpeed						"SwimSpeed"
		#define AIDB_ATTRIBUTES_fRotationTimeLimited			"RotationTimeLimited"
		#define AIDB_ATTRIBUTES_fRotationTimeStatic				"RotationTimeStatic"
		#define AIDB_ATTRIBUTES_fRotationTimeMoving				"RotationTimeMoving"
		#define AIDB_ATTRIBUTES_fSoundRadius					"SoundRadius"
		#define AIDB_ATTRIBUTES_fSoundInnerRadius				"SoundInnerRadius"
		#define AIDB_ATTRIBUTES_fAccuracy						"Accuracy"
		#define AIDB_ATTRIBUTES_fAccuracyIncreaseRate			"AccuracyIncreaseRate"
		#define AIDB_ATTRIBUTES_fAccuracyDecreaseRate			"AccuracyDecreaseRate"
		#define AIDB_ATTRIBUTES_fFullAccuracyRadius				"FullAccuracyRadius"
		#define AIDB_ATTRIBUTES_fAccuracyMissPerturb			"AccuracyMissPerturb"
		#define AIDB_ATTRIBUTES_fMaxMovementAccuracyPerturb		"MaxMovementAccuracyPerturb"
		#define AIDB_ATTRIBUTES_fMovementAccuracyPerturbTime	"MovementAccuracyPerturbTime"
		#define AIDB_ATTRIBUTES_fUpdateRate						"UpdateRate"
		#define AIDB_ATTRIBUTES_fVerticalThreshold				"VerticalThreshold"
		#define AIDB_ATTRIBUTES_fSeeDistance					"SeeDistance"
		#define AIDB_ATTRIBUTES_fHearDistance					"HearDistance"
		#define AIDB_ATTRIBUTES_bEnableAnimationBlending		"EnableAnimationBlending"
		#define AIDB_ATTRIBUTES_bInstantDeath					"InstantDeath"
		#define AIDB_ATTRIBUTES_bPreventMovementIntoObstacles	"UsePreventMovementIntoObstacles"


	#define AIDB_LIMITS_CATEGORY								AIDB_CATEGORY "Limits"

		#define AIDB_LIMITS_fFrequency							"Frequency"
		#define AIDB_LIMITS_fProbability						"Probability"
		#define AIDB_LIMITS_fThreshold							"Threshold"

	#define AIDB_STIMULUS_CATEGORY								AIDB_CATEGORY "Stimuli"

		#define AIDB_STIMULUS_sName									"Name"
		#define AIDB_STIMULUS_sSense								"Sense"
		#define AIDB_STIMULUS_sRequiredStance						"RequiredStance"
		#define AIDB_STIMULUS_fDistance								"Distance"
		#define AIDB_STIMULUS_fDuration								"Duration"
		#define AIDB_STIMULUS_nAlarmLevel							"AlarmLevel"
		#define AIDB_STIMULUS_fVerticalRadius						"VerticalRadius"
		#define AIDB_STIMULUS_v2ReactionDelay						"ReactionDelay"
		#define AIDB_STIMULUS_fStimulationIncreaseRateAlert			"StimulationIncreaseRateAlert"
		#define AIDB_STIMULUS_fStimulationIncreaseRateSuspicious	"StimulationIncreaseRateSuspicious"
		#define AIDB_STIMULUS_fStimulationIncreaseRateUnalert		"StimulationIncreaseRateUnalert"
		#define AIDB_STIMULUS_fStimulationDecreaseRateAlert			"StimulationDecreaseRateAlert"
		#define AIDB_STIMULUS_fStimulationDecreaseRateSuspicious	"StimulationDecreaseRateSuspicious"
		#define AIDB_STIMULUS_fStimulationDecreaseRateUnalert		"StimulationDecreaseRateUnalert"
		#define AIDB_STIMULUS_nFalseStimulationLimit				"FalseStimulationLimit"
		#define AIDB_STIMULUS_bRequireSourceIsNotSelf				"RequireSourceIsNotSelf"
		#define AIDB_STIMULUS_bRequireSourceIsSelf					"RequireSourceIsSelf"

	#define AIDB_STIMULUSMASK_CATEGORY							AIDB_CATEGORY "StimulusMasks"

		#define AIDB_STIMULUSMASK_sStimulus						"Stimulus"

	#define AIDB_SENSOR_CATEGORY								AIDB_CATEGORY "Sensors"

		#define AIDB_SENSOR_sClass								"Class"
		#define AIDB_SENSOR_fUpdateRate							"UpdateRate"
		#define AIDB_SENSOR_sNodeType							"NodeType"
		#define AIDB_SENSOR_sStimulus							"Stimulus"
		#define AIDB_SENSOR_sSmartObject						"SmartObject"

	#define AIDB_TARGETSELECT_CATEGORY							AIDB_CATEGORY "TargetSelects"

		#define AIDB_TARGETSELECT_sClass						"Class"
		#define AIDB_TARGETSELECT_fCost							"Cost"
        #define AIDB_TARGETSELECT_sAwareness                    "Awareness"

	#define AIDB_TARGETSELECTSET_CATEGORY						AIDB_CATEGORY "TargetSelectSets"

		#define AIDB_TARGETSELECTSET_sTarget					"TargetSelect"
		#define AIDB_TARGETSELECTSET_sInclude					"Include"

	#define AIDB_GOAL_CATEGORY									AIDB_CATEGORY "Goals"

		#define AIDB_GOAL_sClass								"Class"
		#define AIDB_GOAL_sContext								"Context"
		#define AIDB_GOAL_fRelevance							"Relevance"
		#define AIDB_GOAL_bReEvalOnSatisfaction					"ReEvalOnSatisfaction"
		#define AIDB_GOAL_sNodeType								"NodeType"
		#define AIDB_GOAL_sSensor								"Sensor"
		#define AIDB_GOAL_sSmartObject							"SmartObject"
		#define AIDB_GOAL_v2RecalcRate							"RecalcRate"
		#define AIDB_GOAL_fActivateChance						"ActivateChance"
		#define AIDB_GOAL_fFrequency							"Frequency"
		#define AIDB_GOAL_fInterruptPriority					"InterruptPriority"
		#define AIDB_GOAL_bCanReactivateDuringTransitions		"CanReactivateDuringTransitions"
		#define AIDB_GOAL_sMinAwareness							"MinAwareness"
		#define AIDB_GOAL_sMaxAwareness							"MaxAwareness"

	#define AIDB_GOALSET_CATEGORY								AIDB_CATEGORY "GoalSets"

		#define AIDB_GOALSET_sRequiredAIType					"RequiredAIType"
		#define AIDB_GOALSET_sIncludeGoalSet					"IncludeGoalSet"
		#define AIDB_GOALSET_bHidden							"Hidden"
		#define AIDB_GOALSET_bPermanent							"Permanent"
		#define AIDB_GOALSET_sGoal								"Goal"

	#define AIDB_MOVEMENT_CATEGORY								AIDB_CATEGORY "Movement"

		#define AIDB_MOVEMENT_sContext							"Context"
		#define AIDB_MOVEMENT_sRelaxed							"Relaxed"
		#define AIDB_MOVEMENT_sSuspicious						"Suspicious"
		#define AIDB_MOVEMENT_sAlert							"Alert"
		#define AIDB_MOVEMENT_sOverrideAwareness				"Overrides.%d.Awareness"
		#define AIDB_MOVEMENT_sOverrideAwarenessModifier		"Overrides.%d.AwarenessModifier"
		#define AIDB_MOVEMENT_sOverrideAnimProps				"Overrides.%d.AnimProps.0.%s"

	#define AIDB_MOVEMENTSET_CATEGORY							AIDB_CATEGORY "MovementSets"

		#define AIDB_MOVEMENTSET_sDefault						"Default"
		#define AIDB_MOVEMENTSET_sMovement						"Movements"

	#define AIDB_SMARTOBJECT_CATEGORY							AIDB_CATEGORY "SmartObjects"

		#define AIDB_SMARTOBJECT_sFlag							"Flag"
		#define AIDB_SMARTOBJECT_bLooping						"Looping"
		#define AIDB_SMARTOBJECT_v2LoopTime						"LoopTime"
		#define AIDB_SMARTOBJECT_v2FidgetFreq					"FidgetFreq"
		#define AIDB_SMARTOBJECT_fTimeout						"Timeout"
		#define AIDB_SMARTOBJECT_fUnpreferredTime				"UnpreferredTime"
		#define AIDB_SMARTOBJECT_bLockNode						"LockNode"
		#define AIDB_SMARTOBJECT_sWeaponState					"WeaponState"
		#define AIDB_SMARTOBJECT_sDependencyType				"DependencyType"
		#define AIDB_SMARTOBJECT_fEntryOffsetDistA				"EntryOffsetDistA"
		#define AIDB_SMARTOBJECT_fEntryOffsetDistB				"EntryOffsetDistB"
		#define AIDB_SMARTOBJECT_fExitOffsetDistA				"ExitOffsetDistA"
		#define AIDB_SMARTOBJECT_fExitOffsetDistB				"ExitOffsetDistB"
		#define AIDB_SMARTOBJECT_fMinDist						"MinDist"
		#define AIDB_SMARTOBJECT_fMaxDist						"MaxDist"
		#define AIDB_SMARTOBJECT_fNodeOffset					"NodeOffset"
		#define AIDB_SMARTOBJECT_fFindFloorOffset				"FindFloorOffset"
		#define AIDB_SMARTOBJECT_sChildModelName				"ChildModelName"
		#define AIDB_SMARTOBJECT_sWorldEditModelName			"WorldEditModelName"
		#define AIDB_SMARTOBJECT_sActionAbility					"ActionAbilityRequired"

	#define AIDB_WEAPON_CATEGORY								AIDB_CATEGORY "Weapons"

		#define AIDB_WEAPON_sClass								"Class"
		#define AIDB_WEAPON_sType								"Type"
		#define AIDB_WEAPON_sAnimProp							"AnimProp"
		#define AIDB_WEAPON_sChangeWeaponActionAnimProp			"ChangeWeaponActionAnimProp"
		#define AIDB_WEAPON_nAnimPriority						"AnimPriority"
		#define AIDB_WEAPON_v2Range								"Range"
		#define AIDB_WEAPON_bAllowDirectionalRun				"AllowDirectionalRun"
		#define AIDB_WEAPON_bIsDangerous						"IsDangerous"
		#define AIDB_WEAPON_bAIAnimatesReload					"AIAnimatesReload"
		#define AIDB_WEAPON_bForceMissToFloor					"ForceMissToFloor"
		#define AIDB_WEAPON_v2BurstInterval						"BurstInterval"
		#define AIDB_WEAPON_v2BurstShots						"BurstShots"
		#define AIDB_WEAPON_bStrictBursts						"StrictBursts"
		#define AIDB_WEAPON_bSuppressionBursts					"SuppressionBursts"
		#define AIDB_WEAPON_fPlayerPusherRadius					"PlayerPusherRadius"
		#define AIDB_WEAPON_fPlayerPusherForce					"PlayerPusherForce"
		#define AIDB_WEAPON_bAllowAmmoGeneration				"AllowAmmoGeneration"
		#define AIDB_WEAPON_bAllowAutoReload					"AllowAutoReload"
		#define AIDB_WEAPON_nPreference							"Preference"
		#define AIDB_WEAPON_rDamageScalar						"DamageScalar"		
		#define AIDB_WEAPON_rAccuracyScalar						"AccuracyScalar"		
		#define AIDB_WEAPON_bSyncToUserAnimation				"SyncToUserAnimation"		
		#define AIDB_WEAPON_fFireDuringDeathProbability			"FireDuringDeathProbability"
		#define AIDB_WEAPON_bRestrictFireCone					"RestrictFireCone"
		#define AIDB_WEAPON_vFireRotationOffset					"FireRotationOffset"
		#define AIDB_WEAPON_bLoopFireAnimation					"LoopFireAnimation"
		#define AIDB_WEAPON_szDefaultAnimationName				"DefaultAnimationName"

	#define AIDB_WEAPONOVERRIDESET_CATEGORY						AIDB_CATEGORY "WeaponOverrideSets"

		#define AIDB_WEAPONOVERRIDESET_hWeapon					"WeaponOverride.%d.Weapon"
		#define AIDB_WEAPONOVERRIDESET_hAIWeapon				"WeaponOverride.%d.AIWeapon"

	#define AIDB_AMMOLOAD_CATEGORY								AIDB_CATEGORY "WeaponAmmoLoads"

		#define AIDB_AMMOLOAD_sAmmo								"AmmoAmountList.%d.Ammo"
		#define AIDB_AMMOLOAD_nAmount							"AmmoAmountList.%d.Amount"

	#define AIDB_NODE_CATEGORY									AIDB_CATEGORY "Nodes"

		#define AIDB_NODE_sAtNodeStatusFlags					"AtNodeStatusFlags"
		#define AIDB_NODE_sNotAtNodeStatusFlags					"NotAtNodeStatusFlags"

	#define AIDB_DROPPEDITEMS_CATEGORY							AIDB_CATEGORY "DroppedItems"

		#define AIDB_DROPPEDITEMS_Items							"Items"
		#define AIDB_DROPPEDITEMS_rGearItem						"GearItem"
		#define AIDB_DROPPEDITEMS_rWeaponItem					"WeaponItem"

	#define AIDB_AINAVMESHTYPE_CATEGORY							AIDB_CATEGORY "NavMeshTypes"

		// No attributes yet

//
// Forward declarations...
//

class	CAIDB;

extern CAIDB* g_pAIDB;


//
// Structs... 
//

struct AIChildModelInfo 
{
	AIChildModelInfo() { }

	std::string	strFilename;
};
typedef std::list<AIChildModelInfo, LTAllocator<AIChildModelInfo, LT_MEM_TYPE_OBJECTSHELL> > AICHILD_MODEL_INFO_LIST;

enum ENUM_AISmartObjectID { kAISmartObjectID_Invalid = -1 };
struct AIDB_SmartObjectRecord
{
	enum ENUM_IsArmedRequirement
	{
		kIsArmedRequirement_Any,
		kIsArmedRequirement_Holstered,
		kIsArmedRequirement_Drawn
	};

	std::string					strName;
	std::string					strWorldEditModelName;
	ENUM_AISmartObjectID		eSmartObjectID;

	EnumAINodeType				eNodeType;
	ENUM_IsArmedRequirement		eIsArmedRequirement;
	EnumAINodeDependencyType	eDependencyType;
	float						fEntryOffsetDistA;
	float						fEntryOffsetDistB;
	float						fExitOffsetDistA;
	float						fExitOffsetDistB;
	float						fMinDist;
	float						fMaxDist;
	float						fNodeOffset;
	float						fFindFloorOffset;
	CAnimationProps				Props;
	bool						bLooping;
	float						fMinLoopTime;
	float						fMaxLoopTime;
	float						fMinFidgetTime;
	float						fMaxFidgetTime;
	float						fTimeout;
	float						fUnpreferredTime;
	bool						bLockNode;

	AICHILD_MODEL_INFO_LIST		lstChildModels;
	ENUM_ActionAbility			eActionAbilityRequired;
};

struct AIDB_ActionRecord
{
	AIDB_ActionRecord();

	EnumAIActionType		eActionType;
	EnumAIActionType		eActionClass;
	EnumAINodeType			eNodeType;
	float					fActionCost;
	float					fActionPrecedence;
	bool					bActionIsInterruptible;
	float					fActionProbability;
	ENUM_AISmartObjectID	eSmartObjectID;
	EnumAIAwareness			eAwareness;
	std::bitset<kActionAbility_Count> dwActionAbilitySet;
};

struct AIDB_ActionSetRecord
{
	AIDB_ActionSetRecord() { eActionSet = kAIActionSet_Invalid; }

	std::string				strName;
	ENUM_AIActionSet		eActionSet;
	AIActionBitSet			ActionMask;
};

struct AIDB_ActivitySetRecord
{
	AIDB_ActivitySetRecord() { eActivitySet = kAIActivitySet_Invalid; }

	std::string				strName;
	ENUM_AIActivitySet		eActivitySet;
	AIActivityBitSet		ActivityMask;
};

enum ENUM_AIDamageMaskID { kAIDamageMaskID_Invalid = -1 };
struct AIDB_DamageMaskRecord
{
	std::string			strName;
	ENUM_AIDamageMaskID	eAIDamageMaskID;

	DamageFlags			dfDamageTypes;
};

enum ENUM_AIBrainID { kAIBrainID_Invalid = -1 };
struct AIDB_BrainRecord
{
	std::string			strName;
	ENUM_AIBrainID		eAIBrainID;

	float				fDodgeVectorShuffleDist;
	float				fDodgeVectorRollDist;

	float				fAttackPoseCrouchTime;
	float				fAttackGrenadeThrowTimeMin;
	float				fAttackGrenadeThrowTimeMax;

	uint32				nMajorAlarmThreshold;
	uint32				nImmediateAlarmThreshold;

	bool				bCanLipSync;
};

typedef std::bitset<kAwarenessMod_Count> AIAWARENESSMOD_BITS;
struct AIDB_ConstantsRecord
{
	float fNoFOVDistanceSqr;
	float fInstantSeeDistanceSqr;
	float fAlertImmediateThreatInstantSeeDistanceSqr;
	float fPersonalBubbleDistanceSqr;
	float fPersonalBubbleStimulusRateMod;
	float fThreatTooCloseDistanceSqr;
	float fThreatTooCloseDistance;

	float fFOVAlertImmediateThreat;
	float fFOVAlert;
	float fFOVSuspicious;
	float fFOVRelaxed;

	double fThreatUnseenTime;

	float fHoldPositionTime;
	float fHoldPositionDistance;
	float fHoldPositionDistanceSqr;

	float fDamagedAtNodeAvoidanceTimeMin;
	float fDamagedAtNodeAvoidanceTimeMax;

	float fBlockedDoorAvoidanceTime;
	float fBlockedDoorWeightMultiplier;

	float fAISoundFrequencyChatter;
	float fAISoundFrequencyEvent;
	float fAISoundFrequencyLocation;
	float fAISoundFrequencyMelee;

	float fDifficultyFactorEasy;
	float fDifficultyFactorNormal;
	float fDifficultyFactorHard;
	float fDifficultyFactorVeryHard;
	float fDifficultyFactorPlayerIncrease;

	float fCycleCutOffForward;
	float fCycleCutOffLeft;
	float fCycleCutOffBackward;
	float fCycleCutOffRight;

	std::string strObjectAlignmentName;

	AIAWARENESSMOD_BITS bitsValidAwarenessMods;

	bool bAllowRecoilsToInterruptRecoils;
	bool bWhenDroppedWeaponsInheritSocketVelocities;

	uint32 nStringPullingMaxIterations;
	float fAntiPenetrationRadiusBuffer;
	EnumAnimProp eUnarmedWeaponProp;

	float fCombatOpportunityFrequency;

	float fNavMeshLinkDeathDelay;

	bool bLoopInvestigateIfCantSearch;

	bool bDisturbancesCauseTargetSelection;
};

enum ENUM_AILimitsID { kAILimitsID_Invalid = -1 };
struct AIDB_LimitsRecord
{
	std::string			strName;
	ENUM_AILimitsID		eAILimitsID;

	float fFrequency;
	float fProbability;
	float fThreshold;
};

enum ENUM_AIAttributesID { kAIAttributesID_Invalid = -1 };
struct AIDB_AttributesRecord
{
	std::string			strName;
	ENUM_AIAttributesID	eAIAttributesID;

	std::string	strAlignment;
	std::string	strAIActionSet;
	std::string	strTargetSelectSet;
	std::string	strAIActivitySet;
	std::string	strAIMovementSet;
	std::string strAIWeaponOverrideSet;
	std::string strDroppedItems;


	ENUM_AILimitsID		eLimpLimitsID;
	ENUM_AIDamageMaskID	eDamageMaskID;

	bool  bCanLipSync;
	bool  bCanJoinSquads;
	float fRunSpeed;
	float fJumpSpeed;
	float fJumpOverSpeed;
	float fFallSpeed;
	float fWalkSpeed;
	float fSwimSpeed;
	float fRotationTimeLimited;
	float fRotationTimeStatic;
	float fRotationTimeMoving;
	float fSoundOuterRadius;
	float fSoundInnerRadius;
	float fAccuracy;
	float fAccuracyIncreaseRate;
	float fAccuracyDecreaseRate;
	float fFullAccuracyRadiusSqr;
	float fAccuracyMissPerturb;
	float fMaxMovementAccuracyPerturb;
	float fMovementAccuracyPerturbDecay;
	float fUpdateRate;
	float fVerticalThreshold;

	float fSeeDistance;
	float fHearDistance;
	float fTargetIsAimingAtMeFOVDp;
	float fTargetIsLookingAtMeFOVDp;

	float fMinDirectionalRunChangeDistanceSqr;
	bool bEnableAnimationBlending;
	bool bInstantDeath;
	bool bUsePreventMovementIntoObstacles;
};
typedef std::vector<ENUM_AIAttributesID, LTAllocator<ENUM_AIAttributesID, LT_MEM_TYPE_OBJECTSHELL> > AIATTRIBUTES_LIST;

struct AIDB_StimulusRecord
{
	EnumAIStimulusType	eStimulusType;
	EnumAISenseType		eSenseType;
	STANCE_BITS			bitsRequiredStance;
	float 				fDistance;
	float				fVerticalRadius;
	float 				fDuration;
	uint8				nAlarmLevel;
	LTVector2 			v2ReactionDelay;
	float 				fStimulationIncreaseRateAlert;
	float 				fStimulationIncreaseRateSuspicious;
	float 				fStimulationIncreaseRateUnalert;
	float 				fStimulationDecreaseRateAlert;
	float 				fStimulationDecreaseRateSuspicious;
	float 				fStimulationDecreaseRateUnalert;
	uint8				nFalseStimulationLimit;
	bool				bRequireSourceIsNotSelf;
	bool				bRequireSourceIsSelf;
};

struct AIDB_StimulusMaskRecord
{
	std::string		strName;
	uint32			dwBlockedStimuli;
};

struct AIDB_SensorRecord
{
	EnumAISensorType	eSensorType;
	EnumAISensorType	eSensorClass;
	float				fSensorUpdateRate;
	EnumAINodeType		eNodeType;
	ENUM_AISmartObjectID	eSmartObjectID;
	uint32				dwStimulusTypes;
};

enum ENUM_AIAmmoLoadRecordID { kAIAmmoLoadRecordID_Invalid = -1 };
struct AIDB_AIAmmoLoadRecord
{
	std::string strName;

	struct AmmoAmount
	{
		HAMMO	m_hAmmo;
		int		m_nRounds;
	};

	typedef std::vector<AmmoAmount, LTAllocator<AmmoAmount, LT_MEM_TYPE_OBJECTSHELL> > AMMOAMOUNT_TYPE_LIST;

	AMMOAMOUNT_TYPE_LIST m_AmmoAmountList;
};

typedef std::vector<EnumAISensorType, LTAllocator<EnumAISensorType, LT_MEM_TYPE_OBJECTSHELL> > AISENSOR_TYPE_LIST;

struct AIDB_TargetSelectRecord
{
	AIDB_TargetSelectRecord();

	EnumAITargetSelectType	eTargetSelectType;
	EnumAITargetSelectType	eTargetSelectClass;
	float					fCost;
	EnumAIAwareness			eAwareness;
};

struct AIDB_TargetSelectSetRecord
{
	AIDB_TargetSelectSetRecord() { eTargetSelectSet = kAITargetSelectSet_Invalid; }

	std::string				strName;
	ENUM_AITargetSelectSet	eTargetSelectSet;
	AITargetSelectBitSet	TargetSelectMask;
};

struct AIDB_GoalRecord
{
	EnumAIGoalType		eGoalType;
	EnumAIGoalType		eGoalClass;
	EnumAIContext		eAIContext;
	float				fIntrinsicRelevance;
	bool				bReEvalOnSatisfaction;
	EnumAINodeType			eNodeType;
	ENUM_AISmartObjectID	eSmartObjectID;
	float				fRecalcRateMin;
	float				fRecalcRateMax;
	float				fActivateChance;
	float				fInterruptPriority;
	bool				bCanReactivateDuringTransitions;
	float				fFrequency;
	EnumAIAwareness		eMinAwareness;
    EnumAIAwareness		eMaxAwareness;

	AISENSOR_TYPE_LIST	lstSensorTypes;
};
typedef std::vector<EnumAIGoalType, LTAllocator<EnumAIGoalType, LT_MEM_TYPE_OBJECTSHELL> > AIGOAL_TYPE_LIST;

enum ENUM_AIGoalSetID { kAIGoalSetID_Invalid = -1 };
struct AIDB_GoalSetRecord
{
	enum {
		kGS_None		= 0x00,
		kGS_Permanent	= 0x01,
		kGS_Hidden		= 0x02,
	};

	AIDB_GoalSetRecord() { dwGoalSetFlags = kGS_None; }

	std::string			strName;
	ENUM_AIGoalSetID	eGoalSet;
	uint32				dwGoalSetFlags;
	AIGOAL_TYPE_LIST	lstGoalSet;
	AIATTRIBUTES_LIST	lstRequiredAITypes;
};

struct AIMovementOverride 
{
	EnumAIAwareness		eAwareness;
	EnumAIAwarenessMod	eAwarenessMod;
	CAnimationProps		Props;
};
typedef std::vector<AIMovementOverride, LTAllocator<AIMovementOverride, LT_MEM_TYPE_OBJECTSHELL> > AIMOVEMENT_OVERRIDE_LIST;

enum ENUM_AIMovementID { kAIMovementID_Invalid = -1 };
struct AIDB_MovementRecord
{
	std::string			strName;
	ENUM_AIMovementID	eAIMovement;
	EnumAIContext		eAIContext;
	CAnimationProps		Props[kAware_Count];

	AIMOVEMENT_OVERRIDE_LIST	lstMovementOverrides;
};
typedef std::vector<ENUM_AIMovementID, LTAllocator<ENUM_AIMovementID, LT_MEM_TYPE_OBJECTSHELL> > AIMOVEMENT_LIST;

enum ENUM_AIMovementSetID { kAIMovementSetID_Invalid = -1 };
struct AIDB_MovementSetRecord
{
	std::string				strName;
	ENUM_AIMovementSetID	eAIMovementSet;
	ENUM_AIMovementID		eDefaultAIMovement;
	AIMOVEMENT_LIST			lstAIMovementSet;
};

enum ENUM_AIWeaponID { kAIWeaponID_Invalid = -1 };
struct AIDB_AIWeaponRecord
{
	std::string				strName;
	ENUM_AIWeaponID			eAIWeaponID;
	EnumAIWeaponClassType	eAIWeaponClass;
	ENUM_AIWeaponType		eAIWeaponType;

	float				fMaxRange;
	float				fMaxRangeSqr;
	float				fMinRange;
	float				fMinRangeSqr;
	EnumAnimProp		eAIWeaponAnimProp;
	EnumAnimProp		eAIChangeWeaponActionAnimProp;
	int32				nAIWeaponAnimPriority;
	bool				bAllowDirectionalRun;
	bool				bAllowWalk;
	bool				bIsDangerous;
	uint32				nAIMinBurstShots;
	uint32				nAIMaxBurstShots;
	float				fAIMinBurstInterval;
	float				fAIMaxBurstInterval;
	bool				bStrictBursts;
	bool				bSuppressionBursts;
	bool				bAIAnimatesReload;
	bool				bForceMissToFloor;
	float				fPlayerPusherRadius;
	float				fPlayerPusherForce;
	bool				bAllowAmmoGeneration;
	bool				bAllowAutoReload;
	int					nPreference;
	float				fAccuracyScalarMin;
	float				fAccuracyScalarMax;
	float				fDamageScalarMin;
	float				fDamageScalarMax;
	bool				bSyncToUserAnimation;
	float				fFireDuringDeathProbability;
	bool				bRestrictFireCone;
	LTRotation			mFireRotationOffset;
	bool				bLoopFireAnimation;
	char				szDefaultAnimationName[48];
};


enum ENUM_AIWeaponOverrideSetID { kAIWeaponOverrideSetID_Invalid = -1, };
struct AIDB_AIWeaponOverrideSetRecord
{
	struct WeaponPair
	{
		HWEAPON			hWeapon;
		ENUM_AIWeaponID	eAIWeapon;
	};

	std::string				strName;

	typedef std::vector< WeaponPair, LTAllocator< WeaponPair, LT_MEM_TYPE_OBJECTSHELL> > AIWEAPONOVERRIDE_LIST;
	AIWEAPONOVERRIDE_LIST	AIWeaponOverrideList;
};

struct AIDB_AINodeRecord
{
	AIDB_AINodeRecord()
	{
		// By default, all status queries are performed.
		dwRelevantNotAtNodeStatusFlags = 0;
		dwRelevantAtNodeStatusFlags = 0;
	}

	uint32				dwRelevantNotAtNodeStatusFlags;
	uint32				dwRelevantAtNodeStatusFlags;
};

struct AIDB_AINavMeshTypeRecord
{
	AIDB_AINavMeshTypeRecord()
	{
		m_szName[0] = '\0';
		m_dwCharacterTypeMask = 0;
	}

	char				m_szName[30];
	uint32				m_dwCharacterTypeMask;
};

//
// AIDB... 
//

class CAIDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CAIDB );

	public :	// Methods...

		bool	Init( const char *szDatabaseFile = DB_Default_File );
		void	Term() {};

		// Actions.

		uint32					GetNumAIActionRecords() const { return m_cAIActionRecords; }
		AIDB_ActionRecord*		GetAIActionRecord( uint32 iRecord );

		uint32					GetNumAIActionSetRecords() const { return m_cAIActionSetRecords; }
		AIDB_ActionSetRecord*	GetAIActionSetRecord( uint32 iRecord );
		ENUM_AIActionSet		GetAIActionSetRecordID( const char* const szName );
		const char*				GetAIActionSetRecordName( ENUM_AIActionSet eActionSet );

		// Activities.

		uint32					GetNumAIActivitySetRecords() const { return m_cAIActivitySetRecords; }
		AIDB_ActivitySetRecord*	GetAIActivitySetRecord( uint32 iRecord );
		ENUM_AIActivitySet		GetAIActivitySetRecordID( const char* const szName );
		const char*				GetAIActivitySetRecordName( ENUM_AIActivitySet eActivitySet );

		// Attributes.

		uint32					GetNumAIAttributesRecords() const { return m_cAIAttributesRecords; }
		AIDB_AttributesRecord*	GetAIAttributesRecord( uint32 iRecord );
		ENUM_AIAttributesID		GetAIAttributesRecordID( const char* const szName );

		// Limits.

		AIDB_LimitsRecord*		GetAILimitsRecord( uint32 iRecord );
		ENUM_AILimitsID			GetAILimitsRecordID( const char* const szName );

		// Brains.

		uint32					GetNumAIBrainRecords() const { return m_cAIBrainRecords; }
		AIDB_BrainRecord*		GetAIBrainRecord( uint32 iRecord );
		ENUM_AIBrainID			GetAIBrainRecordID( const char* const szName );

		// Constants.

		AIDB_ConstantsRecord*	GetAIConstantsRecord() { return &m_AIConstantsRecord; }

		// DamageMasks.

		AIDB_DamageMaskRecord*	GetAIDamageMaskRecord( uint32 iRecord );
		ENUM_AIDamageMaskID		GetAIDamageMaskRecordID( const char* const szName );

		// Stimuli.

		AIDB_StimulusRecord*	GetAIStimulusRecord( EnumAIStimulusType eStimulusType );

		// Sensors.

		AIDB_SensorRecord*		GetAISensorRecord( EnumAISensorType eSensorType );

		// Targets.

		uint32						GetNumAITargetSelectRecords() const { return m_cAITargetSelectRecords; }
		AIDB_TargetSelectRecord*	GetAITargetSelectRecord( uint32 iRecord );

		uint32						GetNumAITargetSelectSetRecords() const { return m_cAITargetSelectRecords; }
		AIDB_TargetSelectSetRecord*	GetAITargetSelectSetRecord( uint32 iRecord );
		ENUM_AITargetSelectSet		GetAITargetSelectSetRecordID( const char* const szName );
		const char*					GetAITargetSelectSetRecordName( ENUM_AITargetSelectSet eTargetSet );

		// Goals.

		AIDB_GoalRecord*		GetAIGoalRecord( EnumAIGoalType eGoalType );

		uint32					GetNumAIGoalSetRecords() const { return m_cAIGoalSetRecords; }
		AIDB_GoalSetRecord*		GetAIGoalSetRecord( uint32 iRecord );
		ENUM_AIGoalSetID		GetAIGoalSetRecordID( const char* const szName );
		const char*				GetAIGoalSetRecordName( ENUM_AIGoalSetID eGoalSet );

		// Movement.

		AIDB_MovementRecord*	GetAIMovementRecord( uint32 iRecord );
		ENUM_AIMovementID		GetAIMovementRecordID( const char* const szName );
		AIDB_MovementSetRecord*	GetAIMovementSetRecord( uint32 iRecord );
		ENUM_AIMovementSetID	GetAIMovementSetRecordID( const char* const szName );
		const char*				GetAIMovementSetRecordName( ENUM_AIMovementSetID eMovementSet );

		// SmartObjects.

		uint32					GetNumAISmartObjectRecords() const { return m_cAISmartObjectRecords; }
		AIDB_SmartObjectRecord*	GetAISmartObjectRecord( uint32 iRecord );
		ENUM_AISmartObjectID	GetAISmartObjectRecordID( const char* const szName );
		const char*				GetAISmartObjectRecordName( ENUM_AISmartObjectID eSmartObject );

		// AI Weapons.

		ENUM_AIWeaponID			GetAIWeaponRecordID( const char* const szName );
		AIDB_AIWeaponRecord*	GetAIWeaponRecord( uint32 iRecord );

		// AI AIWeapon override Sets.
		ENUM_AIWeaponOverrideSetID GetAIWeaponOverrideSetRecordID( const char* const szName );
		AIDB_AIWeaponOverrideSetRecord* GetAIWeaponOverrideSetRecord( ENUM_AIWeaponOverrideSetID iRecord );


		HRECORD					GetDroppedItemRecord( const char* const szName );

		// AI AmmoLoad.

		uint32					GetNumAIAmmoLoadRecords() const { return m_cAIAmmoLoadRecords; }
		ENUM_AIAmmoLoadRecordID GetAIAmmoLoadRecordID( const char* const szName );
		AIDB_AIAmmoLoadRecord*	GetAIAmmoLoadRecord( ENUM_AIAmmoLoadRecordID iRecord );

		// AI Nodes.

		AIDB_AINodeRecord*		GetAINodeRecord( EnumAINodeType eNodeType );

		// AI Nav Mesh

		uint32					GetNumAINavMeshTypes() const { return m_cAIAINavMeshTypeRecords; }
		const AIDB_AINavMeshTypeRecord* GetAINavMeshTypeRecord( uint32 i ) const;

		// Misc.

		HRECORD					GetMiscRecord();
		HRECORD					GetMiscRecordLink( const char* pszLink );
		const char*				GetMiscString( const char* pszString );
		float					GetMiscFloat( const char* pszFloat );

		// Conversion.

		uint32	String2EnumIndex( const char* szName, uint32 cMax, uint32 iInvalid, const char* aszEnums[] );
		uint32	String2BitFlag( const char* szName, uint32 cMax, const char* aszEnums[] );

	private :

		void	CreateAIActionRecords();
		void	CreateAIActionSetRecords();
		void	CreateAIActivitySetRecords();
		void	CreateAIDamageMaskRecords();
		void	CreateAIBrainRecords();
		void	CreateAIConstantsRecord();
		void	CreateAIAttributesRecords();
		void	CreateAILimitsRecords();
		void	CreateAIStimulusRecords();
		void	CreateAIStimulusMaskRecords();
		void	CreateAISensorRecords();
		void	CreateAITargetRecords();
		void	CreateAITargetSetRecords();
		void	CreateAIGoalRecords();
		void	CreateAIGoalSetRecords();
		void	CreateAIMovementRecords();
		void	CreateAIMovementSetRecords();
		void	CreateAISmartObjectRecords();
		void	CreateAIWeaponRecords();
		void	CreateAIWeaponOverrideSets();
		void	CreateAIWeaponContextSets();
		void	CreateAIAmmoLoadRecords();
		void	CreateAINodeRecords();
		void	CreateAINavMeshTypeRecords();

		void	CollapseIncludedActionSets( HRECORD hRecord, AIActionBitSet* pActionMask );
		void	CollapseIncludedGoalSets( HRECORD hRecord, AIGOAL_TYPE_LIST* plstGoalSet );
		void	CollapseIncludedTargetSelectSets( HRECORD hRecord, AITargetSelectBitSet* pTargetSelectMask );

		uint32	CountMovementOverrides( HRECORD hRecord );

		void	ConvertFOV( float* pFOV );

		uint32	BuildMaskFromAttributeList(HRECORD hRecord, const char* const pszAttributeName, const char** pszStringList, const int nStringListSize);
		uint32	BuildBitsetFromAttributeList(HRECORD hRecord, const char* const pszAttributeName, uint32 iInvalid, const char** pszStringList, const int nStringListSize);

	private	:	// Members...

		uint32					m_cAIActionRecords;
		AIDB_ActionRecord*		m_aAIActionRecords;

		uint32					m_cAIActionSetRecords;
		AIDB_ActionSetRecord*	m_aAIActionSetRecords;

		uint32					m_cAIActivitySetRecords;
		AIDB_ActivitySetRecord*	m_aAIActivitySetRecords;

		uint32					m_cAIDamageMaskRecords;
		AIDB_DamageMaskRecord*	m_aAIDamageMaskRecords;

		uint32					m_cAIBrainRecords;
		AIDB_BrainRecord*		m_aAIBrainRecords;

		AIDB_ConstantsRecord	m_AIConstantsRecord;

		uint32					m_cAIAttributesRecords;
		AIDB_AttributesRecord*	m_aAIAttributesRecords;

		uint32					m_cAILimitsRecords;
		AIDB_LimitsRecord*		m_aAILimitsRecords;

		uint32					m_cAIStimulusRecords;
		AIDB_StimulusRecord*	m_aAIStimulusRecords;

		uint32					m_cAIStimulusMaskRecords;
		AIDB_StimulusMaskRecord*m_aAIStimulusMaskRecords;

		AIDB_SensorRecord		m_aAISensorRecords[kSensor_Count];

		uint32						m_cAITargetSelectRecords;
		AIDB_TargetSelectRecord*	m_aAITargetSelectRecords;

		uint32						m_cAITargetSelectSetRecords;
		AIDB_TargetSelectSetRecord*	m_aAITargetSelectSetRecords;

		AIDB_GoalRecord			m_aAIGoalRecords[kGoal_Count];

		uint32					m_cAIGoalSetRecords;
		AIDB_GoalSetRecord*		m_aAIGoalSetRecords;

		uint32					m_cAIMovementRecords;
		AIDB_MovementRecord*	m_aAIMovementRecords;

		uint32					m_cAIMovementSetRecords;
		AIDB_MovementSetRecord*	m_aAIMovementSetRecords;

		uint32					m_cAISmartObjectRecords;
		AIDB_SmartObjectRecord*	m_aAISmartObjectRecords;

		uint32					m_cAIWeaponRecords;
		AIDB_AIWeaponRecord*	m_aAIWeaponRecords;

		uint32					m_cAIWeaponOverrideSetRecords;
		AIDB_AIWeaponOverrideSetRecord* m_aAIWeaponOverrideSetRecords;

		uint32					m_cAIAmmoLoadRecords;
		AIDB_AIAmmoLoadRecord*	m_aAIAmmoLoadRecords;

		uint32					m_cAINodeRecords;
		AIDB_AINodeRecord*		m_aAINodeRecords;

		uint32					m_cAIAINavMeshTypeRecords;
		AIDB_AINavMeshTypeRecord* m_aAIAINavMeshTypeRecords;
};


#endif // __AI_DATABASE_H__
