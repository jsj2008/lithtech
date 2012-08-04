// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTargetLost.h
//
// PURPOSE : AIActionGotoTargetLost class definition
//
// CREATED : 01/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_TARGET_LOST_H__
#define __AIACTION_GOTO_TARGET_LOST_H__

#include "AIActionGotoAbstract.h"

class CAIActionGotoTargetLost : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTargetLost, kAct_GotoTargetLost );

		CAIActionGotoTargetLost();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

};

// ----------------------------------------------------------------------- //

#endif
