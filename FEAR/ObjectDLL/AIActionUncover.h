// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUncover.h
//
// PURPOSE : AIActionUncover class definition
//
// CREATED : 6/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_UNCOVER_H__
#define __AIACTION_UNCOVER_H__


class CAIActionUncover : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUncover, kAct_Uncover );

		CAIActionUncover();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
