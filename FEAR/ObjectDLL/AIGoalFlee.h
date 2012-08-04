// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlee.h
//
// PURPOSE : AIGoalFlee class definition
//
// CREATED : 4/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_FLEE_H__
#define __AIAMGOAL_FLEE_H__

#include "AIGoalUseSmartObjectCombat.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalFlee : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlee, kGoal_Flee );
		
		CAIGoalFlee();

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
