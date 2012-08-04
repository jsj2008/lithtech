// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeInterest.cpp
//
// PURPOSE : AISensorNodeInterest class implementation
//
// CREATED : 10/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeInterest.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeInterest, kSensor_NodeInterest );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeInterest::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNodeInterest::CAISensorNodeInterest()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNodeInterest::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNode 
//              
//----------------------------------------------------------------------------
void CAISensorNodeInterest::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAISensorNodeInterest::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeInterest::SearchForNodes
//
//	PURPOSE:	Generate a list of potentially valid nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeInterest::SearchForNodes( AIVALID_NODE_LIST& lstValidNodes )
{
	uint32 dwNodeValid = kNodePotential_RadiusOrRegion;

	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = m_pAI;
	ValidateNodeStruct.vPos = m_pAI->GetPosition();
	ValidateNodeStruct.fSearchMult = m_fSearchMult;
	ValidateNodeStruct.dwPotentialFlags = dwNodeValid;

	g_pAINodeMgr->FindPotentiallyValidNodes( m_pSensorRecord->eNodeType, 
											 lstValidNodes, 
											 ValidateNodeStruct );
	
	// Re-evaluate targets if we know of any interest nodes.

	if( !lstValidNodes.empty() )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
	}
}
