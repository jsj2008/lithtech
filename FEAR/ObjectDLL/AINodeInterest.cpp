// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeInterest.cpp
//
// PURPOSE : AINodeInterest class implementation
//
// CREATED : 10/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeInterest.h"
#include "DEditColors.h"

LINKFROM_MODULE( AINodeInterest );


// ----------------------------------------------------------------------- //

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODEINTEREST CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODEINTEREST 0

#endif

BEGIN_CLASS(AINodeInterest)

	ADD_DEDIT_COLOR( AINodePatrol )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")

	// Hide many of the AINode properties.

	ADD_BOOLPROP_FLAG(Face,				true,				0|PF_HIDDEN, "Set this to true if you want the AI who uses this node to face along the forward of the node.")
	ADD_ROTATIONPROP_FLAG(FacingOffset,						PF_RELATIVEORIENTATION|PF_HIDDEN, "Offset AI should face from the node's rotation." )

	// Add AINodeInterest properties.

	ADD_BOOLPROP_FLAG(IgnoreDir,				false,			0, "Should the AI ignore the FOV on this node?")
	OWNERNOTLOCKED_PROPS()
	AIBACKTONODE_PROPS()
	AIOUTSIDEFOV_PROPS()
	ADD_REALPROP_FLAG(Radius,					500.0f,			0|PF_RADIUS|PF_FOVFARZ, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	ADD_REALPROP_FLAG(LookTime,					5.0f,			0, "Time AI looks at the node. [Seconds]")
	ADD_REALPROP_FLAG(ReactivationTime,			60.f,			0, "Time after an AI looks at this node before an AI may look at this node again.")

END_CLASS_FLAGS(AINodeInterest, AINode, CF_HIDDEN_AINODEINTEREST, "This is a node that AI may look at while not engaged in combat.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeInterest)
CMDMGR_END_REGISTER_CLASS(AINodeInterest, AINode)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeInterest::Con/destructor
//
//	PURPOSE:	Con/destruct the object.
//
// ----------------------------------------------------------------------- //

AINodeInterest::AINodeInterest()
{
	m_bIgnoreDir = false;
	m_fLookTime = 0.f;
}

AINodeInterest::~AINodeInterest()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeInterest::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINodeInterest::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_bIgnoreDir = pProps->GetBool( "IgnoreDir", m_bIgnoreDir );

	m_LockedByOthersValidator.ReadProps( pProps );
	m_OwnerNotLockedValidator.ReadProps( pProps );
	m_AIBackToNodeValidator.ReadProps( pProps );
	m_AIOutsideFOVValidator.ReadProps( pProps );

	m_fLookTime = pProps->GetReal( "LookTime", m_fLookTime );

	m_fNodeReactivationTime = pProps->GetReal( "ReactivationTime", (float)m_fNodeReactivationTime );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeInterest::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINode
//              
//----------------------------------------------------------------------------

void AINodeInterest::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT( m_fLookTime );

	m_LockedByOthersValidator.Save( pMsg );
	m_OwnerNotLockedValidator.Save( pMsg );
	m_AIBackToNodeValidator.Save( pMsg );
	m_AIOutsideFOVValidator.Save( pMsg );
}

void AINodeInterest::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT( m_fLookTime );

	m_LockedByOthersValidator.Load( pMsg );
	m_OwnerNotLockedValidator.Load( pMsg );
	m_AIBackToNodeValidator.Load( pMsg );
	m_AIOutsideFOVValidator.Load( pMsg );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeInterest::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//              
//----------------------------------------------------------------------------

bool AINodeInterest::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// AI is no longer in the radius or AIRegion.

	if( dwFilteredStatusFlags & kNodeStatus_AIOutsideRadiusOrRegion )
	{
		if( !IsAIInRadiusOrRegion( pAI, vPosAI, 1.f ) )
		{
			return false;
		}
	}

	// Node is locked by someone else.

	if ( !m_LockedByOthersValidator.Evaluate( dwFilteredStatusFlags, pAI, m_eNodeClusterID, GetLockingAI(), NULL, NULL ) )
	{
		return false;
	}

	// Node has an owner that is not locked by this AI.

	if( !m_OwnerNotLockedValidator.Evaluate( this, dwFilteredStatusFlags, pAI ) )
	{
		return false;
	}

	// Node is disabled.

	if ( !m_EnabledValidator.Evaluate( dwFilteredStatusFlags ) )
	{
		return false;
	}

	// AI's back is to the node.

	if( !m_AIBackToNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// AI outside FOV.

	if( !m_AIOutsideFOVValidator.Evaluate( dwFilteredStatusFlags, vPosAI, m_vPos, m_rRot.Forward(), m_bIgnoreDir ) )
	{
		return false;
	}

	// Node is valid.

	return true;
}
