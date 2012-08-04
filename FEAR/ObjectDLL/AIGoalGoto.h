// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGoto.h
//
// PURPOSE : AIGoalGoto class definition
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_GOTO_H__
#define __AIAMGOAL_GOTO_H__

#include "AIGoalAbstract.h"
#include "AIWorkingMemory.h"

// Forward declarations.

class	CAIWMFact;


// Goto Request Struct.

struct SAI_GOTO_REQUEST
{
	SAI_GOTO_REQUEST();

	ENUM_FactID		eFactID;
	LTObjRef		hNode;
	bool			bTaskIsScripted;
};

// ----------------------------------------------------------------------- //

class CAIGoalGoto : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGoto, kGoal_Goto );
		
		CAIGoalGoto();
		virtual ~CAIGoalGoto();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual bool		ReplanRequired();

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

		virtual void		UpdateTaskStatus();

		virtual void		HandleBuildPlanFailure();

	protected:

		void				ClearGotoTask( SAI_GOTO_REQUEST* pGotoRequest );

	protected:

		SAI_GOTO_REQUEST	m_NodeCurrent;
		SAI_GOTO_REQUEST	m_NodePending;

		// The WorkingMemory Task Type does not need to be saved.

		ENUM_AIWMTASK_TYPE	m_eTaskType;
		bool				m_bCheckNodeValidity;
		bool				m_bClearScriptedTaskIfThreatened;
};


#endif
