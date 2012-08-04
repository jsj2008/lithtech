// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromNodeBounded.h
//
// PURPOSE : AIActionAttackFromNodeBounded class definition
//
// CREATED : 6/27/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_NODE_BOUNDED_H__
#define __AIACTION_ATTACK_FROM_NODE_BOUNDED_H__

#include "AIActionAttackFromNode.h"


class CAIActionAttackFromNodeBounded : public CAIActionAttackFromNode
{
	typedef CAIActionAttackFromNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromNodeBounded, kAct_AttackFromNodeBounded );

		CAIActionAttackFromNodeBounded();
};

// ----------------------------------------------------------------------- //

#endif
