// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterSquad.h
//
// PURPOSE : AITargetSelectCharacterSquad class declaration
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_CHARACTER_SQUAD_H__
#define __AITARGETSELECT_CHARACTER_SQUAD_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectCharacterSquad : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterSquad, kTargetSelect_CharacterSquad );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
