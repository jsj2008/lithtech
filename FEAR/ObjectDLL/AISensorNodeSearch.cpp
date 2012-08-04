// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeSearch.cpp
//
// PURPOSE : AISensorNodeSearch class implementation
//
// CREATED : 01/13/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeSearch.h"
#include "AINavMesh.h"
#include "AINodeMgr.h"
#include "AIPathKnowledgeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeSearch, kSensor_NodeSearch );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSearch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNodeSearch::CAISensorNodeSearch()
{
	m_hSearchNode = NULL;
	m_eSearchStatus = kSearchStatus_Unset;
	m_nPathKnowledgeIndex = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNodeSearch::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNodeGuarded and all 
//				of its sensors.
//              
//----------------------------------------------------------------------------\

void CAISensorNodeSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hSearchNode );
	SAVE_DWORD( m_eSearchStatus );
	SAVE_DWORD( m_nPathKnowledgeIndex );

	ENUM_NMComponentID eComponent;
	NMCOMPONENT_LIST::iterator itList;

	// Save Open list.

	SAVE_INT( m_lstOpen.size() );
	for( itList = m_lstOpen.begin(); itList != m_lstOpen.end(); ++itList )
	{
		eComponent = *itList;
		SAVE_DWORD( eComponent );
	}

	// Save Closed list.

	SAVE_INT( m_lstClosed.size() );
	for( itList = m_lstClosed.begin(); itList != m_lstClosed.end(); ++itList )
	{
		eComponent = *itList;
		SAVE_DWORD( eComponent );
	}
}

//----------------------------------------------------------------------------\

void CAISensorNodeSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hSearchNode );
	LOAD_DWORD_CAST( m_eSearchStatus, EnumAISearchStatus );
	LOAD_DWORD( m_nPathKnowledgeIndex );

	uint32 cComponents;
	uint32 iComponent;
	ENUM_NMComponentID eComponent;

	// Load Open list.

	LOAD_INT( cComponents );
	m_lstOpen.reserve( cComponents );
	for( iComponent=0; iComponent < cComponents; ++iComponent )
	{
		LOAD_DWORD_CAST( eComponent, ENUM_NMComponentID );
		m_lstOpen.push_back( eComponent );
	}

	// Load Closed list.

	LOAD_INT( cComponents );
	m_lstClosed.reserve( cComponents );
	for( iComponent=0; iComponent < cComponents; ++iComponent )
	{
		LOAD_DWORD_CAST( eComponent, ENUM_NMComponentID );
		m_lstClosed.push_back( eComponent );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSearch::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeSearch::UpdateSensor()
{
	//
	// Start a new search.
	//

	if( m_eSearchStatus == kSearchStatus_Unset )
	{
		// Start searching for a Search node from our current poly.
		// Bail if we are not in the NavMesh.

		m_hSearchNode = NULL;
		m_nPathKnowledgeIndex = m_pAI->GetPathKnowledgeMgr()->GetPathKnowledgeIndex();
		ENUM_NMPolyID ePoly = m_pAI->GetLastNavMeshPoly();
		if( ePoly == kNMPoly_Invalid )
		{
			m_eSearchStatus = kSearchStatus_Failure;
			return false;
		}
		CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
		if( !pPoly )
		{
			m_eSearchStatus = kSearchStatus_Failure;
			return false;
		}

		// Expand the first component.
		// Bail if it does not exist.

		ENUM_NMComponentID eComponent = pPoly->GetNMComponentID();
		if( eComponent == kNMComponent_Invalid )
		{
			m_eSearchStatus = kSearchStatus_Failure;
			return false;
		}

		// Start searching.

		AddComponentToList( eComponent, m_lstOpen );
		m_eSearchStatus = kSearchStatus_Searching;
	}


	//
	// Search for a node.
	//

	if( m_eSearchStatus == kSearchStatus_Searching )
	{
		// Select a new search origin component to explore.

		ENUM_NMComponentID eComponent = m_lstOpen.front();

		// Expand neighbors of next component to explore.

		ExploreComponent( eComponent );

		// Try to find a node in the component.

		AINode* pNode = g_pAINodeMgr->FindNodeInComponent( m_pAI, kNode_Search, eComponent, FLT_MAX, false );
		if( pNode )
		{
			// Some components may be unreachable to this AI.

			////ENUM_NMPolyID ePoly = pNode->GetNodeContainingNMPoly();
			////if( g_pAIPathMgrNavMesh->HasPath( m_pAI, m_pAI->GetCharTypeMask(), ePoly ) )
			{
				m_hSearchNode = pNode->m_hObject;

				// Create a memory of the search node.

				CAIWMFact factQuery;
				factQuery.SetFactType( kFact_Node );
				factQuery.SetNodeType( kNode_Search );
				m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

				CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
				pFact->SetNodeType( kNode_Search, 1.f );
				pFact->SetTargetObject( m_hSearchNode, 1.f );

				m_eSearchStatus = kSearchStatus_Success;
			}
		}

		// No more components to explore.

		else if( m_lstOpen.empty() )
		{
			m_eSearchStatus = kSearchStatus_Failure;
		}
	}


	//
	// Invalid node if something has changed.
	//
	
	if( m_eSearchStatus == kSearchStatus_Success )
	{
		// Sanity check.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hSearchNode );
		if( !pNode )
		{
			m_eSearchStatus = kSearchStatus_Unset;
			return false;
		}

		// No longer have a path to the node.
		/**
		ENUM_NMPolyID ePoly = pNode->GetNodeContainingNMPoly();
		if( !g_pAIPathMgrNavMesh->HasPath( m_pAI, m_pAI->GetCharTypeMask(), ePoly ) )
		{
			m_eSearchStatus = kSearchStatus_Unset;
			return false;
		}
		**/
		if( m_nPathKnowledgeIndex != m_pAI->GetPathKnowledgeMgr()->GetPathKnowledgeIndex() )
		{
			m_eSearchStatus = kSearchStatus_Unset;
			return false;
		}
	}


	//
	// Start a new search if something has changed.
	//

	if( m_eSearchStatus == kSearchStatus_Failure )
	{
		if( m_nPathKnowledgeIndex != m_pAI->GetPathKnowledgeMgr()->GetPathKnowledgeIndex() )
		{
			m_eSearchStatus = kSearchStatus_Unset;
			return false;
		}
	}

	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::IsComponentInList
//
//	PURPOSE:	Return true if component exists in a list.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeSearch::IsComponentInList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return false;
	}

	// Find component in list.

	NMCOMPONENT_LIST::iterator itComponent;
	for( itComponent = lstComponents.begin(); itComponent != lstComponents.end(); ++itComponent )
	{
		if( eComponent == *itComponent )
		{
			return true;
		}
	}

	// Component is not in list.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSearch::AddComponentToList
//
//	PURPOSE:	Add a component to a list.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeSearch::AddComponentToList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return;
	}

	// Ensure ID is not already in the list.

	if( IsComponentInList( eComponent, lstComponents ) )
	{
		return;
	}

	// Add the component to the list.

	lstComponents.push_back( eComponent );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSearch::RemoveComponentFromList
//
//	PURPOSE:	Remove component from a list.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeSearch::RemoveComponentFromList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return;
	}

	// Erase component from list.

	NMCOMPONENT_LIST::iterator itComponent;
	for( itComponent = lstComponents.begin(); itComponent != lstComponents.end(); ++itComponent )
	{
		if( eComponent == *itComponent )
		{
			lstComponents.erase( itComponent );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeSearch::ExploreComponent
//
//	PURPOSE:	Add component's neighbors to Open, and component to Closed.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeSearch::ExploreComponent( ENUM_NMComponentID eComponent )
{
	// Sanity check.

	CAINavMeshComponent* pComponent = g_pAINavMesh->GetNMComponent( eComponent );
	if( !pComponent )
	{
		return;
	}

	// Add neighbors to Open list.

	CAINavMeshComponent* pNeighbor;
	int cNeighbors = pComponent->GetNumNMComponentNeighbors();
	for( int iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
	{
		pNeighbor = pComponent->GetNMComponentNeighbor( iNeighbor );
		if( pNeighbor )
		{
			// Don't re-add components that are already explored.

			if( !IsComponentInList( pNeighbor->GetNMComponentID(), m_lstClosed ) )
			{
				AddComponentToList( pNeighbor->GetNMComponentID(), m_lstOpen );
			}
		}
	}

	// Add component to Closed list.

	AddComponentToList( eComponent, m_lstClosed );

	// Remove component from Open list.

	RemoveComponentFromList( eComponent, m_lstOpen );
}

