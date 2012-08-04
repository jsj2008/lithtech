// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromVehicle.h
//
// PURPOSE : AIActionAttackFromVehicle class definition
//
// CREATED : 1/05/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_FROM_VEHICLE_H__
#define __AIACTION_ATTACK_FROM_VEHICLE_H__

#include "AIActionAttack.h"


class CAIActionAttackFromVehicle : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromVehicle, kAct_AttackFromVehicle );

		CAIActionAttackFromVehicle();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
		EnumAnimProp	SelectAttackPose( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
