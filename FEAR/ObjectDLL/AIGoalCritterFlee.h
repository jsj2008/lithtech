// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCritterFlee.h
//
// PURPOSE : AIGoalCritterFlee class definition
//
// CREATED : 2/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CRITTER_FLEE_H__
#define __AIGOAL_CRITTER_FLEE_H__

#include "AIGoalUseSmartObjectCombat.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalCritterFlee : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCritterFlee, kGoal_CritterFlee );
		
		CAIGoalCritterFlee();

		virtual void		Save(ILTMessage_Write *pMsg);
		virtual void		Load(ILTMessage_Read *pMsg);

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();
		virtual bool		IsGoalInProgress();
		virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );

	protected:

		LTObjRef			m_hCachedBestNode;
};


#endif
