// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseBlockedDoor.h
//
// PURPOSE : AIActionTraverseBlockedDoor class definition
//
// CREATED : 7/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIONTRAVERSEBLOCKED_H__
#define __AIACTIONTRAVERSEBLOCKED_H__

#include "AIActionTraverseLink.h"

class CAIWorldState;
class CAI;

class CAIActionTraverseBlockedDoor : public CAIActionTraverseLink
{
	typedef CAIActionTraverseLink super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseBlockedDoor, kAct_TraverseBlockedDoor );

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction(CAI* pAI);
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
