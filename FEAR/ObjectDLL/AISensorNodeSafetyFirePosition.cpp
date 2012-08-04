// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeSafetyFirePosition.cpp
//
// PURPOSE : 
//
// CREATED : 2/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeSafetyFirePosition.h"
#include "AIWorkingMemory.h"
#include "AINodeSafety.h"
#include "AINodeSafetyFirePosition.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeSafetyFirePosition, kSensor_SafetyNodeFirePosition );

// Helper function to handle removing all AINodeSafetyFirePostion facts 
// from an AIs working memory.
static void ClearSafetyFirePositions( CAIWorkingMemory* pWorkingMemory )
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Node );
	factQuery.SetNodeType( kNode_SafetyFirePosition );
	pWorkingMemory->ClearWMFacts( factQuery );
};

// Helper function for adding all AINodeSafetyFirePostion instances that 
// are a child of the passed in AINodeSafety to an AIs working memory.
static void UpdateSafetyFirePositionCache( CAIWorkingMemory* pWorkingMemory, HOBJECT hSafetyNode )
{
	AIASSERT( IsKindOf(hSafetyNode, "AINodeSafety"), hSafetyNode, 
		"UpdateSafetyFirePositionCache: AINode is not of the expected type." );
	AINodeSafety* pSafetyNode = AINodeSafety::DynamicCast( hSafetyNode );
	if ( !pSafetyNode )
	{
		return;
	}

	int nFirePositions = pSafetyNode->GetFirePositionCount();
	for ( int i = 0; i < nFirePositions; ++i )
	{
		AINodeSafetyFirePosition* pNode = pSafetyNode->GetFirePosition( i );
		if ( pNode )
		{
			CAIWMFact* pFact = pWorkingMemory->CreateWMFact( kFact_Node );
			if ( pFact )
			{
				pFact->SetNodeType( kNode_SafetyFirePosition );
				pFact->SetTargetObject( pNode->GetHOBJECT() );
				pFact->SetPos( pNode->GetPos() );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSafetyFirePosition::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNodeSafetyFirePosition::CAISensorNodeSafetyFirePosition()
{
}

CAISensorNodeSafetyFirePosition::~CAISensorNodeSafetyFirePosition()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNodeSafetyFirePosition::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNodeSafetyFirePosition
//              
//----------------------------------------------------------------------------

void CAISensorNodeSafetyFirePosition::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hSafetyNodeCacheSource);
}

void CAISensorNodeSafetyFirePosition::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hSafetyNodeCacheSource);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSafetyFirePosition::UpdateSensor()
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeSafetyFirePosition::UpdateSensor()
{
	// Intentionally skip super class' UpdateSensor().
	// Behavior varies depending on presence of Safety node.

	if( !CAISensorAbstract::UpdateSensor() )
	{
		return false;
	}

	// Check to see if the AI is at a node of type AINodeSafety.  
	// 1) If not, clear the AINodeSafetyFirePosition memories
	// 2) If the AI is, check to see if the AI is still at the same node
	//	2a) If so, nothing needs to be done (cached state is still correct)
	//	2b) If not, clear the facts, generate a new list, and update the 
	//		cache tracking handle.

	SAIWORLDSTATE_PROP* pNodeTypeProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNodeType, m_pAI->GetHOBJECT() );
	if ( !pNodeTypeProp || kNode_Safety != pNodeTypeProp->eAINodeTypeWSValue )
	{
		if ( m_hSafetyNodeCacheSource )
		{
			ClearSafetyFirePositions( m_pAI->GetAIWorkingMemory() );
			m_hSafetyNodeCacheSource = NULL;
		}
		return false;
	}

	SAIWORLDSTATE_PROP* pAtNodeProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->GetHOBJECT() );
	if ( !pAtNodeProp || !pAtNodeProp->hWSValue )
	{
		if ( m_hSafetyNodeCacheSource )
		{
			ClearSafetyFirePositions( m_pAI->GetAIWorkingMemory() );
			m_hSafetyNodeCacheSource = NULL;
		}
		return false;
	}

	HOBJECT hCurrentSafetyNode = pAtNodeProp->hWSValue;
	if ( m_hSafetyNodeCacheSource != hCurrentSafetyNode )
	{
		if ( m_hSafetyNodeCacheSource )
		{
			ClearSafetyFirePositions( m_pAI->GetAIWorkingMemory() );
			m_hSafetyNodeCacheSource = NULL;
		}

		UpdateSafetyFirePositionCache( m_pAI->GetAIWorkingMemory(), hCurrentSafetyNode );
		m_hSafetyNodeCacheSource = hCurrentSafetyNode;
		return false;
	}

	return false;
}
