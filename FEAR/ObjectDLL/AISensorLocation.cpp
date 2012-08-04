// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLocation.cpp
//
// PURPOSE : AISensorLocation class implementation
//
// CREATED : 05/06/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorLocation.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AINavMesh.h"
#include "AIRegion.h"
#include "AISoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLocation, kSensor_Location );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLocation::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorLocation::CAISensorLocation()
{
	m_eLastPoly = kNMPoly_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLocation::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLocation::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_DWORD( m_eLastPoly );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLocation::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLocation::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_DWORD_CAST( m_eLastPoly, ENUM_NMPolyID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLocation::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorLocation::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if AI is not targeting a character.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if( !pChar )
	{
		return false;
	}

	// Bail if target is not in the NavMesh.

	ENUM_NMPolyID ePoly = m_pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();
	if( ePoly == kNMPoly_Invalid )
	{
		return false;
	}

	// Bail if target is still in the same place as last update.

	if( m_eLastPoly == ePoly )
	{
		return false;
	}

	// Bail if no NavMesh poly.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
	if( !pPoly )
	{
		return false;
	}
	m_eLastPoly = ePoly;

	// Bail if no poly is not part of any AIRegions.

	int cAIRegions = pPoly->GetNumAIRegions();
	if( cAIRegions == 0 )
	{
		return false;
	}

	// Iterate over all AIRegions that the NavMesh poly is a part of.
	// Stop if we find an AI Region with an associated Location.

	AIRegion* pAIRegion;
	ENUM_AIRegionID eAIRegion;
	for( int iAIRegion=0; iAIRegion < cAIRegions; ++iAIRegion )
	{
		// Skip invalid AIRegions.

		eAIRegion = pPoly->GetAIRegion( iAIRegion );
		if( eAIRegion == kAIRegion_Invalid )
		{
			continue;
		}

		pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
		if( pAIRegion && ( pAIRegion->GetLocationAISoundType() != kAIS_InvalidType ) )
		{
			m_pAI->GetAIBlackBoard()->SetBBTargetLocation( pAIRegion->GetLocationAISoundType() );
			return true;
		}
	}

	m_pAI->GetAIBlackBoard()->SetBBTargetLocation( kAIS_InvalidType );
	return true;
}
