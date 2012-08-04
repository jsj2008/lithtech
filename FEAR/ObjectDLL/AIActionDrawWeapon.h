// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDrawWeapon.h
//
// PURPOSE : AIActionDrawWeapon abstract class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DRAW_WEAPON_H__
#define __AIACTION_DRAW_WEAPON_H__

#include "AIActionAbstract.h"


class CAIActionDrawWeapon : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDrawWeapon, kAct_DrawWeapon );

		CAIActionDrawWeapon();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual bool	ValidateContextPreconditions(CAI* pAI, CAIWorldState& wzWorldStateGoal, bool bIsPlanning);

	protected:

};

// ----------------------------------------------------------------------- //

#endif
