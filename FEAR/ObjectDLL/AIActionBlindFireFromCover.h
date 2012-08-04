// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBlindFireFromCover.h
//
// PURPOSE : AIActionBlindFireFromCover class definition
//
// CREATED : 02/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_BLINDFIRE_FROM_COVER_H__
#define __AIACTION_BLINDFIRE_FROM_COVER_H__

#include "AIActionAttackFromCover.h"


class CAIActionBlindFireFromCover : public CAIActionAttackFromCover
{
	typedef CAIActionAttackFromCover super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBlindFireFromCover, kAct_BlindFireFromCover );

		CAIActionBlindFireFromCover();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
