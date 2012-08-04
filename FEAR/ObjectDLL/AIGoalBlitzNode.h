// ----------------------------------------------------------------------- //
//
// MODULE  :	AIGoalBlitzNode.h
//
// PURPOSE :	Contains the declaration of the BlitzNode goal.  This goal 
//				handles scripted movement to nodes using a particular 
//				movement animation.
//
// CREATED :	2/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALBLITZNODE_H_
#define _AIGOALBLITZNODE_H_

#include "AIGoalGoto.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalBlitzNode
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalBlitzNode : public CAIGoalGoto
{
	typedef CAIGoalGoto super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzNode, kGoal_BlitzNode );

	// Ctor/Dtor

	CAIGoalBlitzNode();
	virtual ~CAIGoalBlitzNode();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void ActivateGoal();
	virtual void DeactivateGoal();

private:
	virtual EnumAIContext OnGetContext() const;

	PREVENT_OBJECT_COPYING(CAIGoalBlitzNode);

	// Store the goal Context from the task which activated this goal.
	EnumAIContext m_eActiveGoalContext;
};

#endif // _AIGOALBLITZNODE_H_
