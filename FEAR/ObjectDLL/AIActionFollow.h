// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollow.h
//
// PURPOSE : AIActionFollow class definition
//
// CREATED : 4/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_FOLLOW_H__
#define __AIACTION_FOLLOW_H__

#include "AIActionGotoAbstract.h"

class CAIActionFollow : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollow, kAct_Follow );

		CAIActionFollow();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	ValidateAction( CAI* pAI );
		virtual	bool	IsActionComplete( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
