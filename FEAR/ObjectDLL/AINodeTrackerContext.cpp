// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeTrackerContext.cpp
//
// PURPOSE : AINodeTrackerContext implementation
//           Manages the current node tracking settings per model instance
//           on the server.
//
// CREATED : 11/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeTrackerContext.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "iperformancemonitor.h"

// Performance monitoring.
CTimedSystem g_tsAINodeTrackers("AINodeTrackers", "AI");

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeTrackerContext::CAINodeTrackerContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAINodeTrackerContext::CAINodeTrackerContext()
{
	m_eTrackerTarget = CNodeTracker::eTarget_Aimer;
	m_hTarget = NULL;
	m_hTargetNode = INVALID_MODEL_NODE;
	m_vTarget.Init(0.0f, 0.0f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeTrackerContext::UpdateNodeTrackers
//
//	PURPOSE:	Update node trackers.
//
// ----------------------------------------------------------------------- //

void CAINodeTrackerContext::UpdateNodeTrackers( CAI* pAI )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAINodeTrackers);

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Target or active tracker groups have changed.

	CAIBlackBoard* pBB = pAI->GetAIBlackBoard();
	if( ( m_bitsActiveTrackingFlags.to_ulong() != pBB->GetBBTargetTrackerFlags() ) ||
		( m_eTrackerTarget != pBB->GetBBTargetTrackerType() ) ||
		( m_hTarget != pBB->GetBBTargetTrackerModel() ) ||
		( m_hTargetNode != pBB->GetBBTargetTrackerModelNode() ) ||
		( m_vTarget != pBB->GetBBTargetTrackerPos() ) )
	{
		m_eTrackerTarget = pBB->GetBBTargetTrackerType();
		m_hTarget = pBB->GetBBTargetTrackerModel();
		m_hTargetNode = pBB->GetBBTargetTrackerModelNode();
		m_vTarget = pBB->GetBBTargetTrackerPos();

		// Set the target.

		switch( m_eTrackerTarget )
		{
			case CNodeTracker::eTarget_Node:
				SetTrackedTarget( m_hTarget, m_hTargetNode, LTVector(0.f, 0.f, 0.f) );
				break;

			case CNodeTracker::eTarget_Object:
				SetTrackedTarget( m_hTarget, LTVector(0.f, 0.f, 0.f) );
				break;

			case CNodeTracker::eTarget_World:
				SetTrackedTarget( m_vTarget );
				break;
		}

		// Set the currently active tracker groups.

		SetActiveTrackerGroups( pBB->GetBBTargetTrackerFlags() );
	}

	// Update the node trackers.

	super::UpdateNodeTrackers();

	// Check the horizontal limit.

	bool bAtLimit = false;
	if( IsTrackerGroupActive( kTrackerGroup_AimAt ) )
	{
		LTPolarCoord polarExtents;
		GetCurrentExtents( kTrackerGroup_AimAt, polarExtents );
		if( ( polarExtents.x <= -1.f ) ||
			( polarExtents.x >=  1.f ) )
		{
			bAtLimit = true;
		}
	}

	// Record the limit on the blackboard.

	pBB->SetBBTargetTrackerAtLimitX( bAtLimit );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeTrackerContext::Save / Load
//
//	PURPOSE:	Save / Load.
//
// ----------------------------------------------------------------------- //

void CAINodeTrackerContext::Save( ILTMessage_Write *pMsg )
{
	super::Save( pMsg );

	SAVE_DWORD( m_eTrackerTarget );
	SAVE_HOBJECT( m_hTarget );
	SAVE_DWORD( m_hTargetNode );
	SAVE_VECTOR( m_vTarget );
}

// ----------------------------------------------------------------------- //

void CAINodeTrackerContext::Load( ILTMessage_Read *pMsg )
{
	super::Load( pMsg );

	LOAD_DWORD_CAST( m_eTrackerTarget, CNodeTracker::ETargetType );
	LOAD_HOBJECT( m_hTarget );
	LOAD_DWORD( m_hTargetNode );
	LOAD_VECTOR( m_vTarget );
}


