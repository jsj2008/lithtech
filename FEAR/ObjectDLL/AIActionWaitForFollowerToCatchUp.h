// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionWaitForFollowerToCatchUp.h
//
// PURPOSE : 
//
// CREATED : 4/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONWAITFORFOLLOWERTOCATCHUP_H_
#define _AIACTIONWAITFORFOLLOWERTOCATCHUP_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionWaitForFollowerToCatchUp
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionWaitForFollowerToCatchUp : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionWaitForFollowerToCatchUp, kAct_WaitForFollowerToCatchUp );

	// Ctor/Dtor

	CAIActionWaitForFollowerToCatchUp();
	virtual ~CAIActionWaitForFollowerToCatchUp();

	bool ValidateContextPreconditions( CAI* /*pAI*/, CAIWorldState& /*wsWorldStateGoal*/, bool /*bIsPlanning*/ );
	void ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	void DeactivateAction( CAI* pAI );
	bool ValidateAction( CAI* /*pAI*/ );
	void InitAction( AIDB_ActionRecord* pActionRecord );
	bool IsActionComplete( CAI* /*pAI*/ );

private:
	PREVENT_OBJECT_COPYING(CAIActionWaitForFollowerToCatchUp);
};

#endif // _AIACTIONWAITFORFOLLOWERTOCATCHUP_H_
