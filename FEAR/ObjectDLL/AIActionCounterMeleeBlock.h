// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionCounterMeleeBlock.h
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONCOUNTERMELEEBLOCK_H_
#define _AIACTIONCOUNTERMELEEBLOCK_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionCounterMeleeBlock
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionCounterMeleeBlock : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionCounterMeleeBlock, kAct_CounterMeleeBlock );

	// Ctor/Dtor

	CAIActionCounterMeleeBlock();
	virtual ~CAIActionCounterMeleeBlock();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionCounterMeleeBlock);
};

#endif // _AIACTIONCOUNTERMELEEBLOCK_H_
