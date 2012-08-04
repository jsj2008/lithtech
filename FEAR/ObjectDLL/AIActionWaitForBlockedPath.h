// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionWaitForBlockedPath.h
//
// PURPOSE : 
//
// CREATED : 10/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONWAITFORBLOCKEDPATH_H_
#define _AIACTIONWAITFORBLOCKEDPATH_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionWaitForBlockedPath
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionWaitForBlockedPath : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionWaitForBlockedPath, kAct_WaitForBlockedPath );

	// Ctor/Dtor

	CAIActionWaitForBlockedPath();
	virtual ~CAIActionWaitForBlockedPath();

	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

private:
	PREVENT_OBJECT_COPYING(CAIActionWaitForBlockedPath);
};

#endif // _AIACTIONWAITFORBLOCKEDPATH_H_
