// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "ServerUtilities.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "VolumeBrushTypes.h"
#include "SurfaceFunctions.h"
#include "ctype.h"
#include "SFXMsgIds.h"
#include "AIUtils.h"
#include "AIPathKnowledgeMgr.h"
#include "AIRegion.h"
#include "AINavMesh.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AI.h"
#include "CharacterMgr.h"
#include "CharacterAlignment.h"
#include "PlayerObj.h"

// Globals/statics

CAINodeMgr* g_pAINodeMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::CAINodeMgr
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINodeMgr::CAINodeMgr()
{
	g_pAINodeMgr = this;
	m_bInitialized = false;
	m_fDrawingNodes = 0.f;
	m_itDebugNodeUpdate = m_aAINodeLists[(uint32)m_fDrawingNodes].begin();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Term
//
//	PURPOSE:	Terminates the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Term()
{
	if( m_bInitialized )
	{
		for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
		{
			m_aAINodeLists[iNodeType].resize( 0 );
		}

		m_bInitialized = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Init
//
//	PURPOSE:	Create a list of all the Nodes in the level.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Init()
{
	if( m_bInitialized )
	{
		return;
	}

	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		pNodeList = &( m_aAINodeLists[iNodeType] );
		for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
		{
			pNode = *itNode;
			pNode->InitNode();
		}
	}

	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		pNodeList = &( m_aAINodeLists[iNodeType] );
		for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
		{
			pNode = *itNode;
			pNode->AllNodesInitialized();
		}
	}
	m_itDebugNodeUpdate = m_aAINodeLists[(uint32)m_fDrawingNodes].begin();


	m_bInitialized = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::AddNode
//
//	PURPOSE:	Add a node to the list of all the Nodes in the level.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::AddNode(EnumAINodeType eNodeType, AINode* pNode)
{
	// NOTE: Nodes may be added to the NodeMgr multiple times for different
	//       node types.  For example, SmartObject nodes may be added multiple
	//       times for each defined SmartObject node type.

	if( eNodeType == kNode_InvalidType )
	{
		AIASSERT( 0, NULL, "CAINodeMgr::AddNode: Attempted to insert node with null type into map" );
		return;
	}

	m_aAINodeLists[eNodeType].push_back( pNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Verify
//
//	PURPOSE:	Verifies all our nodes
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Verify()
{
	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		pNodeList = &( m_aAINodeLists[iNodeType] );
		for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
		{
			pNode = *itNode;
			pNode->Verify();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Load
//
//	PURPOSE:	Restores the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Load(ILTMessage_Read *pMsg)
{
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		m_aAINodeLists[iNodeType].resize( 0 );
	}

	LOAD_BOOL( m_bInitialized );

	uint32 cNodes;
	LOAD_DWORD( cNodes );

	EnumAINodeType eNodeType;
	HOBJECT hNode;
	AINode* pNode;

	for( uint32 iNode=0; iNode < cNodes; ++iNode )
	{
		LOAD_DWORD_CAST( eNodeType, EnumAINodeType );
		LOAD_HOBJECT( hNode );

		pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		AddNode( eNodeType, pNode );
	}

	// Load clusters.

	uint32 cClusters;
	LOAD_INT( cClusters );
	m_lstNodeClusters.resize( cClusters );

	CAINodeCluster* pCluster;
	AINODE_CLUSTER_LIST::iterator itCluster;
	for( itCluster = m_lstNodeClusters.begin(); itCluster != m_lstNodeClusters.end(); ++itCluster )
	{
		pCluster = &( *itCluster );
		pCluster->Load( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::Save
//
//	PURPOSE:	Saves the state of the AINodeMgr
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL( m_bInitialized );

	uint32 cNodes = 0;
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		cNodes += m_aAINodeLists[iNodeType].size();
	}
	SAVE_DWORD( cNodes );

	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		pNodeList = &( m_aAINodeLists[iNodeType] );
		for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
		{
			pNode = *itNode;

			SAVE_DWORD( iNodeType );
			SAVE_HOBJECT( pNode->m_hObject );
		}
	}

	// Save clusters.

	SAVE_INT( m_lstNodeClusters.size() );

	CAINodeCluster* pCluster;
	AINODE_CLUSTER_LIST::iterator itCluster;
	for( itCluster = m_lstNodeClusters.begin(); itCluster != m_lstNodeClusters.end(); ++itCluster )
	{
		pCluster = &( *itCluster );
		pCluster->Save( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestNodeInRadius
//
//	PURPOSE:	Finds the nearest node to vPos within some radius.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestNodeInRadius(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, float fRadiusSqr, bool bMustBeUnowned)
{
    float fMinDistanceSqr = (float)INT_MAX;
    AINode* pClosestNode = NULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = NULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;

	pNodeList = &( m_aAINodeLists[eNodeType] );
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		pNode = *itNode;

		// Skip nodes in unreachable NavMesh polys.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingNMPoly() ) == CAIPathMgrNavMesh::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in NavMesh.

		if( pNode->GetNodeContainingNMPoly() == kNMPoly_Invalid )
		{
			continue;
		}

		// Skip node if required character type does not match.

		if( !( pNode->GetCharTypeMask() & pAI->GetCharTypeMask() ) )
		{
			continue;
		}

		if( bMustBeUnowned && pNode->GetNodeOwner() )
		{
			continue;
		}

		if (!pNode->IsLockedDisabledOrTimedOut( pAI->GetHOBJECT() ))
		{
			// The AI must be within the node's radius plus the radius passed into this function.
	        float  fDistanceSqr = vPos.DistSqr(pNode->GetPos());
			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < (pNode->GetRadiusSqr() + fRadiusSqr)) )
			{
				fMinDistanceSqr	= fDistanceSqr;
				pClosestNode	= pNode;
			}
		}
	}

	// Ensure that AI can pathfind to the destination node.
	// Ideally, we would like to do this check for each node as we iterate,
	// but that could result in multiple runs of BuildVolumePath() which
	// is expensive.  So instead we just check the final returned node.
	// The calling code can call this function again later, and will not get
	// this node again.

	if( pAI && pClosestNode )
	{
		ENUM_NMPolyID ePoly = pClosestNode->GetNodeContainingNMPoly();
		if( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), ePoly ) )
		{
			return NULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNodeInComponent
//
//	PURPOSE:	Finds a node within some component.
//
// ----------------------------------------------------------------------- //

AINode*	CAINodeMgr::FindNodeInComponent( CAI* pAI, EnumAINodeType eNodeType, ENUM_NMComponentID eComponent, double fLastActivationCutOff, bool bScripted )
{
	// Sanity check.

	if( ( !pAI ) ||
		( eNodeType == kNode_InvalidType ) ||
		( eComponent == kNMComponent_Invalid ) )
	{
		return NULL;
	}

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = NULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINode* pSelectedNode = NULL;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;
	CAINavMeshPoly* pPoly;

	pNodeList = &( m_aAINodeLists[eNodeType] );
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		pNode = *itNode;

		// Skip nodes that have been activated too recently.

		if( pNode->GetNodeLastActivationTime() > fLastActivationCutOff )
		{
			continue;
		}

		// Skip nodes that are dynamic-only when we are scripted.

		if( bScripted && pNode->IsDynamicOnly() )
		{
			continue;
		}

		// Skip nodes in unreachable NavMesh polys.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingNMPoly() ) == CAIPathMgrNavMesh::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in NavMesh.

		if( pNode->GetNodeContainingNMPoly() == kNMPoly_Invalid )
		{
			continue;
		}

		// Skip nodes in the wrong component.

		pPoly = g_pAINavMesh->GetNMPoly( pNode->GetNodeContainingNMPoly() );
		if( !( pPoly && pPoly->GetNMComponentID() == eComponent ) )
		{
			continue;
		}

		// Skip node if required character type does not match.

		if( !( pNode->GetCharTypeMask() & pAI->GetCharTypeMask() ) )
		{
			continue;
		}

		// Skip locked or disabled nodes.

		if( pNode->IsLockedDisabledOrTimedOut( pAI->GetHOBJECT() ) )
		{
			continue;
		}

		// Select the node.

		pSelectedNode = pNode;
		break;
	}

	/**
	// Ensure that AI can pathfind to the destination node.
	// Ideally, we would like to do this check for each node as we iterate,
	// but that could result in multiple runs of BuildVolumePath() which
	// is expensive.  So instead we just check the final returned node.
	// The calling code can call this function again later, and will not get
	// this node again.

	if( pAI && pSelectedNode )
	{
		ENUM_NMPolyID ePoly = pSelectedNode->GetNodeContainingNMPoly();
		if( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), ePoly ) )
		{
			return NULL;
		}
	}
	**/

	return pSelectedNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindOwnedNode
//
//	PURPOSE:	Finds node owned by someone.
//
// ----------------------------------------------------------------------- //

AINode*	CAINodeMgr::FindOwnedNode(EnumAINodeType eNodeType, HOBJECT hOwner )
{
	// It is NOT OK for hOwner to be NULL.  Only return nodes that are owned by someone.

	if( !hOwner )
	{
		return NULL;
	}

	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;

	pNodeList = &( m_aAINodeLists[eNodeType] );
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		pNode = *itNode;
		if( pNode->GetNodeOwner() == hOwner )
		{
			return pNode;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindPotentiallyValidNodes
//
//	PURPOSE:	Finds the nodes that are potentially valid.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::FindPotentiallyValidNodes( EnumAINodeType eNodeType, AIVALID_NODE_LIST& lstValidNodes, const SAIVALIDATE_NODE& ValidateNodeStructParam )
{
	// Sanity check.

	if( !ValidateNodeStructParam.pAI )
	{
		return;
	}

	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;

	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = ValidateNodeStructParam.pAI;
	ValidateNodeStruct.vPos = ValidateNodeStructParam.vPos;
	ValidateNodeStruct.fSearchMult = ValidateNodeStructParam.fSearchMult;
	ValidateNodeStruct.dwPotentialFlags = ValidateNodeStructParam.dwPotentialFlags;
	ValidateNodeStruct.hChar = ValidateNodeStructParam.hChar;

	// Constrain node search to guarded area.

	if( ValidateNodeStruct.dwPotentialFlags & kNodePotential_NodeInGuardedArea )
	{
		ValidateNodeStruct.pNodeGuard = NULL;

		// AI is guarding a Guard node.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(kNode_Guard);
		CAIWMFact* pFact = ValidateNodeStruct.pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && IsAINode( pFact->GetTargetObject() ) )
		{
			HOBJECT hNode = pFact->GetTargetObject();
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
			if( pNode->GetType() == kNode_Guard )
			{
				ValidateNodeStruct.pNodeGuard = (AINodeGuard*)pNode;
			}
		}
	}

	// Iterate over all nodes of the specified type and add valid nodes to the list.

	pNodeList = &( m_aAINodeLists[eNodeType] );
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		ValidateNodeStruct.pNode = *itNode;

		if( IsNodePotentiallyValid( &ValidateNodeStruct ) )
		{
			SAIVALID_NODE vnNode;
			vnNode.hNode = ValidateNodeStruct.pNode->m_hObject;
			vnNode.fDistSqr = ValidateNodeStruct.fDistanceSqr;

			lstValidNodes.push_back( vnNode );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindPotentiallyValidNodesInAIRegion
//
//	PURPOSE:	Finds the nodes that are potentially valid.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::FindPotentiallyValidViewNodesInAIRegion( CAI* pAI, ENUM_AIRegionID eAIRegion, const LTVector& vPos, AIVALID_NODE_LIST& lstValidNodes )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if invalid AIRegion.

	AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
	if( !pAIRegion )
	{
		return;
	}

	// Iterate over nodes in AIRegion, adding valid nodes to the list.

	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = pAI;
	ValidateNodeStruct.vPos = vPos;
	ValidateNodeStruct.fSearchMult = 0.f;
	ValidateNodeStruct.dwPotentialFlags = kNodePotential_HasPathToNode | 
										  kNodePotential_NodeInNavMesh |
										  kNodePotential_NodeInGuardedArea |
										  kNodePotential_NodeUnowned;


	// Constrain node search to guarded area.

	ValidateNodeStruct.pNodeGuard = NULL;

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && IsAINode( pFact->GetTargetObject() ) )
	{
		HOBJECT hNode = pFact->GetTargetObject();
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode->GetType() == kNode_Guard )
		{
			ValidateNodeStruct.pNodeGuard = (AINodeGuard*)pNode;
		}
	}


	// Iterate over all view nodes and add valid nodes to the list.

	HOBJECT hNode;
	int cViewNodes = pAIRegion->GetNumViewNodes();
	for( int iViewNode=0; iViewNode < cViewNodes; ++iViewNode )
	{
		hNode = pAIRegion->GetViewNode( iViewNode );
		if( !hNode )
		{
			continue;
		}

		ValidateNodeStruct.pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !ValidateNodeStruct.pNode )
		{
			continue;
		}

		if( IsNodePotentiallyValid( &ValidateNodeStruct ) )
		{
			SAIVALID_NODE vnNode;
			vnNode.hNode = ValidateNodeStruct.pNode->m_hObject;
			vnNode.fDistSqr = ValidateNodeStruct.fDistanceSqr;

			lstValidNodes.push_back( vnNode );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::IsNodePotentiallyValid
//
//	PURPOSE:	Return true if node is potentially valid.
//
// ----------------------------------------------------------------------- //

bool CAINodeMgr::IsNodePotentiallyValid( SAIVALIDATE_NODE* pValidateNodeStruct )
{
	// Sanity check.

	if( !(	pValidateNodeStruct && 
			pValidateNodeStruct->pAI &&
			pValidateNodeStruct->pNode ) )
	{
		return false;
	}

	// Get AIs Path Knowledge.

	CAI* pAI = pValidateNodeStruct->pAI;
	CAIPathKnowledgeMgr* pPathKnowledgeMgr = NULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode = pValidateNodeStruct->pNode;

	// Check the node being in a pathable destination.

	if ( pValidateNodeStruct->dwPotentialFlags & kNodePotential_HasPathToNode )
	{
		// Bail if node is in unreachable NavMesh poly.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingNMPoly() ) == CAIPathMgrNavMesh::kPath_NoPathFound ) )
		{
			return false;
		}
	}


	// Check the node being in a poly.

	if ( pValidateNodeStruct->dwPotentialFlags & kNodePotential_NodeInNavMesh )
	{
		// Bail if node is not in NavMesh.

		if( kNMPoly_Invalid == pNode->GetNodeContainingNMPoly() )
		{
			return false;
		}
	}

	// Bail if node is owned by someone.
	// Find owned nodes via FindGuardedNodes().

	if ( pValidateNodeStruct->dwPotentialFlags & kNodePotential_NodeUnowned )
	{
		if( pNode->GetNodeOwner() )
		{
			return false;
		}
	}

	// Bail if node's required character type does not match.

	if( !( pNode->GetCharTypeMask() & pAI->GetCharTypeMask() ) )
	{
		return false;
	}

	// Bail if node is locked by someone else, disabled, or timed out.

	if (pNode->IsNodeLocked() && pNode->GetLockingAI() != pValidateNodeStruct->pAI->GetHOBJECT())
	{
		return false;
	}

	if( pNode->IsNodeDisabled() || pNode->IsNodeTimedOut() )
	{
		return false;
	}

	pValidateNodeStruct->fDistanceSqr = pValidateNodeStruct->vPos.DistSqr( pNode->GetPos() );
	float fSearchMult = pValidateNodeStruct->fSearchMult;

	//
	// Constrain to guarded area.
	//

	if( pValidateNodeStruct->dwPotentialFlags & kNodePotential_NodeInGuardedArea )
	{
		if( pValidateNodeStruct->pNodeGuard && 
			( !pValidateNodeStruct->pNodeGuard->IsNodeInRadiusOrRegion( pNode ) ) )
		{
			return false;
		}
	}

	//
	// Check the radius or region.
	//

	if( pValidateNodeStruct->dwPotentialFlags & kNodePotential_RadiusOrRegion )
	{
		if( !pNode->IsAIInRadiusOrRegion( pAI, pValidateNodeStruct->vPos, fSearchMult ) )
		{
			return false;
		}
	}

	//
	// Check the boundary radius.
	//

	if( pValidateNodeStruct->dwPotentialFlags & kNodePotential_BoundaryRadius )
	{
		// Bail if no target.

		if( !pValidateNodeStruct->pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_Disturbance) )
		{
			return false;
		}

		// Is the target within the BoundaryAIRegion.

		ENUM_AIRegionID eAIRegion = pNode->GetBoundaryAIRegion();
		if( eAIRegion != kAIRegion_Invalid )
		{
			// Fail if AIRegion does not exist.

			AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
			if( !pAIRegion )
			{
				return false;
			}

			// Fail if target is not in the AIRegion.

			ENUM_NMPolyID ePoly = pValidateNodeStruct->pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();
			if( !pAIRegion->ContainsNMPoly( ePoly ) )
			{
				return false;
			}
		}

		// Check the radius.

		else 
		{
			LTVector vTargetPos = pValidateNodeStruct->pAI->GetAIBlackBoard()->GetBBTargetPosition();
			float fTargetDistSqr = vTargetPos.DistSqr( pNode->GetPos() );

			// Apply search mult.

			float fRadiusSqr;
			if( fSearchMult != 1.f )
			{
				fRadiusSqr = pNode->GetBoundaryRadiusSqr() * (fSearchMult*fSearchMult);
			}
			else {
				fRadiusSqr = pNode->GetBoundaryRadiusSqr();
			}

			// Bail if outside radius.

			if( fTargetDistSqr > fRadiusSqr )
			{
				return false;
			}
		}
	}

	//
	// Check if some character is in the radius or region.
	//

	if( pValidateNodeStruct->dwPotentialFlags & kNodePotential_CharacterInRadiusOrRegion )
	{
		if( !pNode->IsCharacterInRadiusOrRegion( pValidateNodeStruct->hChar ) )
		{
			return false;
		}
	}

	// Node is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindGuardedNodes
//
//	PURPOSE:	Finds the nodes that are currently guarded by the AI.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::FindGuardedNodes( CAI* pAI, EnumAINodeType eNodeType, AIVALID_NODE_LIST& lstValidNodes )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// AI is not guarding.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && pFact->GetTargetObject() ) )
	{
		return;
	}
	AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	if( !pGuardNode  )
	{
		return;
	}


	AINode* pNode;
	SAIVALID_NODE vnNode;
	HOBJECT_LIST::const_iterator itNode;
	const HOBJECT_LIST* pNodeList;
	HOBJECT hNode;

	// Iterate over all nodes of the specified type and add valid nodes to the list.

	pNodeList = pGuardNode->GetGuardedNodesList();
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		hNode = *itNode;
		if( !IsAINode( hNode ) )
		{
			continue;
		}

		pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !pNode )
		{
			continue;
		}

		// Found a guarded node. Add it to the list.

		float fDistanceSqr = pAI->GetPosition().DistSqr( pNode->GetPos() );

		vnNode.hNode = pNode->m_hObject;
		vnNode.fDistSqr = fDistanceSqr;

		lstValidNodes.push_back( vnNode );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNodesInCluster
//
//	PURPOSE:	Finds nodes in the specified cluster.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::FindNodesInCluster( EnumAINodeClusterID eNodeClusterID, EnumAINodeType eNodeType, AINODE_LIST* pClusteredNodeList )
{
	// Sanity check.

	if( ( eNodeClusterID == kNodeCluster_Invalid ) ||
		( eNodeType == kNode_InvalidType ) ||
		( !pClusteredNodeList ) )
	{
		return;
	}

	pClusteredNodeList->resize( 0 );

	// Collect nodes in cluster.

	AINode* pNode;
	AINODE_LIST::iterator itNode;
	AINODE_LIST* pNodeList = &( m_aAINodeLists[eNodeType] );
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		pNode = *itNode;
		if( pNode->GetAINodeClusterID() == eNodeClusterID )
		{
			pClusteredNodeList->push_back( pNode );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::GetNode(const char *szName)
{
    if ( !g_pLTServer ) return NULL;

	AINode* pNode;
	AINODE_LIST* pNodeList;
	AINODE_LIST::iterator itNode;
	for( int iNodeType=0; iNodeType < kNode_Count; ++iNodeType )
	{
		pNodeList = &( m_aAINodeLists[iNodeType] );
		for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
		{
			pNode = *itNode;
			const char* szNodeName = pNode->GetNodeName();

			if ( !LTStrICmp(szNodeName, szName) )
			{
				return pNode;
			}
		}
	}

    return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNodeList
//
//	PURPOSE:	Return node list if it exists.
//
// ----------------------------------------------------------------------- //

AINODE_LIST* CAINodeMgr::GetNodeList( EnumAINodeType eNodeType )
{
	if( eNodeType < kNode_Count )
	{
		return &( m_aAINodeLists[eNodeType] );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::CreateNodeCluster
//
//	PURPOSE:	Create a node cluster with the specified ID if it 
//              does not already exist.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::CreateNodeCluster( EnumAINodeClusterID eCluster )
{
	// Cluster already exists.

	if( GetNodeCluster( eCluster ) )
	{
		return;
	}

	// Create new cluster.

	CAINodeCluster Cluster;
	Cluster.SetAINodeClusterID( eCluster );
	m_lstNodeClusters.push_back( Cluster );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNodeCluster
//
//	PURPOSE:	Return the node cluster with the specified ID if it exists. 
//
// ----------------------------------------------------------------------- //

CAINodeCluster* CAINodeMgr::GetNodeCluster( EnumAINodeClusterID eCluster )
{
	CAINodeCluster* pCluster;
	AINODE_CLUSTER_LIST::iterator itCluster;
	for( itCluster = m_lstNodeClusters.begin(); itCluster != m_lstNodeClusters.end(); ++itCluster )
	{
		// Cluster exists.

		pCluster = &( *itCluster );
		if( pCluster->GetAINodeClusterID() == eCluster )
		{
			return pCluster;
		}
	}

	// Cluster does not exist.

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::UpdateDebugRendering()
//              
//	PURPOSE:	Draw or hide nodes.
//              
//----------------------------------------------------------------------------

void CAINodeMgr::UpdateDebugRendering(float fVarTrack)
{
	if( m_fDrawingNodes != fVarTrack )
	{
		EnumAINodeType eNodeType = (EnumAINodeType)( (uint32)m_fDrawingNodes );
		HideNodes( eNodeType );
		eNodeType = (EnumAINodeType)( (uint32)fVarTrack );
		DrawNodes( eNodeType );
		m_fDrawingNodes = fVarTrack;

		if(  m_aAINodeLists[eNodeType].size() == 0 )
		{
			m_itDebugNodeUpdate = m_aAINodeLists[eNodeType].end();
		}
		else {
			m_itDebugNodeUpdate = m_aAINodeLists[eNodeType].begin();
		}
	}

	// Update drawing of nodes based on node status.

	EnumAINodeType eNodeType = (EnumAINodeType)( (uint32)fVarTrack );
	if( m_itDebugNodeUpdate != m_aAINodeLists[eNodeType].end() )
	{
		if( ( eNodeType == kNode_Cover ) 
			|| ( eNodeType == kNode_Armored ) 
			|| ( eNodeType == kNode_Ambush ) 
			|| ( eNodeType == kNode_Safety ) 
			|| ( eNodeType == kNode_DarkChant )
			)
		{
			UpdateNodeStatusDebugRendering( eNodeType );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::UpdateNodeStatusDebugRendering()
//              
//	PURPOSE:	Update color coding of nodes based on status.
//              
//----------------------------------------------------------------------------

void CAINodeMgr::UpdateNodeStatusDebugRendering( EnumAINodeType eNodeType )
{
	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
	if( !pPlayer )
	{
		return;
	}

	HOBJECT hTarget = pPlayer->m_hObject;
	if( pPlayer->GetTurret() )
	{
		hTarget = pPlayer->GetTurret();
	}

	AINode* pNode = (AINode*)(*m_itDebugNodeUpdate);
	AINode* pNodeFirst = pNode;

	// Update up to 5 nodes per frame.

	for( int iNode=0; iNode < 5; ++iNode )
	{
		// Update the debug drawing status of the node.

		pNode->UpdateDebugDrawStatus( hTarget );

		// Iterate to the next node, wrapping if necessary.

		++m_itDebugNodeUpdate;
		if( m_itDebugNodeUpdate == m_aAINodeLists[eNodeType].end() )
		{
			m_itDebugNodeUpdate = m_aAINodeLists[eNodeType].begin();
		}

		// Get a pointer to the next node.
		// Bail if the the next node was the first we updated.
		// (In other words, bail if this node has already been 
		// updated this frame).

		pNode = (AINode*)(*m_itDebugNodeUpdate);
		if( pNode == pNodeFirst )
		{
			break;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::DrawNodes()
//              
//	PURPOSE:	Sets all nodes to draw and remembers that the nodes are 
//				being drawn.
//              
//----------------------------------------------------------------------------
void CAINodeMgr::DrawNodes(EnumAINodeType eNodeType)
{
	if( ( eNodeType <= kNode_InvalidType ) ||
		( eNodeType >= kNode_Count ) )
	{
		return;
	}

	AINode* pNode;
	AINODE_LIST::iterator it;
	for(it = m_aAINodeLists[eNodeType].begin(); it != m_aAINodeLists[eNodeType].end(); ++it)
	{
		pNode = *it;
		pNode->DrawSelf();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::HideNodes()
//              
//	PURPOSE:	Sets all nodes to Hide
//              
//----------------------------------------------------------------------------
void CAINodeMgr::HideNodes(EnumAINodeType eNodeType)
{
	if( ( eNodeType <= kNode_InvalidType ) ||
		( eNodeType >= kNode_Count ) )
	{
		return;
	}

	AINode* pNode;
	AINODE_LIST::iterator it;
	for(it = m_aAINodeLists[eNodeType].begin(); it != m_aAINodeLists[eNodeType].end(); ++it)
	{
		pNode = *it;
		pNode->HideSelf();
	}
}


