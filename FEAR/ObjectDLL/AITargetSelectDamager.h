// ----------------------------------------------------------------------- //
//
// MODULE  : CAITargetSelectDamager.h
//
// PURPOSE : CAITargetSelectDamager class declaration
//
// CREATED : 7/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DAMAGER_H__
#define __AITARGETSELECT_DAMAGER_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectDamager : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDamager, kTargetSelect_Damager );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
