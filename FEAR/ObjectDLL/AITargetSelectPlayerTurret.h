// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectPlayerTurret.h
//
// PURPOSE : AITargetSelectPlayerTurret class declaration
//
// CREATED : 8/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_PLAYER_TURRET_H__
#define __AITARGETSELECT_PLAYER_TURRET_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectPlayerTurret : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectPlayerTurret, kTargetSelect_PlayerTurret );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
