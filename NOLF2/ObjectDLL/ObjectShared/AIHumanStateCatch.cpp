
//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateCatch.cpp
//              
//	PURPOSE:	CAIHumanStateCatch implementation
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


// Includes
#include "stdafx.h"

#ifndef __AIHUMANSTATECATCH_H__
#include "AIHumanStateCatch.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif 

#ifndef __AI_TARGET_H__
#include "AITarget.h"
#endif

// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCatch, kState_HumanCatch);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::CAIHumanStateCatch()
//              
//	PURPOSE:	Handle initialization of all variables whose value is known
//				at construction of the class (without the AI present) and 
//				which are potentially not initialized later.
//              
//----------------------------------------------------------------------------
CAIHumanStateCatch::CAIHumanStateCatch()
{
	// AnimProp1 starts the Catch
	// AnimProp2 can be stretched to cover a variable Catch time.
	// AnimProp3 completes the Catch.
	m_eAnimProp1 = kAP_None;
	m_eAnimProp2 = kAP_None;
	m_eAnimProp3 = kAP_None;

	// Reset the state tracking 
	m_bCaughtNamedObject = LTFALSE;

	m_hObjectToCatch = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Load()
//              
//	PURPOSE:	Restore data after a load
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Load(ILTMessage_Read *pMsg)
{
	CAIHumanState::Load(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Save()
//              
//	PURPOSE:	Save data
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Save(ILTMessage_Write *pMsg)
{
	CAIHumanState::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleTouch()
//              
//	PURPOSE:	Registers a catch when the object touches the catcher.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleTouch(HOBJECT hObject)
{
	if ( hObject == m_hObjectToCatch )
	{
		m_bCaughtNamedObject = LTTRUE;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Init()
//              
//	PURPOSE:	Handles initialization of the CAIHumanStateCatch class,
//				Retreive initialization information from the AI, and change
//				the AI state to reflect entry to this state
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateCatch::Init(CAIHuman* pAIHuman)
{
	LTBOOL bParentInitializationSuccess = CAIHumanState::Init(pAIHuman);
	UBER_ASSERT( bParentInitializationSuccess, "CAIHumanStateCatch::Init Failed parent class initialization"	);

	// Retrieve the max hold time from the bute file, failing if it is not
	// defined for this AI
	m_fMaxCatchDuration = pAIHuman->GetBrain()->GetAIData(kAIData_MaxCatchDuration);

	// Be sure that we retrieved a MaxCatch value..
	UBER_ASSERT( m_fMaxCatchDuration > 0.0000001, "SetupCatch: MaxCatchDuration not set" );

	// Set the incoming position to a position 48 units in front of the AI.
	m_vIncomingPosition =  GetAI()->GetPosition() + GetAI()->GetEyeForward() * 48;

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	// Set the Catching AI to an alert state/combat type
	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::Update(void)
{
	CAIHumanState::Update();

	// Remember the entry status so we can see if the status changed
	// over the course of this run.
	EnumAIStateStatus eEntryStatus = m_eStateStatus;

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
		{
			HandleSetupCatch();
			break;
		}
		case kSStat_TriplePhaseOne:
		{
			HandleStartCatch();
			break;
		}
		case kSStat_TriplePhaseTwo:
		{
			HandleHoldCatch();
			break;
		}
		case kSStat_TriplePhaseThree:
		{
			HandleDropCatch();
			break;
		}
	}

	// If we changed states, force an animation recalculation by locking,
	// and then clearing the lock
	if ( m_eStateStatus != eEntryStatus )
	{
		GetAnimationContext()->Lock();
		GetAnimationContext()->ClearLock();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::UpdateAnimation()
//              
//	PURPOSE:	Sets animation flags depending on state, then locks the 
//				animation in place.  Here and only here do we handle setting
//				the props.
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::UpdateAnimation(void)
{
	CAIHumanState::UpdateAnimation();

	// Set the common flags..
	GetAnimationContext()->SetProp( kAPG_Posture, kAP_Stand );
	GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_Up );

	switch( m_eStateStatus )
	{
		case kSStat_TriplePhaseOne:
		{
			GetAnimationContext()->SetProp( kAPG_WeaponAction, m_eAnimProp1 );
			break;
		}
		case kSStat_TriplePhaseTwo:
		{
			GetAnimationContext()->SetProp( kAPG_WeaponAction, m_eAnimProp2 );
			break;
		}
		case kSStat_TriplePhaseThree:
		{
			GetAnimationContext()->SetProp( kAPG_WeaponAction, m_eAnimProp3 );
			break;
		}
		case kSStat_StateComplete:
		{
			break;
		}
	}

	// Always lock the animation.  Let the caller handle unlocking later
	// if the animation ends.
	GetAnimationContext()->Lock();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::ShouldDropCatch()
//              
//	PURPOSE:	Tests to see if the AI ought to stop the catch for a reason 
//				other than termination of the animation.  This would include
//				events such as catching an object.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateCatch::ShouldDropCatch(void)
{
	// If the AI should stop after Catching ANY object, then see
	// if we have defected one.
	if ( m_bCaughtNamedObject )
	{
		return LTTRUE;
	}

	// If the named object no longer exists, then we obviously don't want to
	// wait around for it to show up.  Stop trying to catch it now.
	if ( m_hObjectToCatch == LTNULL )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleSetupCatch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleSetupCatch()
{
	// All of the sequence animations MUST be set by the time we start
	// the initialization process
	UBER_ASSERT( m_eAnimProp1 != kAP_None,
		"CAIHumanStateCatch::UpdateAnimation Goal failed to set m_eAnimProp1");
	UBER_ASSERT( m_eAnimProp2 != kAP_None,
		"CAIHumanStateCatch::UpdateAnimation Goal failed to set m_eAnimProp2");
	UBER_ASSERT( m_eAnimProp3 != kAP_None,
		"CAIHumanStateCatch::UpdateAnimation Goal failed to set m_eAnimProp3");
	// Be sure that we retrieved a MaxCatch value..
	UBER_ASSERT( m_fMaxCatchDuration > 0.0000001,
		"SetupCatch: MaxCatchDuration not set" );

	CAnimationProps Props;
	Props.Set(kAPG_Posture, kAP_Stand);
	Props.Set(kAPG_WeaponPosition, kAP_Up);
	Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
	Props.Set(kAPG_WeaponAction, m_eAnimProp2); 

	if ( GetAI()->HasTarget() )
	{
		// Setting a target push speed allows the AI to knock the 
		// player back if they collide while the AI Catching
		GetAI()->GetTarget()->SetPushSpeed( GetAI()->GetSpeed() * 2.f );
	}

	// Advance to the first phase..
	m_eStateStatus = kSStat_TriplePhaseOne;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleStartCatch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleStartCatch()
{
	// Set up the direction and duration of the Catch
	if ( m_hObjectToCatch != LTNULL )
	{
		// Reset the facing direction
		GetAI()->FacePos( m_vIncomingPosition );
	}

	// If we are done with the previous animation (locking is off)
	// then we can enter the next phase
	if( !GetAnimationContext()->IsLocked() )
	{
		LTVector vOrigin = GetAI()->GetPosition();

		// Set up the direction and duration of the Catch
		if ( m_hObjectToCatch != LTNULL )
		{
			// If we have a specific object, then orient ourselves to face it
			g_pLTServer->GetObjectPos( m_hObjectToCatch, &m_vIncomingPosition );	
		}

		// Find the length of the fly animation.

		CAnimationProps Props;
		Props.Set(kAPG_Posture, kAP_Stand);
		Props.Set(kAPG_WeaponPosition, kAP_Up);
		Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
		Props.Set(kAPG_WeaponAction, m_eAnimProp2);

		// Advance to the next phase..
		m_eStateStatus = kSStat_TriplePhaseTwo;
	}

	// Decide if we ought to abort this phase -- we might not want 
	// to hold the catch for the animations duration
	if( ShouldDropCatch() )
	{
		// Advance to the last phase..
		m_eStateStatus = kSStat_TriplePhaseThree;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleHoldCatch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleHoldCatch()
{
	// Decide if we ought to abort this phase -- we might not want 
	// to hold the catch for the animations duration
	if( ShouldDropCatch() )
	{
		// Advance to the last phase..
		m_eStateStatus = kSStat_TriplePhaseThree;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::HandleDropCatch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::HandleDropCatch()
{
	// If we are done with the previous animation (locking is off)
	// then we can enter the next phase
	if( !GetAnimationContext()->IsLocked() )
	{
		// If we have a target, turn to face it now.
		if ( GetAI()->HasTarget() )
		{
			GetAI()->FaceTarget();
		}
		
		m_eStateStatus = kSStat_StateComplete;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateCatch::SetObjectToCatch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateCatch::SetObjectToCatch(HOBJECT hObject)
{
	UBER_ASSERT( hObject != LTNULL, "CAIHumanStateCatch::SetObjectToCatch: Bad target handle" );
	m_hObjectToCatch = hObject; 
}