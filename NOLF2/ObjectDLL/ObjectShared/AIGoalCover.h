// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCover.h
//
// PURPOSE : AIGoalCover class definition
//
// CREATED : 7/19/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_COVER_H__
#define __AIGOAL_COVER_H__

#include "AIGoalAbstractSearch.h"

// Forward declarations.
class AINode;


class CAIGoalCover : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCover, kGoal_Cover);

		CAIGoalCover( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateCover();

	protected:

		LTObjRef		m_hNodeCover;
		LTObjRef		m_hLastNodeCover;
};


#endif
