// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTraverseLink.h
//
// PURPOSE : AIGoalTraverseLink class definition
//
// CREATED : 7/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_TRAVERSE_LINK_H__
#define __AIGOAL_TRAVERSE_LINK_H__

#include "AIGoalAbstract.h"
#include "AIEnumNavMeshLinkTypes.h"

class CAIGoalTraverseLink : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalTraverseLink, kGoal_TraverseLink );

		CAIGoalTraverseLink();
		virtual ~CAIGoalTraverseLink();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		ENUM_NMLinkID	m_eTraversingLink;
};


#endif
