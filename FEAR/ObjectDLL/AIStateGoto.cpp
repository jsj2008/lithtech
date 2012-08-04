// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateGoto.cpp
//
// PURPOSE : AIStateGoto class implementation
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIStateGoto.h"
#include "AI.h"
#include "AIBrain.h"
#include "AINode.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include "AINavigationMgr.h"
#include "AIUtils.h"
#include "AnimationContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateGoto, kState_Goto );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIStateGoto::CAIStateGoto()
{
	m_vDest = LTVector(0,0,0);
	m_hDestNode = NULL;
	m_hDestObject = NULL;
	m_vDestObjectOffset = LTVector(0,0,0);

	m_eMovement = kAP_MOV_Walk;
	m_eActivity = kAP_None;
	m_eDir = kAP_None;

	m_bDestinationBlockedLastUpdate = false;
}

CAIStateGoto::~CAIStateGoto()
{
	// If the AI is not at the node, unlock the dest node.  Normally the lock
	// is released on AINode::HandleAIDeparture, but if the AI fails to arrive,
	// the lock must be released here.

	if( m_hDestNode )
	{
		SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
		if ( !pProp || ( pProp->hWSValue != m_hDestNode ) )
		{
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
			if( pNode && pNode->IsNodeLocked() )
			{
				pNode->UnlockNode( m_pAI->m_hObject );
			}
		}
	}

	// If AI was heading to a destination, and did not make it,
	// clear the DestinationStatus.

	m_pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Unset );

	m_pAI->GetAIBlackBoard()->SetBBDestIsDynamic( false );
	m_pAI->GetAIBlackBoard()->SetBBDestRepathDistance( 0.f );

	// Revert blend looping to the normal state.
	m_pAI->GetAnimationContext()->SetBlendLoopedAnimations( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::Load
//
//	PURPOSE:	Load state.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::Load( ILTMessage_Read *pMsg )
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vDest);
	LOAD_HOBJECT(m_hDestNode);
	LOAD_HOBJECT(m_hDestObject);
	LOAD_VECTOR(m_vDestObjectOffset);

	LOAD_DWORD_CAST( m_eMovement, EnumAnimProp );
	LOAD_DWORD_CAST( m_eActivity, EnumAnimProp );
	LOAD_DWORD_CAST( m_eDir, EnumAnimProp );
	LOAD_bool( m_bDestinationBlockedLastUpdate );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::Save
//
//	PURPOSE:	Save state.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::Save( ILTMessage_Write *pMsg )
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vDest);
	SAVE_HOBJECT(m_hDestNode);
	SAVE_HOBJECT(m_hDestObject);
	SAVE_VECTOR(m_vDestObjectOffset);

	SAVE_DWORD( m_eMovement );
	SAVE_DWORD( m_eActivity );
	SAVE_DWORD( m_eDir );
	SAVE_bool( m_bDestinationBlockedLastUpdate );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::Init
//
//	PURPOSE:	Initialize state.
//
// ----------------------------------------------------------------------- //

bool CAIStateGoto::Init(CAI* pAI)
{
	if ( !super::Init(pAI) )
	{
		return false;
	}

	// Depart from old node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			pNode->HandleAIDeparture( pAI );
		}
	}

AITRACE( AIShowGoals, ( m_pAI->m_hObject, "INIT GOTO" ) );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::SetDestNode
//
//	PURPOSE:	Set the destination node.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::SetDestNode(HOBJECT hDestNode)
{
	// Clear any previous dest.

	AINode* pNode;
	if( m_hDestNode )
	{
		pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
		if( pNode )
		{
			pNode->UnlockNode( m_pAI->m_hObject );
		}
	}

	// Set a new dest.

	m_hDestNode = hDestNode;
	pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
	if( pNode )
	{
		// AI is already at the node.

		LTVector vNodeDestination;
		if (!pNode->GetDestinationPosition(m_pAI, m_pAI->GetAIBlackBoard()->GetBBTargetPosition(), vNodeDestination))
		{
			AIASSERT(0, m_pAI->GetHOBJECT(), "CAIStateGoto::SetDestNode : Failed to find a valid destination position.")
			m_eStateStatus = kAIStateStatus_Complete;
		}

		if( ( m_pAI->GetPosition().x == vNodeDestination.x ) &&
			( m_pAI->GetPosition().z == vNodeDestination.z ) &&
			( m_pAI->GetPosition().DistSqr( vNodeDestination ) < m_pAI->GetVerticalThreshold() * m_pAI->GetVerticalThreshold() ) )
		{
			m_eStateStatus = kAIStateStatus_Complete;
			ArriveAtNode();
		}

		// AI needs to goto the node.

		else 
		{
			SetDest( vNodeDestination );
		}
		pNode->LockNode( m_pAI->m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::SetDest
//
//	PURPOSE:	Set the destination.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::SetDest( const LTVector& vDest )
{
	m_vDest = vDest;
	m_eStateStatus = kAIStateStatus_Initialized;

	m_pAI->GetAIBlackBoard()->SetBBDest( m_vDest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::SetDestObject
//
//	PURPOSE:	Set destination object.  
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::SetDestObject( HOBJECT hObj, const LTVector& vOffset )
{
	m_hDestObject = hObj;
	m_eStateStatus = kAIStateStatus_Initialized;
	
	m_vDestObjectOffset = vOffset;

	GetObjectNMPosition( m_hDestObject, &m_vDest );
	m_pAI->GetAIBlackBoard()->SetBBDest( m_vDest + m_vDestObjectOffset );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::SetDynamicRepathDistance
//
//	PURPOSE:	Flag the end path end position as dynamic, and sets the 
//				repathing distance 
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::SetDynamicRepathDistance(float flDist)
{
	m_pAI->GetAIBlackBoard()->SetBBDestIsDynamic( true );
	m_pAI->GetAIBlackBoard()->SetBBDestRepathDistance( flDist );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::ArriveAtNode
//
//	PURPOSE:	Arrive at the destination node.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::ArriveAtNode()
{
	if( m_hDestNode )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
		if( !pNode )
		{
			return;
		}

		// Arrive at new node.

		pNode->HandleAIArrival( m_pAI );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::Update
//
//	PURPOSE:	Update state.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::Update()
{
	super::Update();

	if( m_eStateStatus != kAIStateStatus_Complete )
	{
		switch(m_pAI->GetAIBlackBoard()->GetBBDestStatus())
		{
		case kNav_Done:
			{
				m_eStateStatus = kAIStateStatus_Complete;

				// We successfully got there.

				if( m_hDestNode )
				{
					ArriveAtNode();
				}
			}
			break;

		case kNav_Failed:
			{
				// We failed to get there.

				m_eStateStatus = kAIStateStatus_Failed;
			}
			break;

		case kNav_Set:
			{
				// If the patching is dynamic and we have a dest object, update
				// our destination if it has changed.  

				if (m_hDestObject && 
					m_pAI->GetAIBlackBoard()->GetBBDestIsDynamic())
				{
					GetObjectNMPosition( m_hDestObject, &m_vDest );
					if (!m_pAI->GetAIBlackBoard()->GetBBDest().NearlyEquals(m_vDest + m_vDestObjectOffset))
					{
						m_pAI->GetAIBlackBoard()->SetBBDest( m_vDest + m_vDestObjectOffset );
					}
				}
				
				// Handle blocked path response by potentially
				// resetting the destination or by flagging the
				// path itself as invalid.

				UpdateCollisionResponse();
			}
			break;

		default:
			ObjectCPrint(m_pAI->GetHOBJECT(), "Unexpected DestStatus");
			break;
		};
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::UpdateCollisionResponse
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::UpdateCollisionResponse()
{
	// Force a path reevaluation if the destination is blocked, as this may 
	// result in a new path that does not have a blocked destination if our
	// destination has moved (or if AIMovement constraints such as sliding, 
	// rotation limiting, or obstacle avoidance adjusted the path).

	uint32 iCollisionFlags = m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags();
	if ( kAIMovementFlag_BlockedDestination & iCollisionFlags )
	{
		// Remove the destination blocked flag and either 
		// attempt a new path destination or flag the path 
		// itself as blocked.  This will occur if AIStateGoto
		// requested a new path last frame and that path also
		// failed to resolve the blocked destination.

		iCollisionFlags ^= kAIMovementFlag_BlockedDestination;
		
		if ( m_bDestinationBlockedLastUpdate )
		{
			HOBJECT hBlockingObject = NULL;
			LTVector vBlockedPosition;

			// Find the fact about the blocked destination; use it to build 
			// a fact about the blocked path.  Remove the blocked destination
			// fact.

			CAIWMFact queryFact;
			queryFact.SetFactType( kFact_Knowledge );
			queryFact.SetKnowledgeType( kKnowledge_BlockedDestination );
			CAIWMFact* pBlockedDestinationFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
			if ( pBlockedDestinationFact )
			{
				hBlockingObject = pBlockedDestinationFact->GetTargetObject();
				vBlockedPosition = pBlockedDestinationFact->GetPos();
				m_pAI->GetAIWorkingMemory()->ClearWMFact( pBlockedDestinationFact );
			}

			if ( hBlockingObject )
			{
				// Update the collision flag

				iCollisionFlags |= kAIMovementFlag_BlockedPath;

				// Update or create a fact with details about the obstruction.

				CAIWMFact queryFact;
				queryFact.SetFactType( kFact_Knowledge );
				queryFact.SetKnowledgeType( kKnowledge_BlockedPath );
				CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
				if ( !pFact )
				{
					pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
					if ( pFact )
					{
						pFact->SetKnowledgeType( kKnowledge_BlockedPath );
					}
				}

				if ( pFact )
				{
					pFact->SetPos( vBlockedPosition );
					pFact->SetTargetObject( hBlockingObject );
				}
			}
		}

		else
		{
			m_pAI->GetAIBlackBoard()->SetBBInvalidatePath( true );
			m_bDestinationBlockedLastUpdate = true;
		}

		m_pAI->GetAIBlackBoard()->SetBBMovementCollisionFlags( iCollisionFlags );
	}
	else
	{
		m_bDestinationBlockedLastUpdate = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::UpdateAnimation
//
//	PURPOSE:	Update state's animation.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::UpdateAnimation()
{
	super::UpdateAnimation();

	// Suppress blending of looped animations -- runs and walks should not 
	// blend when they loop.
	m_pAI->GetAnimationContext()->SetBlendLoopedAnimations( false );

	m_pAI->GetAnimationContext()->SetProp( kAPG_Posture, kAP_POS_Stand );
	m_pAI->GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_WPOS_Down );
	m_pAI->GetAnimationContext()->SetProp( kAPG_Activity, m_eActivity );

	if( m_eStateStatus != kAIStateStatus_Complete )
	{
		m_pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, m_eDir );
		m_pAI->GetAnimationContext()->SetProp( kAPG_Movement, m_eMovement );
	}

  	// Don't try to fire while moving if some special awareness has been set.
  
  	if( m_eActivity != kAP_None )
  	{
  		return;
  	}

  	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) && 
		( m_pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert ) )
  	{
  		m_pAI->GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_WPOS_Up );	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateGoto::GetObjectNMPosition
//
//	PURPOSE:	Returns the position of an object, using the navmesh 
//				position if avalible.
//
// ----------------------------------------------------------------------- //

void CAIStateGoto::GetObjectNMPosition( HOBJECT hObj, LTVector* pvPos )
{
	// Sanity check.

	if( !pvPos )
	{
		return;
	}

	if( m_pAI->GetAIBlackBoard()->GetBBTargetObject() == hObj ) 
	{
		*pvPos =  m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition();
	}
	else {
		g_pLTServer->GetObjectPos( m_hDestObject, pvPos );
	}
}

// ----------------------------------------------------------------------- //

