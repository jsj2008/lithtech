// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectAllyDisturbance.h
//
// PURPOSE : AITargetSelectAllyDisturbance class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_ALLY_DISTURBANCE_H__
#define __AITARGETSELECT_ALLY_DISTURBANCE_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectAllyDisturbance : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectAllyDisturbance, kTargetSelect_AllyDisturbance );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
