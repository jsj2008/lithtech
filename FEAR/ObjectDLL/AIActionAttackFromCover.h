// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromCover.h
//
// PURPOSE : AIActionAttackFromCover class definition
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_COVER_H__
#define __AIACTION_ATTACK_FROM_COVER_H__

#include "AIActionAttackFromNode.h"

class CAI;
class CAIWorldState;


class CAIActionAttackFromCover : public CAIActionAttackFromNode
{
	typedef CAIActionAttackFromNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromCover, kAct_AttackFromCover );

		CAIActionAttackFromCover();

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
