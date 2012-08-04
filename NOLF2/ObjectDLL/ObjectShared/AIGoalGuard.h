// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGuard.h
//
// PURPOSE : AIGoalGuard class definition
//
// CREATED : 11/12/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_GUARD_H__
#define __AIGOAL_GUARD_H__

#include "AIGoalAbstractTargeted.h"


// Forward Declarations.
class AINodeGuard;
enum  EnumAINodeType;

class CAIGoalGuard : public CAIGoalAbstractTargeted
{
	typedef CAIGoalAbstractTargeted super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGuard, kGoal_Guard);

		 CAIGoalGuard( );
		~CAIGoalGuard( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// Guard Node.

		virtual void	FindGuardNode();
		void			SetGuardNode(AINode* pGuardNode);

		// State Handling.

		virtual void	HandleStateIdle();
		virtual void	HandleStateGoto();
		
		virtual void	GotoNode();

	protected:

        LTObjRef		m_hGuardNode;
		LTBOOL			m_bInRadius;
		EnumAINodeType	m_eNodeType;
		LTFLOAT			m_fMinImportance;
};

#endif
