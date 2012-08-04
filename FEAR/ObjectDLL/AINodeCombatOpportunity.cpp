// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCombatOpportunity.cpp
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeCombatOpportunity.h"
#include "AICombatOpportunity.h"
#include "AIAssert.h"
#include "AI.h"

LINKFROM_MODULE(AINodeCombatOpportunity);

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeCombatOpportunityPlugin
//
//	PURPOSE:	This plugin supports filtering the smartobject types 
//				displayed in the WorldEdit dropdown menu.
//
// ----------------------------------------------------------------------- //

class AINodeCombatOpportunityPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeCombatOpportunityPlugin()
	{
		AddValidNodeType( kNode_CombatOpportunity );
	}
};


// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeCombatOpportunity 0

#elif defined ( PROJECT_FEAR )
	
	#define CF_HIDDEN_AINodeCombatOpportunity CF_HIDDEN

#endif

BEGIN_CLASS(AINodeCombatOpportunity)

	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	PLAYERONNODE_PROPS()
	THREATRADIUS_PROPS( 256.0f )
	BOUNDARYRADIUS_PROPS( 1024.0f )

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,	"None", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeCombatOpportunity, AINodeSmartObject, CF_HIDDEN_AINodeCombatOpportunity, AINodeCombatOpportunityPlugin, "Used to handle action based classes, where the AI is moving to a location and playing a specific animation to trigger the AICombatOpportunity")


CMDMGR_BEGIN_REGISTER_CLASS(AINodeCombatOpportunity)
CMDMGR_END_REGISTER_CLASS(AINodeCombatOpportunity, AINodeSmartObject)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeCombatOpportunity::Con/destructor
//
//	PURPOSE:	Construct the object into an inert state.
//
// ----------------------------------------------------------------------- //

AINodeCombatOpportunity::AINodeCombatOpportunity()
{
}

AINodeCombatOpportunity::~AINodeCombatOpportunity()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunity::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeCombatOpportunity
//              
//----------------------------------------------------------------------------

void AINodeCombatOpportunity::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hCombatOpportunity);

	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
}

void AINodeCombatOpportunity::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hCombatOpportunity);

	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunity::SetCombatOpportunity
//              
//	PURPOSE:	Handles setting the CombatOpportunity object associated with
//				this node.
//
//----------------------------------------------------------------------------

void AINodeCombatOpportunity::SetCombatOpportunity(AICombatOpportunity* pCombatOpportunity)
{
	// Invalid pointer.

	if (!pCombatOpportunity)
	{
		AIASSERT(0, GetHOBJECT(), "AINodeCombatOpportunity::SetCombatOpportunity : Opportunity is NULL");	
		return;
	}

	// CombatOpportunity already set.  Print out a warning, and ignore the 
	// latest SetCombatOpportunity call.

	if (m_hCombatOpportunity)
	{
		char szCurrentCombatOpportunityObjectName[128];
		szCurrentCombatOpportunityObjectName[0] = '\0';
		g_pLTServer->GetObjectName(m_hCombatOpportunity, szCurrentCombatOpportunityObjectName, LTARRAYSIZE(szCurrentCombatOpportunityObjectName));

		char szNewCombatOpportunityObjectName[128];
		szNewCombatOpportunityObjectName[0] = '\0'; 
		g_pLTServer->GetObjectName(pCombatOpportunity->GetHOBJECT(), szNewCombatOpportunityObjectName, LTARRAYSIZE(szNewCombatOpportunityObjectName));

		AIASSERT2(0, GetHOBJECT(), 
			"AINodeCombatOpportunity::SetCombatOpportunity : Node is already set.  Ignoring new CombatOpportunity (current AICombatOpportunity: %s, new AICombatOpportunity: %s)", 
			szCurrentCombatOpportunityObjectName, szNewCombatOpportunityObjectName);	

		return;
	}

	m_hCombatOpportunity = pCombatOpportunity->GetHOBJECT();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINodeCombatOpportunity::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunity::InitNode
//              
//	PURPOSE:	Initialize the node.
//              
//----------------------------------------------------------------------------

void AINodeCombatOpportunity::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeCombatOpportunity::Lock / Unlock
//
//  PURPOSE:	Lock/unlock nodes.
//
// ----------------------------------------------------------------------- //

void AINodeCombatOpportunity::LockNode(HOBJECT hAI)
{
	super::LockNode( hAI );

	// Lock the associated CombatOpportunity.

	// No associated AICombatOpportunity

	if (!m_hCombatOpportunity)
	{
		return;
	}

	AICombatOpportunity* pAICombatOpportunity = AICombatOpportunity::HandleToObject(m_hCombatOpportunity);
	if (pAICombatOpportunity)
	{
		pAICombatOpportunity->LockCombatOpportunity( hAI );
	}
}

void AINodeCombatOpportunity::UnlockNode(HOBJECT hAI)
{
	super::UnlockNode( hAI );

	// Unlock the associated CombatOpportunity.

	// No associated AICombatOpportunity

	if (!m_hCombatOpportunity)
	{
		return;
	}

	AICombatOpportunity* pAICombatOpportunity = AICombatOpportunity::HandleToObject(m_hCombatOpportunity);
	if (pAICombatOpportunity)
	{
		pAICombatOpportunity->UnlockCombatOpportunity( hAI );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunity::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//              
//----------------------------------------------------------------------------

bool AINodeCombatOpportunity::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if (!super::IsNodeValid(pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags))
	{
		return false;
	}

	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Player is standing on the node.

	if( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}


	if (hThreat)
	{
		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
		float fNodeDistSqr = m_vPos.DistSqr(vThreatPos);

		// Threat inside radius.

		if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		// Threat outside boundary.

		if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}
	}

	// No associated AICombatOpportunity

	if (!m_hCombatOpportunity)
	{
		return false;
	}

	AICombatOpportunity* pAICombatOpportunity = AICombatOpportunity::HandleToObject(m_hCombatOpportunity);
	if (!pAICombatOpportunity)
	{
		return false;
	}

	// Verify that this nodes CombatOpportunity node is valid.

	if (!pAICombatOpportunity->IsValid(pAI->GetHOBJECT(), hThreat, AICombatOpportunity::kStatusFlag_ThreatPosition))
	{
		return false;
	}

	return true;
}

