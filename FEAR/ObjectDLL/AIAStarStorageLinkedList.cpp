// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarStorageLinkedList.cpp
//
// PURPOSE : AStar Storage class using a linked list.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

// Includes required for AIAStarStorageLinkedList.h

#include "AIClassFactory.h"
#include "AIAStarMachine.h"

// Includes required for AIAStarStorageLinkedList.cpp

#include "AIAStarStorageLinkedList.h"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarStorageLinkedList::CAIAStarStorageLinkedList()
{
	m_pOpenListHead = NULL;
	m_pClosedListHead = NULL;
}

CAIAStarStorageLinkedList::~CAIAStarStorageLinkedList()
{
	ResetAStarStorage();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::CreateAStarNode
//              
//	PURPOSE:	Create an AStarNode with a specified ID. 
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageLinkedList::CreateAStarNode( ENUM_AStarNodeID eAStarNode )
{
	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::DestroyAStarNode
//              
//	PURPOSE:	Destroy an AStarNode. 
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode )
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::ResetAStarStorage
//              
//	PURPOSE:	Delete nodes from Open and Closed lists.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::ResetAStarStorage()
{
	// Delete nodes from both lists.

	ClearList( m_pOpenListHead );
	ClearList( m_pClosedListHead );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::ClearList
//              
//	PURPOSE:	Delete nodes from list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::ClearList( CAIAStarNodeAbstract*& pListHead )
{
	// List is empty.

	if( !pListHead )
	{
		return;
	}

	CAIAStarNodeAbstract* pNode = pListHead;
	CAIAStarNodeAbstract* pNodeDelete;
	pListHead = NULL;

	// Iterate over all nodes in list, and delete them.

	while( pNode )
	{
		pNodeDelete = pNode;
		pNode = pNode->pListNext;
		DestroyAStarNode( pNodeDelete );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::AddToOpenList
//              
//	PURPOSE:	Add nodes to Open list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::AddToOpenList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap )
{
	// Bail is node is already in Open list.

	unsigned long dwFlags = pAStarMap->GetAStarFlags( pAStarNode->eAStarNodeID );
	if( dwFlags & CAIAStarMapAbstract::kASTAR_Open )
	{
		return;
	}

	// Add node to Open list, and set AStar flags.

	AddToList( m_pOpenListHead, pAStarNode );
	pAStarMap->SetAStarFlags( pAStarNode->eAStarNodeID, CAIAStarMapAbstract::kASTAR_Open, CAIAStarMapAbstract::kASTAR_OpenOrClosed );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::AddToClosedList
//              
//	PURPOSE:	Add nodes to Closed list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::AddToClosedList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap )
{
	// The AStar algorithm will never try to insert an already closed 
	//node in the Closed list, so no need to check for it.

	// Add node to Closed list, and set AStar flags.

	AddToList( m_pClosedListHead, pAStarNode );
	pAStarMap->SetAStarFlags( pAStarNode->eAStarNodeID, CAIAStarMapAbstract::kASTAR_Closed, CAIAStarMapAbstract::kASTAR_OpenOrClosed );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::AddToList
//              
//	PURPOSE:	Add nodes to list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::AddToList( CAIAStarNodeAbstract*& pListHead, CAIAStarNodeAbstract* pAStarNode )
{
	// Insert a node at the head of the list.

	if( pListHead )
	{
		pListHead->pListPrev = pAStarNode;
		pAStarNode->pListNext = pListHead;
	}

	pListHead = pAStarNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::RemoveFromOpenList
//              
//	PURPOSE:	Remove node from Open list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::RemoveFromOpenList( CAIAStarNodeAbstract* pAStarNode )
{
	// No need to clear AStar flags, because when node is inserted into
	// the Closed list, flags be be cleared and reset.

	RemoveFromList( m_pOpenListHead, pAStarNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::RemoveFromClosedList
//              
//	PURPOSE:	Remove node from Closed list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::RemoveFromClosedList( CAIAStarNodeAbstract* pAStarNode )
{
	// No need to clear AStar flags, because when node is inserted into
	// the Open list, flags be be cleared and reset.

	RemoveFromList( m_pClosedListHead, pAStarNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::RemoveFromList
//              
//	PURPOSE:	Remove node from list.
//              
//----------------------------------------------------------------------------

void CAIAStarStorageLinkedList::RemoveFromList( CAIAStarNodeAbstract*& pListHead, CAIAStarNodeAbstract* pAStarNode )
{
	// Remove a node from the middle or end of the list.

	if( pAStarNode->pListPrev )
	{
		pAStarNode->pListPrev->pListNext = pAStarNode->pListNext;
	}

	// Remove a node from the head of the list.

	else {
		pListHead = pAStarNode->pListNext;
	}

	// Complete 2-way link.

	if( pAStarNode->pListNext )
	{
		pAStarNode->pListNext->pListPrev = pAStarNode->pListPrev;
	}

	// NULLify links.

	pAStarNode->pListNext = NULL;
	pAStarNode->pListPrev = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::RemoveCheapestOpenNode
//              
//	PURPOSE:	Remove and return cheapest node from Open list.
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageLinkedList::RemoveCheapestOpenNode()
{
	// NOTE: Finding the cheapest node could be optimized by 
	//       maintaining a short, sorted list of the cheapest
	//       nodes. See "How to Achieve Lightning-Fast A*" in 
	//       AI Game Programming Wisdom, p. 133.
	//       Specifically "Be a Cheapskate" on p. 140.

	CAIAStarNodeAbstract* pNodeCheapest = m_pOpenListHead;

	// Iterate over all nodes, and keep track of the cheapest.

	CAIAStarNodeAbstract* pNode;
	for( pNode = m_pOpenListHead; pNode; pNode = pNode->pListNext )
	{
		if( pNode->fFitness < pNodeCheapest->fFitness )
		{
			pNodeCheapest = pNode;
		}
	}

	// Remove the cheapest node from the Open list.

	if( pNodeCheapest )
	{
		RemoveFromOpenList( pNodeCheapest );
	}

	// Return the cheapest node.

	return pNodeCheapest;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::FindInOpenList
//              
//	PURPOSE:	Return node if found in Open list.
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageLinkedList::FindInOpenList( ENUM_AStarNodeID eAStarNode )
{
	return FindInList( m_pOpenListHead, eAStarNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::FindInClosedList
//              
//	PURPOSE:	Return node if found in Closed list.
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageLinkedList::FindInClosedList( ENUM_AStarNodeID eAStarNode )
{
	return FindInList( m_pClosedListHead, eAStarNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageLinkedList::FindInList
//              
//	PURPOSE:	Return node if found in list.
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageLinkedList::FindInList( CAIAStarNodeAbstract* pListHead, ENUM_AStarNodeID eAStarNode )
{
	// NOTE: List searches could be optimized by using a has table of lists rather than one flat list.

	// Iterate over all nodes in list.

	CAIAStarNodeAbstract* pNode = pListHead;
	while( pNode )
	{
		// Return node if a match is found.

		if( pNode->eAStarNodeID == eAStarNode )
		{
			return pNode;
		}

		pNode = pNode->pListNext;
	}

	// No match was found.

	return NULL;
}
