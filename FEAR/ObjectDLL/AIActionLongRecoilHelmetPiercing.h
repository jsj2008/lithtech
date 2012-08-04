// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLongRecoilHelmetPiercing.h
//
// PURPOSE : AIActionLongRecoilHelmetPiercing class definition
//
// CREATED : 02/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_LONG_RECOIL_HELMET_PIERCING_H__
#define __AIACTION_LONG_RECOIL_HELMET_PIERCING_H__

#include "AIActionShortRecoil.h"


// Forward declarations.
class CAIWMFact;
class CAnimationProps;

class CAIActionLongRecoilHelmetPiercing : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLongRecoilHelmetPiercing, kAct_LongRecoilHelmetPiercing );

		// CAIActionAbstract members.

		virtual float	GetActionProbability( CAI* pAI );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
