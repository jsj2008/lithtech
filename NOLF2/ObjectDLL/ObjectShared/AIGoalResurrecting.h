//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalResurrecting.h
//              
//	PURPOSE:	CAIGoalResurrecting declaration
//              
//	CREATED:	28.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIGOALRESURRECTING_H__
#define __AIGOALRESURRECTING_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIGoalAbstractSearch.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CAIGoalResurrecting
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIGoalResurrecting : public CAIGoalAbstractSearch
{
	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalResurrecting, kGoal_Resurrecting);

		// Public members

		// Ctors/Dtors/etc
		CAIGoalResurrecting();
		virtual ~CAIGoalResurrecting();

		// Save/Load
		virtual void Save(ILTMessage_Write *pMsg);
        virtual void Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);
		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord );

	protected:
		// Protected members

		void HandleStateResurrecting();

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CAIGoalResurrecting(const CAIGoalResurrecting& rhs) {}
		CAIGoalResurrecting& operator=(const CAIGoalResurrecting& rhs ) {}

		LTFLOAT		m_fTimeToResurrect;
		LTBOOL		m_bReactivateGoalOnUpdate;
};

#endif // __AIGOALRESURRECTING_H__

