//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalSentryMark.h
//              
//	PURPOSE:	CAIGoalSentryMark declaration
//              
//	CREATED:	10.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALSENTRYMARK_H__
#define __AIGOALSENTRYMARK_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#ifndef __AIGOAL_ABSTRACT_STIMULATED_H__
#include "AIGoalAbstractStimulated.h"
#endif

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CAIGoalSentryMark
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalSentryMark : public CAIGoalAbstractStimulated
{
	public:
		// Public members

		// Ctors/Dtors/etc
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSentryMark, kGoal_SentryMark);

		CAIGoalSentryMark();
		virtual ~CAIGoalSentryMark();

		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		void ActivateGoal();
		void DeactivateGoal();
		void UpdateGoal();
		LTBOOL	HandleNameValuePair(const char *szName,const char *szValue);
		LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:
		// Protected members

		void	SetStateSentryMark();
		void	SetStateDraw();

		void	HandleStateSentryMark();
		void	HandleStateDraw();
	
	private:
};

#endif // __AIGOALSENTRYMARK_H__

