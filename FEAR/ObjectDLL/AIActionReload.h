// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReload.h
//
// PURPOSE : AIActionReload class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_RELOAD_H__
#define __AIACTION_RELOAD_H__

#include "AIActionAbstract.h"


class CAIActionReload : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReload, kAct_Reload );

		CAIActionReload();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

		EnumAnimProp	m_ePosture;
};

// ----------------------------------------------------------------------- //

#endif
