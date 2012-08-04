// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLookAtDisturbanceFromView.h
//
// PURPOSE : AIActionLookAtDisturbanceFromView class definition
//
// CREATED : 3/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_LOOK_AT_DISTURBANCE_FROM_VIEW_H__
#define __AIACTION_LOOK_AT_DISTURBANCE_FROM_VIEW_H__

#include "AIActionLookAtDisturbance.h"

// Forward declarations.

class	CAnimationProps;

class CAIActionLookAtDisturbanceFromView : public CAIActionLookAtDisturbance
{
	typedef CAIActionLookAtDisturbance super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLookAtDisturbanceFromView, kAct_LookAtDisturbanceFromView );

		CAIActionLookAtDisturbanceFromView();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif


