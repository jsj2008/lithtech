// ----------------------------------------------------------------------- //
//
// MODULE  : NodeTrackerContext.cpp
//
// PURPOSE : NodeTrackerContext implementation
//           Manages the current node tracking settings per model instance
//           on the client or server.
//
// CREATED : 11/14/03
//
// (c) 2003-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "NodeTrackerContext.h"
#include "ModelsDB.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::CNodeTrackerContext
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNodeTrackerContext::CNodeTrackerContext()
{
	m_hModel = NULL;
	m_bitsActiveTrackingFlags.reset();

	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		m_NodeTrackerInstances[iGroup].m_hModelTrackerGroup = NULL;
		m_NodeTrackerInstances[iGroup].m_pNodeTracker = NULL;
		m_NodeTrackerInstances[iGroup].m_eTrackerState = kTrackerState_Idle;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::~CNodeTrackerContext
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CNodeTrackerContext::~CNodeTrackerContext()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::Init( HOBJECT hModel, ModelsDB::HSKELETON hSkeleton )
{
	// Start clean.
	Term( );

	LTASSERT( hModel, "CNodeTrackerContext::Init: No model specified." );
	LTASSERT( hSkeleton, "CNodeTrackerContext::Init: Invalid skeleton specified." );

	// Model instance.

	m_hModel = hModel;

	// Initialize each tracked node group.
	// It is OK for a skeleton not to specify tracking nodes,
	// so eTrackingNodes may be eModelTrackingNodeGroupInvalid (-1).
	
	m_NodeTrackerInstances[kTrackerGroup_LookAt].m_hModelTrackerGroup = g_pModelsDB->GetSkeletonTrackerNodesLookAt( hSkeleton );
	_Init( kTrackerGroup_LookAt );

	m_NodeTrackerInstances[kTrackerGroup_AimAt].m_hModelTrackerGroup = g_pModelsDB->GetSkeletonTrackerNodesAimAt( hSkeleton );
	_Init( kTrackerGroup_AimAt );

	m_NodeTrackerInstances[kTrackerGroup_Arm].m_hModelTrackerGroup = g_pModelsDB->GetSkeletonTrackerNodesArm( hSkeleton );
	_Init( kTrackerGroup_Arm );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::_Init
//
//	PURPOSE:	Protected initialization, called from Init per group.
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::_Init( EnumNodeTrackerGroup eGroup )
{
	LTASSERT( eGroup != kTrackerGroup_None, "CNodeTrackerContext::_Init: No tracking group specified." );

	// Tracking node groups are optional per skeleton in ModelButes.txt.
	// A skeleton may not specify all (or any) tracking nodes.

	ModelsDB::HTRACKERNODEGROUP hModelTrackerGroup = m_NodeTrackerInstances[eGroup].m_hModelTrackerGroup;
	if( !hModelTrackerGroup )
	{
		return;
	}

	// Get the number of nodes.  

	uint32 cNodes = g_pModelsDB->GetNumTrackerControlledNodes( hModelTrackerGroup );
	if( cNodes == 0 )
	{
		LTASSERT( 0, "CNodeTrackerContext::_Init: No tracker nodes in group." );
		return;
	}

	// Allocate a node tracker.

	CNodeTracker* pNodeTracker = debug_new( CNodeTracker );
	m_NodeTrackerInstances[eGroup].m_pNodeTracker = pNodeTracker;
	pNodeTracker->SetObject( m_hModel );
	pNodeTracker->SetSystemBlendWeight( 0.f );


	//
	// Setup the aimer node.
	//

	// Aimer node.

	HMODELNODE hAimerNode;
	const char* pszAimerNodeName = g_pModelsDB->GetTrackerAimerNodeName( hModelTrackerGroup );
	g_pModelLT->GetNode( m_hModel, pszAimerNodeName, hAimerNode);
	pNodeTracker->SetAimerNode( hAimerNode );

	// Aimer limits (converted to radians).

	LTRect2f rLimits = g_pModelsDB->GetTrackerAimerNodeLimits( hModelTrackerGroup );
	rLimits.m_vMin.x = MATH_DEGREES_TO_RADIANS( rLimits.m_vMin.x );
	rLimits.m_vMax.x = MATH_DEGREES_TO_RADIANS( rLimits.m_vMax.x );
	rLimits.m_vMin.y = MATH_DEGREES_TO_RADIANS( rLimits.m_vMin.y );
	rLimits.m_vMax.y = MATH_DEGREES_TO_RADIANS( rLimits.m_vMax.y );
	pNodeTracker->SetAimerLimits( rLimits );

	// Max speed (converted to radians).

	float fMaxSpeed;
	fMaxSpeed = g_pModelsDB->GetTrackerAimerNodeMaxSpeed( hModelTrackerGroup );
	fMaxSpeed = MATH_DEGREES_TO_RADIANS( fMaxSpeed );
	pNodeTracker->SetMaxSpeed( fMaxSpeed );


	//
	// Setup the controlled nodes.
	//

	HMODELNODE hControlledNode;
	float fControlledNodeWeight;
	pNodeTracker->SetNumControlledNodes( cNodes );
	for( uint32 iNode=0; iNode < cNodes; ++iNode )
	{
		// Controlled node.

		const char* pszControlledNodeName = g_pModelsDB->GetTrackerControlledNodeName( hModelTrackerGroup, iNode );
		g_pModelLT->GetNode( m_hModel, pszControlledNodeName, hControlledNode);
		pNodeTracker->SetControlledNode( iNode, hControlledNode );

		// Controlled node weight.

		fControlledNodeWeight = g_pModelsDB->GetTrackerControlledNodeWeight( hModelTrackerGroup, iNode );
		pNodeTracker->SetControlledNodeWeight( iNode, fControlledNodeWeight );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::Term()
{	
	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		if( m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			debug_delete( m_NodeTrackerInstances[iGroup].m_pNodeTracker );
			m_NodeTrackerInstances[iGroup].m_pNodeTracker = NULL;
			m_NodeTrackerInstances[iGroup].m_hModelTrackerGroup = NULL;
		}
	}

	m_hModel = NULL;
	m_bitsActiveTrackingFlags.reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::SetActiveTrackerGroups
//
//	PURPOSE:	Turn on/off tracker groups.
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetActiveTrackerGroups( uint32 dwActiveTrackerGroups )
{
	m_bitsActiveTrackingFlags = NODE_TRACKER_FLAGS( dwActiveTrackerGroups );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::EnableTrackerGroup
//
//	PURPOSE:	Turn on a specified tracker groups...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::EnableTrackerGroup( EnumNodeTrackerGroup eGroup )
{
	m_bitsActiveTrackingFlags.set( eGroup );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNodeTrackerContext::DisableTrackerGroup
//
//	PURPOSE:	Turn off a specified tracker groups...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::DisableTrackerGroup( EnumNodeTrackerGroup eGroup,	bool bImmediate )
{
	m_bitsActiveTrackingFlags.reset( eGroup );

	if (bImmediate)
	{
		m_NodeTrackerInstances[eGroup].m_eTrackerState = kTrackerState_Idle;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::IsTrackerGroupActive
//
//	PURPOSE:	Return true if specified group is active.
//
// ----------------------------------------------------------------------- //

bool CNodeTrackerContext::IsTrackerGroupActive( EnumNodeTrackerGroup eGroup ) const
{
	return m_bitsActiveTrackingFlags.test( eGroup );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::SetTarget
//
//	PURPOSE:	Set a target for a tracking group.
//              Overloads for targeting models, objects, or positions.
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, const char* pszNodeName, const LTVector& vOffset )
{
	if( eGroup == kTrackerGroup_None )
	{
		return;
	}

	if( hModel == m_hModel )
	{
		LTASSERT( 0, "CNodeTrackerNodeContext::SetTrackedTarget object told to track self." );
		return;
	}

	// Aim at a node on a model with some offset.

	HMODELNODE hModelNode;
	g_pModelLT->GetNode( hModel, pszNodeName, hModelNode);

	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetTargetNode( hModel, hModelNode, vOffset );
}

// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset )
{
	if( eGroup == kTrackerGroup_None )
	{
		return;
	}

	if( hModel == m_hModel )
	{
		LTASSERT( 0, "CNodeTrackerContext::SetTrackedTarget object told to track self." );
		return;
	}

	// Aim at a node on a model with some offset.
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetTargetNode( hModel, hNode, vOffset );
}

// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, HOBJECT hObject, const LTVector& vOffset )
{
	if( eGroup == kTrackerGroup_None )
	{
		return; 
	}

	if( hObject == m_hModel )
	{
		LTASSERT( 0, "CNodeTrackerContext::SetTrackedTarget object told to track self." );
		return;
	}

	// Aim at an object with some offset.
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetTargetObject( hObject, vOffset );
}

// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTVector& vPosition )
{
	if( eGroup == kTrackerGroup_None )
	{
		return;
	}

	// Aim at a world position.
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetTargetWorld( vPosition );
}

// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetTrackedTarget( EnumNodeTrackerGroup eGroup, const LTPolarCoord& polarExtents )
{
	if( eGroup == kTrackerGroup_None )
	{
		return;
	}

	// Set the extents of the node tracker so it can determine the position based on those...
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetTargetAimerExtents( polarExtents );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::UpdateNodeTrackers
//
//	PURPOSE:	Enable or disable node trackers based on flags.
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::UpdateNodeTrackers( float fElapsedTimeS )
{
	// Update each node tracker.

	for( uint32 iGroup=0; iGroup < kTrackerGroup_Count; ++iGroup )
	{
		// Skip unset trackers.

		if( !m_NodeTrackerInstances[iGroup].m_pNodeTracker )
		{
			continue;
		}

		// Tracker is enabled.

		if( m_bitsActiveTrackingFlags.test( iGroup ) )
		{
			m_NodeTrackerInstances[iGroup].m_pNodeTracker->SetSystemBlendWeight( 1.f );
			m_NodeTrackerInstances[iGroup].m_pNodeTracker->Update( fElapsedTimeS );
			m_NodeTrackerInstances[iGroup].m_eTrackerState = kTrackerState_Updating;
		}

		// Tracker is disabled.

		else 
		{
			float fBlendWeight = m_bitsActiveTrackingFlags.none() ? 1.f : 0.f;

			// Begin shut-down.

			if( m_NodeTrackerInstances[iGroup].m_eTrackerState == kTrackerState_Updating )
			{
				m_NodeTrackerInstances[iGroup].m_pNodeTracker->Update( fElapsedTimeS );
				m_NodeTrackerInstances[iGroup].m_pNodeTracker->ClearTarget();
				m_NodeTrackerInstances[iGroup].m_eTrackerState = kTrackerState_ShuttingDown;
			}
			// Smooth shut-down.

			else if( m_NodeTrackerInstances[iGroup].m_eTrackerState == kTrackerState_ShuttingDown )
			{
				m_NodeTrackerInstances[iGroup].m_pNodeTracker->Update( fElapsedTimeS );

				if( DidAimAtTarget( (EnumNodeTrackerGroup)iGroup ) )
				{
					m_NodeTrackerInstances[iGroup].m_eTrackerState = kTrackerState_Idle;
				}
			}

			// Done shutting down.

			else 
			{
				fBlendWeight = 0.f;
			}

			m_NodeTrackerInstances[iGroup].m_pNodeTracker->SetSystemBlendWeight( fBlendWeight );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackedNodeContext::DidAimAtTarget
//
//	PURPOSE:	Return true if specified target was aimed at its target last update.
//
// ----------------------------------------------------------------------- //

bool CNodeTrackerContext::DidAimAtTarget( EnumNodeTrackerGroup eGroup )
{
	// Sanity check.

	if( ( eGroup == kTrackerGroup_Count ) || 
		( !m_NodeTrackerInstances[eGroup].m_pNodeTracker ) ||
		( m_NodeTrackerInstances[eGroup].m_hModelTrackerGroup == NULL ) )
	{
		return false;
	}

	// Node tracker returns result.

	return m_NodeTrackerInstances[eGroup].m_pNodeTracker->DidAimAtTarget();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::GetCurrentExtents
//
//	PURPOSE:	Return true if the extents have been set.
//				Entents are in the range of [-1..1] with respect 
//				to the aiming node extents.
//
// ----------------------------------------------------------------------- //

bool CNodeTrackerContext::GetCurrentExtents( EnumNodeTrackerGroup eGroup, LTPolarCoord& polarExtents )
{
	// Sanity check.

	if( ( eGroup == kTrackerGroup_Count ) || 
		( !m_NodeTrackerInstances[eGroup].m_pNodeTracker ) ||
		( m_NodeTrackerInstances[eGroup].m_hModelTrackerGroup == NULL ) )
	{
		return false;
	}

	// Set the extents.

	polarExtents = m_NodeTrackerInstances[eGroup].m_pNodeTracker->GetCurrentExtents();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::SetMaxSpeed
//
//	PURPOSE:	Sets the max speed for the node tracker associated with the given group...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetMaxSpeed( EnumNodeTrackerGroup eGroup, float fMaxSpeed )
{
	if( eGroup == kTrackerGroup_None )
	{
		return;
	}

	// Set the max speed of the node tracker so it can interpolate the position...
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetMaxSpeed( fMaxSpeed );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::SetDefaultMaxSpeed
//
//	PURPOSE:	Revert back to the default max speed for the node tracker associated with the given group...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetDefaultMaxSpeed( EnumNodeTrackerGroup eGroup )
{
	if( eGroup == kTrackerGroup_None )
		return;

	ModelsDB::HTRACKERNODEGROUP hModelTrackerGroup = m_NodeTrackerInstances[eGroup].m_hModelTrackerGroup;
	CNodeTracker *pNodeTracker = m_NodeTrackerInstances[eGroup].m_pNodeTracker;
	if( !hModelTrackerGroup || !pNodeTracker )
		return;

	pNodeTracker->SetMaxSpeed( g_pModelsDB->GetTrackerAimerNodeMaxSpeed( hModelTrackerGroup ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::SetLimits
//
//	PURPOSE:	Specify the limits for the node tracker associated with the given group...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetLimits( EnumNodeTrackerGroup eGroup, const LTRect2f &rLimits )
{
	if( eGroup == kTrackerGroup_None )
		return;

	// Set the extents of the node...
	if( m_NodeTrackerInstances[eGroup].m_pNodeTracker )
		m_NodeTrackerInstances[eGroup].m_pNodeTracker->SetAimerLimits( rLimits );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::GetLimits
//
//	PURPOSE:	Retrieve the limits for the node tracker associated with the given group...
//
// ----------------------------------------------------------------------- //

bool CNodeTrackerContext::GetLimits( EnumNodeTrackerGroup eGroup, LTRect2f &rLimits )
{
	if( (eGroup != kTrackerGroup_None) && m_NodeTrackerInstances[eGroup].m_pNodeTracker )
	{
		rLimits = m_NodeTrackerInstances[eGroup].m_pNodeTracker->GetAimerLimits( );
		return true;
	}

	rLimits.Init( );
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeTrackerContext::SetDefaultLimits
//
//	PURPOSE:	Revert back to the default limits for the node tracker associated with the given group...
//
// ----------------------------------------------------------------------- //

void CNodeTrackerContext::SetDefaultLimits( EnumNodeTrackerGroup eGroup )
{
	if( eGroup == kTrackerGroup_None )
		return;

	ModelsDB::HTRACKERNODEGROUP hModelTrackerGroup = m_NodeTrackerInstances[eGroup].m_hModelTrackerGroup;
	CNodeTracker *pNodeTracker = m_NodeTrackerInstances[eGroup].m_pNodeTracker;
	if( !hModelTrackerGroup || !pNodeTracker )
		return;
	
	// Aimer limits (converted to radians).

	LTRect2f rLimits = g_pModelsDB->GetTrackerAimerNodeLimits( hModelTrackerGroup );
	rLimits.m_vMin.x = MATH_DEGREES_TO_RADIANS( rLimits.m_vMin.x );
	rLimits.m_vMax.x = MATH_DEGREES_TO_RADIANS( rLimits.m_vMax.x );
	rLimits.m_vMin.y = MATH_DEGREES_TO_RADIANS( rLimits.m_vMin.y );
	rLimits.m_vMax.y = MATH_DEGREES_TO_RADIANS( rLimits.m_vMax.y );
	pNodeTracker->SetAimerLimits( rLimits );
}

// EOF
