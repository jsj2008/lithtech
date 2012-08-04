//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateResurrecting.h
//              
//	PURPOSE:	CAIHumanStateResurrecting declaration
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

#ifndef __AIHUMANSTATERESURRECTING_H__
#define __AIHUMANSTATERESURRECTING_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#ifndef __AI_HUMAN_STATE_H__
#include "AIHumanState.h"
#endif

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CAIHumanStateResurrecting
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIHumanStateResurrecting : public CAIHumanState
{
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateResurrecting, kState_HumanResurrecting);

		// Public members

		// Ctors/Dtors/etc
		
		CAIHumanStateResurrecting();
		virtual ~CAIHumanStateResurrecting();
		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Initialization

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		// Update: 

		virtual void Update();
		virtual void UpdateAnimation();

		// Overridden methods:

		virtual void HandleDamage(const DamageStruct& damage);
		virtual LTBOOL RejectChangeState();
		virtual HMODELANIM GetDeathAni(LTBOOL bFront);
		

		// Simple Accessors:

		void SetResurrectingTime(LTFLOAT fTime);
		void ResetExpirationTime(void);

	protected:
		// Protected members

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CAIHumanStateResurrecting(const CAIHumanStateResurrecting& rhs) {}
		CAIHumanStateResurrecting& operator=(const CAIHumanStateResurrecting& rhs ) {}

		// Save:

		LTFLOAT		m_fResurrectCompleteTime;
		LTFLOAT		m_fResurrectCompleteDuration;
		LTBOOL		m_bEntryCanDistruct;
};

#endif // __AIHUMANSTATERESURRECTING_H__

