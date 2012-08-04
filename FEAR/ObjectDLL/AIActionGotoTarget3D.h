// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTarget3D.h
//
// PURPOSE : AIActionGotoTarget3D abstract class definition
//
// CREATED : 9/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_TARGET_3D_H__
#define __AIACTION_GOTO_TARGET_3D_H__

#include "AIActionGotoTarget.h"

class CAIActionGotoTarget3D : public CAIActionGotoTarget
{
	typedef CAIActionGotoTarget super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTarget3D, kAct_GotoTarget3D );

		// CAIActionAbstract members.

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
