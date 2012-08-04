//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateCatch.h
//              
//	PURPOSE:	CAIHumanStateCatch declaration
//              
//	CREATED:	12.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTATECATCH_H__
#define __AIHUMANSTATECATCH_H__

//#if _MSC_VER >= 1000
//#pragma once
//#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStateCatch
//              
//	PURPOSE:	Handle catching an incoming projectile, but raising hands, and
//				either waiting for it to show up, or timing out if it doesn't 
//				show up soon enough.
//              
//----------------------------------------------------------------------------
class CAIHumanStateCatch : public CAIHumanState
{
	public :

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCatch, kState_HumanCatch);

		CAIHumanStateCatch( );

		// Handlers

		virtual void HandleTouch(HOBJECT hObject);

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update(void);
		void UpdateAnimation(void);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }
		
		void SetAnimationSequence(EnumAnimProp eAnimProp1, EnumAnimProp eAnimProp2, EnumAnimProp eAnimProp3) { m_eAnimProp1 = eAnimProp1; m_eAnimProp2 = eAnimProp2; m_eAnimProp3 = eAnimProp3; } 
		void SetObjectToCatch( HOBJECT hObject);

	protected:
		void HandleSetupCatch();
		void HandleStartCatch();
		void HandleHoldCatch();
		void HandleDropCatch();

		LTBOOL ShouldDropCatch(void);

	protected:

		// Max time to keep the Catch up
		LTFLOAT			m_fMaxCatchDuration;

		// Direction to the object we mean to Catch, NULL
		// if no direction is set or to be used.
		LTVector		m_vIncomingPosition;

		// If we have a specific object we mean to Catch, keep a link to it
		// BUT be sure that we are handling it if it fails
		LTObjRef		m_hObjectToCatch;

		// if we Catch the object we have a handle to, then set this to true
		LTBOOL			m_bCaughtNamedObject;

		// Animation sequence to play for the Caught
		EnumAnimProp	m_eAnimProp1;
		EnumAnimProp	m_eAnimProp2;
		EnumAnimProp	m_eAnimProp3;

		// Random seed is used to insure that events are replicatable
		// for debugging reasons.
		uint32			m_iAnimRandomSeed;
		
		// How long are we going to be Catching? This is used
		// to determine the animation rate.
		LTFLOAT			m_fCatchSpeed;
};

#endif // __AIHUMANSTATECATCH_H__
