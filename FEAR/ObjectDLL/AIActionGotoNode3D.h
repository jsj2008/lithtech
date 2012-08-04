// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNode3D.h
//
// PURPOSE : AIActionGotoNode3D abstract class definition
//
// CREATED : 9/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_NODE_3D_H__
#define __AIACTION_GOTO_NODE_3D_H__

#include "AIActionGotoNode.h"

class CAIActionGotoNode3D : public CAIActionGotoNode
{
	typedef CAIActionGotoNode super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNode3D, kAct_GotoNode3D );

		// CAIActionAbstract members.

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
