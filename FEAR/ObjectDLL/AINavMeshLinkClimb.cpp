// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkClimb.cpp
//
// PURPOSE : AI NavMesh Link Climb class implementation.
//
// CREATED : 08/01/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkClimb.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIMovement.h"
#include "AIUtils.h"
#include "AINavMesh.h"
#include "AINavigationMgr.h"
#include "AIState.h"
#include "AIStateUseSmartObject.h"
#include "AIBlackBoard.h"

// WorldEdit

LINKFROM_MODULE( AINavMeshLinkClimb );

BEGIN_CLASS( AINavMeshLinkClimb )
	ADD_STRINGPROP_FLAG(SmartObject,	"Ladder",				0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
	ADD_STRINGPROP_FLAG(Object,			"",						0|PF_OBJECTLINK, "The name of the object that the AI is to activate.")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkClimb, AINavMeshLinkAbstract, 0, AINavMeshLinkClimbPlugin, "This link is used to specify that the brush contains something the AI must climb up and down" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkClimb )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkClimb, AINavMeshLinkAbstract )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkClimb::AINavMeshLinkClimb()
{
	m_hAnimObject = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINavMeshLinkClimb
//              
//----------------------------------------------------------------------------
void AINavMeshLinkClimb::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_STDSTRING(m_strObject);
	SAVE_HOBJECT(m_hAnimObject);
}

void AINavMeshLinkClimb::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_STDSTRING(m_strObject);
	LOAD_HOBJECT(m_hAnimObject);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	// Read the object to activate.

	const char* pszPropString = pProps->GetString( "Object", "" );
	if ( pszPropString[0] )
	{
		m_strObject = pszPropString;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::InitialUpdate
//              
//	PURPOSE:	Add the NavMeshLink to the NavMesh.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::InitialUpdate()
{
	super::InitialUpdate();

	// Cache a pointer to the animation object.

	if( !m_strObject.empty() )
	{
		HOBJECT hObj = NULL;
		FindNamedObject( m_strObject.c_str(), hObj, false );
		m_hAnimObject = hObj;
		m_strObject.clear();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::IsLinkRelevant( CAI* pAI )
{
	// Bail if no path set.

	if( pAI->GetAIMovement()->IsUnset() )
	{
		return false;
	}

	// Link is relevant to activate a goal if the AI is 
	// in position to get on or off the ladder.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	const LTVector& vDest = pAI->GetAIMovement()->GetDest();

	// AI is headed down the ladder.

	LTVector vPos = pAI->GetPosition();
	if( ( vDest.y <= pPoly->GetNMPolyAABB()->vMin.y ) &&
		( vPos.y >= m_fFloorTop ) )
	{
		if( IsInLinkOrOffsetEntry( pAI ) )
		{
			return true;
		}
	}


	// Bail if AI is not headed up the ladder.

	if( vDest.y < m_fFloorTop )
	{
		return false;
	}

	// Do not start the dismount animation until the AI is in position 
	// under the top of the link.

	LTVector vDir2D = pAI->GetPosition() - vDest;
	vDir2D.y = 0.f;
	if( vDir2D.MagSqr() > m_fEntryOffsetDistB * m_fEntryOffsetDistB )
	{
		return false;
	}

	// AI is headed up the ladder, and is ready to get off.

	if( ( pAI->GetCurrentNavMeshPoly() == m_eNMPolyID ) &&
		( vPos.y >= m_fFloorTop - m_fExitOffsetDistA ) )
	{
		return true;
	}

	// AI is headed up the ladder, and is ready to get off.
	// AI is coming from an offset entry.

	else if( ( IsInLinkOrOffsetEntry( pAI ) ) &&
			 ( vPos.y >= m_fFloorTop - m_fExitOffsetDistA ) )
	{
		return true;
	}

	// AI is not getting on or off the ladder.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::ActivateTraversal
//              
//	PURPOSE:	Make adjustments needed to activate the traversal of the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Sanity check.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !( pPoly && pAI && pStateUseSmartObject ) )
	{
		return;
	}

	// Set the object to animate.

	pAI->SetAnimObject( m_hAnimObject );


	LTVector vPos = pAI->GetPosition();

	LTVector vPolyN;
	g_pAINavMesh->GetNMPolyNormal( pPoly->GetNMNormalID(), &vPolyN );

	// AI is getting on the top of the ladder.

	const LTVector& vDest = pAI->GetAIMovement()->GetDest();
	if( vDest.y <= pPoly->GetNMPolyAABB()->vMin.y )
	{
		pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_GetOnLadder );
		pAI->GetAIBlackBoard()->SetBBFacePos( pPoly->GetNMPolyCenter() + ( vPolyN * 0.1f ) );
		pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(true);

		if( pAI->GetAIBlackBoard()->GetBBAwareness() >= kAware_Alert )
		{
			pStateUseSmartObject->SetProp( kAPG_Movement, kAP_MOV_Run );
		}
	}

	// AI is getting off the top of the ladder.

	else if( vDest.y >= m_fFloorTop )
	{
		pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_GetOffLadder );
		return;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::DeactivateTraversal
//              
//	PURPOSE:	Cleanup after deactivating the traversal of the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::DeactivateTraversal( CAI* pAI )
{
	super::DeactivateTraversal( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Ensure AI no longer thinks he is climbing something.

	pAI->GetAIBlackBoard()->SetBBEnteringNMLink( kNMLink_Invalid );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::IsTraversalComplete( CAI* pAI )
{
	// Sanity check.

	if( !( pAI->GetState() && pAI->GetState()->GetStateClassType() == kState_UseSmartObject ) )
	{
		return true;
	}

	// Traversal is complete when the animation finishes.
	
	if( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete )
	{
		return true;
	}

	// NavMesh Poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return true;
	}

	// Some AI jump down from the top of ladders.
	// In this case, the GetOnLadder animation is a JumpDown animation.
	// Traversal is complete if the AI was getting on the top
	// of the ladder, and his feet have dipped as low as the bottom
	// of the ladder NavMeshLink's poly.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	if( ( pStateUseSmartObject->GetProp( kAPG_Action ) == kAP_ACT_GetOnLadder ) &&
		( pAI->GetPosition().y - pAI->GetDims().y <= m_fFloorBottom + m_fExitOffsetDistB ) )
	{
		return true;
	}

	// Traversal is not complete.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::ApplyTraversalEffect
//              
//	PURPOSE:	Apply changes when finishing traversing the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::ApplyTraversalEffect( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear the object to animate.

	pAI->SetAnimObject( NULL );

	// Only apply effect if AI is using the link's SmartObject.

	if( !( pAI->GetState() && 
		pAI->GetState()->GetStateClassType() == kState_UseSmartObject ) )
	{
		return;
	}

	// If the AI is going down the ladder, make sure the next nav mesh 
	// link is invalidated to prevent the AI from restarting the animation.
	// This is required because SetBBNextNMLink is not cleared when the AI
	// aborts the use of a path.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	if ( pStateUseSmartObject->GetProp( kAPG_Action ) == kAP_ACT_GetOffLadder )
	{
		pAI->GetAIBlackBoard()->SetBBNextNMLink( kNMLink_Invalid );
	}

	// Unlock the animation when cutting it off before hitting the floor.

	if( ( pStateUseSmartObject->GetProp( kAPG_Action ) == kAP_ACT_GetOnLadder ) &&
		( pAI->GetPosition().y - pAI->GetDims().y <= m_fFloorBottom + m_fExitOffsetDistB ) )
	{
		pAI->GetAnimationContext()->Unlock();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::ModifyMovement
//              
//	PURPOSE:	Modify the new position as an AI moves.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::ModifyMovement( CAI* pAI, CAIMovement::State eStatePrev, LTVector* pvNewPos, CAIMovement::State* peStateCur )
{
	// Sanity check.

	if( !( pAI && pvNewPos ) )
	{
		return;
	}

	// If the AI is heading up the ladder, make sure he climbs from directly
	// below the top of the link, minus the offset.

	if( eStatePrev == CAIMovement::eStateSet )
	{
		// Bail if no poly exists.

		CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
		if( !pPoly )
		{
			return;
		}

		// Bail if not climbing up.

		const LTVector& vDest = pAI->GetAIMovement()->GetDest();
		if( ( pAI->GetPosition().y > pPoly->GetNMPolyAABB()->vMax.y ) ||
		    ( vDest.y != pPoly->GetNMPolyAABB()->vMax.y ) )
		{
			return;
		}

		// Prevent the AI from moving too far into the link.

		LTVector vDir2D = pAI->GetPosition() - vDest;
		vDir2D.y = 0.f;
		if( vDir2D.MagSqr() < m_fEntryOffsetDistB * m_fEntryOffsetDistB )
		{
			LTVector vOffset = vDest + ( m_vLinkDirXZ * m_fEntryOffsetDistB );
			pvNewPos->x = vOffset.x;
			pvNewPos->z = vOffset.z;
		}

		// Prevent the AI from moving above the height at which he should dismount the link.

		if( pvNewPos->y > m_fFloorTop - m_fExitOffsetDistA )
		{
			pvNewPos->y = m_fFloorTop - m_fExitOffsetDistA;
			pAI->GetAIBlackBoard()->SetBBSelectAction( true );

			// Keep the AI directly above the point he climbed/jumped from.

			LTVector vOffset = vDest + ( m_vLinkDirXZ * m_fEntryOffsetDistB );
			pvNewPos->x = vOffset.x;
			pvNewPos->z = vOffset.z;

			// Don't allow AI to finish his path until the traverse link goal
			// activates to get him off the top.

			if( pAI->GetAIMovement()->IsDone() )
			{
				*peStateCur = CAIMovement::eStateSet;
				pAI->GetAIBlackBoard()->SetBBAdvancePath( false );
			}
		}

		return;
	}

	// Handle modifing movement when not following a path.

	if( eStatePrev == CAIMovement::eStateSet )
	{
		return;
	}

	// Only affect movement if AI is using the link's SmartObject.

	if( !( pAI->GetState() && 
		   pAI->GetState()->GetStateClassType() == kState_UseSmartObject ) )
	{
		return;
	}

	// Do not let AI fall through the floor.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	if( ( pStateUseSmartObject->GetProp( kAPG_Action ) == kAP_ACT_GetOnLadder ) &&
		( pvNewPos->y - pAI->GetDims().y <= m_fFloorBottom + m_fExitOffsetDistB ) )
	{
		pvNewPos->y = m_fFloorBottom + pAI->GetDims().y;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::IsPullStringsModifying
//              
//	PURPOSE:	Function returns true when the pull string may modify the 
//				path, false when it promises not to.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::IsPullStringsModifying( CAI* pAI )
{
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::PullStrings
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos )
{
	// Sanity check.

	if( !pvNewPos )
	{
		return false;
	}

	// Bail if NavMesh poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return false;
	}

	// Bail if waypoint is not at the bottom of the ladder.

	if( vPtNext.y != pPoly->GetNMPolyAABB()->vMin.y )
	{
		return false;
	}

	// Adjust the height of the waypoint, and directly under the previous waypoint.

	*pvNewPos = vPtPrev;
	pvNewPos->y = m_fFloorBottom + m_fExitOffsetDistB;
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::HandleAdvancePath
//              
//	PURPOSE:	Return true if link has handled advancing the path.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::HandleAdvancePath( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if no path set.

	if( pAI->GetAIMovement()->IsUnset() )
	{
		return false;
	}

	// Bail if NavMesh poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return false;
	}

	// Do not advance the path if AI is going to climb up,
	// but is still below the top of the poly.
	// Reset the dest to the previous dest.

	const LTVector& vDest = pAI->GetAIMovement()->GetDest();
	if( ( vDest.y == pPoly->GetNMPolyAABB()->vMax.y ) &&
		( pAI->GetPosition().y < vDest.y ) )
	{
		pAI->GetAIMovement()->SetMovementDest( vDest );
		return true;
	}

	// Advance the path normally.

	return false; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::ApplyMovementAnimation
//              
//	PURPOSE:	Modify the AI's movement by changing the animation.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkClimb::ApplyMovementAnimation( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not heading to some destination.

	if( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
	{
		return;
	}

	// Bail if not in the ladder's poly.

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return;
	}

	// Bail if NavMesh poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return;
	}

	LTVector vPos = pAI->GetPosition();
	const LTVector& vDest = pAI->GetAIMovement()->GetDest();

	// AI is headed down the ladder.
	// Cut off the climb animation when AI reaches the floor.

	if( vDest.y <= pPoly->GetNMPolyAABB()->vMin.y )
	{
		if( vPos.y - pAI->GetDims().y <= m_fFloorBottom + m_fExitOffsetDistB )
		{
			pAI->GetAnimationContext()->ClearLock();
			return;
		}
	}

	// AI is headed up the ladder.

	else if( vDest.y == pPoly->GetNMPolyAABB()->vMax.y )
	{
		// Re-evaluate goals if we're at the top minus the offset.

		if( vPos.y >= m_fFloorTop - m_fExitOffsetDistA )
		{
			pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}

		// Cut off the climb animation at the top.

		if( vPos.y - pAI->GetDims().y >= m_fFloorTop )
		{
			pAI->GetAnimationContext()->ClearLock();

			// Clear the object to animate.

			pAI->SetAnimObject( NULL );
			return;
		}

		// Bail if AI is still walking toward the ladder.
		// The AI needs to walk into the link to get under 
		// the back edge, to climb vertically.

		LTVector vDir2D = vPos - vDest;
		vDir2D.y = 0.f;
		if( vDir2D.MagSqr() > m_fEntryOffsetDistB * m_fEntryOffsetDistB )
		{
			return;
		}
	}

	// Clear the action to prevent an AI from aiming or firing while climbing.
	// Do this after check for travellng up of down the ladder, so that we 
	// don't prevent an AI from firing while he is standing in the link at the 
	// bottom of the ladder after completing the climb down.

	pAI->GetAnimationContext()->SetProp( kAPG_Action, kAP_None );	

	// Bail if no SmartObject.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( !pSmartObject )
	{
		return;
	}

	// Set movement animation set in SmartObject.

	pAI->GetAnimationContext()->SetProp( kAPG_Activity, pSmartObject->Props.Get( kAPG_Activity ) );	

	// Modify the movement for the ascending or descending version.

	if( vDest.y <= pPoly->GetNMPolyAABB()->vMin.y )
	{
		pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_MDIR_Down );	
	}
	else 
	{
		pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_MDIR_Up );	

		// Set the object to animate.

		pAI->SetAnimObject( m_hAnimObject );

		// Face the ladder while climbing up.

		LTVector vPolyN;
		g_pAINavMesh->GetNMPolyNormal( pPoly->GetNMNormalID(), &vPolyN );
		pAI->GetAIBlackBoard()->SetBBFaceDir( -vPolyN );
		pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(true);
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
//	ROUTINE:	AINavMeshLinkClimb::GetNewPathClearEnteringLink
//              
//	PURPOSE:	If the AI is inside this link when its path changes, do not 
//				clear the Entering Link, as this will prevent the AI from 
//				continuing to use the ladder; this may result in the AI 
//				popping to the floor.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::GetNewPathClearEnteringLink( CAI* pAI )
{
	if( IsInLinkOrOffsetEntry( pAI ) 
		&& !FinishedClimbingDown( pAI ) )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::GetNewPathSourcePoly
//              
//	PURPOSE:	Allows the Climb link to override the initial poly used 
//			for pathing.  This is required when the AI repaths
//			while climbing up or down the ladder.
//              
//----------------------------------------------------------------------------

ENUM_NMPolyID AINavMeshLinkClimb::GetNewPathSourcePoly( CAI* pAI )
{
	if( IsInLinkOrOffsetEntry( pAI ) 
		&& !FinishedClimbingDown( pAI ) )
	{
		return m_eNMPolyID;
	}

	return kNMPoly_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkClimb::FinishedClimbingDown
//              
//	PURPOSE:	Returns true if the AI has already climbed down the 
//			ladder.  Returns false if the AIs path doesn't include
//			the ladder, the AI is not going down the ladder, or
//              	the AI has not yet reached the floor.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkClimb::FinishedClimbingDown( CAI* pAI ) const
{
	// Bail if NavMesh poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return true;
	}

	LTVector vPos = pAI->GetPosition();
	const LTVector& vDest = pAI->GetAIMovement()->GetDest();

	// AI is not heading down the ladder 

	if( vDest.y > pPoly->GetNMPolyAABB()->vMin.y )
	{
		return false;
	}

	// AI has not yet reached the floor 

	if( vPos.y - pAI->GetDims().y > m_fFloorBottom + m_fExitOffsetDistB )
	{
		return false;
	}

	// AI has reached the floor.

	return true;
}
