// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowWait.h
//
// PURPOSE : AIGoalFollowWait class definition
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FOLLOW_WAIT_H__
#define __AIGOAL_FOLLOW_WAIT_H__

#include "AIGoalUseSmartObject.h"


// Forward declarations.

class CNamedObjectList;


// ----------------------------------------------------------------------- //

class CAIGoalFollowWait : public CAIGoalUseSmartObject
{
	typedef CAIGoalUseSmartObject super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollowWait, kGoal_FollowWait );
		
		CAIGoalFollowWait();

		// CAIGoalUseSmartObject overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		bool				IsGoalInProgress( HOBJECT hLeader );
		HOBJECT				FindBestNode( EnumAINodeType eNodeType, HOBJECT hLeader, LTObjRef* phFollowNode );

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();

	private:

		HOBJECT				FindValidWaitNode( CNamedObjectList* plstWaitingNodes, HOBJECT hLeader );

	private:

		LTObjRef			m_hFollowNode;
};


#endif
