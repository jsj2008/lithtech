//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateDeflect.h
//              
//	PURPOSE:	CAIHumanStateDeflect declaration
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

#ifndef __AIHUMANSTATEDEFLECT_H__
#define __AIHUMANSTATEDEFLECT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"

// Forward declarations

// Globals

// Statics

//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStateDeflect
//              
//	PURPOSE:	Handle deflecting an incoming projectile by raising a weapon
//				To protect ones self.  A deflection can be relative to an 
//				object or a direction.  It can terminate on either a timeout
//				or on being hit by an object.
//              
//----------------------------------------------------------------------------
class CAIHumanStateDeflect : public CAIHumanState
{
	typedef CAIHumanState super;

	public :

		// Time is a default stop condition as the animation, as 
		// animations are streched to a particular length, and at the 
		// termination of the animation we need to stop blocking anyway
		enum SPECIAL_STOP_CONDITIONS
		{
			DEFLECT_NONE,
			DEFLECT_OBJECT,
			DEFLECT_NAMED_OBJECT,
		};

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDeflect, kState_HumanDeflect);

		CAIHumanStateDeflect( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Handlers

		virtual void HandleTouch(HOBJECT hObject);

		// Update

		void Update(void);
		void UpdateAnimation(void);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }
		
		void SetAnimationSequence(EnumAnimProp eAnimProp1, EnumAnimProp eAnimProp2, EnumAnimProp eAnimProp3) { m_eAnimProp1 = eAnimProp1; m_eAnimProp2 = eAnimProp2; m_eAnimProp3 = eAnimProp3; } 
		void SetStopCondition( SPECIAL_STOP_CONDITIONS eCondition );
		void SetObjectToDeflect( HOBJECT );

	protected:

		void SetupDeflection();
		LTBOOL ShouldDropDeflection(void);
		LTBOOL FailureConditionsMet(void);

	protected:

		// Stop deflecting once an object is deflected.
		SPECIAL_STOP_CONDITIONS 	m_eStopCondition;
		
		// Max time to keep the deflector up
		LTFLOAT			m_fMaxDeflectorDuration;

		// Direction to the object we mean to deflect, NULL
		// if no direction is set or to be used.
		LTVector		m_vIncomingPosition;

		// If we have a specific object we mean to deflect, keep a link to it
		// BUT be sure that we are handling it if it fails
		LTObjRef		m_hObjectToDeflect;

		// if we deflect the object we have a handle to, then set this to true
		LTBOOL			m_bDeflectedNamedObject;

		// If we deflect any object, then set this to true.
		LTBOOL			m_bDeflectedAnObject;

		// Animation sequence to play for the deflection
		EnumAnimProp	m_eAnimProp1;
		EnumAnimProp	m_eAnimProp2;
		EnumAnimProp	m_eAnimProp3;

		// Random seed is used to insure that events are replicatable
		// for debugging reasons.
		uint32			m_iAnimRandomSeed;
		
		// How long are we going to be deflecting? This is used
		// to determine the animation rate.
		LTFLOAT			m_fDeflectSpeed;
};

#endif // __AIHUMANSTATEDEFLECT_H__
