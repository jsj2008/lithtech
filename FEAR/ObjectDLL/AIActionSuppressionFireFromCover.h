// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSuppressionFireFromCover.h
//
// PURPOSE : AIActionSuppressionFireFromCover class definition
//
// CREATED : 01/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_SUPPRESSION_FIRE_FROM_COVER_H__
#define __AIACTION_SUPPRESSION_FIRE_FROM_COVER_H__

#include "AIActionAttackFromCover.h"


class CAIActionSuppressionFireFromCover : public CAIActionAttackFromCover
{
	typedef CAIActionAttackFromCover super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSuppressionFireFromCover, kAct_SuppressionFireFromCover );

		CAIActionSuppressionFireFromCover();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
