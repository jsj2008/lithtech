// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdle.h
//
// PURPOSE : AIActionIdle class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_IDLE_H__
#define __AIACTION_IDLE_H__

#include "AIActionAbstract.h"


class CAIActionIdle : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdle, kAct_Idle );

		CAIActionIdle();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
