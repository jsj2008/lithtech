// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSafetyFirePosition.cpp
//
// PURPOSE : 
//
// CREATED : 1/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeSafetyFirePosition.h"
#include "AINodeSafety.h"
#include "DEditColors.h"

LINKFROM_MODULE(AINodeSafetyFirePosition);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeSafetyFirePosition 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeSafetyFirePosition CF_HIDDEN

#endif

BEGIN_CLASS(AINodeSafetyFirePosition)

	ADD_DEDIT_COLOR( AINodeSafetyFirePosition )
	ADD_VECTORPROP_VAL_FLAG(Dims,		16.0f, 16.0f, 16.0f,	PF_HIDDEN | PF_DIMS, "This hidden field handles setting the dims for visualization")
	DAMAGED_PROPS()
	THREATFOV_PROPS()
	BOUNDARYRADIUS_PROPS( 1000.0f )
	ADD_STRINGPROP_FLAG(SmartObject,	"SafetyFirePosition", 		0|PF_STATICLIST|PF_DIMS, "SmartObject used to specify animations for this node")

	// Radius isn't used; these nodes are discovered through the AINodeSafety 
	// instances.
	ADD_REALPROP_FLAG(Radius,					384.0f,			PF_HIDDEN, "The AI must be within this radius to be able to use the node. [WorldEdit units]")

END_CLASS_FLAGS_PLUGIN(AINodeSafetyFirePosition, AINodeSmartObject, CF_HIDDEN_AINodeSafetyFirePosition, AINodeSafetyFirePositionPlugin, "Defines a position an AI can run to after using the owning AINodeSafety instance for cover or reloading.  This node must be placed under an AINodeSafety object.  If it isn't, this node will not be used and an error will be printed at runtime." )

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSafetyFirePosition)
CMDMGR_END_REGISTER_CLASS(AINodeSafetyFirePosition, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSafetyFirePosition::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeSafetyFirePosition::AINodeSafetyFirePosition()
{
}

AINodeSafetyFirePosition::~AINodeSafetyFirePosition()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::Save/Load/ReadProp()
//              
//	PURPOSE:	Handle saving and restoring the AINodeSafetyFirePosition
//              
//----------------------------------------------------------------------------

void AINodeSafetyFirePosition::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	m_ThreatFOV.ReadProps( pProps );
	m_BoundaryRadius.ReadProps( pProps );
	m_DamagedValidator.ReadProps( pProps );

	// NOTE: m_hAINodeSafetyParent is set by the parent.  It is not set at 
	// prop reading time.
}

void AINodeSafetyFirePosition::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_ThreatFOV.Load( pMsg );
	m_BoundaryRadius.Load( pMsg );
	m_DamagedValidator.Load( pMsg );
	LOAD_HOBJECT( m_hAINodeSafetyParent );
}

void AINodeSafetyFirePosition::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_ThreatFOV.Save( pMsg );
	m_BoundaryRadius.Save( pMsg );
	m_DamagedValidator.Save( pMsg );
	SAVE_HOBJECT( m_hAINodeSafetyParent );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::IsNodeValid()
//              
//	PURPOSE:	Returns true if this node is valid given the passed in 
//				paramaters, false if it is not.
//              
//----------------------------------------------------------------------------

bool AINodeSafetyFirePosition::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if ( !super::IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags ) )
	{
		return false;
	}
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Verify the validators.

	const bool kbIsIgnoreDir = false;
	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
	if ( !m_ThreatFOV.Evaluate( dwFilteredStatusFlags, vThreatPos, m_vPos, m_rRot.Forward(), kbIsIgnoreDir ) )
	{
		return false;
	}

	if ( !m_BoundaryRadius.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	if ( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// No parent node.

	if ( !m_hAINodeSafetyParent )
	{
		return false;
	}

	// Verify that the parent node is valid (it is used for FOV tests)
	// Note that we are using this nodes status flags; this allows this 
	// node to control what validators on the parent node are tested.

	AINodeSafety* pNode = AINodeSafety::DynamicCast( m_hAINodeSafetyParent );
	if ( !pNode )
	{
		return false;
	}

	if ( !pNode->IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwFilteredStatusFlags ) )
	{
		return false;
	}

	// The AINodeSafetyParent _must_ be expired for the AI to use a fire 
	// position node.  This insures the AI stays at this node for some period
	// of time before departing.

	if ( pNode->GetExpirationTime( pAI ) > g_pLTServer->GetTime() )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::GetType()
//              
//	PURPOSE:	Returned the 'type' of this node.
//              
//----------------------------------------------------------------------------

EnumAINodeType AINodeSafetyFirePosition::GetType() const
{
	return kNode_SafetyFirePosition;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::LockNode()
//              
//	PURPOSE:	When a SafetyFirePosition node is locked, the parent Safety 
//				node is locked as well.  This should prevent other AIs from 
//				using the Safety node while an AI is using one of its fire 
//				positions.
//              
//----------------------------------------------------------------------------

void AINodeSafetyFirePosition::LockNode(HOBJECT hAI)
{
	super::LockNode( hAI );

	// Lock the parent node.

	if ( m_hAINodeSafetyParent )
	{
		AINodeSafety* pNode = AINodeSafety::DynamicCast( m_hAINodeSafetyParent );
		if ( pNode )
		{
			pNode->LockNode( hAI );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::UnlockNode()
//              
//	PURPOSE:	When a SafetyFirePosition node is unlocked, the parent Safety 
//				node is locked as well.  This should prevent other AIs from 
//				using the Safety node while an AI is using one of its fire 
//				positions.
//              
//----------------------------------------------------------------------------

void AINodeSafetyFirePosition::UnlockNode(HOBJECT hAI)
{
	super::UnlockNode( hAI );

	// Unlock the parent node.

	if ( m_hAINodeSafetyParent )
	{
		AINodeSafety* pNode = AINodeSafety::DynamicCast( m_hAINodeSafetyParent );
		if ( pNode )
		{
			pNode->UnlockNode( hAI );
		}
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::SetParent()
//              
//	PURPOSE:	Sets this nodes parent SafetyNode.  This parent is used for 
//				validation of this node, as well as insuring the whole system 
//				is locked by a single AI.
//              
//----------------------------------------------------------------------------

void AINodeSafetyFirePosition::SetParent( const AINodeSafety* const pParent )
{
	if ( !pParent )
	{
		AIASSERT( pParent, m_hObject, "AINodeSafetyFirePosition::SetParent: Setting the parent node to NULL" );
		return;
	}

	AIASSERT( NULL == (HOBJECT)m_hAINodeSafetyParent, m_hObject, "AINodeSafetyFirePosition::SetParent: AINodeSafetyFirePosition is already owned; multiple parents specified." );
	m_hAINodeSafetyParent = pParent->GetHOBJECT();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSafetyFirePosition::GetDebugColor()
//              
//	PURPOSE:	Returns the color of this node for debug visualization 
//				purposes.  This function is overloaded to handle applying a 
//				different color if the node is also valid.
//              
//----------------------------------------------------------------------------

DebugLine::Color AINodeSafetyFirePosition::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}
