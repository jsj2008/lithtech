// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkJump.cpp
//
// PURPOSE : AI NavMesh Link Jump class implementation.
//
// CREATED : 07/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

// Includes required for AINavMeshLinkJump.h

#include "AINavMeshLinkAbstract.h"
#include "AINavMeshLinkJump.h"

// Includes required for AINavMeshLinkJump.cpp

#include "AI.h"
#include "AINavMesh.h"
#include "AnimationContext.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkJump );

BEGIN_CLASS( AINavMeshLinkJump )
	ADD_STRINGPROP_FLAG(SmartObject,	"JumpDown",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkJump, AINavMeshLinkAbstract, 0, AINavMeshLinkJumpPlugin, "This link is used to specify that the brush contains something the AI must jump down from" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkJump )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkJump, AINavMeshLinkAbstract )


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkJump::AINavMeshLinkJump()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::IsLinkPassable
//              
//	PURPOSE:	Return true if link is currently passable.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkJump::IsLinkPassable( CAI* pAI, ENUM_NMPolyID ePolyTo )
{
	if( !super::IsLinkPassable( pAI, ePolyTo ) )
	{
		return false;
	}

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Link's NavMesh poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return false;
	}

	// Edge does not exist between linked poly and the 
	// poly specified to pathfind from.

	CAINavMeshEdge* pEdge = pPoly->GetNMPolyNeighborEdge( ePolyTo );
	if( !pEdge )
	{
		return false;
	}

	// Trying to path from the bottom of the jump to the top 
	// is not allowed.

	if( pEdge->GetNMEdgeMidPt().y > pPoly->GetNMPolyAABB()->vMin.y )
	{
		return false;
	}

	// Jump is valid.

	return true; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkJump::IsLinkRelevant( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI is not currently standing in this link.

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	// AI needs to jump.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJump::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Face the direction of the jump.

	pAI->GetAIBlackBoard()->SetBBFaceDir( m_vLinkDirXZ );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkJump::IsTraversalComplete( CAI* pAI )
{
	// NavMesh Poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return true;
	}

	// AI is finished jumping if his feet have dipped as low 
	// as the bottom of the jump.

	if( pAI->GetPosition().y - pAI->GetDims().y <= m_fFloorBottom + m_fExitOffsetDistB )
	{
		return true;
	}

	// Waiting to finish jumping.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJump::ApplyTraversalEffect
//              
//	PURPOSE:	Apply changes when finishing traversing the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJump::ApplyTraversalEffect( CAI* pAI )
{
	// Unlock the JumpDown animation so that the goal can switch, 
	// and the transition out can play imediately.

	pAI->GetAnimationContext()->Unlock();
}






