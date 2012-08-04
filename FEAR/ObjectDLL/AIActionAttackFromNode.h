// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromNode.h
//
// PURPOSE : AIActionAttackFromNode class definition
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_NODE_H__
#define __AIACTION_ATTACK_FROM_NODE_H__

#include "AIActionAttack.h"

class CAI;
class CAIWorldState;


class CAIActionAttackFromNode : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromNode, kAct_AttackFromNode );

		CAIActionAttackFromNode();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

	protected:

		uint32			m_dwNodeStatus;
};

// ----------------------------------------------------------------------- //

#endif
