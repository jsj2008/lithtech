// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateSniper.h"
#include "AIHuman.h"
#include "AINode.h"
#include "AITarget.h"
#include "TrackedNodeContext.h"
#include "AICentralKnowledgeMgr.h"
#include "PlayerObj.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSniper, kState_HumanSniper);

// ----------------------------------------------------------------------- //

CAIHumanStateSniper::CAIHumanStateSniper()
{
	m_hTarget = LTNULL;
}

CAIHumanStateSniper::~CAIHumanStateSniper()
{
	// Post-Activate the node.

	if( m_pUseNode )
	{
		m_pUseNode->PostActivate();
	}

	// Decrement counters.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateSniper: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSniper::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hTarget );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSniper::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hTarget );
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSniper::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

CMusicMgr::Mood CAIHumanStateSniper::GetMusicMood()
{
	if( m_eStateStatus == kSStat_Attacking )
	{
		return CMusicMgr::eMoodAggressive;
	}

	return CMusicMgr::eMoodRoutine;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSniper::Update()
{
	// Intentionally skip super's Update().

	CAIHumanState::Update();

	// We are done if the node has been disabled.

	if( !m_pUseNode || m_pUseNode->IsDisabled() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Make sure we have a weapon

	if ( !GetAI()->GetCurrentWeapon() )
	{
		m_eStateStatus = kSStat_StateComplete;		
		return;
	}


	switch( m_eStateStatus )
	{
		//
		// Moving to sniper node.
		//

		case kSStat_Moving:
		{			
			if( m_pStrategyFollowPath->IsSet() )
			{
				// TODO: check for strategy failure
				m_pStrategyFollowPath->Update();
			}

			if ( m_pStrategyFollowPath->IsDone() )
			{
				// Just arrived at the node.

				if( m_pUseNode->ShouldFaceNodeForward() )
				{
					GetAI()->FaceDir( m_pUseNode->GetForward() );
				}

				// Pre-activate the node.

				m_pUseNode->PreActivate();

				// start smartobject animation.

				char const* pszSmartObjectCmd = g_pLTServer->GetStringData(m_hstrSmartObjectCmd);
				AIASSERT(( pszSmartObjectCmd && pszSmartObjectCmd[0] ), m_pAIHuman->m_hObject, "CAIHumanStateSniper::Update: no smartobjectcmd specified!" );
				if( pszSmartObjectCmd && pszSmartObjectCmd[0] )
					SendTriggerMsgToObject(GetAI(), GetAI()->m_hObject, LTFALSE, pszSmartObjectCmd );

				m_eStateStatus = kSStat_Attacking;
			}
		}
		break;

		//
		// Aiming and firing at a target.
		//

		case kSStat_Attacking:

			// Aim at target.

			if( GetAI()->HasTarget() )
			{
				HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
	
				// Target has changed.

				if( hTarget != m_hTarget )
				{
					m_hTarget = hTarget;
				
					// Head and Torso tracking.

					GetAI()->SetNodeTrackingTarget( kTrack_AimAt, hTarget, "Head" );

					// Keep track of how many AI are attacking the same target.

					AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ), m_pAIHuman->m_hObject, "CAIHumanStateSniper::Update: Already registered attacking count!" );
					g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
				}

				GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );

				// Update shooting

				if( m_pStrategyShoot->IsReloading() )
				{
					m_pStrategyShoot->Update();
					return;
				}

				if( m_pStrategyShoot->ShouldReload() )
				{
					m_pStrategyShoot->Reload();
					return;
				}

				if ( m_pStrategyShoot->IsFiring() )
				{
					// Be sure to update the strategy if we are in the middle of
					// firing as this means that we recieved a firing key this frame
					// which has not yet been processed
					m_pStrategyShoot->Update();
				}
				else if( GetAI()->GetSenseRecorder()->HasAnyStimulation(kSense_SeeEnemy) ||
						 GetAI()->GetSenseRecorder()->HasAnyStimulation(kSense_SeeEnemyLean) )
				{
					// Invalidate hiding spots if AI is firing at player.
		
					if( IsPlayer( hTarget ) )
					{
						CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hTarget );
						if( pPlayer )
						{
							pPlayer->SetVisibleToEnemyAI( GetAI(), true ); 
						}
					}

					// TODO: check for strategy failure
					m_pStrategyShoot->Update();
				}
			}

			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSniper::UpdateAnimation()
{
	// Intentionally skip super's UpdateAnimation().

	CAIHumanState::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch( m_eStateStatus )
	{
		case kSStat_Moving:
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		case kSStat_Attacking:
			GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
			m_pStrategyShoot->UpdateAnimation();
			break;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSniper::SetNode(AINodeUseObject* pUseNode)
{
	AINodeUseObject* pOldNode = m_pUseNode;

	if( !super::SetNode( pUseNode ) )
	{
		return LTFALSE;
	}

	// Post-Activate the old node.

	if( pOldNode && ( m_pUseNode != pOldNode ) )
	{
		pOldNode->PostActivate();
	}

	// Always Run.

	m_pStrategyFollowPath->SetMovement( kAP_Run );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSniper::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	m_eStateStatus = kSStat_StateComplete;
}
