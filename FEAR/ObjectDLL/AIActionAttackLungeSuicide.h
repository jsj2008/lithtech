// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeSuicide.h
//
// PURPOSE : AIActionAttackLungeSuicide class definition
//
// CREATED : 11/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_LUNGE_SUICIDE_H__
#define __AIACTION_ATTACK_LUNGE_SUICIDE_H__

#include "AIActionAttackLungeUncloaked.h"


class CAIActionAttackLungeSuicide : public CAIActionAttackLungeUncloaked
{
	typedef CAIActionAttackLungeUncloaked super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeSuicide, kAct_AttackLungeSuicide );

		// CAIActionAbstract members.

		virtual void			DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
