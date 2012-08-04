// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeFromDistance.h
//
// PURPOSE : This melee attack variation supports specifying 'ranges' 
//			 required for a melee attack.  For example, some animations may
//			 only be applicable when the AI is at the max reach for his 
//			 weapon, while others may be more applicable at closer ranges.
//
// CREATED : 11/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONATTACKMELEEFROMDISTANCE_H_
#define _AIACTIONATTACKMELEEFROMDISTANCE_H_

#include "AIActionAttackMelee.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionAttackMeleeFromDistance
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionAttackMeleeFromDistance : public CAIActionAttackMelee
{
	typedef CAIActionAttackMelee super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeFromDistance, kAct_AttackMeleeFromDistance );

	// Ctor/Dtor

	CAIActionAttackMeleeFromDistance();
	virtual ~CAIActionAttackMeleeFromDistance();

	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

private:
	PREVENT_OBJECT_COPYING(CAIActionAttackMeleeFromDistance);
};

#endif // _AIACTIONATTACKMELEEFROMDISTANCE_H_
