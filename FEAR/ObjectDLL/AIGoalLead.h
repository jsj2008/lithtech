// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLead.h
//
// PURPOSE : This goal handles leading the a character (the player) to an 
//			 AINodeLead instance.
//
// CREATED : 4/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALLEAD_H_
#define _AIGOALLEAD_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalLead
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalLead : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalLead, kGoal_Lead );

	// Ctor/Dtor

	CAIGoalLead();
	virtual ~CAIGoalLead();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void CalculateGoalRelevance();
	virtual void ActivateGoal();
	virtual void DeactivateGoal();
	virtual void SetWSSatisfaction( CAIWorldState& /*WorldState*/ );
	virtual void UpdateTaskStatus();
	virtual void HandleBuildPlanFailure();

private:
	PREVENT_OBJECT_COPYING(CAIGoalLead);

	ENUM_FactID	m_ePendingLeadRequest;
	ENUM_FactID	m_eCurrentLeadRequest;
};

#endif // _AIGOALLEAD_H_
