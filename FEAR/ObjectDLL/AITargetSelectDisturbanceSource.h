// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceSource.h
//
// PURPOSE : AITargetSelectDisturbanceSource class declaration
//
// CREATED : 9/20/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DISTURBANCE_SOURCE_H__
#define __AITARGETSELECT_DISTURBANCE_SOURCE_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectDisturbanceSource : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceSource, kTargetSelect_DisturbanceSource );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
