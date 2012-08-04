// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttack.h
//
// PURPOSE : AIActionAttackMelee class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_MELEE_H__
#define __AIACTION_ATTACK_MELEE_H__

#include "AIActionAbstract.h"
#include "AIActionAttack.h"

class CAIActionAttackMelee : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMelee, kAct_AttackMelee );

		CAIActionAttackMelee();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	IsActionInterruptible( CAI* pAI );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );

	protected:

		virtual ENUM_AIWeaponType	GetWeaponType() const { return kAIWeaponType_Melee; }
		virtual void				SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
		virtual bool				GetLoopAttackAnimation( CAI* pAI ) const;

	protected:

		bool m_bDistanceClosingAttack;
};

#endif
