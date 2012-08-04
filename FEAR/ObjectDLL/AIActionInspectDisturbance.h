// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInspectDisturbance.h
//
// PURPOSE : AIActionInspectDisturbance class definition
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_INSPECT_DISTURBANCE_H__
#define __AIACTION_INSPECT_DISTURBANCE_H__


// Forward declarations.

class	CAnimationProps;


class CAIActionInspectDisturbance : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInspectDisturbance, kAct_InspectDisturbance );

		CAIActionInspectDisturbance();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
