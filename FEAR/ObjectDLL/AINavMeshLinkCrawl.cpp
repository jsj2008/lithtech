// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkCrawl.cpp
//
// PURPOSE : AI NavMesh Link Crawl class implementation.
//
// CREATED : 09/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkCrawl.h"
#include "AI.h"
#include "AIDB.h"
#include "AIMovement.h"
#include "AINavMesh.h"
#include "AnimationContext.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkCrawl );

BEGIN_CLASS( AINavMeshLinkCrawl )
	ADD_STRINGPROP_FLAG(SmartObject,	"Crawl",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkCrawl, AINavMeshLinkAbstract, 0, AINavMeshLinkCrawlPlugin, "This link is used to specify that the brush contains a space the AI must crawl through" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkCrawl )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkCrawl, AINavMeshLinkAbstract )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkCrawl::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkCrawl::AINavMeshLinkCrawl()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkCrawl::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkCrawl::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Update the dims to account for posture changes.

	pAI->GetAIBlackBoard()->SetBBUpdateDims( true );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkCrawl::IsTraversalInProgress
//              
//	PURPOSE:	Return true if traversal of the link is in progress.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkCrawl::IsTraversalInProgress( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	return !pAI->GetAIBlackBoard()->GetBBInvalidatePlan();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkCrawl::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkCrawl::IsTraversalComplete( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return true;
	}

	if( pAI->GetAnimationContext()->IsTransitioning() )
	{
		return false;
	}

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkCrawl::HandleNavMeshLinkExit
//              
//	PURPOSE:	Handle exiting the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkCrawl::HandleNavMeshLinkExit( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear locked animation.

	pAI->GetAnimationContext()->ClearLock();

	// Update the dims to account for posture changes.

	pAI->GetAIBlackBoard()->SetBBUpdateDims( true );

	// Force a re-plan.

	pAI->GetAIBlackBoard()->SetBBInvalidatePlan( true );
}













