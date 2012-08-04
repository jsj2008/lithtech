// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionChangePrimaryWeapon.h
//
// PURPOSE : 
//
// CREATED : 6/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONCHANGEPRIMARYWEAPON_H_
#define _AIACTIONCHANGEPRIMARYWEAPON_H_

LINKTO_MODULE(AIActionChangePrimaryWeapon);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionChangePrimaryWeapon
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionChangePrimaryWeapon : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionChangePrimaryWeapon, kAct_ChangePrimaryWeapon );

		// Ctor/Dtor

		CAIActionChangePrimaryWeapon();
		virtual ~CAIActionChangePrimaryWeapon();
		
		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	IsActionComplete( CAI* pAI );

	private:
		PREVENT_OBJECT_COPYING(CAIActionChangePrimaryWeapon);
};

#endif // _AIACTIONCHANGEPRIMARYWEAPON_H_
