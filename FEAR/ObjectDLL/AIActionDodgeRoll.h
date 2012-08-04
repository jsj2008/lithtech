// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeRoll.h
//
// PURPOSE : AIActionDodgeRoll class definition
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_ROLL_H__
#define __AIACTION_DODGE_ROLL_H__

#include "AIActionDodgeShuffle.h"
#include "AnimationProp.h"

class CAIActionDodgeRoll : public CAIActionDodgeShuffle
{
	typedef CAIActionDodgeShuffle super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeRoll, kAct_DodgeRoll );

		// CAIActionDodgeShuffle members.

		virtual float			GetDodgeDist( CAI* pAI );
		virtual void			SetDodgeAnim( CAI* pAI, float fDir, CAnimationProps& animProps );
	
		// CAIActionAbstract members.

		virtual bool			ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
