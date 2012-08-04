// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLostTarget.cpp
//
// PURPOSE : AISensorLostTarget class implementation
//
// CREATED : 08/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorLostTarget.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AISoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLostTarget, kSensor_LostTarget );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLostTarget::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorLostTarget::CAISensorLostTarget()
{
	m_bLostTarget = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLostTarget::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLostTarget::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bLostTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLostTarget::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLostTarget::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_bool( m_bLostTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLostTarget::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorLostTarget::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if AI is not targeting a character.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		LoseTarget( false );
		return false;
	}

	// Bail if AI can see his target.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetVisibilityConfidence() > 0.9f )
	{
		// Announce re-acquiring target.

		if( !( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Normal ) )
		{
			HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
			if( m_pAI->GetAIBlackBoard()->GetBBTargetPosition().DistSqr( m_pAI->GetPosition() ) > g_pAIDB->GetAIConstantsRecord()->fAlertImmediateThreatInstantSeeDistanceSqr )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_DisturbanceSeenAlarmingFar, kAISndCat_Event, hTarget, 0.f );
			}
			else {
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_DisturbanceSeenAlarming, kAISndCat_Event, hTarget, 0.f );
			}
		}

		// We are no longer lost.

		LoseTarget( false );

		// Track target normally after truly seeing him.

		uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
		if( dwFlags & kTargetTrack_Squad )
		{
			dwFlags = dwFlags & ~kTargetTrack_Squad;
			dwFlags |= kTargetTrack_Normal;
			m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );
		}

		return false;
	}

	// We were previously lost.
	// Stay lost until the target is seen.

	if( m_bLostTarget )
	{
		return false;
	}

	// Bail if AI and target are in the same NavMesh poly.

	ENUM_NMPolyID ePolyAI = m_pAI->GetCurrentNavMeshPoly();
	ENUM_NMPolyID ePolyTarget = m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly();
	if( ePolyAI == ePolyTarget )
	{
		return false;
	}

	// Bail if AI or target is outside of the NavMesh.

	CAINavMeshPoly* pPolyAI = g_pAINavMesh->GetNMPoly( ePolyAI );
	CAINavMeshPoly* pPolyTarget = g_pAINavMesh->GetNMPoly( ePolyTarget );
	if( !( pPolyAI && pPolyTarget ) )
	{
		return false;
	}

	// Bail if AI is standing in a LoseTarget link.
	// We don't want the AI to lose his target if the link is right next to the 
	// target's sensory component.

	if( pPolyAI->GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPolyAI->GetNMLinkID() );
		if( pLink && pLink->GetNMLinkType() == kLink_LoseTarget )
		{
			return false;
		}
	}

	// Bail if target is standing in a LoseTarget link.
	// We don't want the AI to lose his target if the link is right next to the 
	// target's sensory component.

	if( pPolyTarget->GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPolyTarget->GetNMLinkID() );
		if( pLink && pLink->GetNMLinkType() == kLink_LoseTarget )
		{
			return false;
		}
	}

	// Bail if AI and target are in the same NavMesh component.

	ENUM_NMComponentID eComponentAI = pPolyAI->GetNMComponentID();
	ENUM_NMComponentID eComponentTarget = pPolyTarget->GetNMComponentID();
	if( eComponentAI == eComponentTarget )
	{
		return false;
	}

	// Bail if AI or target's NavMesh component is invalid.

	CAINavMeshComponent* pComponentAI = g_pAINavMesh->GetNMComponent( eComponentAI );
	CAINavMeshComponent* pComponentTarget = g_pAINavMesh->GetNMComponent( eComponentTarget );
	if( !( pComponentAI && pComponentTarget ) )
	{
		return false;
	}

	// Bail if AI and target are in the same NavMesh sensory component.

	if( pComponentAI->GetNMSensoryComponentID() == pComponentTarget->GetNMSensoryComponentID() )
	{
		return false;
	}

	// AI has lost its target.

	LoseTarget( true );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLostTarget::LoseTarget
//
//	PURPOSE:	Set variables for losing a target, or not.
//
// ----------------------------------------------------------------------- //

void CAISensorLostTarget::LoseTarget( bool bLost )
{
	if( m_bLostTarget == bLost )
	{
		return;
	}

	m_bLostTarget = bLost;

	// Stop tracking the target position if the target is lost.

	uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
	if( bLost )
	{
		dwFlags = dwFlags & ~kTargetTrack_Normal;
		dwFlags = dwFlags & ~kTargetTrack_Squad;

		ENUM_NMPolyID ePoly = m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleNavMeshPoly();
		LTVector vPos = m_pAI->GetAIBlackBoard()->GetBBTargetLastVisiblePosition();
		m_pAI->GetAIBlackBoard()->SetBBTargetLostNavMeshPoly( ePoly );
		m_pAI->GetAIBlackBoard()->SetBBTargetLostPosition( vPos );
		m_pAI->GetAIBlackBoard()->SetBBTargetLostTime( g_pLTServer->GetTime() );

AITRACE( AIShowGoals, ( m_pAI->m_hObject, "+++ LOST TARGET: %d", ePoly ) );
	}
	else {
		dwFlags |= kTargetTrack_Normal;
	}

	m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );
}
