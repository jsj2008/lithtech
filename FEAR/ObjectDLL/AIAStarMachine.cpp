// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarMachine.cpp
//
// PURPOSE : A* machine implementation.
//
// CREATED : 11/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

// Includes required for AIAStarMachine.h

#include "AIClassFactory.h"

// Includes required for AIAStarMachine.cpp

#include "AIAStarMachine.h"

/*----------------------------------------------------------------------------

THE A* MACHINE:
The A* machine is a generic machine that runs the A* search algorithm. The
A* machine is composed 3 components: A*Storage, A*Goal, and A*Map. This 
architecture allows for the creation of multiple A* machines for different
purposes.  

For example, an A* machine may find shortest paths on a NavMesh 
if provided with an A*Map wrapped around a NavMesh, and an A*Goal
that can estimate and measure distances between locations on a NavMesh.

A*Storage:
The A*Storage contains the Open and Closed lists. The implementaion of the 
lists is up to the derived A*Storage class. Lists may be linked lists, static
arrays or something else.

A*Goal:
The A*Goal determines when the algorithm has completed (successfully or 
unsuccessfully), and the estimated and actual distances between A*Nodes.

A*Map:
The A*Map determines how to correlate A* NodeIDs with locations on 
a map of any kind. The map holds flags with teh status of each A* node.

This architecture is based on the A* machine described in "Generic A* 
Pathfinding", AI Game Programming Wisdom, p.114

----------------------------------------------------------------------------*/

// 
// CAIAStarNodeAbstract
//

CAIAStarNodeAbstract::CAIAStarNodeAbstract()
{
	eAStarNodeID = kASTARNODE_Invalid;

	fGoal = 0.f;
	fHeuristic = 0.f;
	fFitness = FLT_MAX;

	pListPrev = NULL;
	pListNext = NULL;
	pAStarParent = NULL;
}

CAIAStarNodeAbstract::~CAIAStarNodeAbstract()
{
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// 
// CAIAStarMachine
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMachine::Con/Destructor
//              
//	PURPOSE:	Create/destroy the A* machine.
//              
//----------------------------------------------------------------------------

CAIAStarMachine::CAIAStarMachine()
{
	m_pAStarStorage = NULL;
	m_pAStarGoal = NULL;
	m_pAStarMap = NULL;

	m_pAStarNodeCur = NULL;

	m_eAStarNodeSource = kASTARNODE_Invalid;
	m_eAStarNodeDest = kASTARNODE_Invalid;
}

CAIAStarMachine::~CAIAStarMachine()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMachine::InitAStar
//              
//	PURPOSE:	Initialize the A* machine by setting the stoarge, goal, 
//              and map components.
//              
//----------------------------------------------------------------------------

void CAIAStarMachine::InitAStar( CAIAStarStorageAbstract* pAStarStorage, CAIAStarGoalAbstract* pAStarGoal, CAIAStarMapAbstract* pAStarMap )
{
	m_pAStarStorage = pAStarStorage;
	m_pAStarGoal = pAStarGoal;
	m_pAStarMap = pAStarMap;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMachine::SetAStarSource
//              
//	PURPOSE:	Set the A* machine's source node.
//              This resets the machine to start a new search.
//              
//----------------------------------------------------------------------------

void CAIAStarMachine::SetAStarSource( ENUM_AStarNodeID eAStarNodeSource )
{
	// Reset the storage for a new search.

	m_pAStarStorage->ResetAStarStorage();
	m_eAStarNodeSource = eAStarNodeSource;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMachine::SetAStarDest
//              
//	PURPOSE:	Set the A* machine's destination goal node.
//              
//----------------------------------------------------------------------------

void CAIAStarMachine::SetAStarDest( ENUM_AStarNodeID eAStarNodeDest )
{
	// Set the A* goal's destination.

	m_pAStarGoal->SetDestNode( eAStarNodeDest );
	m_eAStarNodeDest = eAStarNodeDest;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMachine::RunAStar
//              
//	PURPOSE:	Run the A* algorithm.
//
//  The A* Algorithm (from Artificial Intelligence: A New Synthesis, p. 144 ):
// 
//    g(n) = cost of best path from n0 to n.
//    h(n) = best guess (heuristic) of cost of path from n to a goal.
//    f(n) = g(n) + h(n) = cost of lowest-cost path to a goal - contrained
//           to go through node n.
//
//  1. Create a search graph G, consisting solely of the start node, n0. 
//     Put n0 on a list called OPEN.
//  2. Create a list called CLOSED that is initially empty.
//  3. If OPEN is empty, exit with failure.
//  4. Select the first node on OPEN, remove it from OPEN, and put it on CLOSED. 
//     Call this node n.
//  5. If n is a goal node, exit successfully with the solution obtained by tracing 
//     a path along the pointers from n to n0 in G. (The pointers define a search tree 
//     and are established in step 7).
//  6. Expand node n, generating the set M, of its successors that are not already 
//     ancestors of n in G. Install these members of M as successors of n in G.
//  7. Establish a pointer to n from each of those members of M that were not already 
//     in G (ie not already on either OPEN or CLOSED). Add these members of M to OPEN. 
//     For each member m of M that was already on OPEN or CLOSED, redirect its pointer 
//     to n if the best path to m found so far is through n. For each member of M already 
//     on CLOSED, redirect the pointers of each of its decendants in G so that they point 
//     backward along the best paths found so far to these descendants.
//  8. Reorder the list OPEN in order of increasing f values. (Ties among minimal f values 
//     are resolved in favor of the deepest node in the search tree).
//  9. Go to step 3.
//
//  NOTE: For step 7, we re-insert CLOSED nodes back into the OPEN list 
//        rather than re-directing the pointers of its decendants.
//        Less book keeping.
//             
//----------------------------------------------------------------------------

void CAIAStarMachine::RunAStar( CAI* pAI )
{
	// Sanity check.

	int cNeighbors, iNeighbor;
	CAIAStarNodeAbstract* pAStarNodeNeighbor;
	ENUM_AStarNodeID eAStarNeighborNode;
	unsigned long dwFlags = 0;
	float g, h, f;

	// Step 1:

	// Add source node to the Open list.
	// The source will be the only node in the Open list so far.

	m_pAStarNodeCur = m_pAStarStorage->CreateAStarNode( m_eAStarNodeSource );
	if( !m_pAStarNodeCur )
	{
		return;
	}
	m_pAStarStorage->AddToOpenList( m_pAStarNodeCur, m_pAStarMap );

	// Setup first node.

	m_pAStarMap->SetupPotentialNeighbor( NULL, m_pAStarNodeCur );

	// Set f, g, and h for source node.

	h = m_pAStarGoal->GetHeuristicDistance( m_pAStarNodeCur );
	m_pAStarNodeCur->fGoal = 0.f;
	m_pAStarNodeCur->fHeuristic = h;
	m_pAStarNodeCur->fFitness = h;


	// Step 2 is taken care of by the A* storage's ResetAStarStorage().

	// Infinite loop, broken when A* goal determines 
	// that the algorithm has terminated.

	while( true )
	{
		// Steps 3 and 4:

		// Remove the cheapest node found in the Open list,
		// and explore the node's neighbors.

		m_pAStarNodeCur = m_pAStarStorage->RemoveCheapestOpenNode();

		// Add the current node to the Closed list.

		if( m_pAStarNodeCur )
		{
			m_pAStarNodeCur->DebugPrintExpand();
			m_pAStarStorage->AddToClosedList( m_pAStarNodeCur, m_pAStarMap );
		}

		// Step 5:

		// The goal determines if A* is finished.
		// A* terminates when the goal has been reached, or
		// there are no more nodes to explore.

		if( m_pAStarGoal->IsAStarFinished( m_pAStarNodeCur ) )
		{
			break;
		}

		// Step 6:

		// Iterate over neighbors of the current node.

		cNeighbors = m_pAStarMap->GetNumAStarNeighbors( pAI, m_pAStarNodeCur );
		for( iNeighbor = 0; iNeighbor < cNeighbors; ++iNeighbor )
		{
			// Get the A* nodeID of the neighboring node.

			eAStarNeighborNode = m_pAStarMap->GetAStarNeighbor( pAI, m_pAStarNodeCur, iNeighbor, m_pAStarStorage );
			
			// No neighbor exists with this index.

			if( eAStarNeighborNode == kASTARNODE_Invalid )
			{
				continue;
			}

			// Step 7:

			// Get the A* flags for the neighboring node.

			dwFlags = m_pAStarMap->GetAStarFlags( eAStarNeighborNode );

			// The node is already open, so get it from the Open list.

			if( dwFlags & CAIAStarMapAbstract::kASTAR_Open )
			{
				pAStarNodeNeighbor = m_pAStarStorage->FindInOpenList( eAStarNeighborNode );
			}

			// The node is already closed, so get it from the Closed list.

			else if( dwFlags & CAIAStarMapAbstract::kASTAR_Closed )
			{
				pAStarNodeNeighbor = m_pAStarStorage->FindInClosedList( eAStarNeighborNode );
			}

			// The node is not on any of the lists, so create a new node with the 
			// neighbor's A* nodeID if the goal determines that it is passable.

			else if( m_pAStarGoal->IsAStarNodePassable( eAStarNeighborNode ) )
			{
				pAStarNodeNeighbor = m_pAStarStorage->CreateAStarNode( eAStarNeighborNode );
				pAStarNodeNeighbor->DebugPrintNeighbor();
			}

			// Neighbor is not passable.

			else {
				continue;
			}

			// Setup potential neighbor node.

			m_pAStarMap->SetupPotentialNeighbor( m_pAStarNodeCur, pAStarNodeNeighbor );

			// If our current best path is through this neighbor,
			// we do not need to re-assess the neighbor.
			if( m_pAStarNodeCur->pAStarParent == pAStarNodeNeighbor )
			{
				continue;
			}

			// Calculate new f, g, and h values for the neighbor node.

			g = m_pAStarNodeCur->fGoal;
			g += m_pAStarGoal->GetActualCost( pAI, m_pAStarNodeCur, pAStarNodeNeighbor );
			h = m_pAStarGoal->GetHeuristicDistance( pAStarNodeNeighbor );
			f = g + h;
		
			// Bail from processing this neighbor if it is more expensive to get
			// to this neighbor node from the current node than from its previous 
			// parent.

			
			// To prevent infinite loops caused by floating point error, 
			// f must be more than a floating point epsilon smaller than
			// the stored fitness.  This is because f will be higher precision
			// as it is stored in a registered, where as the stored fitness value
			// was stored as a float.
			// Just to be extra safe, we are using a value larger than FLT_EPSILON. 
			const float fPathingEpsilon = 1e-6f;  
			if( f >= pAStarNodeNeighbor->fFitness*(1. - fPathingEpsilon) )
			{
				continue;
			}

			// Assign new A* values to the neighbor node.

			pAStarNodeNeighbor->fGoal = g;
			pAStarNodeNeighbor->fHeuristic = h;
			pAStarNodeNeighbor->fFitness = g + h;

			// If the node was previously closed, remove it from the Closed list,
			// because it will be re-opened to re-explore its neighbors.

			if( dwFlags & CAIAStarMapAbstract::kASTAR_Closed )
			{
				m_pAStarStorage->RemoveFromClosedList( pAStarNodeNeighbor );
			}

			// Add neighbor node to the Open list, and set the neighbor's
			// parent as the current node.

			m_pAStarStorage->AddToOpenList( pAStarNodeNeighbor, m_pAStarMap );
			pAStarNodeNeighbor->pAStarParent = m_pAStarNodeCur;
			
			// Finalize neighbor node setup.

			m_pAStarMap->FinalizeNeighbor( pAStarNodeNeighbor );
		}

		// Step 8 is taken care of the A* storage.
		// The implemenation A* storage may choose to sort nodes
		// on insertion, or do a search for the cheapest node.
	}
}

//----------------------------------------------------------------------------
