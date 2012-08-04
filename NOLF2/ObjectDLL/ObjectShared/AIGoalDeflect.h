//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalDeflect.h
//              
//	PURPOSE:	CAIGoalDeflect declaration
//              
//	CREATED:	06.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALDEFLECT_H__
#define __AIGOALDEFLECT_H__

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
//	CLASS:	CAIGoalDeflect
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalDeflect : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDeflect, kGoal_Deflect);

		CAIGoalDeflect( );

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

		void HandleStateDeflect();

	protected:

		LTFLOAT m_fDeflectTimeNext;

	private:
		// Private members
		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
//		CAIGoalDeflect(const CAIGoalDeflect& rhs) {}
//		CAIGoalDeflect& operator=(const CAIGoalDeflect& rhs ) {}
};

#endif // __AIGOALDEFLECT_H__

