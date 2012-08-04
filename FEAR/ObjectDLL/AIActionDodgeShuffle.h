// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeShuffle.h
//
// PURPOSE : AIActionDodgeShuffle class definition
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_SHUFFLE_H__
#define __AIACTION_DODGE_SHUFFLE_H__

#include "AIActionAbstract.h"


// Forward declarations.

class	CAnimationProps;


class CAIActionDodgeShuffle : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeShuffle, kAct_DodgeShuffle );

		CAIActionDodgeShuffle();

		// Virtual functions allow other dodge Actions to be derived from shuffle.

		virtual float			GetDodgeDist( CAI* pAI );
		virtual void			SetDodgeAnim( CAI* pAI, float fDir, CAnimationProps& animProps );

		bool					IsClearForDodge( CAI* pAI, float fDodgeDist, CAI* pIgnoreAI=NULL, CAI** pIntersectedAI=NULL );

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
