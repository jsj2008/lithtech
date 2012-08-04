// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "ServerUtilities.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "VolumeBrushTypes.h"
#include "SurfaceFunctions.h"
#include "ctype.h"
#include "SFXMsgIDs.h"
#include "NodeLine.h"
#include "AIUtils.h"
#include "AIVolume.h"
#include "AIPathKnowledgeMgr.h"
#include "AI.h"
#include "ObjectRelationMgr.h"

// Globals/statics

CAINodeMgr* g_pAINodeMgr = LTNULL;
AINODE_LIST CAINodeMgr::s_lstTempNodes;

// Externs

extern int g_cIntersectSegmentCalls;

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
	m_bInitialized = LTFALSE;
	m_fDrawingNodes = 0.f;
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
		m_mapAINodes.clear();
		m_bInitialized = LTFALSE;
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
	Term();

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.begin(); it != m_mapAINodes.end(); ++it)
	{
		pNode = it->second;
		pNode->Init();
	}

	m_bInitialized = LTTRUE;
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
	//       node types.  For example, UseObject nodes may be added multiple
	//       times for each defined UseObject node type.
	UBER_ASSERT( eNodeType != kNode_InvalidType,
		"Attempted to insert node with null type into map" );

	m_mapAINodes.insert( AINODE_MAP::value_type(eNodeType, pNode) );
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
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.begin(); it != m_mapAINodes.end(); ++it)
	{
		pNode = it->second;
		pNode->Verify();
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
	m_mapAINodes.clear( );

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
		m_mapAINodes.insert( AINODE_MAP::value_type(eNodeType, pNode) );
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

	SAVE_DWORD( m_mapAINodes.size() );

	EnumAINodeType eNodeType;
	AINode* pNode;

	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.begin(); it != m_mapAINodes.end(); ++it)
	{
		eNodeType = it->first;
		pNode = it->second;

		SAVE_DWORD( eNodeType );
		SAVE_HOBJECT( pNode->m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindTailNode
//
//	PURPOSE:	Determines the tail node that one should be at
//
// ----------------------------------------------------------------------- //

AINodeTail* CAINodeMgr::FindTailNode(CAI* pAI, const LTVector& vTargetPos, const LTVector& vPos)
{
    LTFLOAT fTailedDistance;
    LTFLOAT fTailerDistance;
    LTFLOAT fMinTailedDistance = (float)INT_MAX;
    LTFLOAT fMinTailerDistance = (float)INT_MAX;
	AINode* pNode;
	AINode* pTailedNode = LTNULL;
	AINode* pTailerNode = LTNULL;
	AINODE_MAP::iterator it;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	// Find the node closest to the tailed object and to the tailer.

	for(it = m_mapAINodes.lower_bound(kNode_Tail); it != m_mapAINodes.upper_bound(kNode_Tail); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

        fTailedDistance = VEC_DISTSQR(vTargetPos, pNode->GetPos());
        fTailerDistance = VEC_DISTSQR(vPos, pNode->GetPos());

		if ( fTailedDistance < fMinTailedDistance )
		{
			pTailedNode = pNode;
			fMinTailedDistance = fTailedDistance;
		}

		if ( fTailerDistance < fMinTailerDistance )
		{
			pTailerNode = pNode;
			fMinTailerDistance = fTailerDistance;
		}

	}

	// Figure out what the tail node is based on these two nodes

	uint32 iTailedNode = GetNodeIndexFromName( pTailedNode );
	uint32 iTailerNode = GetNodeIndexFromName( pTailerNode );
	uint32 iTailNode = -1;

	// If the tailer is less than the tailed node, the tail is the tailed node minus 1

	if ( iTailerNode < iTailedNode )
	{
		iTailNode = Max<uint32>(0, iTailedNode-1);
	}

	// If the tailer is greater than the tailed node, the tail is the tailed node plus 1

	if ( iTailerNode > iTailedNode )
	{
		iTailNode = Min<uint32>( m_mapAINodes.count(kNode_Tail)-1, iTailedNode+1);
	}

	// If the tail node is equal to the tailednode, then there is no good node to go to.

	if ( iTailerNode == iTailedNode )
	{
		return LTNULL;
	}
	else
	{
		AINodeTail* pNodeTail = (AINodeTail*)FindNodeByIndex(kNode_Tail, iTailNode);

		// Ensure that AI can pathfind to the destination node.
		// Ideally, we would like to do this check for each node as we iterate,
		// but that could result in multiple runs of BuildVolumePath() which
		// is expensive.  So instead we just check the final returned node.
		// The calling code can call this function again later, and will not get
		// this node again.

		if( pAI && pNodeTail )
		{
			AIVolume* pVolumeDest = pNodeTail->GetNodeContainingVolume();
			if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
			{
				return LTNULL;
			}
		}

		return pNodeTail;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNodeIndexFromName
//
//	PURPOSE:	Extract a node index from a name, e.g. AINodePatrol4
//
// ----------------------------------------------------------------------- //

uint32 CAINodeMgr::GetNodeIndexFromName(AINode* pNode)
{
	const char *pChar;
	pChar = g_pLTServer->GetStringData(pNode->GetName());
	pChar += strlen(pChar) - 1;

	if(!isdigit(*pChar))
	{
		return -1;
	}

	while( isdigit(*pChar) )
	{
		pChar--;
	}

	return atoi(pChar + 1);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNodeByIndex
//
//	PURPOSE:	Finds node by index in name, e.g. AINodePatrol4.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNodeByIndex(EnumAINodeType eNodeType, uint32 iNode)
{
	AINode* pNode;
	AINODE_MAP::iterator it;

	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if( iNode == GetNodeIndexFromName( pNode ) )
		{
			return pNode;
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestNode
//
//	PURPOSE:	Finds the nearest node to vPos
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTBOOL bRequiresPath, LTBOOL bRequiresCommand)
{
    LTFLOAT fMinDistanceSqr = (float)INT_MAX;
    AINode* pClosestNode = LTNULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if ( (!pNode->IsLockedDisabledOrTimedOut()) && ((!bRequiresCommand) || pNode->HasCmd()) )
		{
	        LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < pNode->GetRadiusSqr()) )
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

	if( pAI && pClosestNode && bRequiresPath )
	{
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestNodeInRadius
//
//	PURPOSE:	Finds the nearest node to vPos within some radius.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestNodeInRadius(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fRadiusSqr, LTBOOL bMustBeUnowned)
{
    LTFLOAT fMinDistanceSqr = (float)INT_MAX;
    AINode* pClosestNode = LTNULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if( bMustBeUnowned && pNode->GetNodeOwner() )
		{
			continue;
		}

		if (!pNode->IsLockedDisabledOrTimedOut())
		{
			// The AI must be within the node's radius plus the radius passed into this function.
	        LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());
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
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestNodeFromThreat
//
//	PURPOSE:	Finds the nearest node that has an OK status from the hThreat
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestNodeFromThreat(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hThreat, LTFLOAT fSearchFactor)
{
    LTFLOAT fMinDistance = (float)INT_MAX;
    AINode*	pClosestNode = LTNULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if ( !pNode->IsLockedDisabledOrTimedOut() )
		{
			// Check of there is a SearchFactor, scaling the radius of the node.

			LTFLOAT fNodeRadiusSqr;
			if( fSearchFactor != 1.f )
			{
				fNodeRadiusSqr = pNode->GetRadius() * fSearchFactor;
				fNodeRadiusSqr *= fNodeRadiusSqr;
			}
			else {
				fNodeRadiusSqr = pNode->GetRadiusSqr();
			}

			LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());

			if ( ( fDistanceSqr < fMinDistance ) && ( fDistanceSqr < fNodeRadiusSqr ) )
			{
				if ( kStatus_Ok == pNode->GetStatus(vPos, hThreat) )
				{
					fMinDistance = fDistanceSqr;
					pClosestNode = pNode;
				}
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
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindRandomNodeFromThreat
//
//	PURPOSE:	Finds a random node that has an OK status from the hThreat
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindRandomNodeFromThreat(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hThreat)
{
	s_lstTempNodes.clear();

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	// Generate list of valid nodes.

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if ( !pNode->IsLockedDisabledOrTimedOut() )
		{
			if ( kStatus_Ok == pNode->GetStatus(vPos, hThreat) )
			{
				s_lstTempNodes.push_back( pNode );
			}
		}
	}

	// Randomly select one of the valid nodes.

	if( !s_lstTempNodes.empty() )
	{
		pNode = s_lstTempNodes[ GetRandom( 0, s_lstTempNodes.size() - 1 ) ];
		s_lstTempNodes.clear();
	}
	else {
		pNode = LTNULL;
	}

	// Ensure that AI can pathfind to the destination node.
	// Ideally, we would like to do this check for each node as we iterate,
	// but that could result in multiple runs of BuildVolumePath() which
	// is expensive.  So instead we just check the final returned node.
	// The calling code can call this function again later, and will not get
	// this node again.

	if( pAI && pNode )
	{
		AIVolume* pVolumeDest = pNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pNode;
}


// ---------------------------------------------------------------------------
//
//	ROUTINE:	CAINodeMgr::FindNearestNodeInFrontWithFromThreat
//
//	PURPOSE:	Finds the nearest node that has an OK status from the hThreat,
//				and which is between the checker and the threat
//
// ---------------------------------------------------------------------------
AINode* CAINodeMgr::FindNearestNodeInSameDirectionAsThreat(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hThreat)
{
    LTFLOAT fMinDistance = (float)INT_MAX;
    AINode*	pClosestNode = LTNULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if ( pNode->IsLockedDisabledOrTimedOut() )
			continue;

		if ( AreNodeAndObjectInSameDirection(hThreat, pNode, vPos) )
			continue;

        LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());
		if ( (fDistanceSqr > fMinDistance) || (fDistanceSqr > pNode->GetRadiusSqr()) )
			continue;

		if ( kStatus_Ok == pNode->GetStatus(vPos, hThreat) )
		{
			fMinDistance = fDistanceSqr;
			pClosestNode = pNode;
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
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::AreNodeAndThreatInSameDirection()
//              
//	PURPOSE:	Returns true if the threat and the node are in the same
//				direction, false if they are not.
//              
//----------------------------------------------------------------------------
bool CAINodeMgr::AreNodeAndObjectInSameDirection(HOBJECT hObj,
												 const AINode* const pNode,
												 const LTVector& vTesterPos ) const
{
	LTVector vToThreat, vNormToThreat, vThreatPos;
	g_pLTServer->GetObjectPos( hObj, &vThreatPos );
	vToThreat = vThreatPos - vTesterPos;
	vNormToThreat = vToThreat.Normalize();

	LTVector vToNode, vNormToNode;
	vToNode = pNode->GetPos() - vTesterPos;
	vNormToNode = vToNode.Normalize();

	if ( vNormToNode.Dot( vNormToThreat ) > 0.0 )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestObjectNode
//
//	PURPOSE:	Finds the nearest object node to pos
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestObjectNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, const char* szClass)
{
    LTFLOAT fMinDistanceSqr = (float)INT_MAX;
    AINode* pClosestNode = LTNULL;

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if ( !pNode->IsLockedDisabledOrTimedOut() && pNode->HasObject() )
		{
            LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());

			if ( (fDistanceSqr < fMinDistanceSqr) && (fDistanceSqr < pNode->GetRadiusSqr()) )
			{
				HOBJECT hObject;
				if ( LT_OK == FindNamedObject(pNode->GetObject(), hObject) )
				{
                    HCLASS hClass = g_pLTServer->GetClass((char*)szClass);

                    if ( g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), hClass) )
					{
						fMinDistanceSqr = fDistanceSqr;
						pClosestNode = pNode;
					}
				}
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
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestObjectNode
//
//	PURPOSE:	Finds a UseObject node pointing to an object.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindUseObjectNode(EnumAINodeType eNodeType, HOBJECT hUseObject, LTBOOL bIgnoreActiveState)
{
	AINode* pNode;
	AINodeUseObject* pNodeUseObject;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		if( ( !bIgnoreActiveState ) && ( !pNode->NodeTypeIsActive( eNodeType ) ) )
		{
			continue;
		}

        if ( IsAINodeUseObject( pNode->m_hObject ) )
		{
			pNodeUseObject = (AINodeUseObject*)pNode;
			if( pNodeUseObject->GetHObject() == hUseObject )
			{
				return pNode;
			}
		}
	}

	return LTNULL;
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
		return LTNULL;
	}

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;
		if( pNode->GetNodeOwner() == hOwner )
		{
			return pNode;
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestOwnedNode
//
//	PURPOSE:	Finds the nearest node to vPos with a specified owner.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindNearestOwnedNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hOwner)
{
	// It is NOT OK for hOwner to be NULL.  Only return nodes that are owned by someone.

	if( !hOwner )
	{
		return LTNULL;
	}

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

    LTFLOAT fMinDistanceSqr = (float)INT_MAX;
    AINode* pClosestNode = LTNULL;

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}

		if( pNode->GetNodeOwner() != hOwner )
		{
			continue;
		}

		// Owned nodes are locked by the owner, so just check for
		// disabled and timed out.

		if ( !( pNode->IsDisabled() || pNode->IsTimedOut() ) )
		{
	        LTFLOAT  fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());
			if ( fDistanceSqr < fMinDistanceSqr )
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
		AIVolume* pVolumeDest = pClosestNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindRandomOwnedNode
//
//	PURPOSE:	Finds a random node with a specified owner.
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::FindRandomOwnedNode(CAI* pAI, EnumAINodeType eNodeType, HOBJECT hOwner)
{
	// It is NOT OK for hOwner to be NULL.  Only return nodes that are owned by someone.

	if( !hOwner )
	{
		return LTNULL;
	}

	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}


	s_lstTempNodes.clear();

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;

		// Skip nodes in unreachable volumes.

		if( pPathKnowledgeMgr && 
			( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
		{
			continue;
		}

		// Skip nodes that are not in volumes.

		if( !pNode->GetNodeContainingVolume() )
		{
			continue;
		}

		// Skip node if required alignment does not match.

		if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
			( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
		{
			continue;
		}

		// Do NOT check if node type is active.
		// This will prevent AI from walking up to a disturbed
		// file cabinet to do some work, so he will never noticed the cabinet.
		/**
		
		if( !pNode->NodeTypeIsActive( eNodeType ) )
		{
			continue;
		}
		**/

		if( pNode->GetNodeOwner() != hOwner )
		{
			continue;
		}

		if (!pNode->IsLockedDisabledOrTimedOut())
		{
			s_lstTempNodes.push_back( pNode );
		}
	}

	// Randomly select one of the valid nodes.

	if( !s_lstTempNodes.empty() )
	{
		pNode = s_lstTempNodes[ GetRandom( 0, s_lstTempNodes.size() - 1 ) ];
		s_lstTempNodes.clear();
	}
	else {
		pNode = LTNULL;
	}

	// Ensure that AI can pathfind to the destination node.
	// Ideally, we would like to do this check for each node as we iterate,
	// but that could result in multiple runs of BuildVolumePath() which
	// is expensive.  So instead we just check the final returned node.
	// The calling code can call this function again later, and will not get
	// this node again.

	if( pAI && pNode )
	{
		AIVolume* pVolumeDest = pNode->GetNodeContainingVolume();
		if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
		{
			return LTNULL;
		}
	}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::UnlockNode
//
//	PURPOSE:	Unlocks the node for public use
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::UnlockNode(HOBJECT hNode, HOBJECT hAI)
{
	AINode::HandleToObject(hNode)->Unlock( hAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::LockNode
//
//	PURPOSE:	Claims the node for exclusive use
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::LockNode(HOBJECT hNode, HOBJECT hAI)
{
	AINode::HandleToObject(hNode)->Lock( hAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::GetNode
//
//	PURPOSE:	Finds a node based on its name
//
// ----------------------------------------------------------------------- //

AINode* CAINodeMgr::GetNode(HSTRING hstrName)
{
    if ( !g_pLTServer ) return LTNULL;

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.begin(); it != m_mapAINodes.end(); ++it)
	{
		pNode = it->second;
        if ( g_pLTServer->CompareStringsUpper(pNode->GetName(), hstrName) )
		{
			return pNode;
		}
	}

    return LTNULL;
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
    if ( !g_pLTServer ) return LTNULL;

	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.begin(); it != m_mapAINodes.end(); ++it)
	{
		pNode = it->second;
        const char* szNodeName = g_pLTServer->GetStringData(pNode->GetName());

		if ( !_stricmp(szNodeName, szName) )
		{
			return pNode;
		}
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::EnumerateNodesInVolume
//
//	PURPOSE:	Get a list of nodes of a given type that are inside a volume.
//
// ----------------------------------------------------------------------- //

void CAINodeMgr::EnumerateNodesInVolume(EnumAINodeType eNodeType, AIVolume* pVolume, LTFLOAT fVertThreshold, AINode** apNodes, uint32* pcNodes, const uint32 nMaxSearchNodes)
{
	if(nMaxSearchNodes == (*pcNodes))
	{
		return;
	}

	AINode* pNode;
	AINODE_MAP::const_iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;
		if( pVolume->InsideMasked(pNode->GetPos(), eAxisAll, 53.0f) )
		{
			apNodes[(*pcNodes)++] = pNode;
			if(nMaxSearchNodes == *pcNodes)
			{
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::NodeTypeFromString
//
//	PURPOSE:	Convert a node type enum from a string.
//
// ----------------------------------------------------------------------- //

EnumAINodeType CAINodeMgr::NodeTypeFromString(char* szNodeType)
{
	for(uint32 iNodeType = 0; iNodeType < kNode_Count; ++iNodeType)
	{
		if( stricmp(szNodeType, s_aszAINodeTypes[iNodeType]) == 0 )
		{
			return (EnumAINodeType)(iNodeType);
		}
	}

	return kNode_InvalidType;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINodeMgr::UpdateDebugRendering()
//              
//	PURPOSE:	Draw or hide nodes.
//              
//----------------------------------------------------------------------------
void CAINodeMgr::UpdateDebugRendering(LTFLOAT fVarTrack)
{
	if( m_fDrawingNodes != fVarTrack )
	{
		EnumAINodeType eNodeType = (EnumAINodeType)( (uint32)m_fDrawingNodes );
		HideNodes( eNodeType );
		eNodeType = (EnumAINodeType)( (uint32)fVarTrack );
		DrawNodes( eNodeType );
		m_fDrawingNodes = fVarTrack;
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
	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;
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
	AINode* pNode;
	AINODE_MAP::iterator it;
	for(it = m_mapAINodes.lower_bound(eNodeType); it != m_mapAINodes.upper_bound(eNodeType); ++it)
	{
		pNode = it->second;
		pNode->HideSelf();
	}
}


