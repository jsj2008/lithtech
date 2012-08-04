// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceSquad.h
//
// PURPOSE : AITargetSelectDisturbanceSquad class declaration
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DISTURBANCE_SQUAD_H__
#define __AITARGETSELECT_DISTURBANCE_SQUAD_H__

#include "AITargetSelectDisturbance.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectDisturbanceSquad : public CAITargetSelectDisturbance
{
	typedef CAITargetSelectDisturbance super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceSquad, kTargetSelect_DisturbanceSquad );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
