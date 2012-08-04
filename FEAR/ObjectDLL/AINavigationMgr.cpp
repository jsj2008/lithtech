// ----------------------------------------------------------------------- //
//
// MODULE  : AINavigationMgr.cpp
//
// PURPOSE : AINavigationMgr abstract class implementation
//
// CREATED : 2/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavigationMgr.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AIPathMgrNavMesh.h"
#include "AIGoalMgr.h"
#include "AIMovement.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "DebugLineSystem.h"
#include "AIWorkingMemory.h"
#include "DebugLineSystem.h"
#include "AIQuadTree.h"
#include "iperformancemonitor.h"

extern VarTrack g_ShowAIPathTrack;

// Performance monitoring.
CTimedSystem g_tsAINavigation("AINavigation", "AI");


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAINavigationMgr::CAINavigationMgr()
{
	m_pAI = NULL;

	m_bNavSet = false;

	m_pNMPath = AI_FACTORY_NEW( CAIPathNavMesh );
	m_bDrawingPath = false;

	m_nPathMgrKnowledgeIndex = g_pAIPathMgrNavMesh->GetPathKnowledgeIndex() - 1;
}

CAINavigationMgr::~CAINavigationMgr()
{
	AI_FACTORY_DELETE( m_pNMPath );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavigationMgr::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAINavigationMgr 
//              
//----------------------------------------------------------------------------

void CAINavigationMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);
	SAVE_VECTOR(m_vDest);
	SAVE_bool(m_bNavSet);
	m_pNMPath->Save(pMsg);
	SAVE_bool(m_bDrawingPath);
	SAVE_INT(m_nPathMgrKnowledgeIndex);
}

void CAINavigationMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);
	LOAD_VECTOR(m_vDest);
	LOAD_bool(m_bNavSet);
	m_pNMPath->Load(pMsg);
	LOAD_bool(m_bDrawingPath);
	LOAD_INT(m_nPathMgrKnowledgeIndex);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::InitNavigationMgr
//
//	PURPOSE:	Initialize the NavigationMgr.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::InitNavigationMgr( CAI* pAI )
{
	m_pAI = pAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::IsPathValid
//
//	PURPOSE:	Returns true if a new path should be generated, otherwise
//				returns false.
//
// ----------------------------------------------------------------------- //

bool CAINavigationMgr::IsPathValid(const LTVector& vDest) const
{
	// Has the global path index has been invalidated since our path was generated?

	if (m_nPathMgrKnowledgeIndex != g_pAIPathMgrNavMesh->GetPathKnowledgeIndex())
	{
		return false;
	}



	// A new path was requested.

	if (m_pAI->GetAIBlackBoard()->GetBBInvalidatePath())
	{
		m_pAI->GetAIBlackBoard()->SetBBInvalidatePath(false);
		return false;
	}
	
	// Pathing is dynamic, and destination changed enough to 
	// require a new path.

	if ( m_pAI->GetAIBlackBoard()->GetBBDestIsDynamic() )
	{
		float flDistanceDiffSqr = (m_vDest-vDest).MagSqr();
		if (flDistanceDiffSqr >= m_pAI->GetAIBlackBoard()->GetBBDestRepathDistanceSqr())
		{
			return false;
		}
	}
	
	// Pathing is not dynamic, and destination changed.

	else if (m_vDest != vDest)
	{
		return false;
	}

	// Path is still valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::SetPath
//
//	PURPOSE:	Sets up the AI with a new path to the passed in 
//				destination.  Returns true if a path is found, false if a 
//				path is not found.
//
// ----------------------------------------------------------------------- //

bool CAINavigationMgr::SetPath( const LTVector& vDest )
{
	//
	// Before altering any of the state, evaluate the current state to 
	// determine how to behave.
	//

	// If we have a link, allow the link to set the source poly.  This is
	// required by the climb link.  When an AI uses a climb link, he may 
	// not actually be inside of it.  This may result in the AI breaking
	// if its path is invalidated while still on the link; if the AIs 
	// current poly is not the climbs poly, the AI won't realize it needs 
	// to get off the ladder before moving to the next location.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !( pLink && pLink->IsNMLinkActiveToAI( m_pAI ) ) )
	{
		pLink = NULL;
	}

	// Find the dest start poly.

	ENUM_NMPolyID ePolySource = kNMPoly_Invalid;
	if ( pLink )
	{
		ePolySource = pLink->GetNewPathSourcePoly( m_pAI );
	}
	if ( kNMPoly_Invalid == ePolySource )
	{
		ePolySource = g_pAIQuadTree->GetContainingNMPoly( m_pAI->GetPosition(), m_pAI->GetCharTypeMask(), m_pAI->GetLastNavMeshPoly(), m_pAI );
	}

	// Find the dest NMPoly.

	ENUM_NMPolyID ePolyDest = g_pAIQuadTree->GetContainingNMPoly( vDest, m_pAI->GetCharTypeMask(), kNMPoly_Invalid, m_pAI );

	// By default, clear the entering link.  Allow a link to prevent this from
	// being cleared; this is required by the climb link, as if the path 
	// changes while the AI is in the link, the AI will not continue to use 
	// the ladder as movement modification depends on the entering link.  This 
	// occurs because the AI is not required to be inside the climb links poly
	// to climb the ladder.

	if ( !pLink || pLink->GetNewPathClearEnteringLink( m_pAI ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBEnteringNMLink( kNMLink_Invalid );
	}

	m_vDest = vDest;
	m_bNavSet = true;

	// Clear out knowledge regarding paths

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_PathInfo);
	m_pAI->GetAIWorkingMemory()->ClearWMFact(factQuery);

	ENUM_AI_PATH_TYPE ePathType = m_pAI->GetAIBlackBoard()->GetBBPathType();
	uint32 nPullStringsMaxIters = g_pAIDB->GetAIConstantsRecord()->nStringPullingMaxIterations;
	if ( !g_pAIPathMgrNavMesh->FindPath( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vDest, ePolySource, ePolyDest, nPullStringsMaxIters, ePathType, m_pNMPath ) )
	{
		// Failed to find a path.

		m_pNMPath->ReserveNavMeshLinks( m_pAI, false );
		m_pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Failed );
		m_bNavSet = false;
		
		m_nPathMgrKnowledgeIndex = g_pAIPathMgrNavMesh->GetPathKnowledgeIndex() - 1;

		return false;
	}

	m_nPathMgrKnowledgeIndex = g_pAIPathMgrNavMesh->GetPathKnowledgeIndex();

	// Path found.

	m_pAI->GetAIBlackBoard()->SetBBInvalidatePath(false);
	m_pNMPath->ReserveNavMeshLinks( m_pAI, true );
	m_pAI->GetAIMovement()->ClearMovement();
	m_bDrawingPath = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::UpdateNavigation
//
//	PURPOSE:	Update the NavigationMgr.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::UpdateNavigation()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAINavigation);

	// Update debug path rendering.

	UpdateDebugRendering( g_ShowAIPathTrack.GetFloat() );

	// No destination set to navigate to.

	if( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
	{
		m_bNavSet = false;
		return;
	}

	// Get the last requested destination from the BlackBoard.

	LTVector vDest = m_pAI->GetAIBlackBoard()->GetBBDest();

	// See if a new path should be set.

	if( ( !m_bNavSet || !IsPathValid(vDest) ) && 
		( !m_pAI->GetAnimationContext()->IsLocked() ) )
	{
		// Bail if no path found to new dest.

		if( !SetPath( vDest ) )
		{
			return;
		}
	}

	// We have reached the destination.
	// Kill any movement animation.

	if( m_pAI->GetPosition() == vDest )
	{
		SetNavDone();
		return;
	}

	// Update the actual navigation of the path.

	if (m_pAI->GetAIBlackBoard()->GetBBAdvancePath())
	{
		// Movement is complete.  Move to the next node.

		AdvancePath();
		m_pAI->GetAIBlackBoard()->SetBBAdvancePath(false);
	}
	else if (m_pAI->GetAIMovement()->IsUnset())
	{
		// Movement is unset, but path is not complete.  Move to the current node.

		m_pNMPath->SkipOptionalPathNodes( m_pAI );

		if( m_pNMPath->IsPathComplete() )
		{
			SetNavDone();
			return;
		}

		SPATH_NODE* pPathNode = m_pNMPath->GetCurPathNode();
		AIASSERT(pPathNode, m_pAI->GetHOBJECT(), "CAINavigationMgr::UpdateNavigation : Path is unset, and waypoint is NULL.");
		if( pPathNode )
		{
			MoveToPathNode(pPathNode);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::SetNavDone
//
//	PURPOSE:	Clear pathfinding when navigation is complete.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::SetNavDone()
{
	m_pNMPath->ReserveNavMeshLinks( m_pAI, false );
	m_pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Done );
	m_bNavSet = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::HandleAIDeath
//
//	PURPOSE:	Handle an AI dying.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::HandleAIDeath()
{
	// Other AI prefer not to use links along last path
	// of AI who died.

	m_pNMPath->ApplyNMLinkDeathDelay( m_pAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::AdvancePath
//
//	PURPOSE:	Advances the path to the next waypoint.
//
// ----------------------------------------------------------------------- //
void CAINavigationMgr::AdvancePath()
{
	// Advance the waypoints.

	m_pNMPath->SkipOptionalPathNodes( m_pAI );

	// Path is complete.

	if( m_pNMPath->IsPathComplete() )
	{
		SetNavDone();
		return;
	}

	// Go to the next path point.

	SPATH_NODE* pPathNode = m_pNMPath->GetCurPathNode();
	if( pPathNode )
	{
		// Special handling for entering/exiting NavMeshLinks.

		SPATH_NODE* pPrevNode = m_pNMPath->GetPathNode( m_pNMPath->GetCurPathNodeIndex() - 1 );
		if( pPrevNode )
		{
			// Link can override advancing of path.

			CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pPrevNode->ePoly );
			if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
			{
				AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
				if( pLink && pLink->HandleAdvancePath( m_pAI ) )
				{
					return;
				}
			}

			// Re-evaluate goals immediately if we just arrived
			// at a Link's offset entry position.

			if( pPrevNode->eOffsetEntryToLink != kNMLink_Invalid )
			{
				m_pAI->GetAIBlackBoard()->SetBBEnteringNMLink( pPrevNode->eOffsetEntryToLink );
				m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			}
			else {
				m_pAI->GetAIBlackBoard()->SetBBEnteringNMLink( kNMLink_Invalid );
			}
		}

		// Advance to the next dest on path.

		MoveToPathNode(pPathNode);
		m_pNMPath->IncrementCurPathNodeIndex( m_pAI );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::MoveToPathNode
//
//	PURPOSE:	Handles instructing the AI to move to the passed in node.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::MoveToPathNode(SPATH_NODE* pPathNode)
{
	if (!pPathNode)
	{
		AIASSERT(0, m_pAI->m_hObject, "CAINavigationMgr::MoveToPathNode : Invalid point pointer.");
		return;
	}

	// Set the movement destination.

	m_pAI->GetAIMovement()->SetMovementDest( pPathNode->vWayPt );

	// Keep track of the direction to the next path dest,
	// and the direction from the next path dest to the
	// path dest after that.
 
	LTVector vDest = pPathNode->vWayPt;
	LTVector vDirToDest = vDest - m_pAI->GetPosition();
	if( vDirToDest != LTVector::GetIdentity() )
	{
		vDirToDest.Normalize();
	}
	m_pAI->GetAIBlackBoard()->SetBBDirToPathDest( vDirToDest );

	// Next node in the path.

	SPATH_NODE* pNextPathNode = m_pNMPath->GetPathNode(m_pNMPath->GetCurPathNodeIndex()+1);
	if( pNextPathNode )
	{
		vDirToDest = pNextPathNode->vWayPt - vDest;
		if( vDirToDest != LTVector::GetIdentity() )
		{
			vDirToDest.Normalize();
		}
		m_pAI->GetAIBlackBoard()->SetBBDirToNextPathDest( vDirToDest );
	}
	else {
		m_pAI->GetAIBlackBoard()->SetBBDirToNextPathDest( LTVector( 0.f, 0.f, 0.f ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINavigationMgr::UpdateNavigationAnimation
//
//	PURPOSE:	Update the animation corresponding to the navigation.
//
// ----------------------------------------------------------------------- //

void CAINavigationMgr::UpdateNavigationAnimation()
{
	// Keep track of posture changes.

	EnumAnimProp ePosture = m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
	m_pAI->GetAIBlackBoard()->SetBBPosture( ePosture );

	// Get the next Link to cross.
	// It may be inactive.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !( pLink && pLink->IsNMLinkActiveToAI( m_pAI ) ) )
	{
		pLink = NULL;
	}

	// No destination set to navigate to.

	if( ( m_pAI->GetAIBlackBoard()->GetBBDestStatus() == kNav_Unset ) ||
		( m_pAI->GetAIBlackBoard()->GetBBDestStatus() == kNav_Failed ) )
	{
		// Do not stomp a locked animation.
		// (e.g. animation set by CAIStateUseSmartObject).

		if( ( m_pAI->GetAnimationContext()->GetProp( kAPG_Movement ) != kAP_None ) &&
			( !m_pAI->GetAnimationContext()->IsLocked() ) )
		{
			m_pAI->GetAnimationContext()->SetProp( kAPG_Movement, kAP_None );
			m_pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_None );
		}
	}

	else {
		//
		// Select an appropriate movement animation.
		//

		static CAnimationProps Props;
		Props.Clear();
		SelectMovementAnimProps( &Props );
		
		// AI is firing.

		if( ( Props.Get( kAPG_Action ) == kAP_ACT_Fire ) &&
			( m_pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID() != kAIWeaponID_Invalid ) )
		{
			// Run forward by default.

			Props.Set( kAPG_MovementDir, kAP_MDIR_Forward );

			// Keep the target in view.

			if( ( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) ) &&
				( m_pAI->GetAIBlackBoard()->GetBBAllowDirectionalRun() ) &&
				( !m_pAI->GetAnimationContext()->IsLocked() ) &&
				( !( pLink && pLink->IsInLinkOrOffsetEntry( m_pAI ) && !pLink->GetAllowDirectionalMovement()) ) )
			{
				// Adjust run animation  
				// 1) The weapon allows it and
				// 2) Either the weapon has ammo in its clip, or the weapon allows automatic reloading.

				const AIDB_AIWeaponRecord* pRecord = g_pAIDB->GetAIWeaponRecord(
					m_pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());

				SAIWORLDSTATE_PROP* pWeaponLoadedProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponLoaded );

				if ( pRecord 
					&& pWeaponLoadedProp
					&& pRecord->bAllowDirectionalRun
					&& ( pWeaponLoadedProp->bWSValue || pRecord->bAllowAutoReload ) )
				{
					EnumAnimProp eMovementDir = SelectMovementDirection();
					Props.Set( kAPG_MovementDir, eMovementDir );
				}
			}
		}

		// Set selected movement animation, unless someone has already set something other than walk.
		
		if( m_pAI->GetAnimationContext()->GetProp( kAPG_Movement ) == kAP_MOV_Walk )
		{
			m_pAI->GetAnimationContext()->SetProp( kAPG_Movement, Props.Get( kAPG_Movement ) );
		}
		if( Props.Get( kAPG_MovementDir ) != kAP_None )
		{
			m_pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, Props.Get( kAPG_MovementDir ) );
		}
		if( Props.Get( kAPG_Activity ) != kAP_None )
		{
			m_pAI->GetAnimationContext()->SetProp( kAPG_Activity, Props.Get( kAPG_Activity ) );
		}
		if( Props.Get( kAPG_Action ) != kAP_None )
		{
			m_pAI->GetAnimationContext()->SetProp( kAPG_Action, Props.Get( kAPG_Action ) );
		}
	}


	// Allow NavMeshLink to modify the movement animation.

	if( pLink )
	{
		pLink->ApplyMovementAnimation( m_pAI );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SelectMovementAnimProps()
//              
//	PURPOSE:	Set AnimProps for movement in the current context.
//              
//----------------------------------------------------------------------------

void CAINavigationMgr::SelectMovementAnimProps( CAnimationProps* pProps )
{
	// Sanity check.

	if( !pProps )
	{
		return;
	}

	// No selection is possible without a movement set.

	ENUM_AIMovementSetID eAIMovementSet = m_pAI->GetAIBlackBoard()->GetBBAIMovementSet(); 
	AIDB_MovementSetRecord* pAIMovementSet = g_pAIDB->GetAIMovementSetRecord( eAIMovementSet );
	if( !pAIMovementSet )
	{
		return;
	}

	// Movement animations are defined by the AI's context.

	EnumAIContext eAIContext = kContext_Invalid;
	CAIGoalAbstract* pGoal = m_pAI->GetGoalMgr()->GetCurrentGoal();
	if( pGoal )
	{
		eAIContext = pGoal->GetContext();
	}

	// Find the movement record with a matching context.

	ENUM_AIMovementID eAIMovement;
	AIDB_MovementRecord* pAIMovement = NULL;
	AIMOVEMENT_LIST::iterator itMovement;
	for( itMovement = pAIMovementSet->lstAIMovementSet.begin(); itMovement != pAIMovementSet->lstAIMovementSet.end(); ++itMovement )
	{
		eAIMovement = *itMovement;
		pAIMovement = g_pAIDB->GetAIMovementRecord( eAIMovement );
		if( pAIMovement && ( pAIMovement->eAIContext == eAIContext ) )
		{
			break;
		}
	}

	// Failed to find a match, so use the default.

	if( ( !pAIMovement ) ||
		( pAIMovement->eAIContext != eAIContext ) )
	{
		// Bail if no default exists.

		pAIMovement = g_pAIDB->GetAIMovementRecord( pAIMovementSet->eDefaultAIMovement );
		if( !pAIMovement )
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAINavigationMgr::SelectMovementAnimProps: No default movement found in set." )
			return;
		}
	}

	// Check for overrides.

	EnumAIAwareness eAwareness = m_pAI->GetAIBlackBoard()->GetBBAwareness();
	EnumAIAwarenessMod eAwarenessMod = m_pAI->GetAIBlackBoard()->GetBBAwarenessMod();
	AIMOVEMENT_OVERRIDE_LIST* plstOverrides = &( pAIMovement->lstMovementOverrides );
	if( !plstOverrides->empty() )
	{
		AIMovementOverride* pOverride;
		AIMOVEMENT_OVERRIDE_LIST::iterator itOverride;
		for( itOverride = plstOverrides->begin(); itOverride != plstOverrides->end(); ++itOverride )
		{
			// Skip if awareness mods don't match.

			pOverride = &( *itOverride );
			if( pOverride->eAwarenessMod != eAwarenessMod )
			{
				continue;
			}

			// Found a match.

			if( ( pOverride->eAwareness == eAwareness ) ||
				( pOverride->eAwareness == kAware_Any ) )
			{
				pProps->Set( kAPG_Activity, pOverride->Props.Get( kAPG_Activity ) );
				pProps->Set( kAPG_Movement, pOverride->Props.Get( kAPG_Movement ) );
				pProps->Set( kAPG_Action, pOverride->Props.Get( kAPG_Action ) );
				return;
			}
		}
	}

	// Use the default movement for the current awareness.

	pProps->Set( kAPG_Activity, pAIMovement->Props[eAwareness].Get( kAPG_Activity ) );
	pProps->Set( kAPG_Movement, pAIMovement->Props[eAwareness].Get( kAPG_Movement ) );
	pProps->Set( kAPG_Action, pAIMovement->Props[eAwareness].Get( kAPG_Action ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	GetMovementDirection()
//              
//	PURPOSE:	Helper function to handle determining what direction
//			an AI should face given his target direction and 
//			destination direction
//              
//----------------------------------------------------------------------------

static EnumAnimProp GetMovementDirection(CAI* pAI, EnumAnimProp eCurrent, const LTVector& vTargetDir, const LTVector& vDestDir)
{
	float fDot = vDestDir.Dot( vTargetDir );

	// Apply a threshold so that animations do not dither too frequently.

	float fTest;
	switch( eCurrent  )
	{
		case kAP_MDIR_Forward:
		case kAP_MDIR_Backward:
			fTest = 0.4f;
			break;

		case kAP_MDIR_Right:
		case kAP_MDIR_Left:
			fTest = 0.6f;
			break;

		default:
			fTest = 0.5f;
			break;
	}


	// Forward.

	EnumAnimProp eMovementDir;
	if( fDot > fTest )
	{
		eMovementDir = kAP_MDIR_Forward;
	}

	// Backward.

	else if( fDot < -fTest )
	{
		eMovementDir = kAP_MDIR_Backward;
	}

	else {
		// Right.

		LTVector vUp( 0.f, 1.f, 0.f );
		LTVector vRight = vDestDir.Cross( vUp );
		if( vRight.Dot( vTargetDir ) < 0.f )
		{
			eMovementDir = kAP_MDIR_Right;
		}
	
		// Left.

		else {
			eMovementDir = kAP_MDIR_Left;
		}
	}

	return eMovementDir;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavigationMgr::SelectMovementDirection()
//              
//	PURPOSE:	Returns the direction the AI should face for while moving.
//				Optionally performs a look-ahead to insure the AI will be 
//				facing the new direction long enough for rotation to face 
//				it to be worth it.
//              
//----------------------------------------------------------------------------

EnumAnimProp CAINavigationMgr::SelectMovementDirection()
{
	// Check if AI is currently playing an animation for some movement direction.

	EnumAnimProp eCurrent = m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_MovementDir );
	if( eCurrent == kAP_None )
	{
		eCurrent = kAP_MDIR_Forward;
	}

	// Determine the correct pose based on the direction to the target,
	// and direction to the dest.

	const LTVector vTargetPosition = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();

	LTVector vTargetDir = vTargetPosition - m_pAI->GetPosition();
	vTargetDir.y = 0.f;
	if( vTargetDir == LTVector::GetIdentity() )
	{
		return eCurrent;
	}
	vTargetDir.Normalize();

	LTVector vDestDir = m_pAI->GetAIMovement()->GetDest() - m_pAI->GetPosition();
	vDestDir.y = 0.f;
	if( vDestDir == LTVector::GetIdentity() )
	{
		return eCurrent;
	}
	vDestDir.Normalize();

	EnumAnimProp eMovementDir = GetMovementDirection(m_pAI, eCurrent, vTargetDir, vDestDir);
	
	// If the the ideal movement dir this frame is not forward, and if it is
	// a change, insure there is enough distance with this prop to make the
	// change worth while.  This avoids fluttering when the path changes direction rapidly

	float flMinDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBMinDirectionalRunChangeDistanceSqr();
	if ( eMovementDir != eCurrent 
		&& eMovementDir != kAP_MDIR_Forward
		&& 0.f != flMinDistanceSqr )
	{
		// Set a min threshold time for the change based on min times.  To 
		// get a min distance, multiply by the AIs runspeed

		float		flCurrentDistanceSqr = 0.f;
		LTVector	vCurrentPosition = m_pAI->GetPosition();
		
		// Insure that the distance is greater than some min value before a change 
		// would be required.
	
		uint32 nCurrentPathIndex = m_pNMPath->GetCurPathNodeIndex();
		for (; nCurrentPathIndex < m_pNMPath->GetPathLength(); ++nCurrentPathIndex)
		{
			SPATH_NODE* pNode = m_pNMPath->GetPathNode(nCurrentPathIndex);
			if (!pNode)
			{
				break;
			}

			// The node has a nav mesh poly.  Be conservative as there is no way of 
			// knowing if the AI will react/use this poly for something other than 
			// pathing.

			CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pNode->ePoly );
			if (pPoly && ( kNMLink_Invalid != pPoly->GetNMLinkID()))
			{
				break;
			}

			// Determine if the ideal directional movement changed by the next waypoint.
			//
			// Note that this does NOT test for directional changes during a segment due 
			// to the direction to the enemy changing, as this is likely to be a rare 
			// occurrence.

			LTVector vToTargetDir = vTargetPosition - pNode->vWayPt;
			vToTargetDir.Normalize();

			LTVector vToWaypointDir = pNode->vWayPt - vCurrentPosition;
			vToWaypointDir.Normalize();

			if (eMovementDir != GetMovementDirection(m_pAI, eMovementDir, vToTargetDir, vToWaypointDir))
			{
				break;
			}

			// Determine the waypoints position.

			flCurrentDistanceSqr += (pNode->vWayPt - vCurrentPosition).MagSqr();
			
			// Distance great enough to play a new directional animation

			if (flCurrentDistanceSqr >= flMinDistanceSqr)
			{
				break;
			}

			// Store the new position.

			vCurrentPosition = pNode->vWayPt;
		}

		// Distance not great enough.  Set the direction to forward.

		if (flCurrentDistanceSqr < flMinDistanceSqr)
		{
			eMovementDir = kAP_MDIR_Forward;
		}
	}

	if( eMovementDir != eCurrent )
	{
		m_pAI->GetAIBlackBoard()->SetBBMovementDirChangeTime( g_pLTServer->GetTime() );
	}

	return eMovementDir;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavigationMgr::UpdateDebugRendering()
//              
//	PURPOSE:	Draw or hide path.
//              
//----------------------------------------------------------------------------

void CAINavigationMgr::UpdateDebugRendering( float fVarTrack )
{
	// Update the line to the point the AI is currently pathing towards

	if (m_bDrawingPath)
	{
		DebugLineSystem& system = LineSystem::GetSystem(this, "ShowCurrentLocalWaypoint");
		system.Clear();

		SPATH_NODE* pCurrentWaypoint = m_pNMPath->GetPathNode( (m_pNMPath->GetCurPathNodeIndex() - 1 >= 0) ? m_pNMPath->GetCurPathNodeIndex() - 1 : 0 );
		if (pCurrentWaypoint)
		{
			system.AddArrow(m_pAI->GetPosition(), pCurrentWaypoint->vWayPt);
		}
	}

	if( !m_bDrawingPath && fVarTrack )
	{
		DrawPath();
	}
	else if( m_bDrawingPath && !fVarTrack )
	{
		HidePath();

		// Clear the line to the current node.

		DebugLineSystem& system = LineSystem::GetSystem(this, "ShowCurrentLocalWaypoint");
		system.Clear();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavigationMgr::DrawPath()
//              
//	PURPOSE:	Draw the path.
//              
//----------------------------------------------------------------------------

void CAINavigationMgr::DrawPath()
{
	// Sanity check.

	if( !m_pNMPath )
	{
		return;
	}

	m_bDrawingPath = true;

	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowPath");
	system.Clear();

	// Bail if there is no path.

	unsigned int cNodes = m_pNMPath->GetPathLength();
	if( cNodes == 0 )
	{
		return;
	}

	// Draw arrows between each node along the path.

	LTVector vLastPos = m_pNMPath->GetPathSource();
	SPATH_NODE* pNode;

	for( unsigned int iNode=1; iNode < cNodes; ++iNode )
	{
		pNode = m_pNMPath->GetPathNode( iNode );
		if( pNode )
		{
			system.AddArrow( vLastPos, pNode->vWayPt, Color::Blue );
			vLastPos = pNode->vWayPt;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavigationMgr::HidePath()
//              
//	PURPOSE:	Hide the path.
//              
//----------------------------------------------------------------------------

void CAINavigationMgr::HidePath()
{
	m_bDrawingPath = false;

	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowPath");
	system.SetDebugString("");
	system.Clear();
}
