// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGetOutOfTheWay.h
//
// PURPOSE : Contains the declaration of the 'get out of the way' action
//
// CREATED : 10/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIONGETOUTOFTHEWAY_H_
#define __AIACTIONGETOUTOFTHEWAY_H_

#include "AIActionGotoAbstract.h"

class CAIActionGetOutOfTheWay : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGetOutOfTheWay, kAct_GetOutOfTheWay );

		CAIActionGetOutOfTheWay();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

#endif // __AIACTIONGETOUTOFTHEWAY_H_
