// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeFallBack.cpp
//
// PURPOSE : 
//
// CREATED : 9/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeFallBack.h"
#include "DEditColors.h"

LINKFROM_MODULE(AINodeFallBack);

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODEFALLBACK CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODEFALLBACK CF_HIDDEN

#endif

BEGIN_CLASS( AINodeFallBack )

	ADD_DEDIT_COLOR( AINodeFallBack )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")
	ADD_BOOLPROP_FLAG(OneWay,					false,			PF_HIDDEN, "TODO:PROPDESC")

	// Add the properties for this node.

	BOUNDARYRADIUS_PROPS( 1024.0f )
	THREATRADIUS_PROPS( 256.0f )
	THREATFOV_PROPS()
	DAMAGED_PROPS()
	LOCKEDBYOTHER_PROPS()
	ADD_STRINGPROP_FLAG(SmartObject,			"None", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeFallBack, AINodeSmartObject, CF_HIDDEN_AINODEFALLBACK, AINodeFallBackPlugin, "These specify places where AI can move to satisfy CAIGoalFallBack.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeFallBack)
CMDMGR_END_REGISTER_CLASS(AINodeFallBack, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFallBack::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeFallBack::AINodeFallBack()
{
}

AINodeFallBack::~AINodeFallBack()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeFallBack::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeFallBack
//              
//----------------------------------------------------------------------------

void AINodeFallBack::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	m_BoundaryRadiusValidator.Load( pMsg );
	m_DamagedValidator.Load( pMsg );
	m_ThreatFOVValidator.Load( pMsg );
	m_LockedByOthersValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
}

void AINodeFallBack::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	m_BoundaryRadiusValidator.Save( pMsg );
	m_DamagedValidator.Save( pMsg );
	m_ThreatFOVValidator.Save( pMsg );
	m_LockedByOthersValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFallBack::ReadProp
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void AINodeFallBack::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	m_BoundaryRadiusValidator.ReadProps( pProps );
	m_DamagedValidator.ReadProps( pProps );
	m_ThreatFOVValidator.ReadProps( pProps );
	m_LockedByOthersValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeFallBack::InitNode
//              
//	PURPOSE:	Initialize the node.
//              
//----------------------------------------------------------------------------

void AINodeFallBack::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeFallBack::IsNodeValid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool AINodeFallBack::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if ( !super::IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags ) )
	{
		return false;
	}

	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	if ( !m_EnabledValidator.Evaluate( dwFilteredStatusFlags ) )
	{
		return false;
	}
	
	if ( !m_LockedByOthersValidator.Evaluate( dwFilteredStatusFlags, pAI, m_eNodeClusterID, GetLockingAI(), GetDependency(), NULL ) )
	{
		return false;
	}

	if ( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	if ( hThreat )
	{
		// Get the threat position, as it is used by several validators

		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

		if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		const bool bIsIgnoreDir = false;
		if ( !m_ThreatFOVValidator.Evaluate( dwFilteredStatusFlags, vThreatPos, m_vPos, m_rRot.Forward(), bIsIgnoreDir ) )
		{
			return false;
		}
	}

	return true;
}

