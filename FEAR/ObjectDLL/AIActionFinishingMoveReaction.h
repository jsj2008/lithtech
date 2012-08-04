// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishingMoveReaction.h
//
// PURPOSE : 
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONFINISHINGMOVEREACTION_H_
#define _AIACTIONFINISHINGMOVEREACTION_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionFinishingMoveReaction
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionFinishingMoveReaction : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishingMoveReaction, kAct_FinishingMoveReaction );

	// Ctor/Dtor

	CAIActionFinishingMoveReaction();
	virtual ~CAIActionFinishingMoveReaction();

	virtual	void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual	bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionFinishingMoveReaction);
};

#endif // _AIACTIONFINISHINGMOVEREACTION_H_
