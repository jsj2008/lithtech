// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackRangedDynamic.h
//
// PURPOSE : AIGoalAttackRangedDynamic class definition
//
// CREATED : 6/5/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_RANGED_DYNAMIC_H__
#define __AIGOAL_ATTACK_RANGED_DYNAMIC_H__

#include "AIGoalAttackRanged.h"


class CAIGoalAttackRangedDynamic : public CAIGoalAttackRanged
{
	typedef CAIGoalAttackRanged super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackRangedDynamic, kGoal_AttackRangedDynamic);

		CAIGoalAttackRangedDynamic( );

		virtual void	InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

	protected:

		// Attack Moves.

		void	ResetMoveTime();
		LTBOOL	SelectAttackMove();

		// State Handling.

		void HandleStateAttackMove();

	protected:

		enum { kNumAttackMoves = 7, };

		// Save:

		LTFLOAT			m_fMoveTime;

		// Does not need to be saved:

		EnumAnimProp	m_eAttackMoves[kNumAttackMoves];
};


#endif
