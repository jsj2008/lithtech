//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalSentryChallenge.h
//              
//	PURPOSE:	CAIGoalSentryChallenge declaration
//              
//	CREATED:	04.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALSENTRYCHALLENGE_H__
#define __AIGOALSENTRYCHALLENGE_H__

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
//	CLASS:	CAIGoalSentryChallenge
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalSentryChallenge : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:
		// Public members

		// Ctors/Dtors/etc
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSentryChallenge, kGoal_SentryChallenge);

		CAIGoalSentryChallenge( );
		virtual CAIGoalSentryChallenge::~CAIGoalSentryChallenge();

		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		void ActivateGoal();
		void DeactivateGoal();
		void UpdateGoal();
		LTBOOL	HandleNameValuePair(const char *szName,const char *szValue);
		LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:
		// Protected members
		void SetStateDraw();
		void SetStateChallenge();

		void HandleStateDraw();
		void HandleStateSentryChallenge();

	private:
		// Private members

		// True if for the AI who is scanning and who caused the global lock
		// to be set.  MUST unset the global lock once when the AI leaves the
		// goal
		LTBOOL m_bHasLockedGoal;

		// See note in implementation header -- truely ugly method for 
		// preventing more than 1 AI from scanning at a time.  This really
		// should be replaced with a squad based system which restricts/allows
		// AI entry into specific behaviors.
		static LTBOOL sm_bGlobalScanLocked;
};

#endif // __AIGOALSENTRYCHALLENGE_H__

