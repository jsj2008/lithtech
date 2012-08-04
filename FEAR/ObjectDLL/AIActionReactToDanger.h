// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToDanger.h
//
// PURPOSE : AIActionReactToDanger declaration
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_REACT_TO_DANGER_H_
#define __AIACTION_REACT_TO_DANGER_H_

#include "AIActionAbstract.h"

class CAIActionReactToDanger : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToDanger, kAct_ReactToDanger );

		CAIActionReactToDanger();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

	protected:

		void			SearchForDangerOrigin( CAI* pAI, const LTVector& vDangerPos );
};

#endif // __AIACTION_REACT_TO_DANGER_H_
