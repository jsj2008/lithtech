// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeFromRun.h
//
// PURPOSE : 
//
// CREATED : 11/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONATTACKMELEEFROMRUN_H_
#define _AIACTIONATTACKMELEEFROMRUN_H_

#include "AIActionAttackMelee.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionAttackMeleeFromRun
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionAttackMeleeFromRun : public CAIActionAttackMelee
{
	typedef CAIActionAttackMelee super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeFromRun, kAct_AttackMeleeFromRun );

	// Ctor/Dtor

	CAIActionAttackMeleeFromRun();
	virtual ~CAIActionAttackMeleeFromRun();

	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

private:
	PREVENT_OBJECT_COPYING(CAIActionAttackMeleeFromRun);
};

#endif // _AIACTIONATTACKMELEEFROMRUN_H_
