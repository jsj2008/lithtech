// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleCombat.h
//
// PURPOSE : This action handles an AI who is engaged in combat but has 
//			 nothing to do; this may occur if the AI is constrainted to 
//			 a guard node radius while using a melee weapon, unabled to 
//			 attack his target due to attacker count constraints, etc.
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONIDLECOMBAT_H_
#define _AIACTIONIDLECOMBAT_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionIdleCombat
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionIdleCombat : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleCombat, kAct_IdleCombat );

	// Ctor/Dtor

	CAIActionIdleCombat();
	virtual ~CAIActionIdleCombat();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* /*pAI*/, CAIWorldState& /*wsWorldStateGoal*/, bool /*bIsPlanning*/ );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionIdleCombat);
};

#endif // _AIACTIONIDLECOMBAT_H_
