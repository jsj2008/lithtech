// ----------------------------------------------------------------------- //
//
// MODULE  : TrackedNodeContext.cpp
//
// PURPOSE : TrackedNodeContext implementation
//           Manages the current node tracking settings per model instance
//           on the client or server.
//
// CREATED : 3/27/02
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TrackedNodeContext.h"
#include "ModelButeMgr.h"
#include "UberAssert.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::CTrackedNodeContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTrackedNodeContext::CTrackedNodeContext()
{
	m_pTrackedNodeMgr = LTNULL;
	m_hModel = LTNULL;
	m_eActiveTrackingGroup = kTrack_None;

	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		m_aTrackedNodeGroups[iGroup] = LTNULL;
		m_cTrackedNodesPerGroup[iGroup] = 0;
		m_eTrackingNodes[iGroup] = eModelTrackingNodeGroupInvalid;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::~CTrackedNodeContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTrackedNodeContext::~CTrackedNodeContext()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

void CTrackedNodeContext::Init(HOBJECT hModel, ModelSkeleton eSkeleton, CTrackedNodeMgr* pTrackedNodeMgr)
{
	UBER_ASSERT( hModel, "CTrackedNodeContext::Init: No model specified." );
	UBER_ASSERT( eSkeleton != eModelSkeletonInvalid, "CTrackedNodeContext::Init: Invalid skeleton specified." );
	UBER_ASSERT( pTrackedNodeMgr, "CTrackedNodeContext::Init: TrackedNodeMgr is NULL." );

	// Model instance.

	m_hModel = hModel;

	// Client or ServerTrackedNodeMgr.

	m_pTrackedNodeMgr = pTrackedNodeMgr;

	// Initialize each tracked node group.
	// It is OK for a skeleton not to specify tracking nodes,
	// so eTrackingNodes may be eModelTrackingNodeGroupInvalid (-1).
	
	m_eTrackingNodes[kTrack_LookAt] = g_pModelButeMgr->GetSkeletonTrackingNodesLookAt( eSkeleton );
	_Init( kTrack_LookAt, false );

	m_eTrackingNodes[kTrack_AimAt] = g_pModelButeMgr->GetSkeletonTrackingNodesAimAt( eSkeleton );
	_Init( kTrack_AimAt, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::_Init
//
//	PURPOSE:	Protected initialization, called from Init per group.
//
// ----------------------------------------------------------------------- //

void CTrackedNodeContext::_Init(EnumTrackedNodeGroup eGroup, bool bTrackOnAnim)
{
	UBER_ASSERT( eGroup != kTrack_None, "CTrackedNodeContext::_Init: No tracking group specified." );

	// Tracking node groups are optional per skeleton in ModelButes.txt.
	// A skeleton may not specify all (or any) tracking nodes.

	if( m_eTrackingNodes[eGroup] == eModelTrackingNodeGroupInvalid )
	{
		return;
	}


	HTRACKEDNODE hTrackedNode, hTrackedNodeClone;
	const char* szName;
	LTVector vForward, vUp;
	LTFLOAT fDiscomfortX, fDiscomfortY, fMaxX, fMaxY, fMaxVel;
	uint32 iNode, iHNode;

	// Get the number of nodes.  
	// Cloned nodes are specified within the node being cloned.

	uint32 cNodes = g_pModelButeMgr->GetNumTrackingNodes( m_eTrackingNodes[eGroup] );
	uint32 cClonedNodes = g_pModelButeMgr->GetNumClonedTrackingNodes( m_eTrackingNodes[eGroup] );

	UBER_ASSERT( cNodes + cClonedNodes > 0, "CTrackedNodeContext::_Init: No tracking nodes in group." );

	m_cTrackedNodesPerGroup[eGroup] = cNodes + cClonedNodes;
	m_aTrackedNodeGroups[eGroup] = debug_newa( HTRACKEDNODE, cNodes + cClonedNodes );

	// Keep separate index counters for nodes and node handles.
	// This is necessary because cloned nodes are specified within nodes being cloned,
	// so there are more iHNodes than iNodes.

	iHNode = 0;
	for( iNode=0; iNode < cNodes; ++iNode )
	{
		// Create a tracking node.

		szName = g_pModelButeMgr->GetTrackingNodeName( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		hTrackedNode = m_pTrackedNodeMgr->CreateTrackingNode( m_hModel, szName );

		// Get tracked node constraints from modelbutes.txt.

		fDiscomfortX = g_pModelButeMgr->GetTrackingNodeDiscomfortAngleX( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		fDiscomfortY = g_pModelButeMgr->GetTrackingNodeDiscomfortAngleY( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		fMaxX = g_pModelButeMgr->GetTrackingNodeMaxAngleX( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		fMaxY = g_pModelButeMgr->GetTrackingNodeMaxAngleY( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		fMaxVel = g_pModelButeMgr->GetTrackingNodeMaxVelocity( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );

		// Set tracked node constraints.
		// Specifying the forward and up vectors are optional.

		if( g_pModelButeMgr->GetTrackingNodeAxesSpecified( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode ) )
		{
			g_pModelButeMgr->GetTrackingNodeForward( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode, &vForward );
			g_pModelButeMgr->GetTrackingNodeUp( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode, &vUp );

			m_pTrackedNodeMgr->SetNodeConstraints( hTrackedNode, vForward, vUp, DEG2RAD(fDiscomfortX), DEG2RAD(fDiscomfortY), DEG2RAD(fMaxX), DEG2RAD(fMaxY), DEG2RAD(fMaxVel) );
		}
		else {
			m_pTrackedNodeMgr->SetNodeConstraints( hTrackedNode, DEG2RAD(fDiscomfortX), DEG2RAD(fDiscomfortY), DEG2RAD(fMaxX), DEG2RAD(fMaxY), DEG2RAD(fMaxVel) );
		}

		// Each node in modelbutes.txt is a child of the subsequent node.

		if( iNode > 0 )
		{
			m_pTrackedNodeMgr->LinkNodes( m_aTrackedNodeGroups[eGroup][iHNode-1], hTrackedNode );
		}

		// It is optional for a node to have a cloned node (e.g. left eye clones right eye orientation).

		szName = g_pModelButeMgr->GetTrackingNodeClonedName( m_eTrackingNodes[eGroup], (ModelTrackingNode)iNode );
		if( szName[0] )
		{
			hTrackedNodeClone = m_pTrackedNodeMgr->CreateTrackingNode( m_hModel, szName );
			m_pTrackedNodeMgr->LinkNodeOrientation( hTrackedNodeClone, hTrackedNode );
			m_aTrackedNodeGroups[eGroup][iHNode++] = hTrackedNodeClone; 
		}

		// Record the new node handle.

		m_aTrackedNodeGroups[eGroup][iHNode++] = hTrackedNode; 
	}

	// Track with rotations relative to animation rotations.

	SetOrientOnAnim( eGroup, bTrackOnAnim );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CTrackedNodeContext::Term()
{
	HTRACKEDNODE hTrackedNode;
	
	for( uint32 iGroup=0; iGroup < kTrack_Count; ++iGroup )
	{
		for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[iGroup]; ++iNode )
		{
			hTrackedNode = m_aTrackedNodeGroups[iGroup][iNode];
			if( hTrackedNode != INVALID_TRACKEDNODE )
			{
				m_pTrackedNodeMgr->DestroyTrackingNode( hTrackedNode );
			}
		}

		debug_deletea( m_aTrackedNodeGroups[iGroup] );
		m_aTrackedNodeGroups[iGroup] = LTNULL;
		m_cTrackedNodesPerGroup[iGroup] = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::SetOrientOnAnim
//
//	PURPOSE:	Tracking is relative to animations rotation.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::SetOrientOnAnim(EnumTrackedNodeGroup eGroup, bool bTrackOnAnim)
{
	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
////m_pTrackedNodeMgr->SetIgnoreParentAnimation( m_aTrackedNodeGroups[eGroup][iNode], bTrackOnAnim );
		if( !m_pTrackedNodeMgr->SetOrientOnAnim( m_aTrackedNodeGroups[eGroup][iNode], bTrackOnAnim ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::SetActiveTrackingGroup
//
//	PURPOSE:	Turn on a tracking group, and turn off previous group.
//
// ----------------------------------------------------------------------- //

void CTrackedNodeContext::SetActiveTrackingGroup(EnumTrackedNodeGroup eGroup)
{
	// Only one group of tracked nodes may be active at a time.

	// Disable any previously active group of nodes.

	EnableTrackingGroup( m_eActiveTrackingGroup, false );

	// Enable any currently active group of nodes.

	m_eActiveTrackingGroup = eGroup;
	EnableTrackingGroup( m_eActiveTrackingGroup, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::IsValidTrackingGroup
//
//	PURPOSE:	Query if a tracking group is valid.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::IsValidTrackingGroup(EnumTrackedNodeGroup eGroup)
{
	// Group is not valid if set to eModelTrackingNodeGroupInvalid (-1)
	// in a skeleton in modelbutes.txt

	if( ( eGroup == kTrack_None ) ||
		( m_eTrackingNodes[eGroup] == eModelTrackingNodeGroupInvalid ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::EnableTrackingGroup
//
//	PURPOSE:	Turn on/off a tracking group.
//
// ----------------------------------------------------------------------- //

void CTrackedNodeContext::EnableTrackingGroup(EnumTrackedNodeGroup eGroup, bool bEnable)
{
	if( eGroup == kTrack_None )
	{
		return;
	}

	// Enable or disable each tracked node in the specified group.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		// This is code we would like to use to smoothly turn off node tracking.
		// It does not seem to do anything.

		if( ( !bEnable ) && ( eGroup == kTrack_LookAt ) )
		{
			LTVector vRight, vUp, vForward;
			m_pTrackedNodeMgr->GetOrientationSpace(m_aTrackedNodeGroups[eGroup][iNode], vRight, vUp, vForward);

			m_pTrackedNodeMgr->SetAutoDisable( m_aTrackedNodeGroups[eGroup][iNode], true );
			m_pTrackedNodeMgr->TargetAnimation( m_aTrackedNodeGroups[eGroup][iNode], vForward * 100.0f );
			continue;
		}

		m_pTrackedNodeMgr->EnableTracking( m_aTrackedNodeGroups[eGroup][iNode], bEnable );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::SetTarget
//
//	PURPOSE:	Set a target for a tracking group.
//              Overloads for targeting models, objects, or positions.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset)
{
	if( eGroup == kTrack_None )
	{
		return false;
	}

	if( hModel == m_hModel )
	{
		UBER_ASSERT( 0, "CTrackedNodeContext::SetTrackedTarget object told to track self." );
		return false;
	}

	// Set target for each tracked node in the specified group.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->SetTarget( m_aTrackedNodeGroups[eGroup][iNode], hModel, pszNodeName, vOffset ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset)
{
	if( eGroup == kTrack_None )
	{
		return false;
	}

	if( hModel == m_hModel )
	{
		UBER_ASSERT( 0, "CTrackedNodeContext::SetTrackedTarget object told to track self." );
		return false;
	}

	// Set target for each tracked node in the specified group.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->SetTarget( m_aTrackedNodeGroups[eGroup][iNode], hModel, hNode, vOffset ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, HOBJECT hObject, const LTVector& vOffset)
{
	if( eGroup == kTrack_None )
	{
		return false;
	}

	if( hObject == m_hModel )
	{
		UBER_ASSERT( 0, "CTrackedNodeContext::SetTrackedTarget object told to track self." );
		return false;
	}

	// Set target for each tracked node in the specified group.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->SetTarget( m_aTrackedNodeGroups[eGroup][iNode], hObject, vOffset ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::SetTrackedTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPosition)
{
	if( eGroup == kTrack_None )
	{
		return false;
	}

	// Set target for each tracked node in the specified group.
	// Always assume world position.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->SetTargetWorld( m_aTrackedNodeGroups[eGroup][iNode], vPosition) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::GetRootBasis
//
//	PURPOSE:	Get basis space for root node of constraints hierarchy.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::GetRootBasis(EnumTrackedNodeGroup eGroup, 
										LTVector& vRight, 
										LTVector& vUp, 
										LTVector& vForward,
										LTVector& vPos)
{
	if( eGroup == kTrack_None )
	{
		return false;
	}

	// Last constraint node listed is the root.

	uint32 iRootNode = m_cTrackedNodesPerGroup[eGroup] - 1;
	if( iRootNode < 0 )
	{
		return false;
	}

	return m_pTrackedNodeMgr->GetBasisSpace( m_aTrackedNodeGroups[eGroup][iRootNode],
											vRight, 
											vUp, 
											vForward,
											vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::IsAtDiscomfort
//
//	PURPOSE:	Determine if a group of nodes has hit their discomfort angle.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::IsAtDiscomfort(EnumTrackedNodeGroup eGroup)
{
	// If there are no nodes, the limit is always hit.

	if( ( eGroup == kTrack_None ) || 
		( m_eTrackingNodes[eGroup] == eModelTrackingNodeGroupInvalid ) )
	{
		return true;
	}

	

	// If any node is not at its limit, the group has not hit its limit.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->IsAtDiscomfort( m_aTrackedNodeGroups[eGroup][iNode] ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::IsAtLimit
//
//	PURPOSE:	Determine if a group of nodes has hit their limit.
//
// ----------------------------------------------------------------------- //

bool CTrackedNodeContext::IsAtLimit(EnumTrackedNodeGroup eGroup)
{
	// If there are no nodes, the limit is always hit.

	if( ( eGroup == kTrack_None ) ||
		( m_eTrackingNodes[eGroup] == eModelTrackingNodeGroupInvalid ) )
	{
		return true;
	}

	// If any node is not at its limit, the group has not hit its limit.

	for( uint32 iNode=0; iNode < m_cTrackedNodesPerGroup[eGroup]; ++iNode )
	{
		if( !m_pTrackedNodeMgr->IsAtLimit( m_aTrackedNodeGroups[eGroup][iNode] ) )
		{
			return false;
		}
	}

	return true;
}
