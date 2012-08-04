// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkStairs.cpp
//
// PURPOSE : AI NavMesh Link Stairs class implementation.
//
// CREATED : 07/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkStairs.h"
#include "AI.h"
#include "AIDB.h"
#include "AIMovement.h"
#include "AIUtils.h"
#include "AnimationContext.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkStairs );

BEGIN_CLASS( AINavMeshLinkStairs )
	ADD_STRINGPROP_FLAG(SmartObject,	"Stairs",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkStairs, AINavMeshLinkAbstract, 0, AINavMeshLinkStairsPlugin, "This link is used to specify that the brush contains a staircase" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkStairs )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkStairs, AINavMeshLinkAbstract )


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkStairs::AINavMeshLinkStairs()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::ApplyMovementAnimation
//              
//	PURPOSE:	Modify the AI's movement by changing the animation.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkStairs::ApplyMovementAnimation( CAI* pAI )
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

	// Bail if the AI is not moving straight through the link, as the 
	// animations are designed to support perpendicular movement, not 
	// diagonal movement.  By bailing here, the AI will treat stairs 
	// as normal nav mesh and will pop -- this will not break the game 
	// however.

	// This is a bit of a hack; we have an optimization which doesn't pop 
	// the AI to the ground continuously.  When an AI is on stairs and not 
	// stair stepping, we DO want to glue him to the ground.  This insures 
	// the AI doesn't float if he plays a movement encoded attack (for 
	// instance) while on the stairs.  At this is the case, we want to do 
	// this BEFORE bailing due to the AI not having a path.

	bool bForceToGround = false;
	LTVector vToDest2DNorm = pAI->GetAIMovement()->GetDest() - pAI->GetPosition();
	vToDest2DNorm.y = 0.0f;
	if (vToDest2DNorm == LTVector::GetIdentity())
	{
		bForceToGround = true;
	}
	else
	{
		vToDest2DNorm.Normalize();

		LTVector vEdge2DNorm = m_vLinkEdgeA0 - m_vLinkEdgeA1;
		vEdge2DNorm.y = 0.0f;
		vEdge2DNorm.Normalize();

		float flAngle = vEdge2DNorm.Dot( vToDest2DNorm );
		if ( fabs(flAngle) > 0.01f )
		{
			bForceToGround = true;
		}
	}

	if ( bForceToGround )
	{
		pAI->SetForceGround( true );
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

	// Modify the movement for the ascending or descending version.

	float fCurHeight = pAI->GetPosition().y - pAI->GetDims().y;
	const LTVector& vDest = pAI->GetAIMovement()->GetDest();
	if( vDest.y == m_vMidPtLinkEdgeB.y )
	{
		pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_MDIR_Down );	
	}
	else {
		pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_MDIR_Up );	
	}

	// Lock the animation for one cycle.
	// Clear the lock between cycles to allow repeatedly playing
	// the same locked animation.

	if( pAI->GetAnimationContext()->WasLocked() )
	{
		pAI->GetAnimationContext()->ClearLock();
		pAI->GetAnimationContext()->DoInterpolation( false );
	}
	pAI->GetAnimationContext()->Lock();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::HandleNavMeshLinkExit
//              
//	PURPOSE:	Handle exiting the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkStairs::HandleNavMeshLinkExit( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear locked animation.

		pAI->GetAnimationContext()->ClearLock();
	}













