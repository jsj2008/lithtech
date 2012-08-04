// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromVantage.h
//
// PURPOSE : AIGoalAttackFromVantage class definition
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_FROM_VANTAGE_H__
#define __AIGOAL_ATTACK_FROM_VANTAGE_H__

#include "AIGoalAttack.h"


class CAIGoalAttackFromVantage : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromVantage, kGoal_AttackFromVantage);

		CAIGoalAttackFromVantage( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

	protected:

		virtual void SetStateAttack();
		void HandleStateCharge();

		AINodeChangeWeapons* GetNodeAsChangeWeapons() const;
		const char* const	GetWeaponChangeDescription() const;

		// State Handling.

		virtual void HandleStateAttackFromVantage();

	protected:
};

#endif
