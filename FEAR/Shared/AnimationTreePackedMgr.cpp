// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePackedMgr.cpp
//
// PURPOSE : AnimationTreePackedMgr class implementation.
//           Manager for creating and accessing packed AnimTrees.
//
// CREATED : 6/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AnimationTreePackedMgr.h"
#include "AnimationTreePackedLoader.h"
#include "AnimationTreePacked.h"
#include "AnimationContext.h"


//
// Globals...
//

CAnimationTreePackedMgr* g_pAnimationTreePackedMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAnimationTreePackedMgr::CAnimationTreePackedMgr()
{
	m_iNextTreeID = 0;

	// Global transition IDs allow searching for transitions across multiple trees.

	m_iNextTransitionID = 0;
}

CAnimationTreePackedMgr::~CAnimationTreePackedMgr()
{
	TermAnimationTreePackedMgr();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::InitAnimationTreePackedMgr()
//
//	PURPOSE:	Initialize the packed AnimTree manager...
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePackedMgr::InitAnimationTreePackedMgr()
{
	// Set the global pointer...

	g_pAnimationTreePackedMgr = this;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::TermAnimationTreePackedMgr()
//
//	PURPOSE:	Terminate the packed AnimTree manager...
//
// ----------------------------------------------------------------------- //

void CAnimationTreePackedMgr::TermAnimationTreePackedMgr()
{
	// Delete all trees.

	CAnimationTreePacked* pTree;
	ANIM_TREE_PACKED_LIST::iterator itTree;
	for( itTree = m_lstAnimTrees.begin(); itTree != m_lstAnimTrees.end(); ++itTree )
	{
		pTree = *itTree;
		debug_delete( pTree );
	}
	m_lstAnimTrees.resize( 0 );

	// Clear the global pointer...

	g_pAnimationTreePackedMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::GetAnimationTreePacked
//
//	PURPOSE:	Return a pointer to the specified tree.
//
// ----------------------------------------------------------------------- //

CAnimationTreePacked* CAnimationTreePackedMgr::GetAnimationTreePacked( const char* pszFilename )
{
	// Sanity check.

	if( !pszFilename[0] )
	{
		return NULL;
	}

	// Return an existing tree.

	CAnimationTreePacked* pTree = FindAnimationTreePacked( pszFilename );
	if( pTree )
	{
		return pTree;
	}

	// Create a new tree.

	return CreateAnimationTreePacked( pszFilename );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::FindAnimationTreePacked
//
//	PURPOSE:	Return a pointer to the specified tree.
//
// ----------------------------------------------------------------------- //

CAnimationTreePacked* CAnimationTreePackedMgr::FindAnimationTreePacked( const char* pszFilename )
{
	// Find the specified tree.

	CAnimationTreePacked* pTree;
	ANIM_TREE_PACKED_LIST::iterator itTree;
	for( itTree = m_lstAnimTrees.begin(); itTree != m_lstAnimTrees.end(); ++itTree )
	{
		pTree = *itTree;
		if( LTStrIEquals( pszFilename, pTree->GetFilename() ) )
		{
			return pTree;
		}
	}

	// No tree found.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::CreateAnimationTreePacked
//
//	PURPOSE:	Return a pointer to the specified tree.
//
// ----------------------------------------------------------------------- //

CAnimationTreePacked* CAnimationTreePackedMgr::CreateAnimationTreePacked( const char* pszFilename )
{
	// Create a new tree.

	CAnimationTreePacked* pTree = debug_new( CAnimationTreePacked );

	// Bail if we fail to load the specified packed anim tree file.

	CAnimationTreePackedLoader Loader;
	if( !Loader.LoadAnimationTreePacked( pTree, pszFilename ) )
	{
		debug_delete( pTree );
		return NULL;
	}

	// Assign the new tree a unique ID.

	pTree->m_eTreeID = (AT_TREE_ID)m_iNextTreeID;
	++m_iNextTreeID;

	// Assign global transition IDs.
	// These IDs allow searching for transitions across multiple trees.

	AssignGlobalTransitionIDs( pTree );

	// Add the new tree to the list.

	m_lstAnimTrees.push_back( pTree );
	return pTree;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::AssignGlobalTransitionIDs
//
//	PURPOSE:	Assign global transition IDs to transitions in tree.
//
// ----------------------------------------------------------------------- //

void CAnimationTreePackedMgr::AssignGlobalTransitionIDs( CAnimationTreePacked* pTree )
{
	// Sanity check.

	if( !pTree )
	{
		return;
	}

	// Assign global IDs.
	// Global transition IDs allow searching for transitions across multiple trees.

	AT_TRANSITION* pTrans;
	for( uint32 iTrans=0; iTrans < pTree->GetNumTransitions(); ++iTrans )
	{
		pTrans = pTree->GetTransition( (AT_TRANSITION_ID)iTrans );
		if( pTrans )
		{
			pTrans->eGlobalTransitionID = GetGlobalTransitionID( pTrans->szName );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::GetGlobalTransitionID
//
//	PURPOSE:	Return the global transition ID for the transition name.
//
// ----------------------------------------------------------------------- //

AT_GLOBAL_TRANSITION_ID CAnimationTreePackedMgr::GetGlobalTransitionID( const char* pszName )
{
	// Return an existing index.

	GLOBAL_ANIM_TREE_TRANSITION* pTrans;
	GLOBAL_ANIM_TREE_TRANSITION_LIST::iterator itTrans;
	for( itTrans = m_lstGlobalTransitions.begin(); itTrans != m_lstGlobalTransitions.end(); ++itTrans )
	{
		pTrans = &( *itTrans );
		if( LTStrIEquals( pszName, pTrans->pszName ) )
		{
			return pTrans->eGlobalID;
		}
	}

	// Add a new index.

	GLOBAL_ANIM_TREE_TRANSITION TransID;
	TransID.pszName = pszName;
	TransID.eGlobalID = (AT_GLOBAL_TRANSITION_ID)m_iNextTransitionID;
	m_lstGlobalTransitions.push_back( TransID );
	++m_iNextTransitionID;

	return TransID.eGlobalID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedMgr::FindTransition
//
//	PURPOSE:	Return true if a transition can be found.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePackedMgr::FindTransition( const ANIM_TREE_INDEX &IndexAnimationFrom, 
											  const ANIM_TREE_INDEX &IndexAnimationTo, 
											  const ANIM_TREE_PACKED_LIST &lstAnimTreePacked, 
											  TRANS_QUERY_RESULTS &rTransResults ) const
{
	// Never play a transition if the From and To are the same animation.

	if( ( IndexAnimationFrom.iAnimation == IndexAnimationTo.iAnimation ) &&
		( IndexAnimationFrom.iAnimTree == IndexAnimationTo.iAnimTree ) )
	{
		return false;
	}

	// Tree index out of range.

	uint32 cTrees = lstAnimTreePacked.size();
	if( ( IndexAnimationFrom.iAnimTree >= cTrees ) ||
		( IndexAnimationTo.iAnimTree >= cTrees ) )
	{
		return false;
	}

	// Fail if trees are missing.

	CAnimationTreePacked* pTreeFrom = lstAnimTreePacked[IndexAnimationFrom.iAnimTree];
	CAnimationTreePacked* pTreeTo   = lstAnimTreePacked[IndexAnimationTo.iAnimTree];
	if( !( pTreeFrom && pTreeTo ) )
	{
		return false;
	}

	// Fail if animations are missing.

	AT_ANIMATION_ID eAnimFrom = (AT_ANIMATION_ID)IndexAnimationFrom.iAnimation;
	AT_ANIMATION_ID eAnimTo = (AT_ANIMATION_ID)IndexAnimationTo.iAnimation;
	AT_ANIMATION* pAnimFrom = pTreeFrom->GetAnimation( eAnimFrom );
	AT_ANIMATION* pAnimTo = pTreeTo->GetAnimation( eAnimTo );
	if( !( pAnimFrom && pAnimTo ) )
	{
		return false;
	}

	// Search for a transition listed as an Out of the From and an In of the To.

	AT_TRANSITION* pTransIn;
	AT_TRANSITION* pTransOut;
	AT_TRANSITION_SET* pSetOut = pAnimFrom->pTransitionSetOut;
	AT_TRANSITION_SET* pSetIn = pAnimTo->pTransitionSetIn;
	if( pSetOut && pSetIn )
	{
		uint32 iIn;
		for( uint32 iOut=0; iOut < pSetOut->cTransitions; ++iOut )
		{
			pTransOut = pSetOut->aTransitions[iOut];
			for( iIn=0; iIn < pSetIn->cTransitions; ++iIn )
			{
				pTransIn = pSetIn->aTransitions[iIn];
				if( pTransOut && pTransIn && 
					pTransOut->eGlobalTransitionID == pTransIn->eGlobalTransitionID )
				{
					rTransResults.Index.iAnimTree = IndexAnimationFrom.iAnimTree;
					rTransResults.Index.iAnimation = pTransOut->eTransitionID;
					rTransResults.pszName = pTransOut->szName;
					rTransResults.BlendData = pTransOut->BlendData;
					pTreeFrom->GetTransitionDescriptors( pTransOut->eTransitionID, rTransResults.Descriptors );

					return true;
				}
			}
		}
	}

	// No match found between the Out and In animations, so look
	// for default transitions.

	if( pAnimFrom->pDefaultTransitionOut )
	{
		pTransOut = pAnimFrom->pDefaultTransitionOut;
		rTransResults.Index.iAnimTree = IndexAnimationFrom.iAnimTree;
		rTransResults.Index.iAnimation = pTransOut->eTransitionID;
		rTransResults.pszName = pTransOut->szName;
		rTransResults.BlendData = pTransOut->BlendData;
		pTreeFrom->GetTransitionDescriptors( pTransOut->eTransitionID, rTransResults.Descriptors );
		return true;
	}

	if( pAnimTo->pDefaultTransitionIn )
	{
		pTransIn = pAnimTo->pDefaultTransitionIn;
		rTransResults.Index.iAnimTree = IndexAnimationTo.iAnimTree;
		rTransResults.Index.iAnimation = pTransIn->eTransitionID;
		rTransResults.pszName = pTransIn->szName;
		rTransResults.BlendData = pTransIn->BlendData;
		pTreeTo->GetTransitionDescriptors( pTransIn->eTransitionID, rTransResults.Descriptors );
		return true;
	}

	// No match found.

	return false;
}

