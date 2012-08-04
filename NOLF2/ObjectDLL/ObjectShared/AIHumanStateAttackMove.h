// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_ATTACK_MOVE_H__
#define __AI_HUMAN_STATE_ATTACK_MOVE_H__

#include "AIHumanState.h"


class CAIHumanStateAttackMove : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackMove, kState_HumanAttackMove);

		CAIHumanStateAttackMove( );
		~CAIHumanStateAttackMove( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		
		void SetAttackMove(EnumAnimProp	eMove) { m_eAttackMove = eMove; }
		void SetAttackMoveDest(LTVector vDest) { m_vAttackMoveDest = vDest; }

	protected:

		void SelectMoveAnim();

	protected :

		EnumAnimProp	m_eAttackMove;
		LTVector		m_vAttackMoveDest;
		LTBOOL			m_bTurnedAround;
};

#endif