// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkAbstractOneAnim.cpp
//
// PURPOSE : AI NavMesh Link OneAnim abstract class implementation.
//
// CREATED : 11/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkAbstractOneAnim.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AIPathKnowledgeMgr.h"
#include "AIUtils.h"
#include "AnimationContext.h"


// WorldEdit
LINKFROM_MODULE( AINavMeshLinkAbstractOneAnim );

BEGIN_CLASS( AINavMeshLinkAbstractOneAnim )
END_CLASS_FLAGS( AINavMeshLinkAbstractOneAnim, AINavMeshLinkAbstract, CF_HIDDEN, "A base class for any NavMeshLink that results in AI activating AIGoalTraverseLink, and playing a single animation to traverse the link." )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkAbstractOneAnim )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkAbstractOneAnim, AINavMeshLinkAbstract )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstractOneAnim::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkAbstractOneAnim::AINavMeshLinkAbstractOneAnim()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstractOneAnim::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstractOneAnim::IsLinkRelevant( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// We have already traversed this link, and have not yet exited it.

	if( pAI->GetAIBlackBoard()->GetBBTraversingNMLink() == m_eNMLinkID )
	{
		return false;
	}

	// AI is not currently standing in this link.

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	// AI needs to traverse.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstractOneAnim::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstractOneAnim::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Determine which LinkEdge is closer to the dest poly.
	// LinkEdge determines the traversal direction.

	LTVector vDir = m_vLinkDirXZ;
	if( pAI->GetPosition().DistSqr( m_vMidPtLinkEdgeA ) >
		pAI->GetPosition().DistSqr( m_vMidPtLinkEdgeB ) )
	{
		vDir *= -1.f;
	}

	// Record the fact that we are traversing this link, 
	// to ensure we don't try to traverse it again while still in it.

	pAI->GetAIBlackBoard()->SetBBTraversingNMLink( m_eNMLinkID );

	// Immediately face the traversal direction.

	pAI->GetAIBlackBoard()->SetBBFaceDir( vDir );
	pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately( true );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstractOneAnim::IsTraversalInProgress
//              
//	PURPOSE:	Return true if traversal is still in progress.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstractOneAnim::IsTraversalInProgress( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	return pAI->GetAnimationContext()->IsLocked();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstractOneAnim::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstractOneAnim::IsTraversalComplete( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return true;
	}

	// NavMesh Poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return true;
	}

	// The traversal just started.

	if( pAI->GetAIBlackBoard()->GetBBStateChangeTime() == g_pLTServer->GetTime() )
	{
		return false;
	}

	// The traversal is complete when the animation completes.

	if( !pAI->GetAnimationContext()->IsLocked() )
	{
		return true;
	}

	// Traversal is not complete.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::ApplyTraversalEffect
//              
//	PURPOSE:	Apply changes when finishing traversing the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstractOneAnim::ApplyTraversalEffect( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear any cached pathfinding data, because the AI may have
	// landed somewhere disconnected from where he started.

	pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
}
