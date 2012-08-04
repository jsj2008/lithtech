// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceUrgent.h
//
// PURPOSE : AITargetSelectDisturbanceUrgent class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DISTURBANCE_URGENT_H__
#define __AITARGETSELECT_DISTURBANCE_URGENT_H__

#include "AITargetSelectDisturbance.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectDisturbanceUrgent : public CAITargetSelectDisturbance
{
	typedef CAITargetSelectDisturbance super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceUrgent, kTargetSelect_DisturbanceUrgent );

		CAITargetSelectDisturbanceUrgent();

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
