// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateUseSmartObject.cpp
//
// PURPOSE : AIStateUseSmartObject class implementation
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIStateUseSmartObject.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIWorldState.h"
#include "AnimationContext.h"
#include "AIUtils.h"
#include "ParsedMsg.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateUseSmartObject, kState_UseSmartObject );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIStateUseSmartObject::CAIStateUseSmartObject()
{
	m_hSmartObjectNode = NULL;

	m_Props.Set( kAPG_Posture, kAP_POS_Stand );
	m_Props.Set( kAPG_Action, kAP_ACT_Idle );
	m_Props.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );

	m_fMinFidgetTime = 0.f;
	m_fMaxFidgetTime = 0.f;
	m_fNextFidgetTime = 0.f;

	m_bLooping = true;
	m_fLoopTime = 0.f;
	m_fLoopStartTime = 0.f;

	m_bLockNode = false;
	m_bFirstUpdate = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::Load
//
//	PURPOSE:	Load state.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::Load( ILTMessage_Read *pMsg )
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hSmartObjectNode);

	m_Props.Load(pMsg);
		
	LOAD_FLOAT(m_fMinFidgetTime);
	LOAD_FLOAT(m_fMaxFidgetTime);
	LOAD_DOUBLE(m_fNextFidgetTime);
	
	LOAD_bool(m_bLooping);
	LOAD_DOUBLE(m_fLoopTime);
	LOAD_DOUBLE(m_fLoopStartTime);
		
	LOAD_bool(m_bLockNode);
	LOAD_bool(m_bFirstUpdate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::Save
//
//	PURPOSE:	Save state.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::Save( ILTMessage_Write *pMsg )
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hSmartObjectNode);

	m_Props.Save(pMsg);
		
	SAVE_FLOAT(m_fMinFidgetTime);
	SAVE_FLOAT(m_fMaxFidgetTime);
	SAVE_DOUBLE(m_fNextFidgetTime);

	SAVE_bool(m_bLooping);
	SAVE_DOUBLE(m_fLoopTime);
	SAVE_DOUBLE(m_fLoopStartTime);

	SAVE_bool(m_bLockNode);
	SAVE_bool(m_bFirstUpdate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::Init
//
//	PURPOSE:	Initialize state.
//
// ----------------------------------------------------------------------- //

bool CAIStateUseSmartObject::Init(CAI* pAI)
{
	if ( !super::Init(pAI) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::SetNode
//
//	PURPOSE:	Set the node to use.
//
// ----------------------------------------------------------------------- //

bool CAIStateUseSmartObject::SetNode(AINodeSmartObject* pSmartObjectNode)
{
	if( !pSmartObjectNode )
	{
		return false;
	}

	m_hSmartObjectNode = pSmartObjectNode->m_hObject;

	m_eStateStatus = kAIStateStatus_Initialized;
	m_pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, 0 );

	// Attach childmodels.
	
	pSmartObjectNode->ApplyChildModels( m_pAI->m_hObject );
	m_pAI->GetAnimationContext()->ResetInvalidInstances( );

	// Set the Object to interact with.
	// The object may be NULL.

	m_pAI->SetAnimObject( pSmartObjectNode->GetObject() );

	// Pre-activate the node.

	pSmartObjectNode->PreActivate();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::SetSmartObject
//
//	PURPOSE:	Set the smart object.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::SetSmartObject( AIDB_SmartObjectRecord* pSmartObject )
{
	if( !pSmartObject )
	{
		return;
	}

	// Set anim props.

	m_Props = pSmartObject->Props;

	// Set Fidget time.

	m_fMinFidgetTime = pSmartObject->fMinFidgetTime;
	m_fMaxFidgetTime = pSmartObject->fMaxFidgetTime;
	if( ( m_fMinFidgetTime > 0.f ) &&
		( m_fMaxFidgetTime > 0.f ) )
	{
		m_fNextFidgetTime = g_pLTServer->GetTime() + GetRandom( m_fMinFidgetTime, m_fMaxFidgetTime );
	}
	
	// Set Loop time.

	m_bLooping = pSmartObject->bLooping;
	SetLoopTime( pSmartObject->fMinLoopTime, pSmartObject->fMaxLoopTime );

	// Set LockNode flag.

	m_bLockNode = pSmartObject->bLockNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::SetLoopTime
//
//	PURPOSE:	Set the random loop time.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::SetLoopTime( float fMinLoopTime, float fMaxLoopTime )
{
	// If LoopTime is set to [0.0, 0.0], loop infinitely.

	if( ( fMinLoopTime > 0.f ) &&
		( fMaxLoopTime > 0.f ) )
	{
		m_fLoopStartTime = g_pLTServer->GetTime();
		m_fLoopTime = GetRandom( fMinLoopTime, fMaxLoopTime );
	}
	else {
		m_fLoopStartTime = 0.f;
		m_fLoopTime = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::Update
//
//	PURPOSE:	Update state.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::Update()
{
	super::Update();

	double fCurTime = g_pLTServer->GetTime();

	if( ( m_fNextFidgetTime > 0.f ) &&
		( fCurTime > m_fNextFidgetTime ) )
	{
		if( m_Props.Get( kAPG_Action ) == kAP_ACT_Idle )
		{
			m_Props.Set( kAPG_Action, kAP_ACT_Fidget );
		}

		else if( !m_pAI->GetAnimationContext()->IsLocked() )
		{
			m_Props.Set( kAPG_Action, kAP_ACT_Idle );
			m_fNextFidgetTime = fCurTime + GetRandom( m_fMinFidgetTime, m_fMaxFidgetTime );
		}
	}

	// Looptime is set to a non-infinite value.

	if( m_bLooping && ( m_fLoopTime > 0.f ) )
	{
		// Don't count transition time as loop time.

		if( m_pAI->GetAnimationContext()->IsTransitioning() )
		{
			m_fLoopStartTime = g_pLTServer->GetTime();
		}

		// Looptime has expired.

		if( fCurTime > m_fLoopStartTime + m_fLoopTime )
		{
			if( m_bLockNode )
			{
				AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hSmartObjectNode );
				if( pNode )
				{
					pNode->DisableNode();
				}
			}

			// Only set kWSK_UsingObject to m_hSmartObjectNode if the node exists.  
			// Some actions do not set this node; if NULL, this would have the result
			// of clearing kWSK_UsingObject.  Eventually, this class should be split
			// into a version that do and do not operate on a node.

			if ( m_hSmartObjectNode )
			{
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hSmartObjectNode );
			}

			m_eStateStatus = kAIStateStatus_Complete;
		}
	}

	// Finished if animation is not locked,
	// not the first update,
	// and not looping.

	else if( !( m_bFirstUpdate || 
				m_bLooping ||
				m_pAI->GetAnimationContext()->IsLocked() ) )
	{
		if( m_bLockNode )
		{
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hSmartObjectNode );
			if( pNode )
			{
				pNode->DisableNode();
			}
		}

		// Only set kWSK_UsingObject to m_hSmartObjectNode if the node exists.  
		// Some actions do not set this node; if NULL, this would have the result
		// of clearing kWSK_UsingObject.  Eventually, this class should be split
		// into a version that do and do not operate on a node.

		if ( m_hSmartObjectNode )
		{
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hSmartObjectNode );
		}

		m_eStateStatus = kAIStateStatus_Complete;
	}

	// After the first update, it is no longer the first update.

	m_bFirstUpdate = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateUseSmartObject::UpdateAnimation
//
//	PURPOSE:	Update state's animation.
//
// ----------------------------------------------------------------------- //

void CAIStateUseSmartObject::UpdateAnimation()
{
	super::UpdateAnimation();

	m_pAI->GetAnimationContext()->SetProps( m_Props );

	if( ( m_Props.Get( kAPG_Action ) == kAP_ACT_Fidget ) ||
		( !m_bLooping ) )
	{
		m_pAI->GetAnimationContext()->Lock();
	}
}

// ----------------------------------------------------------------------- //
