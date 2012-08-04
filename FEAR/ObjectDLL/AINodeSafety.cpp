// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSafety.cpp
//
// PURPOSE : 
//
// CREATED : 1/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeSafety.h"
#include "DEditColors.h"
#include "AINodeSafetyFirePosition.h"

LINKFROM_MODULE(AINodeSafety);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeSafety 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeSafety CF_HIDDEN

#endif

BEGIN_CLASS( AINodeSafety )

	ADD_DEDIT_COLOR( AINodeSafety )
	ADD_VECTORPROP_VAL_FLAG(Dims,		16.0f, 16.0f, 16.0f,	PF_HIDDEN | PF_DIMS, "This hidden field handles setting the dims for visualization")
	DAMAGED_PROPS()
	PLAYERONNODE_PROPS()
	BOUNDARYRADIUS_PROPS( 1000.0f )
	THREATRADIUS_PROPS( 500.0f )
	THREATFOV_PROPS()
	EXPIRATION_PROPS( 5.0f, 10.0f )
	ADD_STRINGPROP_FLAG(SmartObject,			"Safety", 		0|PF_STATICLIST|PF_DIMS, "SmartObject used to specify animations for this node")
	
	// This is intentionally hidden as our level designers do not want to have 
	// 2 ways of adding child fire position nodes.
	ADD_NAMED_OBJECT_LIST_AGGREGATE( FirePositionNodes, PF_HIDDEN | PF_GROUP(1), Node, "TODO:GROUPDESC", "TODO:PROPDESC" )

END_CLASS_FLAGS_PLUGIN(AINodeSafety, AINodeSmartObject, CF_HIDDEN_AINodeSafety, AINodeSafetyPlugin, "This node is used in conjunction with AINodeSafetyFirePosition.  This node provides the AI with a  location to run for cover, with the firing positions locations to depart to when the AI decides to attack.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSafety)
CMDMGR_END_REGISTER_CLASS(AINodeSafety, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSafety::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeSafety::AINodeSafety()
{
	AddAggregate( &m_FireNodes );
}

AINodeSafety::~AINodeSafety()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::Save/Load/ReadProp()
//              
//	PURPOSE:	Handle saving and restoring the AINodeSafety
//              
//----------------------------------------------------------------------------

void AINodeSafety::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_FireNodes.ReadProp( pProps, "Node" );

	m_DamagedValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );
	m_ThreatFOV.ReadProps( pProps );
	m_ExpirationValidator.ReadProps( pProps );

	if ( NULL == GetSmartObject() )
	{
		AIASSERT1( 0, m_hObject, "AINodeSafety: '%s' does not specify a SmartObject; it cannot be used by the AI.", GetNodeName() );
	}
}

void AINodeSafety::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_DamagedValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
	m_ThreatFOV.Load( pMsg );
	m_ExpirationValidator.Load( pMsg );
}

void AINodeSafety::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_DamagedValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
	m_ThreatFOV.Save( pMsg );
	m_ExpirationValidator.Save( pMsg );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::AllNodesInitialized()
//              
//	PURPOSE:	Handle initializing inter-node dependencies.
//              
//----------------------------------------------------------------------------

void AINodeSafety::AllNodesInitialized()
{
	super::AllNodesInitialized();
	
	m_FireNodes.InitNamedObjectList( m_hObject );

	// Remove any objects which are not of the expected type.
	// Otherwise, if there is a valid node, set this SafetyNode as its parent.
	// Print a warning if there are no valid nodes specified; this SafetyNode 
	// is not correctly configured by level design.

	for ( uint32 i = 0; i < m_FireNodes.GetNumObjectHandles(); ++i )
	{
		HOBJECT hObject = m_FireNodes.GetObjectHandle( i );
		if ( !IsKindOf( hObject, "AINodeSafetyFirePosition" ) )
		{
			g_pLTServer->CPrint( "AINodeSafety '%s' has a child node which is not of type AINodeSafetyFirePosition: '%s'", GetNodeName(), m_FireNodes.GetObjectName( i ) );
			m_FireNodes.ClearObjectHandle( i );
		}
		else
		{
			AINodeSafetyFirePosition* pNode = (AINodeSafetyFirePosition*)g_pLTServer->HandleToObject( hObject );
			pNode->SetParent( this );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//
//----------------------------------------------------------------------------

bool AINodeSafety::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if (!super::IsNodeValid(pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags))
	{
		return false;
	}
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Fail if the AI is outside of the nodes radius.  Normally, we allow AIs 
	// outside a nodes radius to use a node if they detected it previously by 
	// being inside the radius.  Level design has requested this be a hard 
	// radius for this node.
	//
	// POTENTIAL BUG: This may cause a failure if the AI has to run outside 
	// the radius to get to the node.  We currently have special cases for 
	// 'at node' and 'not at node' -- do we need 'moving to node' as well?
	// We could use locking to as a proxy to detect this -- if the AI has 
	// the node locked, he is already 'operating' on it.

	if ( pAI && 
		( GetLockingAI() != pAI->GetHOBJECT() 
		&& !IsAIInRadiusOrRegion( pAI, vPosAI, 1.0f ) ) )
	{
		return false;
	}

	if ( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	if ( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
	if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	const bool kbIsIgnoreDir = false;
	if ( !m_ThreatFOV.Evaluate( dwFilteredStatusFlags, vThreatPos, m_vPos, m_rRot.Forward(), kbIsIgnoreDir ) )
	{
		return false;
	}

	if ( !m_ExpirationValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject, kTask_InvalidType ) )
	{
		return false;
	}

	// Fail if the AI does not have a ranged weapon drawn, with ammo to use it.
	// Note that this does not check holster; this prevents a plan like 
	// 'draw -> goto safety', but this is acceptable as other actions should 
	// trigger drawing. If this is a problem, we can check holster and add a 
	// new action to use this node which requires drawing.

	if ( pAI && 
		( !AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) 
		|| ( !AIWeaponUtils::HasAmmo( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) ) ) )
	{
		return false;
	}

	// Node is valid.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::HandleAIArrival()
//              
//	PURPOSE:	Handles the AI arriving at the node; in addition to base 
//				class functionality, this class sets a new expiration time for 
//				the node to time out.
//              
//----------------------------------------------------------------------------

void AINodeSafety::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	m_ExpirationValidator.SetNewExpirationTime();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::IsLockedDisabledOrTimedOut()
//              
//	PURPOSE:	Returns true if this node is availible to the passed in AI, 
//				false if it is not.  This node is availible if the node is 
//				not Disable/TimedOut and if the AI is either not locked or 
//				locked by the querying AI.
//              
//----------------------------------------------------------------------------

bool AINodeSafety::IsLockedDisabledOrTimedOut( HOBJECT hQueryingAI )
{
	if ( IsNodeDisabled() || IsNodeTimedOut() )
	{
		return true;
	}

	if ( IsNodeLocked() && GetLockingAI() != hQueryingAI )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetType()
//              
//	PURPOSE:	Returned the 'type' of this node.
//              
//----------------------------------------------------------------------------

EnumAINodeType AINodeSafety::GetType() const
{
	return kNode_Safety;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::DrawSelf()
//              
//	PURPOSE:	Override to add drawing the whole safety node system, 
//				including firing positions.
//              
//----------------------------------------------------------------------------

int AINodeSafety::DrawSelf()
{
	super::DrawSelf();

	// Draw owned nodes and draw lines to them.

	// Use a separate line system from the base class, as the base class 
	// places a the nodes name at the nodes location.  On the client,
	// an AABB containing all of the lines is constructed, and the label
	// placed above it.  By adding this line to the base classes location,
	// the label may be placed in an unintended location.
	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowSafetyNodes");

	for ( uint32 i = 0; i < m_FireNodes.GetNumObjectHandles(); ++i )
	{
		HOBJECT hNode = m_FireNodes.GetObjectHandle( i );
		if ( hNode )
		{
			AIASSERT( AINodeSafetyFirePosition::DynamicCast(hNode), m_hObject, "AINodeSafety::DrawSelf: Unexpected node type." );
			AINodeSafetyFirePosition* pNode = (AINodeSafetyFirePosition*)g_pLTServer->HandleToObject( hNode );
			pNode->DrawSelf();

			system.AddArrow( m_vPos, pNode->GetPos() );
		}
	}
	
	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePatrol::HideSelf()
//              
//	PURPOSE:	Override to handle hiding the whole safety node system, 
//				including firing positions.
//              
//----------------------------------------------------------------------------

int AINodeSafety::HideSelf()
{
	super::HideSelf();

	// Hide all of the child nodes

	for ( uint32 i = 0; i < m_FireNodes.GetNumObjectHandles(); ++i )
	{
		HOBJECT hNode = m_FireNodes.GetObjectHandle( i );
		if ( hNode )
		{
			AIASSERT( AINodeSafetyFirePosition::DynamicCast(hNode), m_hObject, "AINodeSafety::DrawSelf: Unexpected node type." );
			AINodeSafetyFirePosition* pNode = (AINodeSafetyFirePosition*)g_pLTServer->HandleToObject( hNode );
			pNode->HideSelf();
		}
	}

	// Clear the ownership arrows.

	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowSafetyNodes");
	system.Clear();

	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetBoundaryRadiusSqr()
//              
//	PURPOSE:	Returns the boundary radius for this node.  This is used by 
//				the AINodeMgr validator.
//              
//----------------------------------------------------------------------------

float AINodeSafety::GetBoundaryRadiusSqr() const
{
	return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetFirePositionCount()
//              
//	PURPOSE:	Returns the number of AINodeSafetyFirePosition instances in 
//				owned by this node.
//              
//----------------------------------------------------------------------------

int AINodeSafety::GetFirePositionCount() const
{
	return m_FireNodes.GetNumObjectHandles();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetFirePositionCount()
//              
//	PURPOSE:	Returns the time the AIs stay at this node expires.  If no AI
//				is passed in, or if the AI is not at this node, 0.0f is 
//				returned.
//              
//----------------------------------------------------------------------------

double AINodeSafety::GetExpirationTime( CAI* pAI ) const
{
	if ( !pAI )
	{
		return 0.0f;
	}

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp || pProp->hWSValue != m_hObject )
	{
		return 0.0f;
	}

	return m_ExpirationValidator.GetExpirationTime();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetFirePosition()
//              
//	PURPOSE:	Returns the handle of the AINodeSafetyFirePosition instance 
//				at this offset.
//              
//----------------------------------------------------------------------------

AINodeSafetyFirePosition* AINodeSafety::GetFirePosition( int i ) const
{
	HOBJECT hObj = m_FireNodes.GetObjectHandle( i );
	if ( !hObj )
	{
		return NULL;
	}

	return (AINodeSafetyFirePosition*)g_pLTServer->HandleToObject( hObj );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::GetDebugColor()
//              
//	PURPOSE:	Returns the color of this node for debug visualization 
//				purposes.  This function is overloaded to handle applying a 
//				different color if the node is also valid.
//              
//----------------------------------------------------------------------------

DebugLine::Color AINodeSafety::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafety::UpdateDebugDrawStatus()
//              
//	PURPOSE:	Overloaded function to handle updating the colors of the 
//				child nodes as well.  This is required as the AINodeMgr, 
//				which handles updating normally, can only update one node type
//				at a time.
//              
//----------------------------------------------------------------------------

void AINodeSafety::UpdateDebugDrawStatus( HOBJECT hTarget )
{
	super::UpdateDebugDrawStatus( hTarget );

	for ( uint32 i = 0; i < m_FireNodes.GetNumObjectHandles(); ++i )
	{
		HOBJECT hNode = m_FireNodes.GetObjectHandle( i );
		if ( hNode )
		{
			AIASSERT( AINodeSafetyFirePosition::DynamicCast(hNode), m_hObject, "AINodeSafety::DrawSelf: Unexpected node type." );
			AINodeSafetyFirePosition* pNode = (AINodeSafetyFirePosition*)g_pLTServer->HandleToObject( hNode );
			pNode->UpdateDebugDrawStatus( hTarget );
		}
	}
}
