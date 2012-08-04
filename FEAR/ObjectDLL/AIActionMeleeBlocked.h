// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMeleeBlocked.h
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONMELEEBLOCKED_H_
#define _AIACTIONMELEEBLOCKED_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionMeleeBlocked
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionMeleeBlocked : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMeleeBlocked, kAct_MeleeBlocked );

	// Ctor/Dtor

	CAIActionMeleeBlocked();
	virtual ~CAIActionMeleeBlocked();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionMeleeBlocked);
};

#endif // _AIACTIONMELEEBLOCKED_H_
