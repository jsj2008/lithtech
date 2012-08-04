// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseLinkUncloaked.h
//
// PURPOSE : AIActionTraverseLinkUncloaked class definition
//
// CREATED : 04/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_TRAVERSE_LINK_UNCLOAKED_H__
#define __AIACTION_TRAVERSE_LINK_UNCLOAKED_H__

#include "AIActionTraverseLink.h"

class CAIWorldState;

class CAIActionTraverseLinkUncloaked : public CAIActionTraverseLink
{
	typedef CAIActionTraverseLink super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseLinkUncloaked, kAct_TraverseLinkUncloaked );

		// CAIActionAbstract members.

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
