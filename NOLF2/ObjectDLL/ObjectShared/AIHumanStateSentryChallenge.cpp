//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateSentryChallenge.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	05.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	The Sentry Charge State is the state where an AI will attempt
//				to determine if another character has permission to be where
//				they are.  This involves attaining some sort of proximity and
//				some form of line of sight.  It results in an AI declaring
//				the other character as friend or enemy.
//              
//				Save/Load is untested.
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIHUMANSTATESENTRYCHALLENGE_H__
#include "AIHumanStateSentryChallenge.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif 

#ifndef __OBJECTRELATIONMGR_H__
#include "ObjectRelationMgr.h"
#endif

#ifndef __AIVOLUME_H__
#include "AIVolume.h"
#endif 

#ifndef __AI_VOLUME_MGR_H__
#include "AIVolumeMgr.h"
#endif

#ifndef __AIINFORMATIONVOLUMEMGR_H__
#include "AIInformationVolumeMgr.h"
#endif

#ifndef __AIREGION_H__
#include "AIRegion.h"
#endif

#include "Weapon.h"

// Forward declarations

// Globals
extern CAIInformationVolumeMgr* g_pAIInformationVolumeMgr;

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSentryChallenge, kState_HumanSentryChallenge);


// Buteable:

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::CAIHumanStateSentryChallenge()
//              
//	PURPOSE:	Handle initialization of all variables whose value is known
//				at construction of the class (without the AI present) and 
//				which are potentially not initialized later.
//              
//----------------------------------------------------------------------------
CAIHumanStateSentryChallenge::CAIHumanStateSentryChallenge()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
}

CAIHumanStateSentryChallenge::~CAIHumanStateSentryChallenge()
{
	AI_FACTORY_DELETE( m_pStrategyFollowPath );
}

enum PropSetSelected
{
	kNull,
	kChallengeAnimProps,
	kPassProps,
	kFailProps
};

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::Load()
//              
//	PURPOSE:	Restore data after a load
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::Load(ILTMessage_Read *pMsg)
{
	CAIHumanState::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_HOBJECT(m_hToChallenge);

	m_ChallengeProps.Load(pMsg);
	m_PassProps.Load(pMsg);
	m_FailProps.Load(pMsg);

	m_LastKnownPosition.Load(pMsg);
	m_Challenge.Load(pMsg);
	
	m_RD.Load(pMsg);
	LOAD_INT(m_iRecipientMask);
	
	LOAD_BYTE_CAST(m_eState, eHumanChallengeState);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::Save()
//              
//	PURPOSE:	Save data
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::Save(ILTMessage_Write *pMsg)
{
	CAIHumanState::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_HOBJECT(m_hToChallenge);

	m_ChallengeProps.Save(pMsg);
	m_PassProps.Save(pMsg);
	m_FailProps.Save(pMsg);

	m_LastKnownPosition.Save(pMsg);
	m_Challenge.Save(pMsg);
	m_RD.Save(pMsg);
	SAVE_INT(m_iRecipientMask);
	
	SAVE_BYTE(m_eState);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::FailureConditionsMet()
//              
//	PURPOSE:	Determine if we have failed out of this state
//              
//----------------------------------------------------------------------------
bool CAIHumanStateSentryChallenge::IsStateStillValid(void) const
{
	// If our thing to challenge is gone, then the state is invalid.
	if ( m_hToChallenge == NULL || GetChallengeCharacter() == NULL )
	{
		return false;	
	}

		return true;
	}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::Init()
//              
//	PURPOSE:	Handles initialization of the CAIHumanStateSentryChallenge class,
//				Retreive initialization information from the AI, and change
//				the AI state to reflect entry to this state
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateSentryChallenge::Init(CAIHuman* pAIHuman)
{
	if ( !CAIHumanState::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);
	m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	m_Challenge.Clear();

	AIASSERT( m_pAI->GetCurrentWeapon(), m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );
	AIASSERT( ((CAIHuman*)m_pAI)->GetWeaponProp( m_pAI->GetCurrentWeapon()->GetWeaponData()) != kAP_Weapon3, m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );

	m_eState = eChasing;

	// Get the max distance we will challenge at.
	m_fCloseEnoughToScanSqr = m_pAI->GetSentryChallengeScanDistMax()*m_pAI->GetSentryChallengeScanDistMax();

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::SetAnimProp()
//              
//	PURPOSE:	Set a property to select the animation to be used in case the
//				challenged object fails
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::SetAnimProp(eActionType Action,
											   EnumAnimPropGroup eGroup,
														EnumAnimProp eProp )
{
	switch (Action)
{
	case kFailedAction:
		m_FailProps.Set( eGroup, eProp );
		break;

	case kPassedAction:
	m_PassProps.Set( eGroup, eProp );
		break;
	
	case kChallengeAction:
		m_ChallengeProps.Set( eGroup, eProp );
		break;
}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::SetObjectToSentryChallenge()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::SetObjectToSentryChallenge(HOBJECT hObject)
{
	UBER_ASSERT( hObject != LTNULL, "CAIHumanStateSentryChallenge::SetObjectToSentryChallenge: Bad target handle" );
	m_hToChallenge = hObject; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::Update(void)
{
	CAIHumanState::Update();

	// Check for failure conditions (are there any for SentryChallenge?)
	if ( IsStateStillValid() == false )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	MaybePlaySound();
	
	m_LastKnownPosition.Clear();
	
	m_LastKnownPosition.Update(GetAI(), m_hToChallenge);

	switch(m_eState)
	{
	case eChasing:
		HandleChasing();
		break;
	
	case eScanning:
		HandleScanning();
		break;

	case eReactingToScan:
		HandleReactToScan();
		break;

	case eDone:
		HandleDone();
		break;

	default:
		AIASSERT( 0, m_pAI->m_hObject, "CAIHumanStateSentryChallenge::Update: Unknown state" );
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Challenge::HandleChasing()
//              
//	PURPOSE:	If we should move closer, do so.  Otherwise if we are close
//				enough, either start scanning, or if we have a result already,
//				jump straight to reacting.
//              
//				If we already have a defined relationship with the character, 
//				then exit now since we havent' started any of the special
//				animations
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::HandleChasing()
{
	CharacterAlignment ca = GetAlignment(m_pAI->GetRelationSet(), GetChallengeCharacter()->GetRelationData());
	if ( ca != UNDETERMINED && ca != TOLERATE )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	if ( m_Challenge.GetResult() == Challenge::kCR_Pass )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	if ( m_Challenge.GetResult() == Challenge::kCR_Fail )
		{
		// If we know what they are, just react.
		ActOnChallengeResult();
		return;
	}

	if ( !ShouldMoveCloser())
	{
		SetState(eScanning);
		return;
	}
			
			MoveCloser();
		}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::ActOnChallengeResult()
//              
//	PURPOSE:	Set our awareness depending on the results of the scan,
//				generate communication info, and change the state
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::ActOnChallengeResult()
{
	switch ( m_Challenge.GetResult() )
		{
	case Challenge::kCR_Fail:
		m_pAI->SetAwareness( kAware_Alert );
		SetState(eReactingToScan);
		break;

	case Challenge::kCR_Pass:
		m_pAI->SetAwareness( kAware_Relaxed );
		SetState(eDone);
		break;
	
	default:
		AIASSERT( 0, m_pAI->m_hObject, "CAIHumanStateSentryChallenge::ActOnChallengeResult" );
		break;
	}

		GenerateCommunicationInfo();

	if ( m_RD.eAlignment != INVALID )
	{
		// Send the message the moment the AI is done with the marking
		// animation ONLY if they managed to find something
		// significant out.  It is possible to play the pass anim
		// in the case where someone else did the mark and we just
		// wanted get the hands down cleanly.
		GetAI()->GetRelationMgr()->CommunicateMessage(m_pAI, m_RD, m_iRecipientMask);
		if ( m_RD.eAlignment == HATE )
		{
			CAIHuman* pAIHuman = (CAIHuman*)GetAI();
			if ( pAIHuman && pAIHuman->HasOnMarkingString() )
			{
				const char* pCmd = pAIHuman->GetOnMarkingString();
				if( g_pCmdMgr->IsValidCmd( pCmd ) )
				{
					g_pCmdMgr->Process( pCmd, pAIHuman->m_hObject, NULL );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::HandleScanning()
//              
//	PURPOSE:	Implement the scanning behavior
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::HandleScanning()
{
	AIASSERT( m_Challenge.HasValidResult(), m_pAI->m_hObject, "" );

	// MUST SET PROP AFTER THE CHECK FOR LOCKING!
	
	// If we now have a valid challenge result, then change our current prop set
	CharacterAlignment ca = GetAlignment(m_pAI->GetRelationSet(), GetChallengeCharacter()->GetRelationData());
	if ( ca != UNDETERMINED && ca != TOLERATE )
	{
		SetState(eDone);
		return;
	}

	switch ( m_Challenge.GetResult() )
	{
		case Challenge::kCR_Unknown:
			GetAI()->FaceObject( m_hToChallenge );
		break;

	case Challenge::kCR_Fail:
		case Challenge::kCR_Pass:
			ActOnChallengeResult();
		break;

	default:
		UBER_ASSERT(0, "Unexpected challenge result in CAIHumanStateSentryChallenge::UpdateInternalState");
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Challenge::HandleReactToScan()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::HandleReactToScan()
{	
	AIASSERT( m_Challenge.GetResult() != Challenge::kCR_Unknown, m_pAI->m_hObject, "" );

	if ( m_hToChallenge != NULL )
	{
		GetAI()->FaceObject( m_hToChallenge );
	}

		// See if we are really done with this State.  If we played our animation
		// and are done with it, then we finished our job.
	if ( GetAnimationContext()->IsLocked() == LTTRUE )
		{
		return;
		}

	SetState(eDone);
	}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::HandleDone()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::HandleDone()
{
	// See if we are really done with this State.  If we played our animation
	// and are done with it, then we finished our job.
	if ( GetAnimationContext()->IsLocked() == LTTRUE )
	{
		return;
	}

	m_eStateStatus = kSStat_StateComplete;
	SetState( eStateComplete );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::SetState()
//              
//	PURPOSE:	State changing function to be sure that locks are cleared
//				whenever states are changed.  Also should assist debugging
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::SetState(eHumanChallengeState eState)
	{
	AIASSERT( m_eState!=eState, m_pAI->m_hObject, "Changed state to current state -- check AI logic" );

	m_eState = eState;
		
	GetAnimationContext()->Lock();
	GetAnimationContext()->ClearLock();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::HandleModelString()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::HandleModelString(ArgList* pArgList)
{
	CAIHumanState::HandleModelString(pArgList);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleModelString(pArgList);
	}

	// See if we recieve our FIRE message.  If we do, then do the scan.
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 )
	{
		return;
	}

	char* szKey = pArgList->argv[0];
	if ( !szKey )
	{
		return;
	}

	if ( !_stricmp(szKey, c_szKeyFireWeapon) )
	{
	UBER_ASSERT( GetChallengeCharacter(), "Unable to retrieve challenge character" );

	if ( HATE == GetAlignment( GetAI()->GetRelationSet(), GetChallengeCharacter()->GetRelationData() ) )
	{
		// If we already know they are bad, the just set the result of fail!
		m_Challenge.SetResult( Challenge::kCR_Fail );
	}
	else
	{
		// Get the result of the challenge
		m_Challenge.DoChallenge( GetChallengeCharacter() );
	}
}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::GenerateCommunicationInfo()
//              
//	PURPOSE:	Generates the Communication info.  
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::GenerateCommunicationInfo()
{
	// Restricts who we are sending the message to.  Always send it to the player
	m_iRecipientMask = CommReqs::kCommReqs_Player;

	m_RD.eTrait = RelationTraits::kTrait_Name;
	LTStrCpy( m_RD.szValue, GetObjectName(GetChallengeCharacter()->m_hObject), sizeof(m_RD.szValue) );

	switch( m_Challenge.GetResult() )
	{
	case Challenge::kCR_Pass:
		m_RD.eAlignment = TOLERATE;
		// When a character passes, its unexceptional, so send to those in the same 
		// region as the tester AI.
		m_iRecipientMask |= CommReqs::kCommReqs_SameRegion;
		m_iRecipientMask |= CommReqs::kCommReqs_Friend;
		break;
	
	case Challenge::kCR_Fail:
		m_RD.eAlignment = HATE;
		// Failing is exceptional, so ignore the region and just send the relationship
		// to everyone!
		m_iRecipientMask |= CommReqs::kCommReqs_Collective;
		m_iRecipientMask |= CommReqs::kCommReqs_Friend;
		break;

	default:
		UBER_ASSERT( 0, "GenerateCommunicationInfo: Unexpected challenge result!" );
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::ShouldMoveCloser()
//              
//	PURPOSE:	Returns true if the AI ought to attempt to move closer to the 
//				challengee, false if it should not.
//
//				If we cannot see what we are trying to challenge, OR if we
//				don't know what they are and they are far away, we ought to
//				get closer
//              
//----------------------------------------------------------------------------
bool CAIHumanStateSentryChallenge::ShouldMoveCloser()
{
	if ( !m_LastKnownPosition.IsVisibile( GetAI(), m_hToChallenge ) )
	{
		// Always move closer if we can't see them, even if it is just to
		// mark.
		return true;
	}

	if ( m_LastKnownPosition.GetRangeSqr(GetAI()) > m_fCloseEnoughToScanSqr )
	{
		// If they are a long wants away, move closer.
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::MoveCloser()
//              
//	PURPOSE:	Does the chasing.
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::MoveCloser(void)
{
	// If the enemy has moved a ways from the last point we were moving to,
	// then reset the path considering this new point.  This should be based 
	// on how far away they are from us, but never should be recalculated
	// every frame as the AI may stop and run in place if this happens.

	LTVector ToLastKnownPosition = m_pStrategyFollowPath->GetDest() - m_LastKnownPosition.GetLKP();
	float flMovementDist = VEC_MAGSQR( ToLastKnownPosition );
	float flRange = m_LastKnownPosition.GetRangeSqr(GetAI());
	if ( flMovementDist > flRange )
	{
		m_pStrategyFollowPath->Reset();
	}

	if ( m_pStrategyFollowPath->IsUnset() || m_pStrategyFollowPath->IsDone() )
	{
		if ( !m_pStrategyFollowPath->Set( m_LastKnownPosition.GetLKP(), LTFALSE ) )
		{
			// WE COULDN'T SET A PATH
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->Update( );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::MaybePlaySound()
//              
//	PURPOSE:	Test to see if a sound out to be played, and play one if we
//				should.
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::MaybePlaySound()
{
	// Play our first sound if we should -- Do we have a marking sound?
	if ( m_bPlayFirstSound )
{
		GetAI()->PlaySound(kAIS_Search, LTFALSE);
		return;
	}

	if ( !m_bFirstUpdate )
	{
		return;
}

	GetAI()->PlayCombatSound(kAIS_Search);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::UpdateAnimation()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateSentryChallenge::UpdateAnimation(void)
{
	CAIHumanState::UpdateAnimation();

	// Challengers always have their weapons up and are standing while 
	// challenging.
	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	// If our pathing is not done, and if we have an path set, then update
	// the path strategy.  If either of these are not true, then update the 
	// challenge animation attempt.

	switch (m_eState)
	{
	case eChasing:
		{
			if( !m_pStrategyFollowPath->IsDone() && m_pStrategyFollowPath->IsSet() )
	{
				m_pStrategyFollowPath->UpdateAnimation();
			}
	}
		break;

	case eScanning:
	{
			GetAnimationContext()->SetProps( m_ChallengeProps );
	}
		break;
	
	case eReactingToScan:
	{
			switch ( m_Challenge.GetResult() )
			{
			case Challenge::kCR_Fail:
				GetAnimationContext()->SetProps( m_FailProps );
				GetAnimationContext()->Lock();
				break;

			default:
				AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateSentryChallenge::UpdateAnimation: Unexpected result" );
				break;
	}
}
		break;

	case eDone:
		{
			GetAnimationContext()->SetProps( m_PassProps );
			GetAnimationContext()->Lock();
		}
		break;

	case eStateComplete:
{
			// Do Nothing
		}
		break;

	default :
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateSentryChallenge::UpdateAnimation: Unexpected state" );
		break;
	}
}







//----------------------------------------------------------------------------
//              
//	ROUTINE:	CLastKnownPosition::Update()
//              
//	PURPOSE:	Refresh the information in the last known position by checking
//				and setting visibility state
//              
//----------------------------------------------------------------------------
void CLastKnownPosition::Update(CAI* pAI, HOBJECT hTarget)
	{
		// Cache where we last saw the Object we are attempting to Challenge
		// so that we can move to that point later on if we lose visibility.
	if ( IsVisibile(pAI, hTarget) )
	{
		SetLKP(hTarget);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CLastKnownPosition::SetLKP()
//              
//	PURPOSE:	Sets the last known position either to the passed in position
//				if there is one, or to the objects absolute position
//              
//----------------------------------------------------------------------------
void CLastKnownPosition::SetLKP(HOBJECT hTarget, const LTVector* pvLastKnownPosition)
{
	if ( pvLastKnownPosition == NULL )
	{
		UBER_ASSERT( hTarget != NULL,
			"Null Challenger in setting last known position" );

		g_pLTServer->GetObjectPos(hTarget, &m_vLastKnownPosition);
	}
	else
	{
		m_vLastKnownPosition = *pvLastKnownPosition;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateSentryChallenge::IsVisibile()
//              
//	PURPOSE:	HELPER FUNCTION: Can we see the object we are attempting to
//				challenge?  This function caches is result.
//              
//----------------------------------------------------------------------------
LTBOOL CLastKnownPosition::IsVisibile(CAI* pAI, HOBJECT hTarget)
{
	if ( !m_bVisibilityCheckCached )
	{
		GenerateVisibility(pAI, hTarget);
	}

	return m_bVisibilityCheckResult;
}

void CLastKnownPosition::Load(ILTMessage_Read *pMsg)
{
	LOAD_VECTOR(m_vLastKnownPosition);
	LOAD_BOOL(m_bVisibilityCheckResult);
}

void CLastKnownPosition::Save(ILTMessage_Write *pMsg)
{
	SAVE_VECTOR(m_vLastKnownPosition);
	SAVE_BOOL(m_bVisibilityCheckResult);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CLastKnownPosition::GenerateVisibility()
//              
//	PURPOSE:	Generates the cached visibility information
//              
//----------------------------------------------------------------------------
void CLastKnownPosition::GenerateVisibility(CAI* pAI, HOBJECT hTarget )
	{
	// Get information about where the target is
		LTVector vPosition;
		g_pLTServer->GetObjectPos(hTarget, &vPosition);
	float flVisualRange = pAI->GetSenseRecorder()->GetSenseDistanceSqr(kSense_SeeEnemy);

	// Remember that we have cached visibility
	m_bVisibilityCheckCached = true;

	if ( !pAI->CanShootThrough() )
		{
		if ( pAI->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL,
			hTarget, vPosition, flVisualRange, !pAI->IsAlert(), LTFALSE) )
			{
			m_bVisibilityCheckResult = true;
			return;
			}
		}
		else
		{
		if ( pAI->IsObjectPositionVisibleFromEye(CAI::ShootThroughFilterFn,
			CAI::ShootThroughPolyFilterFn, hTarget, vPosition, flVisualRange,
			!pAI->IsAlert(), LTFALSE) )
			{
			m_bVisibilityCheckResult = true;
			return;
			}
		}
	
		// Save the result of the expensive operation in the cache, and
		// remember that it was cached
	m_bVisibilityCheckResult = false;
	}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CLastKnownPosition::GetRangeSqr()
//              
//	PURPOSE:	Returns the squared distance to the last known position
//              
//----------------------------------------------------------------------------
float CLastKnownPosition::GetRangeSqr(CAI* pAI) const
{
	return ( VEC_DISTSQR(m_vLastKnownPosition, pAI->GetPosition()) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Challenge::DoChallenge()
//              
//	PURPOSE:	Sets the m_eChallengeResult to true if the challenged
//				character does have permission to be where they are, false if
//				they do not have permission.
//              
//----------------------------------------------------------------------------
void Challenge::DoChallenge( const CCharacter* pChallengedChar )
{
	UBER_ASSERT(pChallengedChar!=NULL, "DoChallenge: Challenged charater is NULL" );

	LTVector vPos;
	g_pLTServer->GetObjectPos( pChallengedChar->m_hObject, &vPos );

	const AIInformationVolume* pInformationVolume = g_pAIInformationVolumeMgr->FindContainingVolume( LTNULL, vPos, eAxisHorizontal );

	if ( !pInformationVolume || pInformationVolume && !pInformationVolume->HasRegion() )
	{
		// If we have no Volume, or if the Volume does not have a Region, issue
		// a warning, and let the character pass
		Warn( "Character at x:%f y:%f z:%f without an information Volume/Region",
			vPos.x, vPos.y, vPos.z );

		m_eChallengeResult = kCR_Pass;
		return;
	}

	uint8 iRequiredSet = pInformationVolume->GetRegion()->GetPsetByte();
	uint8 iPosessedSet = pChallengedChar->GetPermissionSet();

	uint8 iTestSet = iRequiredSet & iPosessedSet;
	bool bPassed = ( iTestSet == iRequiredSet );

	if ( bPassed )
	{
		m_eChallengeResult = kCR_Pass;
	}
	else
	{
		m_eChallengeResult = kCR_Fail;
	}
}

void Challenge::Load(ILTMessage_Read *pMsg)
{
	LOAD_BYTE_CAST(m_eChallengeResult,eChallengeResult);
}

void Challenge::Save(ILTMessage_Write *pMsg)
{
	SAVE_BYTE(m_eChallengeResult);
}

