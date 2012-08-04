// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDarkChant.cpp
//
// PURPOSE : 
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeDarkChant.h"
#include "DEditColors.h"
#include "AIDB.h"
#include "CharacterDB.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIWorkingMemoryCentral.h"

LINKFROM_MODULE(AINodeDarkChant);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeDarkChant 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeDarkChant CF_HIDDEN

#endif

BEGIN_CLASS(AINodeDarkChant)

	ADD_DEDIT_COLOR( AINodeDarkChant )

	// Override the base class version:

	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_STRINGPROP_FLAG(SmartObject,			"Chant",		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")
	DAMAGED_PROPS()
	PLAYERONNODE_PROPS()
	THREATRADIUS_PROPS( 500.0f )
	BOUNDARYRADIUS_PROPS( 500.0f )
	THREATFOV_PROPS()

END_CLASS_FLAGS_PLUGIN(AINodeDarkChant, AINodeSmartObject, CF_HIDDEN_AINodeDarkChant, AINodeDarkChantPlugin, "TODO:CLASSDESC")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeDarkChant)
CMDMGR_END_REGISTER_CLASS(AINodeDarkChant, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeDarkChant::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeDarkChant::AINodeDarkChant()
{
}

AINodeDarkChant::~AINodeDarkChant()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeDarkChant::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeDarkChant
//              
//----------------------------------------------------------------------------

void AINodeDarkChant::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_DamagedValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
	m_ThreatFOV.Load( pMsg );
}

void AINodeDarkChant::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_DamagedValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
	m_ThreatFOV.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeDarkChant::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void AINodeDarkChant::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	m_DamagedValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );
	m_ThreatFOV.ReadProps( pProps );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeDarkChant::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//              
//----------------------------------------------------------------------------

bool AINodeDarkChant::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if ( !super::IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags ) )
	{
		return false;
	}

	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);
	if ( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, GetHOBJECT() ) )
	{
		return false;
	}

	if ( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// Threat exists.

	if( hThreat )
	{
		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

		if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) ) 
		{
			return false;
		}

		if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) ) 
		{
			return false;
		}

		const bool kbIsIgnoreDir = false;
		if ( !m_ThreatFOV.Evaluate( dwFilteredStatusFlags, vThreatPos, m_vPos, m_rRot.Forward(), kbIsIgnoreDir ) )
		{
			return false;
		}
	}

	// Success!

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeDarkChant::GetDebugColor()
//              
//	PURPOSE:	Returns the debug color for this node, based on its current 
//				state.
//              
//----------------------------------------------------------------------------

DebugLine::Color AINodeDarkChant::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}
