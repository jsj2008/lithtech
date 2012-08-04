// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectScripted.h
//
// PURPOSE : AITargetSelectScripted class declaration
//
// CREATED : 7/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_SCRIPTED_H__
#define __AITARGETSELECT_SCRIPTED_H__

#include "AITargetSelectCharacter.h"

// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAITargetSelectScripted : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectScripted, kTargetSelect_Scripted );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
