// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterOnlyOne.h
//
// PURPOSE : AITargetSelectCharacterOnlyOne class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_CHARACTER_ONLY_ONE_H__
#define __AITARGETSELECT_CHARACTER_ONLY_ONE_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectCharacterOnlyOne : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterOnlyOne, kTargetSelect_CharacterOnlyOne );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual bool	Validate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
