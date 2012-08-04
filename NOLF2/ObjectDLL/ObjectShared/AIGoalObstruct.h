//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalObstruct.h
//              
//	PURPOSE:	CAIGoalObstruct declaration
//              
//	CREATED:	11.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALOBSTRUCT_H__
#define __AIGOALOBSTRUCT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIGoalAbstractStimulated.h"
#include "AINode.h"
 
// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CAIGoalObstruct
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalObstruct : public CAIGoalAbstractStimulated
{
	public:
		// Public members
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalObstruct, kGoal_Obstruct);

		CAIGoalObstruct() {}

		// Save / Load
		
		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:
		// Protected members

		// State Handling.

		void	HandleStateObstruct();
		int		GetAttractorCount();
		AINode*	FindAndSetupNode(EnumAINodeType kNodeType);

		void	SetLastStimulusSource(HOBJECT hStim);
		HOBJECT	GetLastStimulusSource(void);

		void	SetObstructNode(HOBJECT hNode);
		HOBJECT GetObstructNode(void);
		AINodeObstruct*	GetObstructObject(void);

		void	SetFailedNode(HOBJECT hNode);
		HOBJECT GetFailedNode(void);

	private:
		// Private members
		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		// Intentionally left unimplemented.
		CAIGoalObstruct(const CAIGoalObstruct& rhs);
		CAIGoalObstruct& operator=(const CAIGoalObstruct& rhs );
	
		// Save:
		LTObjRef	m_hNodeObstruct;
		LTObjRef	m_hNodeObstructFailed;

		// Don't Save:

};

#endif // __AIGOALOBSTRUCT_H__

