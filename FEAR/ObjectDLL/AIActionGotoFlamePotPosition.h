// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoFlamePotPosition.h
//
// PURPOSE : This action handles going to a position designated by a 
//			 kKnowledge_FlamePotPosition fact; the goal specifies a 
//			 location on the perimeter of the link, and this action executes
//			 moving to that position.
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONGOTOFLAMEPOTPOSITION_H_
#define _AIACTIONGOTOFLAMEPOTPOSITION_H_

#include "AIActionGotoAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionGotoFlamePotPosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionGotoFlamePotPosition : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoFlamePotPosition, kAct_GotoFlamePotPosition );

	// Ctor/Dtor

	CAIActionGotoFlamePotPosition();
	virtual ~CAIActionGotoFlamePotPosition();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* /*pAI*/, CAIWorldState& /*wsWorldStateGoal*/, bool /*bIsPlanning*/ );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionGotoFlamePotPosition);
};

#endif // _AIACTIONGOTOFLAMEPOTPOSITION_H_
