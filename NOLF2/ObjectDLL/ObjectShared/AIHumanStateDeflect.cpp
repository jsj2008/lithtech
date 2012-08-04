//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateDeflect.cpp
//              
//	PURPOSE:	CAIHumanStateDeflect implementation
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

#ifndef __AIHUMANSTATEDEFLECT_H__
#include "AIHumanStateDeflect.h"		
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

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDeflect, kState_HumanDeflect);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::CAIHumanStateDeflect()
//              
//	PURPOSE:	Handle initialization of all variables whose value is known
//				at construction of the class (without the AI present) and 
//				which are potentially not initialized later.
//              
//----------------------------------------------------------------------------
CAIHumanStateDeflect::CAIHumanStateDeflect()
{
	// AnimProp1 starts the deflection
	// AnimProp2 can be stretched to cover a variable deflection time.
	// AnimProp3 completes the deflection.
	m_eAnimProp1 = kAP_None;
	m_eAnimProp2 = kAP_None;
	m_eAnimProp3 = kAP_None;

	// By default, there are no special stop conditions
	m_eStopCondition = DEFLECT_NONE;

	// Reset the state tracking 
	m_bDeflectedNamedObject = LTFALSE;
	m_bDeflectedAnObject = LTFALSE;

	m_hObjectToDeflect = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Load()
//              
//	PURPOSE:	Restore data after a load
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Save()
//              
//	PURPOSE:	Save data
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::HandleTouch()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::HandleTouch(HOBJECT hObject)
{
	if ( hObject == m_hObjectToDeflect )
	{
		m_bDeflectedNamedObject = LTTRUE;
	}

	m_bDeflectedAnObject = LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::FailureConditionsMet()
//              
//	PURPOSE:	Determine if we have failed out of this state
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateDeflect::FailureConditionsMet(void)
{
	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Init()
//              
//	PURPOSE:	Handles initialization of the CAIHumanStateDeflect class,
//				Retreive initialization information from the AI, and change
//				the AI state to reflect entry to this state
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateDeflect::Init(CAIHuman* pAIHuman)
{
	LTBOOL bParentInitializationSuccess = super::Init(pAIHuman);
	UBER_ASSERT( bParentInitializationSuccess, "CAIHumanStateDeflect::Init Failed parent class initialization"	);

	// Retrieve the max hold time from the bute file, failing if it is not
	// defined for this AI
	m_fMaxDeflectorDuration = 
		pAIHuman->GetBrain()->GetAIData(kAIData_MaxDeflectDuration);

	// Be sure that we retrieved a MaxDeflector value..
	UBER_ASSERT( m_fMaxDeflectorDuration > 0.0000001,
		"SetupDeflection: MaxDeflectorDuration not set" );

	// Set the incoming position to a position 48 units in front of the AI.
	m_vIncomingPosition = 
		GetAI()->GetPosition() + GetAI()->GetEyeForward() * 48;

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	// Set the deflecting AI to an alert state/combat type
	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::ShouldDropDeflection()
//              
//	PURPOSE:	Tests to see if the AI ought to stop the block for a reason 
//				other than termination of the animation.  This would include
//				events such as blocking an object.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateDeflect::ShouldDropDeflection(void)
{
	// All forms are limited by the animation playing.  If the animation 
	// finishes, then the deflection chance has expired
	if ( !GetAnimationContext()->IsLocked() )
	{
		return LTTRUE;
	}

	switch( m_eStopCondition )
	{
		case DEFLECT_OBJECT:
		{
			// If the AI should stop after deflecting ANY object, then see
			// if we have defected one.
			if ( m_bDeflectedAnObject )
			{
				return LTTRUE;
			}
			break;
		}

		case DEFLECT_NAMED_OBJECT:
		{
			// If the AI should stop after deflecting ANY object, then see
			// if we have defected one.
			if ( m_bDeflectedNamedObject )
			{
				return LTTRUE;
			}
			break;
		}

		default:
		{
			UBER_ASSERT( 0, "CAIHumanStateDeflect::ShouldDropDeflection: Unexplected state in ShouldDropDeflection" );
			break;
		}
	}

	return LTFALSE;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::Update(void)
{
	super::Update();

	// Check for failure conditions (are there any for deflection?)
	if ( FailureConditionsMet() )
	{
		m_eStateStatus = kSStat_FailedComplete;
	}

	// Remember the entry status so we can see if the status changed
	// over the course of this run.
	EnumAIStateStatus eEntryStatus = m_eStateStatus;

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
		{
			// All of the sequence animations MUST be set by the time we start
			// the initialization process
			UBER_ASSERT( m_eAnimProp1 != kAP_None,
				"CAIHumanStateDeflect::UpdateAnimation Goal failed to set m_eAnimProp1");
			UBER_ASSERT( m_eAnimProp2 != kAP_None,
				"CAIHumanStateDeflect::UpdateAnimation Goal failed to set m_eAnimProp2");
			UBER_ASSERT( m_eAnimProp3 != kAP_None,
				"CAIHumanStateDeflect::UpdateAnimation Goal failed to set m_eAnimProp3");
			// Be sure that we retrieved a MaxDeflector value..
			UBER_ASSERT( m_fMaxDeflectorDuration > 0.0000001,
				"SetupDeflection: MaxDeflectorDuration not set" );

			// Get a random seed to use when playing the second animation of 
			// the triple
			CAnimationProps Props;
			Props.Set(kAPG_Posture, kAP_Stand);
			Props.Set(kAPG_WeaponPosition, kAP_Up);
			Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
			Props.Set(kAPG_WeaponAction, m_eAnimProp2); 
			m_iAnimRandomSeed = GetAnimationContext()->ChooseRandomSeed( Props );

			if ( GetAI()->HasTarget() )
			{
				// Setting a target push speed allows the AI to knock the 
				// player back if they collide while the AI deflecting
				GetAI()->GetTarget()->SetPushSpeed( GetAI()->GetSpeed() * 2.f );
			}

			// Advance to the first phase..
			m_eStateStatus = kSStat_TriplePhaseOne;
		}
		break;

		case kSStat_TriplePhaseOne:
		{
			// Set up the direction and duration of the deflection
			if ( m_hObjectToDeflect != LTNULL )
			{
				// If we have a specific object, then orient ourselves to face it
				g_pLTServer->GetObjectPos( m_hObjectToDeflect, &m_vIncomingPosition );	
			
				// Reset the facing direction
				GetAI()->FacePos( m_vIncomingPosition );
			}

			// If we are done with the previous animation (locking is off)
			// then we can enter the next phase
			if( !GetAnimationContext()->IsLocked() )
			{
				SetupDeflection();

				// Advance to the next phase..
				m_eStateStatus = kSStat_TriplePhaseTwo;
			}

			// Decide if we ought to abort this phase -- we might not want 
			// to hold the block for the animations duration
			if( ShouldDropDeflection() )
			{
				// Reset the animation rate which was changed in the 
				// previous phase
				GetAnimationContext()->SetAnimRate( 1.f );
				
				// Advance to the last phase..
				m_eStateStatus = kSStat_TriplePhaseThree;
			}
		}
		break;

		case kSStat_TriplePhaseTwo:
		{
			// Decide if we ought to abort this phase -- we might not want 
			// to hold the block for the animations duration
			if( ShouldDropDeflection() )
			{
				// Reset the animation rate which was changed in the 
				// previous phase
				GetAnimationContext()->SetAnimRate( 1.f );
				
				// Advance to the last phase..
				m_eStateStatus = kSStat_TriplePhaseThree;
			}
		}
		break;

		case kSStat_TriplePhaseThree:
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
		break;
	}

	// If we changed states, force an animation recalculation by locking,
	// and then clearing the lock
	if ( m_eStateStatus != eEntryStatus )
	{
		GetAnimationContext()->Lock();
		GetAnimationContext()->ClearLock();
	}

	if ( m_eStateStatus == kSStat_StateComplete || m_eStateStatus == kSStat_FailedComplete )
	{
		m_pAI->SetDefending( false );
		GetAnimationContext()->SetAnimRate( 1.f );
		GetAnimationContext()->Lock();
		GetAnimationContext()->ClearLock();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::SetStopCondition()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::SetStopCondition(SPECIAL_STOP_CONDITIONS eCondition)
{
	m_eStopCondition = eCondition;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::SetObjectToDeflect()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::SetObjectToDeflect(HOBJECT hTarget)
{
	UBER_ASSERT( hTarget != LTNULL, 
		"CAIHumanStateDeflect::SetObjectToDeflect: Bad target handle" );

	m_hObjectToDeflect = hTarget; 
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::SetupDeflection()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::SetupDeflection(void)
{
	LTVector vOrigin = GetAI()->GetPosition();

	// Set up the direction and duration of the deflection
	if ( m_hObjectToDeflect != LTNULL )
	{
		// If we have a specific object, then orient ourselves to face it
		g_pLTServer->GetObjectPos( m_hObjectToDeflect, &m_vIncomingPosition );	
	}

	// Find the length of the fly animation.

	CAnimationProps Props;
	Props.Set(kAPG_Posture, kAP_Stand);
	Props.Set(kAPG_WeaponPosition, kAP_Up);
	Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
	Props.Set(kAPG_WeaponAction, m_eAnimProp2);

	// Insure that the animation we find the length of is the same
	// animation we later set by setting the randomseed now, and restoring
	// it later when we set the animation
	GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );

	LTFLOAT fAnimLength = GetAnimationContext()->GetAnimationLength( Props );

	LTFLOAT fAnimRate = fAnimLength / m_fMaxDeflectorDuration;
//	GetAnimationContext()->SetAnimRate( fAnimRate );

	m_pAI->SetDefending( true );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateDeflect::UpdateAnimation()
//              
//	PURPOSE:	Sets animation flags depending on state, then locks the 
//				animation in place.  Here and only here do we handle setting
//				the props.
//              
//----------------------------------------------------------------------------
void CAIHumanStateDeflect::UpdateAnimation(void)
{
	super::UpdateAnimation();

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
			// Set the seed generated earlier to keep sync between the 
			// animation and and the earlier selected frame rate
			GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );
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