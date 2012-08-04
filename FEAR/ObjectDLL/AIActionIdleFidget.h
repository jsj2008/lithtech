// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleFidget.h
//
// PURPOSE : AIActionIdleFidget class definition
//
// CREATED : 4/8/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_IDLE_FIDGET_H__
#define __AIACTION_IDLE_FIDGET_H__

#include "AIActionAbstract.h"


class CAIActionIdleFidget : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleFidget, kAct_IdleFidget );

		CAIActionIdleFidget();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
