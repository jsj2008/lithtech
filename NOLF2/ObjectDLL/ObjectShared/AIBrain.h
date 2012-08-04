// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_BRAIN_H_
#define _AI_BRAIN_H_

#include "ltengineobjects.h"
#include "AIClassFactory.h"
#include "AIButeMgr.h"
#include "AIUtils.h"

class CAIHuman;
class CCharacter;
enum Direction;
enum EnumAIWeaponType;

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
	eDodgeActionBlock,
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


class CAIBrain : public CAIClassAbstract
{
	public :
		DECLARE_AI_FACTORY_CLASS(CAIBrain);

		CAIBrain( );
		~CAIBrain( );

		enum Constants
		{
			kMaxNameLength = 128,
		};

	public :

		// Ctors/Dtors/Etc

		void Init(CAIHuman* pAIHuman, const char* szName);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Dodge

		void GetDodgeStatus(DodgeStatus* peDodgeStatus, Direction* peDirection, DodgeAction* peDodgeAction, HOBJECT* phNode, LTBOOL bForceDodge);
		LTBOOL TargetIsAimingAtMe();
		LTBOOL CharacterIsAimingAtMe(CCharacter* pCharacter);

		void DodgeDelay();
		HSTRING GetDodgeProjectileName() const { return m_hstrDodgeProjectileName; }
		const LTVector& GetDodgeProjectilePosition() const { return m_vDodgeProjectilePosition; }

		// Range

		RangeStatus GetRangeStatus();
		RangeStatus GetRangeStatus( EnumAIWeaponType eWeaponType );
		RangeStatus GetRangeStatus( EnumAIWeaponType eWeaponType, const LTVector& vSourcePos );

		// Grenade

		GrenadeStatus GetGrenadeStatus();

		// Simple accessors

		const char* GetName() const { return m_szName; }

		LTFLOAT	GetAttackChaseTime() const { return m_pBrain->fAttackChaseTime; }
		LTFLOAT	GetAttackChaseTimeRandomMin() const { return m_pBrain->fAttackChaseTimeRandomMin; }
		LTFLOAT	GetAttackChaseTimeRandomMax() const { return m_pBrain->fAttackChaseTimeRandomMax; }
		LTFLOAT GetAttackPoseCrouchChance() const { return m_pBrain->fAttackPoseCrouchChance; }
		LTFLOAT GetAttackPoseCrouchTime() const { return m_pBrain->fAttackPoseCrouchTime; }
		LTFLOAT GetAttackSoundTimeMin() const { return m_pBrain->fAttackSoundTimeMin; }
		LTBOOL  AttacksWhileMoving() const { return m_pBrain->bAttackWhileMoving; }

		LTFLOAT	GetAttackFromCoverCoverTime() const;
		LTFLOAT	GetAttackFromCoverCoverTimeRandom() const;
		LTFLOAT	GetAttackFromCoverUncoverTime() const;
		LTFLOAT	GetAttackFromCoverUncoverTimeRandom() const;
		LTFLOAT	GetAttackFromCoverReactionTime() const;
		uint32	GetAttackFromCoverMaxRetries() const { return m_pBrain->nAttackFromCoverMaxRetries; }

		LTFLOAT	GetAttackFromVantageAttackTime() const;
		LTFLOAT	GetAttackFromVantageAttackTimeRandom() const;

		LTFLOAT GetAttackFromViewChaseTime() const { return m_pBrain->fAttackFromViewChaseTime; }

		LTFLOAT	GetChaseExtraTime() const { return m_pBrain->fChaseExtraTime; }
		LTFLOAT	GetChaseExtraTimeRandomMin() const { return m_pBrain->fChaseExtraTimeRandomMin; }
		LTFLOAT	GetChaseExtraTimeRandomMax() const { return m_pBrain->fChaseExtraTimeRandomMax; }
		LTFLOAT GetChaseSearchTime() const { return RAISE_BY_DIFFICULTY(m_pBrain->fChaseSearchTime); }

		LTFLOAT GetDodgeVectorShuffleDist() const { return m_pBrain->fDodgeVectorShuffleDist; }
		LTFLOAT GetDodgeVectorCheckChance() const { return RAISE_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckChance); }

		LTFLOAT	GetPatrolSoundTime() const { return m_pBrain->fPatrolSoundTime; }
		LTFLOAT	GetPatrolSoundTimeRandomMin() const { return m_pBrain->fPatrolSoundTimeRandomMin; }
		LTFLOAT	GetPatrolSoundTimeRandomMax() const { return m_pBrain->fPatrolSoundTimeRandomMax; }
		LTFLOAT	GetPatrolSoundChance() const { return m_pBrain->fPatrolSoundChance; }

		LTFLOAT	GetDistressFacingThreshhold() const { return m_pBrain->fDistressFacingThreshhold; }
		LTFLOAT	GetDistressIncreaseRate() const { return m_pBrain->fDistressIncreaseRate; }
		LTFLOAT	GetDistressDecreaseRate() const { return m_pBrain->fDistressDecreaseRate; }
		uint32	GetDistressLevels() const { return m_pBrain->nDistressLevels; }

		uint32	GetMajorAlarmThreshold() const { return m_pBrain->nMajorAlarmThreshold; }
		uint32	GetImmediateAlarmThreshold() const { return m_pBrain->nImmediateAlarmThreshold; }

		LTFLOAT	GetTauntDelay() const { return m_pBrain->fTauntDelay; }
		LTFLOAT	GetTauntChance() const { return m_pBrain->fTauntChance; }

		LTBOOL  CanLipSync() const { return m_pBrain->bCanLipSync; }

		int		GetDamageMaskID() const { return m_pBrain->nDamageMaskID; }

		uint32 IsAmmoGenerationRestricted( uint32 nAmmoID ) const;


		LTFLOAT	GetAIData(EnumAIDataType eAIDataType);
		LTBOOL	GetAIDataExist(EnumAIDataType eAIDataType);

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
		LTFLOAT		m_fRandomRangedVariation;
};

#endif // _AI_BRAIN_H_
