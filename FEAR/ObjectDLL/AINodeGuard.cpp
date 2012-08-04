// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeGuard.cpp
//
// PURPOSE : AINodeGuard implementation
//
// CREATED : 11/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeGuard.h"
#include "AINodeMgr.h"
#include "AINavMesh.h"
#include "AIRegion.h"
#include "AIUtils.h"
#include "AIAssert.h"
#include "Character.h"

LINKFROM_MODULE( AINodeGuard );

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeGuard)

	ADD_VECTORPROP_VAL_FLAG(Dims,		16.0f, 16.0f, 16.0f,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(Radius,					0.0f,			0|PF_RADIUS, "The AI will guard the space contained within this radius. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI will guard the space contained within this AIRegion.")

	ADD_NAMED_OBJECT_LIST_AGGREGATE( GuardedNodes, PF_GROUP(1), Node, "TODO:GROUPDESC", "TODO:PROPDESC" )

END_CLASS(AINodeGuard, AINode, "This node is the center of an AI's guarding radius.  AI will return to an AINodeGuard after giving up a search for the enemy")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeGuard)
CMDMGR_END_REGISTER_CLASS(AINodeGuard, AINode)

// ----------------------------------------------------------------------- //

AINodeGuard::AINodeGuard()
{
	AddAggregate( &m_GuardedNodes );
}

AINodeGuard::~AINodeGuard()
{
}

void AINodeGuard::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_GuardedNodes.ReadProp( pProps, "Node" );
}

void AINodeGuard::AllNodesInitialized()
{
	super::AllNodesInitialized();

	m_GuardedNodes.InitNamedObjectList( m_hObject );

	// Get the guarded nodes.
	// Guarded nodes will be owned by this guard node.  Nodes that are
	// owned may only be used by the AI assigned to this guard node.

	HOBJECT hNode;
	const char* pszNodeName;
	AINode* pNode;
	uint32 cNodes = m_GuardedNodes.GetNumObjectNames();
	for( uint32 iNode=0; iNode < cNodes; ++iNode )
	{
		// Node not found.

		pszNodeName = m_GuardedNodes.GetObjectName( iNode );
		hNode = m_GuardedNodes.GetObjectHandle( iNode );
		if( !hNode )
		{
			AIASSERT1( 0, m_hObject, "AINodeGuard::Init: Cannot find guarded node \"%s\"", pszNodeName );
			continue;
		}

		// Not a node.

		if( !IsAINode( hNode ) )
		{
			AIASSERT1( 0, m_hObject, "AINodeGuard::Init: Guarded object is not an AINode \"%s\"", pszNodeName );
			continue;
		}

		// Node does not exist.

		pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !pNode )
		{
			AIASSERT1( 0, m_hObject, "AINodeGuard::Init: Cannot find guarded node \"%s\"", pszNodeName );
			continue;
		}

		// Set owner to prevent other AI from using this node.

		pNode->SetNodeOwner( m_hObject );

		// Add node to list of Guarded nodes.
		// Ignore node (and complain) if node is outside of guarding area.

		if( !IsNodeInRadiusOrRegion( pNode ) )
		{
			AIASSERT1( 0, m_hObject, "AINodeGuard::Init: Guarded node \"%s\" is outside of Guard node's radius or AIRegion. No one may use this node!", pszNodeName );
			m_GuardedNodes.ClearObjectHandle( iNode );
			continue;
		}
	}
	
	m_GuardedNodes.ClearStrings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeGuard::IsGuardingNode
//
//	PURPOSE:	Return true if Guard node is guarding the specified node.
//
// ----------------------------------------------------------------------- //

bool AINodeGuard::IsGuardingNode( AINode* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return false;
	}

	// Node is public if it has no owner.
	// Public nodes are guarded by all Guard nodes that include
	// it in their radius or AIRegion.

	if( pNode->GetNodeOwner() == NULL )
	{
		if( !IsNodeInRadiusOrRegion( pNode ) )
		{
			return false;
		}
	}

	// Node is owned by someone else.

	else if( pNode->GetNodeOwner() != m_hObject )
	{
		return false;
	}

	// This node IS guarding the specified node.

	return true;
}

// ----------------------------------------------------------------------- //

void AINodeGuard::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void AINodeGuard::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

