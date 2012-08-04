// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromView.h
//
// PURPOSE : AIGoalAttackFromView class definition
//
// CREATED : 7/31/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_FROM_VIEW_H__
#define __AIGOAL_ATTACK_FROM_VIEW_H__

#include "AIGoalAttack.h"


class CAIGoalAttackFromView : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromView, kGoal_AttackFromView);

		CAIGoalAttackFromView( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		void SetStateAttack();

		// State Handling.

		void HandleStateAttackFromView();
		void HandleStateCharge();

	protected:

};


#endif
