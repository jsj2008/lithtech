// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceBeyondGuard.h
//
// PURPOSE : AITargetSelectDisturbanceBeyondGuard class declaration
//
// CREATED : 6/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DISTURBANCE_BEYOND_GUARD_H__
#define __AITARGETSELECT_DISTURBANCE_BEYOND_GUARD_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectDisturbanceBeyondGuard : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceBeyondGuard, kTargetSelect_DisturbanceBeyondGuard );

		CAITargetSelectDisturbanceBeyondGuard();

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );

	private:

		HOBJECT			SelectDisturbanceSource( CAI* pAI, CAIWMFact* pFact );
};

// ----------------------------------------------------------------------- //

#endif
