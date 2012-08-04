// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeUncloaked.h
//
// PURPOSE : AIActionAttackLungeUncloaked class definition
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_LUNGE_UNCLOAKED_H__
#define __AIACTION_ATTACK_LUNGE_UNCLOAKED_H__

#include "AIActionAttack.h"
#include "AIWeaponUtils.h"


class CAIActionAttackLungeUncloaked : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeUncloaked, kAct_AttackLungeUncloaked );

		CAIActionAttackLungeUncloaked();

		// CAIActionAbstract members.

		virtual bool			ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
		virtual bool			ValidateAction( CAI* pAI );

	protected:

		virtual ENUM_AIWeaponType	GetWeaponType() const { return kAIWeaponType_Melee; }
		virtual void				SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

		virtual float				GetLungeMaxDist( CAI* pAI, CAIWMFact* pDesireFact );
};

// ----------------------------------------------------------------------- //

#endif
