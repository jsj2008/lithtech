// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorGuard.cpp
//
// PURPOSE : AISensorGuard class implementation
//
// CREATED : 4/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AISensorGuard.h

#include "Stdafx.h"
#include "AISensorAbstract.h"
#include "AISensorGuard.h"

// Includes required for AISensorGuard.cpp

#include "AI.h"
#include "AIWorkingMemory.h"
#include "AINodeMgr.h"
#include "AINodeGuard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorGuard, kSensor_Guard );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorGuard::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorGuard::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if we already have a memory of an owned guard node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		if (!IsKindOf( pFact->GetTargetObject(), "AINodeGuard" ) )
		{
			AIASSERT(0, m_pAI->GetHOBJECT(), "CAISensorGuard::UpdateSensor : cast to incorrect type.");
			return false;
		}

		AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( pGuardNode && ( pGuardNode->GetNodeOwner() == m_pAI->m_hObject ) )
		{
			// Relinquish node if disabled.

			if( pGuardNode->IsNodeDisabled() )
			{
				pGuardNode->SetNodeOwner( NULL );
				m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
				pFact = NULL;
			}

			// We still have a valid Guard node.

			else {
				return false;
			}
		}
	}

	// Find an unowned guard node if we don't have one.

	AINodeGuard* pGuardNode = (AINodeGuard*)g_pAINodeMgr->FindNearestNodeInRadius( m_pAI, kNode_Guard, m_pAI->GetPosition(), NODEMGR_MAX_SEARCH, true );
	if( pGuardNode )
	{
		// Set the node's owner.

		pGuardNode->SetNodeOwner( m_pAI->m_hObject );

		// Create a memory for the guard node.

		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
		if( pFact )
		{
			pFact->SetNodeType( kNode_Guard, 1.f );
			pFact->SetTargetObject( pGuardNode->m_hObject, 1.f );
			pFact->SetPos( pGuardNode->GetPos(), 1.f );
		}
	}

	return true;
}


