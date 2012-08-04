// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkJumpOver.cpp
//
// PURPOSE : AI NavMesh Link JumpOver class implementation.
//
// CREATED : 08/15/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

// Includes required for AINavMeshLinkJumpOver.h

#include "AINavMeshLinkAbstract.h"
#include "AINavMeshLinkJumpOver.h"

// Includes required for AINavMeshLinkJumpOver.cpp

#include "AI.h"
#include "AIBlackBoard.h"
#include "AIMovement.h"
#include "AINavMesh.h"
#include "AINavigationMgr.h"
#include "AIState.h"
#include "AIUtils.h"
#include "AnimationContext.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkJumpOver );

BEGIN_CLASS( AINavMeshLinkJumpOver )
	ADD_STRINGPROP_FLAG(SmartObject,	"JumpOver",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkJumpOver, AINavMeshLinkAbstract, 0, AINavMeshLinkJumpOverPlugin, "This link is used to specify that the brush contains something the AI must jump over" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkJumpOver )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkJumpOver, AINavMeshLinkAbstract )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkJumpOver::AINavMeshLinkJumpOver()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJumpOver::ReadProp( const GenericPropList *pProps )
{
	super::ReadProp( pProps );

	m_vPos = pProps->GetVector( "Pos", LTVector( 0.0f, 0.0f, 0.0f ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINavMeshLinkJumpOver
//              
//----------------------------------------------------------------------------
void AINavMeshLinkJumpOver::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vPos);
}

void AINavMeshLinkJumpOver::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vPos);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkJumpOver::IsLinkRelevant( CAI* pAI )
{
	// AI is not currently standing in this link.

///	if( pAI->GetCurrentNavMeshLink() != m_eNMLinkID )
	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	// AI needs to jump.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::ActivateTraversal
//              
//	PURPOSE:	Make adjustments needed to activate the traversal of the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJumpOver::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Sanity check.

	if( !( pAI && pStateUseSmartObject ) )
	{
		return;
	}

	// Head to a point slightly beyond the edge of the NavMeshLink's poly.
	// This ensures that the AI jumps out of the link.

	CAIMovement* pAIMovement = pAI->GetAIMovement(); 
	LTVector vDest = pAIMovement->GetDest();

	LTVector vDir = vDest - pAI->GetPosition();
	vDir.y = 0.f;
	vDir.Normalize();
	vDest += vDir * 20.f;

	pAIMovement->SetMovementDest( vDest );
	pAI->GetAIBlackBoard()->SetBBFacePos( vDest );

	// The height of the parabola is determines by the position of the NavMeshLink.

	pAIMovement->SetParabola( m_vPos.y - ( pAI->GetPosition().y - pAI->GetDims().y ) );


	// If we aren't facing the true direction, the AIMovements jump timing will 
	// be incorrect and will miss the landing.
	pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(true);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkJumpOver::IsTraversalComplete( CAI* pAI )
{
	// Traversal is complete if we have reached the dest.

	if( pAI->GetAIMovement()->IsDone() )
	{
		return true;
	}

	// Traversal is not complete.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::ApplyTraversalEffect
//              
//	PURPOSE:	Apply changes when finishing traversing the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJumpOver::ApplyTraversalEffect( CAI* pAI )
{
	pAI->GetAnimationContext()->ClearOverrideAnimRate();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkJumpOver::ApplyMovementAnimation
//              
//	PURPOSE:	Modify the AI's movement by changing the animation.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkJumpOver::ApplyMovementAnimation( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// The JumpFly animation has just started.

	if( ( pAI->GetAIMovement()->GetCurrentMovementType() == kAD_MOV_JumpOver ) &&
		( pAI->GetAIMovement()->GetLastMovementType() != kAD_MOV_JumpOver ) )
	{
		// Find the horizontal distance to the dest.

		LTVector vPos = pAI->GetPosition();
		LTVector vDest = pAI->GetAIMovement()->GetDest();
		vDest.y = 0.f;
		vPos.y = 0.f;

		float fDist = vPos.Dist( vDest );

		// Calculate the time it will take to cover this distance.

		float fJumpTime = fDist / pAI->GetJumpOverSpeed();

		// Find the length of the fly animation.

		float fAnimLength = pAI->GetAnimationContext()->GetCurAnimationLength();

		// Check for several situations which would result in an invalid 
		// frame rate.

		if ( fDist == 0.0 
			|| pAI->GetJumpOverSpeed() == 0.0f 
			|| fJumpTime <= 0.0f
			|| fAnimLength <= 0.0f )
		{
			AIASSERT( 0, m_hObject, "AINavMeshLinkJumpOver::ApplyMovementAnimation: Invalid jumping attempt." );
			return;
		}

		// Calculate how fast to play the fly animation.

		float fAnimRate = fAnimLength / fJumpTime;
		pAI->GetAnimationContext()->SetOverrideAnimRate( fAnimRate );
	}
}
