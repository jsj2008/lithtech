// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_ATTACK_PRONE_H__
#define __AI_HUMAN_STATE_ATTACK_PRONE_H__

#include "AIHumanState.h"


class CAIHumanStateAttackProne : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackProne, kState_HumanAttackProne);

		CAIHumanStateAttackProne( );
		~CAIHumanStateAttackProne( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		
	protected :

		LTFLOAT		m_fStayProneTime;
		LTFLOAT		m_fLastFiredTime;
		LTFLOAT		m_fHalfMinDistSqr;
};

#endif