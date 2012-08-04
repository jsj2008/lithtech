// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNodeAggressive.h
//
// PURPOSE : Handle leaving a node in an aggressive way
//
// CREATED : 5/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIONDISMOUNTNODEAGGRESSIVE_H_
#define __AIACTIONDISMOUNTNODEAGGRESSIVE_H_

#include "AIActionDismountNode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActionDismountNodeAggressive
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDismountNodeAggressive : public CAIActionDismountNode
{
	typedef CAIActionDismountNode super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNodeAggressive, kAct_DismountNodeAggressive );

	CAIActionDismountNodeAggressive();
	virtual ~CAIActionDismountNodeAggressive();

	// CAIActionDismountNode overrides

	virtual bool			ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
	virtual EnumAnimProp	GetDismountAction() { return kAP_ACT_DismountAggressive; }

private:
	PREVENT_OBJECT_COPYING(CAIActionDismountNodeAggressive);
};

#endif // __AIACTIONDISMOUNTNODEAGGRESSIVE_H_
