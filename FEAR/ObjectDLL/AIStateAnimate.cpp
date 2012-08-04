// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateAnimate.cpp
//
// PURPOSE : AIStateAnimate class implementation
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIStateAnimate.h"
#include "AI.h"
#include "AINode.h"
#include "AIWorldState.h"
#include "AIUtils.h"
#include "AnimationContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateAnimate, kState_Animate );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIStateAnimate::CAIStateAnimate()
{
	m_bLoop = false;

	m_animProps.Clear();

	m_eSpecialType = kSpecial_None;

	m_bCheckDepartedNode = false;
}

CAIStateAnimate::~CAIStateAnimate()
{
	m_pAI->GetAnimationContext()->ClearSpecial();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::SetAnimation
//
//	PURPOSE:	Set the animation with a string.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::SetAnimation( HMODELANIM hAni, bool bLoop )
{
	m_bLoop = bLoop;

	m_pAI->GetAnimationContext()->SetSpecial( hAni );

	m_eSpecialType = kSpecial_Handle;

	m_eStateStatus = kAIStateStatus_Initialized;
	PlaySpecial();

	m_bCheckDepartedNode = false;
	m_vInitialPos = m_pAI->GetPosition();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::SetAnimation
//
//	PURPOSE:	Set the animation with a string.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::SetAnimation( const char* strAnim, bool bLoop )
{
	m_bLoop = bLoop;

	m_pAI->GetAnimationContext()->SetSpecial( strAnim );

	m_eSpecialType = kSpecial_String;

	m_eStateStatus = kAIStateStatus_Initialized;
	PlaySpecial();

	m_bCheckDepartedNode = false;
	m_vInitialPos = m_pAI->GetPosition();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::SetAnimation
//
//	PURPOSE:	Set the animation with anim props.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::SetAnimation( const CAnimationProps& animProps, bool bLoop )
{
	m_bLoop = bLoop;

	m_animProps = animProps;

	m_eSpecialType = kSpecial_Props;

	m_eStateStatus = kAIStateStatus_Initialized;

	m_bCheckDepartedNode = false;
	m_vInitialPos = m_pAI->GetPosition();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::DepartNode
//
//	PURPOSE:	Depart from the node the AI is currently at.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::DepartNode()
{
	// Depart from a node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			pNode->HandleAIDeparture( m_pAI );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::PlaySpecial
//
//	PURPOSE:	Start a new special animation.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::PlaySpecial()
{
	if( m_eSpecialType & ( kSpecial_String | kSpecial_Handle ) )
	{
		if( m_bLoop )
		{
			m_pAI->GetAnimationContext()->LoopSpecial();
		}
		else {
			m_pAI->GetAnimationContext()->LingerSpecial();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::GetAnimationProps
//
//	PURPOSE:	Get the currently set animation props.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::GetAnimationProps( CAnimationProps* pProps )
{
	// Sanity check.

	if( !pProps )
	{
		return;
	}

	*pProps = m_animProps;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::Update
//
//	PURPOSE:	Update state.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::Update()
{
	super::Update();

	// Looping animation never completes.

	if( m_bLoop )
	{
		return;
	}

	// Completed animation set with animProps.

	if( ( m_eSpecialType & kSpecial_Props ) &&
		( !m_pAI->GetAnimationContext()->IsLocked() ) )
	{
		// If the AI was at a node, and moved significantly, depart from the node.
		// AI standing at nodes will refuse to get out of the way of the Player.
		// So, it's important to know if the AI has actually animated off of the node.

		if( m_bCheckDepartedNode &&
			( m_vInitialPos.DistSqr( m_pAI->GetPosition() ) > 100.f * 100.f ) )
		{
			DepartNode();
		}

		m_eStateStatus = kAIStateStatus_Complete;
		return;
	}

	// Completed a special animation.

	if( ( m_eSpecialType & ( kSpecial_String | kSpecial_Handle ) ) &&
		( m_pAI->GetAnimationContext()->IsSpecialDone() ) )
	{
		// If the AI was at a node, and moved significantly, depart from the node.
		// AI standing at nodes will refuse to get out of the way of the Player.
		// So, it's important to know if the AI has actually animated off of the node.

		if( m_bCheckDepartedNode &&
			( m_vInitialPos.DistSqr( m_pAI->GetPosition() ) > 100.f * 100.f ) )
		{
			DepartNode();
		}

		m_eStateStatus = kAIStateStatus_Complete;
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::UpdateAnimation
//
//	PURPOSE:	Update state's animation.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::UpdateAnimation()
{
	super::UpdateAnimation();

	// Update if anim was set with animProps.

	if( m_eSpecialType & kSpecial_Props )
	{
		m_pAI->GetAnimationContext()->SetProps( m_animProps );

		if( !m_bLoop )
		{
			m_pAI->GetAnimationContext()->Lock();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::Load
//
//	PURPOSE:	Load state.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::Load( ILTMessage_Read *pMsg )
{
	super::Load(pMsg);

	LOAD_BOOL(m_bLoop);
	LOAD_INT_CAST(m_eSpecialType, AISPECIAL_ANIMATION_TYPE);
	m_animProps.Load(pMsg);

	LOAD_bool( m_bCheckDepartedNode ); 
	LOAD_VECTOR( m_vInitialPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAnimate::Save
//
//	PURPOSE:	Save state.
//
// ----------------------------------------------------------------------- //

void CAIStateAnimate::Save( ILTMessage_Write *pMsg )
{
	super::Save(pMsg);

	SAVE_BOOL(m_bLoop);
	SAVE_INT(m_eSpecialType);
	m_animProps.Save(pMsg);

	SAVE_bool( m_bCheckDepartedNode ); 
	SAVE_VECTOR( m_vInitialPos );
}
