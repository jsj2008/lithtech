// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToShove.h
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONREACTTOSHOVE_H_
#define _AIACTIONREACTTOSHOVE_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionReactToShove
//
//	PURPOSE:	This action handles reacting to a shove by acting 'shoved'.  
//				This is similar to the ShortRecoil behavior.
//
// ----------------------------------------------------------------------- //

class CAIActionReactToShove : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToShove, kAct_ReactToShove );

	// Ctor/Dtor

	CAIActionReactToShove();
	virtual ~CAIActionReactToShove();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionReactToShove);
};

#endif // _AIACTIONREACTTOSHOVE_H_
