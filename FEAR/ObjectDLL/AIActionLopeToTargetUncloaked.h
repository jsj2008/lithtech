// ----------------------------------------------------------------------- //
//
// MODULE  : CAIActionLopeToTargetUncloaked.h
//
// PURPOSE : CAIActionLopeToTargetUncloaked abstract class definition
//
// CREATED : 5/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_LOPE_TO_TARGET_UNCLOAKED_H__
#define __AIACTION_LOPE_TO_TARGET_UNCLOAKED_H__

#include "AIActionGotoTarget.h"

class CAIActionLopeToTargetUncloaked : public CAIActionGotoTarget
{
	typedef CAIActionGotoTarget super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLopeToTargetUncloaked, kAct_LopeToTargetUncloaked );

		CAIActionLopeToTargetUncloaked();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
