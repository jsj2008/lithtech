// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLunge3D.h
//
// PURPOSE : AIActionAttackLunge3D class definition
//
// CREATED : 9/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_LUNGE_3D_H__
#define __AIACTION_ATTACK_LUNGE_3D_H__

#include "AIActionAttackLungeUncloaked.h"


class CAIActionAttackLunge3D : public CAIActionAttackLungeUncloaked
{
	typedef CAIActionAttackLungeUncloaked super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLunge3D, kAct_AttackLunge3D );

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );

	protected:

		virtual void				SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

		virtual float				GetLungeMaxDist( CAI* pAI, CAIWMFact* pDesireFact );
};

// ----------------------------------------------------------------------- //

#endif
