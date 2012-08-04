// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCombatOpportunityView.cpp
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeCombatOpportunityView.h"
#include "DEditColors.h"
#include "AICombatOpportunity.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AINodeCombatOpportunityView);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeCombatOpportunityView 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeCombatOpportunityView CF_HIDDEN

#endif

BEGIN_CLASS(AINodeCombatOpportunityView)

	ADD_DEDIT_COLOR( AINodeView )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(Radius,			384.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	PLAYERONNODE_PROPS()

END_CLASS_FLAGS(AINodeCombatOpportunityView, AINode, CF_HIDDEN_AINodeCombatOpportunityView, "Used to enable an AI attempting to use a ranged weapon against to trigger a AICombatOpportunity when the AI doesn't have line of sight")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeCombatOpportunityView)
CMDMGR_END_REGISTER_CLASS(AINodeCombatOpportunityView, AINode)

AINodeCombatOpportunityView::AINodeCombatOpportunityView()
{
}

AINodeCombatOpportunityView::~AINodeCombatOpportunityView()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunityView::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeCombatOpportunityView
//              
//----------------------------------------------------------------------------

void AINodeCombatOpportunityView::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_PlayerOnNodeValidator.Load( pMsg );

	LOAD_HOBJECT(m_hCombatOpportunity);
}

void AINodeCombatOpportunityView::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_PlayerOnNodeValidator.Save( pMsg );

	SAVE_HOBJECT(m_hCombatOpportunity);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunityView::SetCombatOpportunity
//              
//	PURPOSE:	Handles setting the CombatOpportunity object associated with
//				this node.
//
//----------------------------------------------------------------------------

void AINodeCombatOpportunityView::SetCombatOpportunity(AICombatOpportunity* pCombatOpportunity)
{
	// Invalid pointer.

	if (!pCombatOpportunity)
	{
		AIASSERT(0, GetHOBJECT(), "AINodeCombatOpportunityView::SetCombatOpportunity : Opportunity is NULL");	
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
			"AINodeCombatOpportunityView::SetCombatOpportunity : Node is already set.  Ignoring new CombatOpportunity (current AICombatOpportunity: %s, new AICombatOpportunity: %s)", 
			szCurrentCombatOpportunityObjectName, szNewCombatOpportunityObjectName);	

		return;
	}

	m_hCombatOpportunity = pCombatOpportunity->GetHOBJECT();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeCombatOpportunityView::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//              
//----------------------------------------------------------------------------

bool AINodeCombatOpportunityView::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
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

	// Ranged Target object does not exist; there is nothing to shoot at.

	if (!pAICombatOpportunity->GetRangedTargetObject())
	{
		return false;
	}
	
	// Target is not currently a CombatOpportunity

	if (dwFilteredStatusFlags & kNodeStatus_TargetisCombatOpportunity)
	{
		if (!pAI->HasTarget(kTarget_CombatOpportunity))
		{
			return false;
		}

		// Ranged Target object is not the same as the passed in target.  This 
		// insures that a different ranged target object is not activated.

		if (hThreat != pAICombatOpportunity->GetRangedTargetObject())
		{
			return false;
		}
	}

	// Distance from node to target is greater than weapon range.
	// NOTE: This may break planning.  This will prevent chaining with 
	// picking up a ranged weapon, as this behavior is ignored if the
	// AI is out of range.

	LTVector vTargetDistance;
	if (LT_OK != g_pLTServer->GetObjectPos(pAICombatOpportunity->GetRangedTargetObject(), &vTargetDistance))
	{
		return false;
	}

	float flViewToTargetDistSqr = vTargetDistance.DistSqr(GetPos());
	bool bInRange = false;
	for (int iWeaponType = 0; iWeaponType != kAIWeaponType_Count; ++iWeaponType)
	{
		float flWeaponRangeSqr = AIWeaponUtils::GetWeaponRange(pAI, (ENUM_AIWeaponType)iWeaponType, true);
		flWeaponRangeSqr *= flWeaponRangeSqr;

		if (flViewToTargetDistSqr < flWeaponRangeSqr)
		{
			bInRange = true;
			break;
		}
	}
	if (!bInRange)
	{
		return false;
	}

	// CombatOpportunity is not valid.

	if ( kNodeStatus_CombatOpportunity & dwFilteredStatusFlags )
	{
		// Verify the enemies position is valid given the object and 
		// AICombatOpportunity instance.  As this view node represents a 
		// location Level Design deems good for an AI to attempt to shoot 
		// at this node from.

		if (!pAICombatOpportunity->IsValid(pAI->GetHOBJECT(), pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget(), 
			AICombatOpportunity::kStatusFlag_ThreatPosition | AICombatOpportunity::kStatusFlag_QueryObjectInEnemyArea))
		{
			return false;
		}
	}

	// Node is valid.

	return true;
}
