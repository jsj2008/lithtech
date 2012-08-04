// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPlayerTooClose.cpp
//
// PURPOSE : AISensorPlayerTooClose class implementation
//
// CREATED : 10/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorPlayerTooClose.h"
#include "CharacterMgr.h"
#include "CharacterDB.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPlayerTooClose, kSensor_PlayerTooClose );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorPlayerTooClose::CAISensorPlayerTooClose()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorPlayerTooClose::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorPlayerTooClose::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorPlayerTooClose::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if AI is at a node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		return false;
	}

	// Check proximity to each Player.

	LTVector vPlayerPos;
	uint32 iPlayer = 0;
	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer( iPlayer );
	while( pPlayer )
	{
		// Bail if we don't like players.

		if( g_pCharacterDB->GetStance( m_pAI->GetAlignment(), pPlayer->GetAlignment() ) != kCharStance_Like )
		{
			m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_PlayerTooClose );
			return false;
		}

		// Ignore Players if we are already en-route to somewhere.
		// Otherwise, we may cause oscillation as the AI repeatedly tries
		// to get somewhere and avoid the Player.

		if( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Unset )
		{	
			return false;
		}

		// Create the desire to get out of the character's way, if too close.

		if( IsCharacterTooClose( pPlayer, 0.25f ) )
		{
			CreateAvoidanceDesire( pPlayer );
			return false;
		}

		// Check the next Player, if others exist.

		++iPlayer;
		pPlayer = g_pCharacterMgr->FindPlayer( iPlayer );
	}

	//
	// AI is not too close to a Player, so check against Allies. 
	//

	// Only check against allies if dynamic obstacle avoidance is turned on.

	if( m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() & kAIMovementFlag_ObstacleAvoidance )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Desire );
		factQuery.SetDesireType( kDesire_GetOutOfTheWay );

		CAI* pAI;
		CAI::AIList::const_iterator it;
		for( it = CAI::GetAIList().begin(); it != CAI::GetAIList().end(); ++it )
		{
			pAI = *it;
			if( !pAI )
			{
				continue;
			}

			// Ignore myself.

			if( pAI == m_pAI )
			{
				continue;
			}

			// Ignore AI that I do not like.

			if( g_pCharacterDB->GetStance( m_pAI->GetAlignment(), pAI->GetAlignment() ) != kCharStance_Like )
			{
				continue;
			}

			// Skip AI that are already getting out of the way.

			if( pAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
			{
				continue;
			}

			// Create the desire to get out of the character's way, if too close.

			if( IsCharacterTooClose( pAI, -0.25f ) )
			{
				CreateAvoidanceDesire( pAI );
				return false;
			}
		}
	}


	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::IsCharacterTooClose
//
//	PURPOSE:	Return true if this character is too close.
//
// ----------------------------------------------------------------------- //

bool CAISensorPlayerTooClose::IsCharacterTooClose( CCharacter* pChar, float fThreshold )
{
	// Sanity check.

	if( !pChar )
	{
		return false;
	}

	// Consider a character too close if the space between us is under 
	// some percentage of the sum of our radii.

	float fMinDist = m_pAI->GetRadius() + pChar->GetRadius();
	fMinDist += fMinDist * fThreshold;

	// Character is too close.

	LTVector vCharPos;
	g_pLTServer->GetObjectPos( pChar->m_hObject, &vCharPos );
	if( vCharPos.DistSqr( m_pAI->GetPosition() ) < fMinDist * fMinDist )
	{
		return true;
	}

	// Character is not too close.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPlayerTooClose::CreateAvoidanceDesire
//
//	PURPOSE:	Create desire to get out of the character's way.
//
// ----------------------------------------------------------------------- //

void CAISensorPlayerTooClose::CreateAvoidanceDesire( CCharacter* pChar )
{
	// Sanity check.

	if( !pChar )
	{
		return;
	}

	// Position is no longer valid.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, false );

	// We have the desire to get out of the Player's way.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_GetOutOfTheWay );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		pFact->SetDesireType( kDesire_GetOutOfTheWay, 1.f );
	}

	if( pFact )
	{
		LTVector vCharPos;
		g_pLTServer->GetObjectPos( pChar->m_hObject, &vCharPos );

		pFact->SetTargetObject( pChar->m_hObject, 1.f );
		pFact->SetPos( vCharPos, 1.f );
	}

	// Re-evaluate Goals.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
}



