// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIBUTE_MGR_H__
#define __AIBUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "DamageTypes.h"
#include "CommandIds.h"
#include "CharacterAlignment.h"
#include "AIUtils.h"

class CAIButeMgr;
extern CAIButeMgr* g_pAIButeMgr;

typedef struct
{
	char	szName[64];
	CharacterClass ccAlignment;
    LTFLOAT  fRunSpeed;
    LTFLOAT  fCreepSpeed;
    LTFLOAT  fWalkSpeed;
    LTFLOAT  fRollSpeed;
    LTFLOAT  fSwimSpeed;
    LTFLOAT  fFlySpeed;
    LTFLOAT  fMarksmanship;
    LTFLOAT  fBravado;
    LTFLOAT  fAwareness;
    LTFLOAT  fSoundRadius;
    LTFLOAT  fHitPoints;
    LTFLOAT  fArmor;
    LTFLOAT  fAccuracy;
    LTFLOAT  fAccuracyIncreaseRate;
    LTFLOAT  fAccuracyDecreaseRate;
    LTFLOAT  fLag;
    LTFLOAT  fLagIncreaseRate;
    LTFLOAT  fLagDecreaseRate;
    LTBOOL   bCanSeeEnemy;
    LTFLOAT  fSeeEnemyDistance;
    LTBOOL   bCanSeeAllyDeath;
    LTFLOAT  fSeeAllyDeathDistance;
    LTBOOL   bCanHearAllyDeath;
    LTFLOAT  fHearAllyDeathDistance;
    LTBOOL   bCanHearAllyPain;
    LTFLOAT  fHearAllyPainDistance;
    LTBOOL   bCanHearEnemyFootstep;
    LTFLOAT  fHearEnemyFootstepDistance;
    LTBOOL   bCanSeeEnemyFootprint;
    LTFLOAT  fSeeEnemyFootprintDistance;
    LTBOOL   bCanHearEnemyWeaponImpact;
    LTFLOAT  fHearEnemyWeaponImpactDistance;
    LTBOOL   bCanHearEnemyWeaponFire;
    LTFLOAT  fHearEnemyWeaponFireDistance;
    LTBOOL   bCanHearAllyWeaponFire;
    LTFLOAT  fHearAllyWeaponFireDistance;
    LTBOOL   bCanSeeEnemyFlashlight;
    LTFLOAT  fSeeEnemyFlashlightDistance;
    LTBOOL   bCanHearEnemyDisturbance;
    LTFLOAT  fHearEnemyDisturbanceDistance;
    LTFLOAT  fBarkDistance;
    LTFLOAT  fAttackDistance;
}
AIBM_Template;

typedef struct
{
	char	szName[128];

	LTFLOAT	fDodgeVectorCheckTime;
	LTFLOAT	fDodgeVectorCheckChance;
	LTFLOAT fDodgeVectorDelayTime;
	LTFLOAT fDodgeVectorShuffleChance;
    LTFLOAT fDodgeVectorRollChance;
    LTFLOAT fDodgeVectorCoverChance;

	LTFLOAT	fDodgeProjectileCheckTime;
	LTFLOAT	fDodgeProjectileCheckChance;
	LTFLOAT fDodgeProjectileDelayTime;
    LTFLOAT fDodgeProjectileFleeChance;

	LTFLOAT	fAttackVectorRangeMin;
	LTFLOAT	fAttackVectorRangeMax;
	LTFLOAT	fAttackProjectileRangeMin;
	LTFLOAT	fAttackProjectileRangeMax;
	LTFLOAT	fAttackMeleeRangeMin;
	LTFLOAT	fAttackMeleeRangeMax;
	LTFLOAT	fAttackChaseTime;
	LTFLOAT	fAttackChaseTimeRandomMin;
	LTFLOAT	fAttackChaseTimeRandomMax;
	LTFLOAT	fAttackPoseCrouchChance;
	LTFLOAT fAttackGrenadeThrowTime;
	LTFLOAT fAttackGrenadeThrowChance;
	LTFLOAT fAttackCoverCheckTime;
	LTFLOAT fAttackCoverCheckChance;
	LTFLOAT fAttackSoundChance;

	LTFLOAT	fAttackFromCoverCoverTime;
	LTFLOAT	fAttackFromCoverCoverTimeRandom;
	LTFLOAT	fAttackFromCoverUncoverTime;
	LTFLOAT	fAttackFromCoverUncoverTimeRandom;
	LTFLOAT	fAttackFromCoverReactionTime;
	uint32	nAttackFromCoverMaxRetries;
	LTFLOAT fAttackFromCoverSoundChance;

	LTFLOAT	fAttackFromVantageAttackTime;
	LTFLOAT	fAttackFromVantageAttackTimeRandom;

	LTFLOAT fAttackFromViewChaseTime;

	LTFLOAT	fChaseSoundTime;
	LTFLOAT	fChaseSoundTimeRandomMin;
	LTFLOAT	fChaseSoundTimeRandomMax;
	LTFLOAT	fChaseFoundSoundChance;
	LTFLOAT	fChaseExtraTime;
	LTFLOAT	fChaseExtraTimeRandomMin;
	LTFLOAT	fChaseExtraTimeRandomMax;
	LTFLOAT	fChaseSearchTime;

	LTFLOAT	fPatrolSoundTime;
	LTFLOAT	fPatrolSoundTimeRandomMin;
	LTFLOAT	fPatrolSoundTimeRandomMax;
	LTFLOAT	fPatrolSoundChance;

	LTFLOAT	fDistressFacingThreshhold;
	LTFLOAT	fDistressIncreaseRate;
	LTFLOAT	fDistressDecreaseRate;
	uint32	nDistressLevels;
}
AIBM_Brain;

#define SENSE_STIMULATION(sense) \
    LTFLOAT  f##sense##StimulationIncreaseRateAlert; \
    LTFLOAT  f##sense##StimulationIncreaseRateUnalert; \
    LTFLOAT  f##sense##StimulationDecreaseRateAlert; \
    LTFLOAT  f##sense##StimulationDecreaseRateUnalert; \
    CRange<LTFLOAT>  rng##sense##StimulationThreshhold; \
	int		n##sense##FalseStimulationLimit;

#define SENSE_DELAY(sense) \
    LTFLOAT  f##sense##ReactionDelay;

typedef struct
{
    LTFLOAT fAllyDeathNoiseDistance;
    LTFLOAT fAllyPainNoiseDistance;
    LTFLOAT fEnemyMovementNoiseDistance;
    LTFLOAT fCoinNoiseDistance;
    LTFLOAT fNoFOVDistanceSqr;
    LTFLOAT fInstantSeeDistanceSqr;

	SENSE_STIMULATION(SeeEnemy);
	SENSE_DELAY(SeeEnemyFootprint);
	SENSE_STIMULATION(SeeEnemyFlashlight);
	SENSE_DELAY(HearEnemyWeaponFire);
	SENSE_DELAY(HearEnemyWeaponImpact);
	SENSE_STIMULATION(HearEnemyFootstep);
	SENSE_DELAY(HearEnemyDisturbance);
	SENSE_DELAY(SeeAllyDeath);
	SENSE_DELAY(HearAllyDeath);
	SENSE_DELAY(HearAllyPain);
	SENSE_DELAY(HearAllyWeaponFire);
}
AIBM_Senses;

typedef struct
{
    LTFLOAT fNoTargetTime;
}
AIBM_Attract;

typedef struct
{
}
AIBM_DEdit;

class CAIButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CAIButeMgr();
		~CAIButeMgr();

        LTBOOL       Init(ILTCSBase *pInterface, const char* szAttributeFile = "Attributes\\AIButes.txt");
		void		Term();

		// Templates

		int	GetTemplateIDByName(const char *szName);
		int	GetNumTemplates() { return m_cTemplateID; }
		AIBM_Template* GetTemplate(int iTemplate) { return &m_aTemplates[iTemplate]; }

		// Brains

		int	GetBrainIDByName(const char *szName);
		int	GetNumBrains() { return m_cBrainID; }
		AIBM_Brain* GetBrain(int iBrain) { return &m_aBrains[iBrain]; }

		// Butes

		AIBM_Senses*			GetSenses() { return &m_Senses; }
		AIBM_Attract*			GetAttract() { return &m_Attract; }
		AIBM_DEdit*				GetDEdit() { return &m_DEdit; }

	protected :

		void SetTemplate(int iTemplate);
		void SetBrain(int iBrain);

		void SetSenses();
		void SetAttract();
		void SetDEdit();

	private :

		int						m_cTemplateID;
		AIBM_Template*			m_aTemplates;

		int						m_cBrainID;
		AIBM_Brain*				m_aBrains;

		AIBM_Senses				m_Senses;
		AIBM_Attract			m_Attract;
		AIBM_DEdit				m_DEdit;
};

#endif // __AIBUTE_MGR_H__