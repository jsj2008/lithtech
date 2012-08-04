// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPatrol.h
//
// PURPOSE : AIGoalPatrol class definition
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_PATROL_H__
#define __AIGOAL_PATROL_H__

#include "AIGoalAbstract.h"


// Forward Declarations.
class AINodePatrol;
enum  EnumAnimProp;


class CAIGoalPatrol : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPatrol, kGoal_Patrol);

		 CAIGoalPatrol( );
		~CAIGoalPatrol( );

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

		// State Handling.

		void HandleStateIdle();
		void HandleStatePatrol();
		void HandleStateDraw();

		// Patrol path locking.

		void SetPatrolNode(AINodePatrol* pNode);
		void LockPatrolPath(AINodePatrol* pNode, LTBOOL bLock);
		void LockPatrolPath(AINodePatrol* pNode) { LockPatrolPath(pNode, LTTRUE); }
		void UnlockPatrolPath(AINodePatrol* pNode) { LockPatrolPath(pNode, LTFALSE); }

	protected:

        LTObjRef		m_hPatrolNode;
		EnumAnimProp	m_eAwareness;
};


#endif
