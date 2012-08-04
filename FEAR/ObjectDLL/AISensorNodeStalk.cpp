// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeStalk.cpp
//
// PURPOSE : 
//
// CREATED : 5/17/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeStalk.h"
#include "AINodeMgr.h"
#include "AIDB.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeStalk, kSensor_NodeStalk );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeStalk::Con/destructor
//
//	PURPOSE:	Factory required default constructor/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNodeStalk::CAISensorNodeStalk()
{
}

CAISensorNodeStalk::~CAISensorNodeStalk()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeStalk::SearchForNodes
//
//	PURPOSE:	Generate a list of potentially valid nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeStalk::SearchForNodes( AIVALID_NODE_LIST& lstValidNodes )
{
	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = m_pAI;
	ValidateNodeStruct.vPos = m_pAI->GetPosition();
	ValidateNodeStruct.fSearchMult = m_fSearchMult;
	ValidateNodeStruct.dwPotentialFlags = kNodePotential_RadiusOrRegion;

	g_pAINodeMgr->FindPotentiallyValidNodes( m_pSensorRecord->eNodeType, lstValidNodes, ValidateNodeStruct );
}
