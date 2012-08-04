// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackGrenadeFromCover.h
//
// PURPOSE : AIActionAttackGrenadeFromCover class definition
//
// CREATED : 01/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_GRENADE_FROM_COVER_H__
#define __AIACTION_ATTACK_GRENADE_FROM_COVER_H__

#include "AIActionAttackGrenade.h"

class CAIActionAttackGrenadeFromCover : public CAIActionAttackGrenade
{
	typedef CAIActionAttackGrenade super;

public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackGrenadeFromCover, kAct_AttackGrenadeFromCover );

		CAIActionAttackGrenadeFromCover();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
