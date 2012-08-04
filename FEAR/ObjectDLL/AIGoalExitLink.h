// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLink.h
//
// PURPOSE : 
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOALEXITLINK_H_
#define __AIGOALEXITLINK_H_

#include "AIGoalAbstract.h"
#include "AINavMeshLinkAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalExitLink
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalExitLink : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalExitLink, kGoal_ExitLink );

	// Constructors

	CAIGoalExitLink();
	virtual ~CAIGoalExitLink();

	virtual void Save(ILTMessage_Write *pMsg);
	virtual void Load(ILTMessage_Read *pMsg);

	// CAIGoalAbstract overrides.

	virtual void	CalculateGoalRelevance();

	virtual void	ActivateGoal();
	virtual void	DeactivateGoal();

	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
	virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	bool			IsGoalInProgress();

private:
	CAIGoalExitLink(const CAIGoalExitLink& src);				// Not implemented
	const CAIGoalExitLink& operator=(const CAIGoalExitLink&);	// Not implemented

	// Used for holding cached information (evaluated in CalculateGoalRelevance)
	// which may need to be used in the goals Activate()
	ENUM_NMLinkID			m_ePendingNavMeshLink;
	LTObjRef				m_hPendingNode;

	// Tracks the node the AI is heading towards when the goal is active.
	LTObjRef				m_hNode;
};

#endif // __AIGOALEXITLINK_H_
