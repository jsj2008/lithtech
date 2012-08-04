// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathAnimated.h
//
// PURPOSE : This action handles playing animations in response to death, 
//			 instead of simply ragdolling.  This can be used to 'keyframe'
//			 aspects of the death such as clientfx, physics events, or any
//			 other model string driven functionality.
//
// CREATED : 3/12/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDEATHANIMATED_H_
#define _AIACTIONDEATHANIMATED_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionDeathAnimated
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDeathAnimated : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathAnimated, kAct_DeathAnimated );

	// Ctor/Dtor

	CAIActionDeathAnimated();
	virtual ~CAIActionDeathAnimated();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );
	virtual	void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionDeathAnimated);
};

#endif // _AIACTIONDEATHANIMATED_H_

