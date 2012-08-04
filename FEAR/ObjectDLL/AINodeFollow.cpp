// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeFollow.cpp
//
// PURPOSE : AINodeFollow implementation
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeFollow.h"

LINKFROM_MODULE( AINodeFollow );

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeFollow)

	ADD_VECTORPROP_VAL_FLAG(Dims,		16.0f, 16.0f, 16.0f,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(Radius,					256.0f,			0|PF_RADIUS, "The AI will guard the space contained within this radius. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI will guard the space contained within this AIRegion.")

	ADD_BOOLPROP_FLAG(Face,						true,			PF_HIDDEN, "Set this to true if you want the AI who uses this node to face along the forward of the node.")
	ADD_ROTATIONPROP_FLAG(FacingOffset,							PF_HIDDEN|PF_RELATIVEORIENTATION, "Offset AI should face from the node's rotation." )

	ADD_NAMED_OBJECT_LIST_AGGREGATE( WaitingNodes, PF_GROUP(1), Node, "TODO:GROUPDESC", "TODO:PROPDESC" )

END_CLASS(AINodeFollow, AINode, "AI following another character may wait at nodes pointed to by an AINodeFollow, if the leader is within the AINodeFollow's radius or AIRegion")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeFollow)
CMDMGR_END_REGISTER_CLASS(AINodeFollow, AINode)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFollow::Con/Destructor
//
//	PURPOSE:	Initialization/Termination
//
// ----------------------------------------------------------------------- //

AINodeFollow::AINodeFollow()
{
	AddAggregate( &m_WaitingNodes );
}

AINodeFollow::~AINodeFollow()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFollow::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

void AINodeFollow::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_WaitingNodes.ReadProp( pProps, "Node" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFollow::InitNode
//
//	PURPOSE:	Initialize the node.
//
// ----------------------------------------------------------------------- //

void AINodeFollow::InitNode()
{
	super::InitNode();

	m_WaitingNodes.InitNamedObjectList( m_hObject );	
	m_WaitingNodes.ClearStrings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFollow::Save
//
//	PURPOSE:	Save the node.
//
// ----------------------------------------------------------------------- //

void AINodeFollow::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFollow::Load
//
//	PURPOSE:	Load the node.
//
// ----------------------------------------------------------------------- //

void AINodeFollow::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

