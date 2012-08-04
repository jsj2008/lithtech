// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMountedFlashlight.cpp
//
// PURPOSE : AIGoalMountedFlashlight implementation
//
// CREATED : 6/24/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalMountedFlashlight.h"
#include "AI.h"
#include "AIVolume.h"
#include "AIUtils.h"
#include "Attachments.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalMountedFlashlight, kGoal_MountedFlashlight);

#define FLASHLIGHT_SOUND		"Guns\\snd\\ak47\\Flashlight_click.wav"
#define FLASHLIGHT_ATTACHMENT	"AIRifleLight"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountedFlashlight::CAIGoalMountedFlashlight
//
//	PURPOSE:	Con/Destructor
//
// ----------------------------------------------------------------------- //

CAIGoalMountedFlashlight::CAIGoalMountedFlashlight()
{
	m_bLightOn = LTFALSE;
	m_bCheckedForRangedWeapon = LTFALSE;
	m_bHasRangedWeapon = LTFALSE;

	// The flashlight needs to be turned on/off regardless of whatever
	// goals are active or locked.

	m_bRequiresUpdates = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountedFlashlight::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalMountedFlashlight::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL( m_bLightOn );
	SAVE_BOOL( m_bCheckedForRangedWeapon );
	SAVE_BOOL( m_bHasRangedWeapon );
}

void CAIGoalMountedFlashlight::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL( m_bLightOn );
	LOAD_BOOL( m_bCheckedForRangedWeapon );
	LOAD_BOOL( m_bHasRangedWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountedFlashlight::HasRangedWeapon
//
//	PURPOSE:	Determine if AI has a ranged weapon.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalMountedFlashlight::HasRangedWeapon()
{
	// AI has a ranged weapon.

	if( m_pAI->HasWeapon( kAIWeap_Ranged ) )
	{
		return LTTRUE;
	}

	// AI has a holstered weapon.

	if( m_pAI->GetHolsterWeaponType() == kAIWeap_Ranged )
	{
		return LTTRUE;
	}

	// AI does not have a ranged weapon.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountedFlashlight::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the presence
//              of a weapon.
//
// ----------------------------------------------------------------------- //

void CAIGoalMountedFlashlight::RecalcImportance()
{
	if( m_pAI->IsFirstUpdate() )
	{
		return;
	}

	// Create a flashlight model if we have a ranged weapon
	// (because it is mounted on the gun).

	if( !m_bCheckedForRangedWeapon )
	{
		if( HasRangedWeapon() )
		{
			m_bHasRangedWeapon = LTTRUE;
		}

		m_bCheckedForRangedWeapon = LTTRUE;
	}


	// Ignore flashlight if we do not know our current volume,
	// or do not have a flashlight.

	if( !( m_bHasRangedWeapon && 
		   m_pAI->HasCurrentVolume() ) )
	{
		return;
	}

	AIVolume* pVolume = m_pAI->GetCurrentVolume();

	// Volume is lit, so turn off flashlight.
	// Turn off light if affected by special damage (e.g. knocked out).
	// Turn off light if ranged weapon is not currently armed.

	if( m_bLightOn && 
		( pVolume->IsLit() || 
		  m_pAI->GetDamageFlags() ||
		  !m_pAI->HasWeapon( kAIWeap_Ranged ) ) )
	{
		char szDetach[128];
		sprintf( szDetach, "%s Light", KEY_DETACH );		
		SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetach );

		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Flashlight OFF" ) );

		m_pAI->PlaySound( FLASHLIGHT_SOUND );
		m_bLightOn = LTFALSE;
	}

	// Volume is dark, so turn on flashlight.
	// Do not turn on a light if affected by special damage (e.g. knocked out).
	// Do not turn on light if ranged weapon is not currently armed.

	else if( !m_bLightOn && 
		     !pVolume->IsLit() && 
			 !m_pAI->GetDamageFlags() &&
			 m_pAI->HasWeapon( kAIWeap_Ranged ) )
	{
		char szAttachment[128];
		sprintf( szAttachment, "%s Light (%s)", KEY_ATTACH, FLASHLIGHT_ATTACHMENT );		
		SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szAttachment );

		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Flashlight ON" ) );

		m_pAI->PlaySound( FLASHLIGHT_SOUND );
		m_bLightOn = LTTRUE;
	}
}
