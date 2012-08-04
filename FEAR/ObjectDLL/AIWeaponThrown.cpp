// ----------------------------------------------------------------------- //
//
// MODULE  : CAIWeaponThrown.cpp
//
// PURPOSE : Thrown Weapon Implementation
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponThrown.h"
#include "AI.h"
#include "AITarget.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "HHWeaponModel.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponThrown, kAIWeaponClassType_Thrown );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::CAIWeaponThrown
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIWeaponThrown::CAIWeaponThrown() :
	m_bHidden( false )
	, m_hLastUserAnimation( INVALID_ANI )
{
}

void CAIWeaponThrown::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bHidden );
	pMsg->WriteHMODELANIM( m_hLastUserAnimation );
}

void CAIWeaponThrown::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bHidden );
	m_hLastUserAnimation = pMsg->ReadHMODELANIM();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::Init
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::Init(CWeapon* pWeapon, CAI* pAI)
{
	if (!DefaultInit(pWeapon, pAI))
	{
		return;
	}

	HideWeapon( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::HideWeapon
//
//	PURPOSE:	Hide of show the weapon model.
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::HideWeapon( bool bHide )
{
	// Bail if hidden state did not change.

	if( m_bHidden == bHide )
	{
		return;
	}

	// Bail if no weapon.

	if( !m_pWeapon )
	{
		return;
	}

	// Bail if no model.

	HOBJECT hModel = m_pWeapon->GetModelObject();
	if( !hModel )
	{
		return;
	}

	// Hide the weapon.

	CHHWeaponModel *pHHWeapon = dynamic_cast< CHHWeaponModel* >( g_pLTServer->HandleToObject( hModel ));
	if( pHHWeapon )
	{
		if( bHide )
		{
			g_pCmdMgr->QueueMessage( pHHWeapon, pHHWeapon, "HIDDEN 1" );
		}
		else {
			g_pCmdMgr->QueueMessage( pHHWeapon, pHHWeapon, "HIDDEN 0" );
		}
	}

	// Update hidden state.

	m_bHidden = bHide;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::Update
//
//	PURPOSE:	Handle updating the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::Update(CAI*)
{
	m_eFiringState = kAIFiringState_Throwing;

	HideWeapon( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::UpdateAnimation
//
//	PURPOSE:	Handle any animation updating that must occur after
//			the owners animations update.	
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::UpdateAnimation( CAI* pAI )
{
	m_hLastUserAnimation = SyncWeaponAnimation( pAI, m_hLastUserAnimation );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::Fire
//
//	PURPOSE:	Fire the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::Fire(CAI* pAI)
{
	DefaultThrow(pAI); 

	HideWeapon( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::GetFirePosition
//
//	PURPOSE:	Returns the position the projectile will be fired from.
//
// ----------------------------------------------------------------------- //

LTVector CAIWeaponThrown::GetFirePosition(CAI* pAI)
{
	if (!pAI)
	{
		return LTVector(0,0,0);
	}

	return pAI->GetWeaponPosition(m_pWeapon, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponThrown::HandleModelString
//
//	PURPOSE:	Model string handling function.
//
// ----------------------------------------------------------------------- //

void CAIWeaponThrown::HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg )
{
	// Do not handle the FIRE key.

	static CParsedMsg::CToken s_cTok_FireWeapon(c_szKeyFireWeapon);
	if( cParsedMsg.GetArg(0) == s_cTok_FireWeapon )
	{
		return;
	}

	// Default behavior.

	DefaultHandleModelString(pAI, cParsedMsg );
}
