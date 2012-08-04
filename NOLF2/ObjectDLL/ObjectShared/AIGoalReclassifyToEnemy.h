//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalReclassifyToEnemy.h
//              
//	PURPOSE:	CAIGoalReclassifyToEnemy declaration
//              
//	CREATED:	27.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Goal to reclassify the source of a stimulus to a new type
//				(hated)  This is a goal as only some characters are allowed
//				to do any type of reclassification.  This does not effect the
//				AIs globally; it is a local change.
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALRECLASSIFYTOENEMY_H__
#define __AIGOALRECLASSIFYTOENEMY_H__

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
//	CLASS:	CAIGoalReclassifyToEnemy
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalReclassifyToEnemy : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalReclassifyToEnemy, kGoal_ReclassifyToEnemy);

		// Activation.

		virtual void ActivateGoal();

		// Sense Handling.

		virtual LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Updating.

		virtual	void	UpdateGoal();

	protected:

		// State Handling.

	protected:

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
//		CAIGoalReclassifyToEnemy(const CAIGoalReclassifyToEnemy& rhs) {}
//		CAIGoalReclassifyToEnemy& operator=(const CAIGoalReclassifyToEnemy& rhs ) {}
};

#endif // __AIGOALRECLASSIFYTOENEMY_H__

