// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCheckBody.h
//
// PURPOSE : AIGoalCheckBody class definition
//
// CREATED : 8/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CHECK_BODY_H__
#define __AIGOAL_CHECK_BODY_H__

#include "AIGoalAbstractSearch.h"
#include "LTObjRef.h"

typedef ObjRefNotifierList SeenBodyList;

class CAIGoalCheckBody : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCheckBody, kGoal_CheckBody);

		CAIGoalCheckBody( );
		~CAIGoalCheckBody( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// ILTObjRefReceiver function.

		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateCheckBody();
		void HandleStateGoto();
		void HandleStateAnimate();
		void SearchOrAware();

		// Body management.

		void DisposeOfBody();
		void AddBodyToSeenList(HOBJECT hBody);
		void RemoveBodyFromSeenList(HOBJECT hBody);

	protected:

		LTObjRefNotifier	m_hBody;
		SeenBodyList		m_lstSeenBodies;
		LTVector			m_vStimulusPos;
};


#endif
