// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadFromSafetyNode.h
//
// PURPOSE : This action handles the desire to reload by moving to an 
//			AINodeSafety and performing the reload there.
//
// CREATED : 1/31/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONRELOADFROMSAFETYNODE_H_
#define _AIACTIONRELOADFROMSAFETYNODE_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionReloadFromSafetyNode
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionReloadFromSafetyNode : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadFromSafetyNode, kAct_ReloadFromSafetyNode );

	// Ctor/Dtor

	CAIActionReloadFromSafetyNode();

	// CAIActionAbstract members.

	virtual void InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual bool ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
	virtual void ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionReloadFromSafetyNode);
};

#endif // _AIACTIONRELOADFROMSAFETYNODE_H_
