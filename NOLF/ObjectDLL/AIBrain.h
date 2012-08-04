// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_BRAIN_H_
#define _AI_BRAIN_H_

#include "ltengineobjects.h"

class CAIHuman;

enum DodgeStatus
{
	eDodgeStatusVector,
	eDodgeStatusProjectile,
	eDodgeStatusOk,
};

enum DodgeAction
{
	eDodgeActionShuffle,
	eDodgeActionRoll,
	eDodgeActionDive,
	eDodgeActionFlee,
	eDodgeActionCover,
};

enum RangeStatus
{
	eRangeStatusOk,
	eRangeStatusTooClose,
	eRangeStatusTooFar,
};

enum GrenadeStatus
{
	eGrenadeStatusThrow,
	eGrenadeStatusDontThrow,
};

class CAIBrain : DEFINE_FACTORY_CLASS(CAIBrain)
{
	DEFINE_FACTORY_METHODS(CAIBrain);

	public :

		enum Constants
		{
			kMaxNameLength = 128,
		};

	public :

		// Ctors/Dtors/Etc

		void Init(CAIHuman* pAIHuman, const char* szName);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Dodge

		void GetDodgeStatus(DodgeStatus* peDodgeStatus, Direction* peDirection, DodgeAction* peDodgeAction, uint32* pdwNode);

		void DodgeDelay();
		HSTRING GetDodgeProjectileName() const { return m_hstrDodgeProjectileName; }
		const LTVector& GetDodgeProjectilePosition() const { return m_vDodgeProjectilePosition; }

		// Range

		RangeStatus GetRangeStatus();

		// Grenade

		GrenadeStatus GetGrenadeStatus();

		// Simple accessors

		const char* GetName() const { return m_szName; }

		LTFLOAT	GetAttackChaseTime() const { return m_pBrain->fAttackChaseTime; }
		LTFLOAT	GetAttackChaseTimeRandomMin() const { return m_pBrain->fAttackChaseTimeRandomMin; }
		LTFLOAT	GetAttackChaseTimeRandomMax() const { return m_pBrain->fAttackChaseTimeRandomMax; }
		LTFLOAT GetAttackPoseCrouchChance() const { return m_pBrain->fAttackPoseCrouchChance; }
		LTFLOAT GetAttackSoundChance() const { return m_pBrain->fAttackSoundChance; }

		LTFLOAT	GetAttackFromCoverCoverTime() const { return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverCoverTime); }
		LTFLOAT	GetAttackFromCoverCoverTimeRandom() const { return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverCoverTimeRandom); }
		LTFLOAT	GetAttackFromCoverUncoverTime() const { return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromCoverUncoverTime); }
		LTFLOAT	GetAttackFromCoverUncoverTimeRandom() const { return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromCoverUncoverTimeRandom); }
		LTFLOAT	GetAttackFromCoverReactionTime() const { return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverReactionTime); }
		uint32	GetAttackFromCoverMaxRetries() const { return m_pBrain->nAttackFromCoverMaxRetries; }
		LTFLOAT GetAttackFromCoverSoundChance() const { return m_pBrain->fAttackFromCoverSoundChance; }

		LTFLOAT	GetAttackFromVantageAttackTime() const { return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromVantageAttackTime); }
		LTFLOAT	GetAttackFromVantageAttackTimeRandom() const { return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromVantageAttackTimeRandom); }

		LTFLOAT GetAttackFromViewChaseTime() const { return m_pBrain->fAttackFromViewChaseTime; }

		LTFLOAT	GetChaseSoundTime() const { return m_pBrain->fChaseSoundTime; }
		LTFLOAT	GetChaseSoundTimeRandomMin() const { return m_pBrain->fChaseSoundTimeRandomMin; }
		LTFLOAT	GetChaseSoundTimeRandomMax() const { return m_pBrain->fChaseSoundTimeRandomMax; }
		LTFLOAT	GetChaseFoundSoundChance() const { return m_pBrain->fChaseFoundSoundChance; }
		LTFLOAT	GetChaseExtraTime() const { return m_pBrain->fChaseExtraTime; }
		LTFLOAT	GetChaseExtraTimeRandomMin() const { return m_pBrain->fChaseExtraTimeRandomMin; }
		LTFLOAT	GetChaseExtraTimeRandomMax() const { return m_pBrain->fChaseExtraTimeRandomMax; }
		LTFLOAT GetChaseSearchTime() const { return RAISE_BY_DIFFICULTY(m_pBrain->fChaseSearchTime); }

		LTFLOAT	GetPatrolSoundTime() const { return m_pBrain->fPatrolSoundTime; }
		LTFLOAT	GetPatrolSoundTimeRandomMin() const { return m_pBrain->fPatrolSoundTimeRandomMin; }
		LTFLOAT	GetPatrolSoundTimeRandomMax() const { return m_pBrain->fPatrolSoundTimeRandomMax; }
		LTFLOAT	GetPatrolSoundChance() const { return m_pBrain->fPatrolSoundChance; }

		LTFLOAT	GetDistressFacingThreshhold() const { return m_pBrain->fDistressFacingThreshhold; }
		LTFLOAT	GetDistressIncreaseRate() const { return m_pBrain->fDistressIncreaseRate; }
		LTFLOAT	GetDistressDecreaseRate() const { return m_pBrain->fDistressDecreaseRate; }
		uint32	GetDistressLevels() const { return m_pBrain->nDistressLevels; }

	protected :

		CAIHuman* GetAI() { return m_pAIHuman; }

	protected :

		char		m_szName[kMaxNameLength];
		CAIHuman*	m_pAIHuman;
		AIBM_Brain*	m_pBrain;

		RangeStatus	m_eRangeStatusLast;
		LTFLOAT		m_fRangeStatusCheckTime;

		DodgeStatus	m_eDodgeStatusLast;
		LTFLOAT		m_fDodgeStatusCheckTimeVector;
		LTFLOAT		m_fDodgeStatusCheckTimeProjectile;
		HSTRING		m_hstrDodgeProjectileName;
		LTVector	m_vDodgeProjectilePosition;
		LTFLOAT		m_fGrenadeStatusCheckTime;
};

#endif // _AI_BRAIN_H_
