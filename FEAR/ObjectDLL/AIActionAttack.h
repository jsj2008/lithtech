// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttack.h
//
// PURPOSE : AIActionAttack class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_H__
#define __AIACTION_ATTACK_H__

// Includes

#include "AIActionAbstract.h"
#include "AIWeaponUtils.h"

// Forward declarations.

class	CAnimationProps;
class	CAI;

class CAIActionAttack : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttack, kAct_Attack );

		CAIActionAttack();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	ValidateAction( CAI* pAI );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
		virtual bool	GetLoopAttackAnimation( CAI* pAI ) const;

		void SetValidateVisibility(bool b) { m_bValidateVisibility = b; } 

		virtual ENUM_AIWeaponType GetWeaponType() const { return kAIWeaponType_Ranged; }

	protected:

		bool	m_bPlayAISound;
		bool	m_bValidateVisibility;
		bool	m_bInterruptActionIfEnemyIsOutOfRange;
};

// ----------------------------------------------------------------------- //

#endif
