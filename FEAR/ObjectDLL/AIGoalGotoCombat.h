// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGotoCombat.h
//
// PURPOSE : AIGoalGotoCombat class definition
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_GOTO_COMBAT_H__
#define __AIAMGOAL_GOTO_COMBAT_H__


// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAIGoalGotoCombat : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGotoCombat, kGoal_GotoCombat );
		
		// CAIGoalGoto overrides.

		virtual CAIWMFact*	FindBestNode( EnumAINodeType eNodeType );

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual bool		IsWSSatisfied( bool bIsPlanning, CAIWorldState* pwsWorldState );
};


#endif
