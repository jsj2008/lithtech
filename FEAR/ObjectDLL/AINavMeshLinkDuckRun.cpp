// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDuckRun.cpp
//
// PURPOSE : AI NavMesh Link DuckRun class implementation.
//
// CREATED : 02/10/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkDuckRun.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkDuckRun );

BEGIN_CLASS( AINavMeshLinkDuckRun )
	ADD_STRINGPROP_FLAG(SmartObject,	"DuckRun",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkDuckRun, AINavMeshLinkAbstract, 0, AINavMeshLinkDuckRunPlugin, "This link is used to specify that the AI must duck and run to traverse the associated brush" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkDuckRun )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkDuckRun, AINavMeshLinkAbstract )


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDuckRun::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkDuckRun::AINavMeshLinkDuckRun()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDuckRun::ApplyMovementAnimation
//              
//	PURPOSE:	Modify the AI's movement by changing the animation.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDuckRun::ApplyMovementAnimation( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not in the link.

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return;
	}

	// Bail if no path set.

	if( pAI->GetAIMovement()->IsUnset() )
	{
		return;
	}

	// Bail if no SmartObject.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( !pSmartObject )
	{
		return;
	}

	// Set movement animation set in SmartObject.

	pAI->GetAnimationContext()->SetProp( kAPG_Activity, pSmartObject->Props.Get( kAPG_Activity ) );	
	pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, pSmartObject->Props.Get( kAPG_MovementDir ) );	
}

