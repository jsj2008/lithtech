// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseSmartObject.h
//
// PURPOSE : AIGoalUseSmartObject class definition
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_USE_OBJECT_H__
#define __AIAMGOAL_USE_OBJECT_H__

#include "AIGoalAbstract.h"


// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAIGoalUseSmartObject : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseSmartObject, kGoal_UseSmartObject );
		
		CAIGoalUseSmartObject();
		virtual ~CAIGoalUseSmartObject();

		virtual bool		IsGoalInProgress();
		virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );

		// CAIGoalAbstract overrides.

        virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual bool		ReplanRequired();

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		HOBJECT		m_hNode;
		HOBJECT		m_hNodeBest;
		
		// This does not need to be saved.

		uint32		m_dwNodeStatus;
};


#endif
