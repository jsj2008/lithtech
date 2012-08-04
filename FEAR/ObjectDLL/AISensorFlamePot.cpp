// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlamePot.cpp
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorFlamePot.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlamePot, kSensor_FlamePot );

static bool IsInFlamePot( CAI* pAI )
{
	if ( !pAI )
	{
		return false;
	}

	// Fail if the target is not in a flame pot poly.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetCurrentNavMeshLink() );
	if ( !pLink 
		|| kLink_FlamePot != pLink->GetNMLinkType() 
		|| NULL == pLink->GetHOBJECT() )
	{
		return false;
	}

	// Fail if the link is not active.

	if ( !pLink->IsNMLinkActiveToAI( pAI ) )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlamePot::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorFlamePot::CAISensorFlamePot() : 
	m_bWasInFlamePotLastFrame( false )
{
}

CAISensorFlamePot::~CAISensorFlamePot()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorFlamePot::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorFlamePot
//              
//----------------------------------------------------------------------------

void CAISensorFlamePot::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bWasInFlamePotLastFrame );
}

void CAISensorFlamePot::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bWasInFlamePotLastFrame );
}

bool CAISensorFlamePot::UpdateSensor()
{
	bool bCurrentlyInFlamePot = IsInFlamePot( m_pAI );

	// If we just entered, start the timer.

	if ( !m_bWasInFlamePotLastFrame && bCurrentlyInFlamePot )
	{
		m_flTimeEnteredFlamePot = g_pLTServer->GetTime();
	}
	else if ( m_bWasInFlamePotLastFrame && !bCurrentlyInFlamePot )
	{
		m_flTimeEnteredFlamePot = 0.0f;
	}

	// If the AI has been in the flame pot long enough, he should consider his 
	// position invalid and flee it.

	if ( bCurrentlyInFlamePot 
		&& ( m_flTimeEnteredFlamePot < g_pLTServer->GetTime() + 2.0f ) 
		&& ( ( AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) || AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER ) ) )
		)
	{
		// If the AI currently has a blitz character task, wait until it is complete
		// before flagging the position as invalid.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_BlitzCharacter );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if ( NULL == pFact )
		{
			SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject );
			if ( pProp && pProp->bWSValue )
			{
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, false );
				m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			}
		}
	}

	m_bWasInFlamePotLastFrame = bCurrentlyInFlamePot;

	// no significant work done, continue

	return false; 
}
