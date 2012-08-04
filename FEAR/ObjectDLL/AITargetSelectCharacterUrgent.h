// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterUrgent.h
//
// PURPOSE : AITargetSelectCharacterUrgent class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_CHARACTER_URGENT_H__
#define __AITARGETSELECT_CHARACTER_URGENT_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectCharacterUrgent : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterUrgent, kTargetSelect_CharacterUrgent );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );

	protected:

		virtual CAIWMFact*	FindValidTarget( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
