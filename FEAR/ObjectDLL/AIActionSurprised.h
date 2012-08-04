// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurprised.h
//
// PURPOSE : 
//
// CREATED : 11/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONSURPRISED_H_
#define _AIACTIONSURPRISED_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionSurprised
//
//	PURPOSE:	This action handles reacting to surprising events.
//
// ----------------------------------------------------------------------- //

class CAIActionSurprised : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurprised, kAct_Surprised );

	// Ctor/Dtor

	CAIActionSurprised();
	virtual ~CAIActionSurprised();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionSurprised);
};

#endif // _AIACTIONSURPRISED_H_
