// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUseSmartObjectNodeMounted.h
//
// PURPOSE : AIActionUseSmartObjectNodeMounted class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_USE_SMART_OBJECT_MOUNTED_H__
#define __AIACTION_USE_SMART_OBJECT_MOUNTED_H__

#include "AIActionUseSmartObjectNode.h"


class CAIActionUseSmartObjectNodeMounted : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUseSmartObjectNodeMounted, kAct_UseSmartObjectNodeMounted );

		CAIActionUseSmartObjectNodeMounted();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
