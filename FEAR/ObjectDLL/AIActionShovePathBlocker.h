// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShovePathBlocker.h
//
// PURPOSE : 
//
// CREATED : 11/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONSHOVEPATHBLOCKER_H_
#define _AIACTIONSHOVEPATHBLOCKER_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionShovePathBlocker
//
//	PURPOSE:	This action handles pushing an AI in the waiting state out
//				of the way; it is used to prevent pathing deadlocks
//
// ----------------------------------------------------------------------- //

class CAIActionShovePathBlocker : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShovePathBlocker, kAct_ShovePathBlocker );

	// Ctor/Dtor

	CAIActionShovePathBlocker();
	virtual ~CAIActionShovePathBlocker();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionShovePathBlocker);
};

#endif // _AIACTIONSHOVEPATHBLOCKER_H_
