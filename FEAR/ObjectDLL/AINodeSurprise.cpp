// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSurprise.cpp
//
// PURPOSE : 
//
// CREATED : 2/03/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeSurprise.h"

LINKFROM_MODULE(AINodeSurprise);

BEGIN_CLASS( SurpriseVolume )
END_CLASS_FLAGS_PLUGIN( SurpriseVolume, VolumeBrush, CF_WORLDMODEL, CVolumePlugin, "This object is used with AINodeSurprise objects.  This object defines a 'area' for either a near or far attack.  See AINodeSurprise property documentation for more usage information." )

CMDMGR_BEGIN_REGISTER_CLASS( SurpriseVolume )
CMDMGR_END_REGISTER_CLASS( SurpriseVolume, VolumeBrush )

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeSurprise 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeSurprise CF_HIDDEN

#endif

BEGIN_CLASS( AINodeSurprise )

	DAMAGED_PROPS()
	PLAYERONNODE_PROPS()
	THREATRADIUS_PROPS( 300.0f )
	BOUNDARYRADIUS_PROPS( 1000.0f )
	THREATFOV_PROPS()

	ADD_STRINGPROP_FLAG(CloseSurpriseVolume,	"",		PF_OBJECTLINK, "Optional name of the SurpriseVolume defining the space an AIs enemy must be in to leave the node performing a 'Near Attack' animation.  If both CloseSurpriseVolume and FarSurpriseVolume properties are empty, a warning will be issued at runtime as the AINode will not be usable.")
	ADD_STRINGPROP_FLAG(FarSurpriseVolume,		"",		PF_OBJECTLINK, "Optional name of the SurpriseVolume defining the space an AIs enemy must be in to leave the node performing a 'Far Attack' animation.  If both CloseSurpriseVolume and FarSurpriseVolume properties are empty, a warning will be issued at runtime as the AINode will not be usable.")

	ADD_STRINGPROP_FLAG(SmartObject,			"", 	0|PF_STATICLIST|PF_DIMS, "SmartObject used to specify animations for this node.")

END_CLASS_FLAGS_PLUGIN( AINodeSurprise, AINodeSmartObject, CF_HIDDEN_AINodeSurprise, AINodeSurpriseObjectPlugin, "An Ai's enemy will trigger a surprise reaction when enters the associated volume" )

CMDMGR_BEGIN_REGISTER_CLASS( AINodeSurprise )
CMDMGR_END_REGISTER_CLASS( AINodeSurprise, AINodeSmartObject )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSurprise::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeSurprise::AINodeSurprise()
{
}

AINodeSurprise::~AINodeSurprise()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSurprise::EngineMessageFn()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 AINodeSurprise::EngineMessageFn(uint32 messageID, void *pvData, float fData)
{
	switch ( messageID )
	{
		case MID_ALLOBJECTSCREATED:
			{
				// Hook up the SurpriseVolume handles and validate them.

				HOBJECT hVolume = NULL;
				FindNamedObject( m_CloseSurpriseVolumeName.c_str(), hVolume );
				if ( hVolume )
				{
					AIASSERT1( IsKindOf( hVolume, "SurpriseVolume" ), 
						m_hObject, "CloseSurpriseVolume volume '%s' is not of the correct type.  This object must be a SurpriseVolume.", m_CloseSurpriseVolumeName.c_str() );
					if ( IsKindOf( hVolume, "SurpriseVolume" ) )
					{
						m_hCloseSurpriseVolume = hVolume;
					}
				}

				hVolume = NULL;
				FindNamedObject( m_FarSurpriseVolumeName.c_str(), hVolume );
				if ( hVolume )
				{
					AIASSERT1( IsKindOf( hVolume, "SurpriseVolume" ), 
						m_hObject, "FarSurpriseVolume volume '%s' is not of the correct type.  This object must be a SurpriseVolume.", m_FarSurpriseVolumeName.c_str() );
					if ( IsKindOf( hVolume, "SurpriseVolume" ) )
					{
						m_hFarSurpriseVolume = hVolume;
					}
				}

				// If neither volume is defined, this node cannot be used 
				// correctly by the AI; he won't be able to perform a Near 
				// or a Far attack.  Instead of adding runtime cost for 
				// checking these, just check on startup and report it to 
				// LDs.

				AIASSERT( m_hFarSurpriseVolume || m_hCloseSurpriseVolume, 
					m_hObject, "No FarSurpriseVolume or CloseSurpriseVolume specified.  At least one attack volume must be listed!" );
			}
			break;
	}

	return super::EngineMessageFn(messageID, pvData, fData);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSurprise::ReadProp/Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeSurprise
//              
//----------------------------------------------------------------------------

void AINodeSurprise::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	m_DamagedValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );
	m_ThreatFOV.ReadProps( pProps );

	m_CloseSurpriseVolumeName = pProps->GetString( "CloseSurpriseVolume", "" );
	m_FarSurpriseVolumeName = pProps->GetString( "FarSurpriseVolume", "" );

	if ( NULL == GetSmartObject() )
	{
		AIASSERT1( 0, m_hObject, "AINodeSurprise: '%s' does not specify a SmartObject; it cannot be used by the AI.", GetNodeName() );
	}

	m_flSuccesfulUseTimeOut = -FLT_MAX;
}

void AINodeSurprise::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	m_DamagedValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
	m_ThreatFOV.Load( pMsg );

	// NOTE: We don't need to save the volume names as these are used only for 
	// temp storage until the objects are found on initial update.
	// m_CloseSurpriseVolumeName
	// m_FarSurpriseVolumeName

	LOAD_HOBJECT( m_hCloseSurpriseVolume );
	LOAD_HOBJECT( m_hFarSurpriseVolume );

	LOAD_DOUBLE( m_flSuccesfulUseTimeOut );
}

void AINodeSurprise::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	m_DamagedValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
	m_ThreatFOV.Save( pMsg );

	// NOTE: We don't need to save the volume names as these are used only for 
	// temp storage until the objects are found on initial update.
	// m_CloseSurpriseVolumeName
	// m_FarSurpriseVolumeName

	SAVE_HOBJECT( m_hCloseSurpriseVolume );
	SAVE_HOBJECT( m_hFarSurpriseVolume );

	SAVE_DOUBLE( m_flSuccesfulUseTimeOut );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSurprise::IsNodeValid()
//
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//
// ----------------------------------------------------------------------- //

bool AINodeSurprise::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if ( !super::IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags ) )
	{
		return false;
	}
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	if ( g_pLTServer->GetTime() < m_flSuccesfulUseTimeOut )
	{
		return false;
	}

	// Fail if the AI is outside of the nodes radius.  Normally, we allow AIs 
	// outside a nodes radius to use a node if they detected it previously by 
	// being inside the radius.  Level design has requested this be a hard 
	// radius for this node.
	//
	// POTENTIAL BUG: This may cause a failure if the AI has to run outside 
	// the radius to get to the node.  We currently have special cases for 
	// 'at node' and 'not at node' -- do we need 'moving to node' as well?
	// We could use locking to as a proxy to detect this -- if the AI has 
	// the node locked, he is already 'operating' on it.

	if ( pAI && 
		( GetLockingAI() != pAI->GetHOBJECT() 
		&& !IsAIInRadiusOrRegion( pAI, vPosAI, 1.0f ) ) )
	{
		return false;
	}

	if ( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	if ( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
	if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		// If the AI just saw the target, he hasn't had enough time to 
		// determine if the target has seen him yet.  Only perform this 
		// additional test if the target change time was at least 1 second 
		// ago.
		// The time here, 1.0, is an arbitrary duration that can be tweaked
		// as needed.  This is meant to be a bit of buffer time to allow the 
		// AI to sense target before performing confidence tests.

		if ( 1.0f > ( g_pLTServer->GetTime() - pAI->GetAIBlackBoard()->GetBBTargetChangeTime() ) )
		{
			return false;
		}
		else
		{
			// Only fail due to the threat radius if the AI is aware that the 
			// player knows where he is.

			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Knowledge );
			factQuery.SetKnowledgeType( kKnowledge_EnemyKnowsPosition );
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if (pFact
				&& (pFact->GetConfidence(CAIWMFact::kFactMask_KnowledgeType) > g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionMaxSurprise" ) ))
			{
				return false;
			}
		}
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

	// This node is only supported by melee weapons.  If the AIs weapon is 
	// anything else, he cannot use this node.  This test should be part of
	// the goal or action which make use of this node; these are both totally 
	// generic and do not have any clean hooks.

	if ( pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() != kAIWeaponType_Melee )
	{
		return false;
	}

	// Node is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSurprise::GetType()
//
//	PURPOSE:	Returned the 'type' of this node.
//
// ----------------------------------------------------------------------- //

EnumAINodeType AINodeSurprise::GetType() const
{
	return kNode_Surprise;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSurprise::GetBoundaryRadiusSqr()
//
//	PURPOSE:	Returns the boundary radius for this node.  This is used by 
//				the AINodeMgr validator.
//
// ----------------------------------------------------------------------- //

float AINodeSurprise::GetBoundaryRadiusSqr() const
{
	return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSurprise::GetDebugColor()
//              
//	PURPOSE:	Returns the color of this node for debug visualization 
//				purposes.  This function is overloaded to handle applying a 
//				different color if the node is also valid.
//              
//----------------------------------------------------------------------------

DebugLine::Color AINodeSurprise::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeSurprise::GetSurpriseAttackAnimationProp()
//              
//	PURPOSE:	Returns the Action animation prop for the volume valid for 
//				using to attack the passed in object, kAP_Invalid if there 
//				are no valid volumes.
//              
//----------------------------------------------------------------------------

EnumAnimProp AINodeSurprise::GetSurpriseAttackAnimationProp( HOBJECT hThreat ) const
{
	// The order these are checked shouldn't matter; the enemy generally will 
	// either start off touching both (if he is close when the AI arrives, at 
	// which point the AI can play either), or the enemy will be inside of one 
	// or the other first.

	HOBJECT aContainers[MAX_TRACKED_CONTAINERS];
	uint32 nContainers = g_pLTServer->GetObjectContainers( hThreat, aContainers, MAX_TRACKED_CONTAINERS );
	for ( uint32 iEachContainer = 0; iEachContainer < nContainers; ++iEachContainer )
	{
		if ( m_hCloseSurpriseVolume == aContainers[iEachContainer] )
		{
			return kAP_ACT_SurpriseAttackNear;
		}

		if ( m_hFarSurpriseVolume == aContainers[iEachContainer] )
		{
			return kAP_ACT_SurpriseAttackFar;
		}
	}

	return kAP_Invalid;
}

void AINodeSurprise::HandleSurpriseAttack()
{
	// Suppress use of the surprise node for while after successful use.

	// TODO: This should NOT be a hardcoded 15 seconds.  We don't know if 
	// this should be per node or global for all surprise nodes however, so 
	// start out with the simplest solution first.

	m_flSuccesfulUseTimeOut = g_pLTServer->GetTime() + 15.0f;
}
