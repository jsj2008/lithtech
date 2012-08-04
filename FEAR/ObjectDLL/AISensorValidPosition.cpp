// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorValidPosition.cpp
//
// PURPOSE : Implements the sensor for detecting if the AI is standing in 
//				a valid position.
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorValidPosition.h"
#include "AINavMeshLinkAbstract.h"
#include "AI.h"
#include "AINavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorValidPosition, kSensor_ValidPosition );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorValidPosition::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorValidPosition::CAISensorValidPosition()
{
	m_bPositionValid = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorValidPosition::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNode 
//              
//----------------------------------------------------------------------------

void CAISensorValidPosition::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bPositionValid );
}

void CAISensorValidPosition::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bPositionValid );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorValidPosition::UpdateSensor.
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorValidPosition::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// If the link the AI is in is not enabled to him, then he is in an invalid position.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetCurrentNavMeshLink() );
	if( pLink )
	{
		if (!pLink->IsNMLinkEnabledToAI(m_pAI, !LINK_CHECK_TIMEOUT))
		{
			if( m_bPositionValid )
			{
				m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			}
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, false );
			m_bPositionValid = false;
			return true;
		}
	}

	// AI was previously in an invalid position, but is not anymore.

	if( !m_bPositionValid )
	{
		m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, true );
		m_bPositionValid = true;
	}

	return false;
}
