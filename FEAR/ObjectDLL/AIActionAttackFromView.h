// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromView.h
//
// PURPOSE : AIActionAttackFromView class definition
//
// CREATED : 10/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_VIEW_H__
#define __AIACTION_ATTACK_FROM_VIEW_H__

#include "AIActionAttack.h"

class CAIActionAttackFromView : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromView, kAct_AttackFromView );

		CAIActionAttackFromView();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
