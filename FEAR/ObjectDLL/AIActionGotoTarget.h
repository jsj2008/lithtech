// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTarget.h
//
// PURPOSE : AIActionGotoTarget abstract class definition
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_TARGET_H__
#define __AIACTION_GOTO_TARGET_H__

#include "AIActionGotoAbstract.h"

class CAIActionGotoTarget : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTarget, kAct_GotoTarget );

		CAIActionGotoTarget();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

};

// ----------------------------------------------------------------------- //

#endif
