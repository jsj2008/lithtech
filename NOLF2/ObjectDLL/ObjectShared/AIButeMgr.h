// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIBUTE_MGR_H__
#define __AIBUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "DamageTypes.h"
#include "CommandIds.h"
#include "CharacterAlignment.h"

#pragma warning (disable : 4786)
#include <map>
#include <bitset>
#include <vector>

#include "UberAssert.h"


// Forward declarations.
class CAIButeMgr;
enum  EnumAISenseType;
enum  EnumAIStimulusType;


// Globals/statics
extern CAIButeMgr* g_pAIButeMgr;

static const char* s_aszSenseTypes[] =
{
	#define SENSE_TYPE_AS_STRING 1
	#include "AISenseTypeEnums.h"
	#undef SENSE_TYPE_AS_STRING
};



typedef std::map<EnumAISenseType, LTFLOAT> AISenseDistanceMap;

typedef struct
{

	char	szName[64];
	char	szAlignment[64];
	char	szHumanType[64];

	LTFLOAT fRunSpeed;
	LTFLOAT fJumpSpeed;
	LTFLOAT fJumpOverSpeed;
	LTFLOAT fFallSpeed;
	LTFLOAT fWalkSpeed;
	LTFLOAT fSwimSpeed;
	LTFLOAT fFlySpeed;
	LTFLOAT fSoundRadius;
	LTFLOAT fHitPoints;
	LTFLOAT fArmor;
	LTFLOAT fAccuracy;
	LTFLOAT fAccuracyIncreaseRate;
	LTFLOAT fAccuracyDecreaseRate;
	LTFLOAT fFullAccuracyRadiusSqr;
	LTFLOAT fAccuracyMissPerturb;
	LTFLOAT fMaxMovementAccuracyPerturb;
	LTFLOAT fMovementAccuracyPerturbDecay;
	LTFLOAT fUpdateRate;
	LTFLOAT fVerticalThreshold;
	LTFLOAT fDefensePercentage;

	LTFLOAT flSentryChallengeScanDistMax;
	LTFLOAT flSentryChallengeDistMax;
	LTFLOAT flSentryMarkDistMax;

	AISenseDistanceMap mapSenseDistance;

	uint32 iUseableVolumesMask;
}
AIBM_Template;


//
// AIDataTypes are used to store info in an AI brain
// that is specific to some kind of AI.
// (e.g. Data that only a ninja needs.)
//

//
// ENUM: Types of AI data.
//
enum EnumAIDataType
{
	kAIData_InvalidType = -1,

	#define AIDATA_TYPE_AS_ENUM 1
	#include "AIDataTypeEnums.h"
	#undef AIDATA_TYPE_AS_ENUM

	kAIData_Count,
};

//
// STRINGS: const strings for AI data types.
//
static const char* s_aszAIDataTypes[] =
{
	#define AIDATA_TYPE_AS_STRING 1
	#include "AIDataTypeEnums.h"
	#undef AIDATA_TYPE_AS_STRING
};

//
// MAP: Map of AIData types and values.
//
typedef std::map<EnumAIDataType, LTFLOAT> AIDATA_MAP;


typedef struct
{
	char	szName[128];

	LTFLOAT fDodgeVectorCheckTime;
	LTFLOAT fDodgeVectorCheckChance;
	LTFLOAT fDodgeVectorDelayTime;
	LTFLOAT fDodgeVectorShuffleDist;
	LTFLOAT fDodgeVectorRollDist;
	LTFLOAT fDodgeVectorShuffleChance;
	LTFLOAT fDodgeVectorRollChance;
	LTFLOAT fDodgeVectorBlockChance;

	LTFLOAT fDodgeProjectileCheckTime;
	LTFLOAT fDodgeProjectileCheckChance;
	LTFLOAT fDodgeProjectileDelayTime;
	LTFLOAT fDodgeProjectileFleeChance;

	LTFLOAT fAttackRangedRangeMin;
	LTFLOAT fAttackRangedRangeMax;
	LTFLOAT fAttackThrownRangeMin;
	LTFLOAT fAttackThrownRangeMax;
	LTFLOAT fAttackMeleeRangeMin;
	LTFLOAT fAttackMeleeRangeMax;
	LTFLOAT fAttackChaseTime;
	LTFLOAT fAttackChaseTimeRandomMin;
	LTFLOAT fAttackChaseTimeRandomMax;
	LTFLOAT fAttackPoseCrouchChance;
	LTFLOAT	fAttackPoseCrouchTime;
	LTFLOAT fAttackGrenadeThrowTime;
	LTFLOAT fAttackGrenadeThrowChance;
	LTFLOAT fAttackCoverCheckTime;
	LTFLOAT fAttackCoverCheckChance;
	LTFLOAT fAttackSoundTimeMin;
	LTBOOL	bAttackWhileMoving;

	LTFLOAT fAttackFromCoverCoverTime;
	LTFLOAT fAttackFromCoverCoverTimeRandom;
	LTFLOAT fAttackFromCoverUncoverTime;
	LTFLOAT fAttackFromCoverUncoverTimeRandom;
	LTFLOAT fAttackFromCoverReactionTime;
	uint32	nAttackFromCoverMaxRetries;

	LTFLOAT fAttackFromVantageAttackTime;
	LTFLOAT fAttackFromVantageAttackTimeRandom;

	LTFLOAT fAttackFromViewChaseTime;

	LTFLOAT fChaseExtraTime;
	LTFLOAT fChaseExtraTimeRandomMin;
	LTFLOAT fChaseExtraTimeRandomMax;
	LTFLOAT fChaseSearchTime;

	LTFLOAT fPatrolSoundTime;
	LTFLOAT fPatrolSoundTimeRandomMin;
	LTFLOAT fPatrolSoundTimeRandomMax;
	LTFLOAT fPatrolSoundChance;

	LTFLOAT fDistressFacingThreshhold;
	LTFLOAT fDistressIncreaseRate;
	LTFLOAT fDistressDecreaseRate;
	uint32	nDistressLevels;

	uint32	nMajorAlarmThreshold;
	uint32	nImmediateAlarmThreshold;

	LTFLOAT fTauntDelay;
	LTFLOAT fTauntChance;

	LTBOOL	bCanLipSync;

	int     nDamageMaskID;

	// Mask of ammo IDs which this AI should not regenerate.  256 was an 
	// arbitrarily high number.  If a higher count is needed, an assert will
	// be triggered, and this value can be made suitably high.  It would be
	// dynamic if a bitset supported dynamic.
	std::bitset<256>	AmmoGenerationRestrictionMask;

	AIDATA_MAP	mapAIData;
}
AIBM_Brain;

typedef struct
{
	EnumAIStimulusType	eStimulusType;
	EnumAISenseType		eSenseType;
	std::vector<CharacterAlignment>	caRequiredAlignment;
	LTFLOAT 			fDistance;
	LTFLOAT				fVerticalRadius;
	LTFLOAT 			fDuration;
	uint8				nAlarmLevel;
	CRange<LTFLOAT> 	rngReactionDelay;
	LTFLOAT 			fStimulationIncreaseRateAlert;
	LTFLOAT 			fStimulationIncreaseRateUnalert;
	LTFLOAT 			fStimulationDecreaseRateAlert;
	LTFLOAT 			fStimulationDecreaseRateUnalert;
	uint8				nFalseStimulationLimit;
	EnumAISenseType		eFalseStimulusSense;
	CRange<LTFLOAT> 	rngStimulationThreshhold;
	uint8				nMaxResponders;
	LTBOOL				bIgnoreOldStimulus;
	LTBOOL				bRequireSourceIsNotSelf;
	LTBOOL				bRequireSourceIsSelf;
}
AIBM_Stimulus;

typedef std::map<EnumAIStimulusType, AIBM_Stimulus*> AIBM_STIMULUS_MAP;

typedef struct
{
	LTFLOAT fJunctionResetTimer;
	LTFLOAT fSearchNodeResetTimer;
	LTFLOAT fBodyResetTimer;
}
AIBM_ResetTimers;

typedef struct
{
	uint32  nApprehendMissionFailedID;
}
AIBM_TextIDs;

typedef struct
{
	LTFLOAT fNoFOVDistanceSqr;
	LTFLOAT fInstantSeeDistanceSqr;
	LTFLOAT fThreatTooCloseDistanceSqr;
	LTFLOAT fPersonalBubbleDistanceSqr;
	LTFLOAT fPersonalBubbleMultiplier;
	LTFLOAT fDarknessMultiplier;
}
AIBM_Senses;

typedef struct
{
	char	szName[64];
	uint32	flagsBlockedSenses;
}
AIBM_SenseMask;

typedef struct
{
	char	szName[64];
	DamageFlags	dfDamageTypes;
}
AIBM_DamageMask;

typedef struct
{
	LTFLOAT fNoTargetTime;

}
AIBM_Attract;

typedef struct
{
	LTBOOL bEnableMeleeRepathing;
	LTBOOL bKeepInvalidNodeHistory;
	LTBOOL bUseTrueWeaponPosition;
}
AIBM_Rules;


class CAIButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CAIButeMgr();
		~CAIButeMgr();

		LTBOOL		Init(const char* szAttributeFile = "Attributes\\AIButes.txt");
		void		Term();

		// Templates

		int GetTemplateIDByName(const char *szName);
		int GetNumTemplates() { return m_cTemplateID; }
		AIBM_Template* GetTemplate(int iTemplate) { return &m_aTemplates[iTemplate]; }

		// Brains

		int GetBrainIDByName(const char *szName);
		int GetNumBrains() { return m_cBrainID; }
		AIBM_Brain* GetBrain(int iBrain) { return &m_aBrains[iBrain]; }

		// SenseMasks

		int GetSenseMaskIDByName(const char *szName);
		int GetNumSenseMasks() { return m_cSenseMaskID; }
		AIBM_SenseMask* GetSenseMask(int iSenseMask) { return &m_aSenseMasks[iSenseMask]; }

		// DamageMasks

		int GetDamageMaskIDByName(const char *szName);
		int GetNumDamageMasks() { return m_cDamageMaskID; }
		AIBM_DamageMask* GetDamageMask(int iDamageMask) { return &m_aDamageMasks[iDamageMask]; }

		// Butes

		AIBM_ResetTimers*	GetResetTimers() { return &m_ResetTimers; }
		AIBM_TextIDs*		GetTextIDs() { return &m_TextIDs; }
		AIBM_Senses*		GetSenses() { return &m_Senses; }
		AIBM_Attract*		GetAttract() { return &m_Attract; }

		AIBM_Stimulus*		GetStimulus(EnumAIStimulusType eStimulusType);

		AIBM_Rules*			GetRules() { return &m_Rules; }

	protected :

		void SetTemplate(int iTemplate);
		void SetBrain(int iBrain);
		void SetSenseMask(int iSenseMask);
		void SetDamageMask(int iDamageMask);
		void SetStimulus(int iStimulus);

		void SetResetTimers();
		void SetTextIDs();
		void SetSenses();
		void SetAttract();
		void SetRules();
		void SetDEdit();

	private :

		int 					m_cTemplateID;
		AIBM_Template*			m_aTemplates;

		int 					m_cBrainID;
		AIBM_Brain* 			m_aBrains;

		int 					m_cSenseMaskID;
		AIBM_SenseMask* 		m_aSenseMasks;

		int 					m_cDamageMaskID;
		AIBM_DamageMask* 		m_aDamageMasks;

		AIBM_ResetTimers		m_ResetTimers;
		AIBM_TextIDs			m_TextIDs;
		AIBM_Senses 			m_Senses;
		AIBM_Attract			m_Attract;
		AIBM_Rules				m_Rules;

		AIBM_STIMULUS_MAP		m_mapStimuli;
};

#endif // __AIBUTE_MGR_H__
