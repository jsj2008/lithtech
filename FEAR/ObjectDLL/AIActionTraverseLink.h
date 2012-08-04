// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseLink.h
//
// PURPOSE : AIActionTraverseLink class definition
//
// CREATED : 7/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_TRAVERSE_LINK_H__
#define __AIACTION_TRAVERSE_LINK_H__

#include "AIActionAbstract.h"

class CAIWorldState;

class CAIActionTraverseLink : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseLink, kAct_TraverseLink );

		CAIActionTraverseLink();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
		virtual bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
