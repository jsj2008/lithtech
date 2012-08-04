// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeBackpedal.h
//
// PURPOSE : AIActionDodgeBackpedal class definition
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_BACKPEDAL_H__
#define __AIACTION_DODGE_BACKPEDAL_H__


// Forward declarations.

class	CAnimationProps;


class CAIActionDodgeBackpedal : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeBackpedal, kAct_DodgeBackpedal );

		CAIActionDodgeBackpedal();

		// Virtual functions allow other dodge Actions to be derived from Backpedal.

		virtual float			GetDodgeDist( CAI* pAI );
		virtual void			SetDodgeAnim( CAI* pAI, CAnimationProps& animProps );

		bool					IsClearForDodge( CAI* pAI, float fDodgeDist );

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
