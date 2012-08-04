// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_BRAIN_H_
#define _AI_BRAIN_H_

#include "ltengineobjects.h"
#include "AIClassFactory.h"
#include "AIUtils.h"
#include "AIDB.h"

class CCharacter;

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

		void Init(CAI* pAI, const char* szName);

		// Simple accessors

		const char* GetName() const { return m_szName; }

		float GetAttackPoseCrouchTime() const { return m_pBrain->fAttackPoseCrouchTime; }

		float	GetAttackGrenadeThrowTimeMin() const { return m_pBrain->fAttackGrenadeThrowTimeMin; }
		float	GetAttackGrenadeThrowTimeMax() const { return m_pBrain->fAttackGrenadeThrowTimeMax; }

		float GetDodgeVectorRollDist() const { return m_pBrain->fDodgeVectorRollDist; }
		float GetDodgeVectorShuffleDist() const { return m_pBrain->fDodgeVectorShuffleDist; }

		uint32	GetMajorAlarmThreshold() const { return m_pBrain->nMajorAlarmThreshold; }
		uint32	GetImmediateAlarmThreshold() const { return m_pBrain->nImmediateAlarmThreshold; }

		bool  CanLipSync() const { return m_pBrain->bCanLipSync; }

	protected :

		CAI* GetAI() { return m_pAI; }

	protected :

		char				m_szName[kMaxNameLength];
		CAI*				m_pAI;
		AIDB_BrainRecord*	m_pBrain;
};

#endif // _AI_BRAIN_H_
