// ----------------------------------------------------------------------- //
//
// MODULE  : CAIActionDodgeCovered.h
//
// PURPOSE : CAIActionDodgeCovered class definition
//
// CREATED : 5/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_COVERED_H__
#define __AIACTION_DODGE_COVERED_H__

#include "AIActionUseSmartObjectNode.h"


// Forward declarations.


class CAIActionDodgeCovered : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeCovered, kAct_DodgeCovered );

		CAIActionDodgeCovered();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
