// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionCounterMeleeDodge.h
//
// PURPOSE : 
//
// CREATED : 9/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONCOUNTERMELEEDODGE_H_
#define _AIACTIONCOUNTERMELEEDODGE_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionCounterMeleeDodge
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionCounterMeleeDodge : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionCounterMeleeDodge, kAct_CounterMeleeDodge );

	// Ctor/Dtor

	CAIActionCounterMeleeDodge();
	virtual ~CAIActionCounterMeleeDodge();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionCounterMeleeDodge);
};

#endif // _AIACTIONCOUNTERMELEEDODGE_H_
