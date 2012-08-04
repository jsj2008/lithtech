//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalCatch.h
//              
//	PURPOSE:	CAIGoalCatch declaration
//              
//	CREATED:	08.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALCATCH_H__
#define __AIGOALCATCH_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes

#include "AIGoalAbstractStimulated.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CAIGoalCatch
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalCatch : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCatch, kGoal_Catch);

		CAIGoalCatch( );

		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateCatch();

	protected:


	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CAIGoalCatch(const CAIGoalCatch& rhs) {}
		CAIGoalCatch& operator=(const CAIGoalCatch& rhs ) {}
};

#endif // __AIGOALCATCH_H__