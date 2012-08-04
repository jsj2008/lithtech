// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromAmbush.h
//
// PURPOSE : AIActionAttackFromAmbush class definition
//
// CREATED : 6/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_AMBUSH_H__
#define __AIACTION_ATTACK_FROM_AMBUSH_H__

#include "AIActionAttackFromNode.h"

class CAI;
class CAIWorldState;


class CAIActionAttackFromAmbush : public CAIActionAttackFromNode
{
	typedef CAIActionAttackFromNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromAmbush, kAct_AttackFromAmbush );

		CAIActionAttackFromAmbush();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
