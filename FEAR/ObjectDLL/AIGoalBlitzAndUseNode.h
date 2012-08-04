// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBlitzAndUseNode.h
//
// PURPOSE : Contains the declaration of the 'blitz and use node' goal.  
//			 This is similar to AIGoalBlitzNode.  The difference is what
//			 this goal additionally requires the AI to use the the 
//			 smartobject node instead of just arriving at its position.
//
// CREATED : 2/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALBLITZANDUSENODE_H_
#define _AIGOALBLITZANDUSENODE_H_

#include "AIGoalUseSmartObject.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalBlitzAndUseNode
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalBlitzAndUseNode : public CAIGoalUseSmartObject
{
	typedef CAIGoalUseSmartObject super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzAndUseNode, kGoal_BlitzAndUseNode );

	// Ctor/Dtor

	CAIGoalBlitzAndUseNode();
	virtual ~CAIGoalBlitzAndUseNode();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void ActivateGoal();
	virtual void DeactivateGoal();
	virtual HOBJECT FindBestNode( EnumAINodeType eNodeType );
	virtual void HandleBuildPlanFailure();

private:
	PREVENT_OBJECT_COPYING(CAIGoalBlitzAndUseNode);

	EnumAIContext OnGetContext() const;

	// Store the goal Context from the task which activated this goal.
	EnumAIContext m_eActiveGoalContext;
};

#endif // _AIGOALBLITZANDUSENODE_H_
