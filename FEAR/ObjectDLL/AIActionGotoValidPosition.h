// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoValidPosition.h
//
// PURPOSE : Contains the declaration of the 'get to a valid postion' action
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIONGOTOVALIDPOSITION_H_
#define __AIACTIONGOTOVALIDPOSITION_H_

#include "AIActionGotoAbstract.h"

class CAIActionGotoValidPosition : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoValidPosition, kAct_GotoValidPosition );

		CAIActionGotoValidPosition();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
		bool GetValidPosition(CAI* pAI, LTVector& outValidPosition);
};

#endif // __AIACTIONGOTOVALIDPOSITION_H_
