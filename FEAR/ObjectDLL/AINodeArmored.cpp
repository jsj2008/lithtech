// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeArmored.cpp
//
// PURPOSE : AINodeArmored class implementation
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeArmored.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AINodeMgr.h"
#include "AIWorkingMemoryCentral.h"
#include "DEditColors.h"

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODEARMORED CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODEARMORED 0

#endif


BEGIN_CLASS(AINodeArmored)

	ADD_DEDIT_COLOR( AINodeArmored )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")
	ADD_BOOLPROP_FLAG(OneWay,					false,			PF_HIDDEN, "TODO:PROPDESC")

	// Add Armored properties.

	ADD_BOOLPROP_FLAG(IgnoreDir,				false,			0, "Should the AI ignore the threat FOV on this cover node?")
	ADD_REALPROP_FLAG(Fov,						20.0f,			0|PF_CONEFOV, "The AI's threat must be within this FOV of the forward vector of the node for it to be valid cover. [Degrees]")
	ADD_REALPROP_FLAG(Radius,					800.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	DAMAGED_PROPS()
	PLAYERONNODE_PROPS()
	THREATRADIUS_PROPS( 500.0f )
	BOUNDARYRADIUS_PROPS( 1000.0f )
	ADD_BOOLPROP_FLAG(ThrowGrenades,			true,			0, "Should the AI throw grenades from this cover node?")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"Armored",		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeArmored, AINodeSmartObject, CF_HIDDEN_AINODEARMORED, AINodeArmoredPlugin, "These nodes are placed out in the open, in good tactical positions to fire from")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeArmored)
CMDMGR_END_REGISTER_CLASS(AINodeArmored, AINodeSmartObject)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeArmored::Con/destructor
//
//	PURPOSE:	Con/destruct the object.
//
// ----------------------------------------------------------------------- //

AINodeArmored::AINodeArmored()
{
	m_bIgnoreDir = false;
	m_fFovDp = 0.0f;
	m_bThrowGrenades = false;
}

AINodeArmored::~AINodeArmored()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeArmored::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINodeArmored::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_bIgnoreDir = pProps->GetBool( "IgnoreDir", m_bIgnoreDir );

	m_fFovDp = pProps->GetReal( "Fov", m_fFovDp );
	m_fFovDp = FOV2DP( m_fFovDp );

	m_DamagedValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );

	m_bThrowGrenades = pProps->GetBool( "ThrowGrenades", m_bThrowGrenades );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeArmored::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINode
//              
//----------------------------------------------------------------------------

void AINodeArmored::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_bool(m_bThrowGrenades);

	m_DamagedValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
}

void AINodeArmored::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_bool(m_bThrowGrenades);

	m_DamagedValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeArmored::InitNode
//              
//	PURPOSE:	Initialize the node.
//              
//----------------------------------------------------------------------------

void AINodeArmored::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeArmored::IsNodeValid
//              
//	PURPOSE:	Returns true if the node is valid given the passed in status 
//				flags, which define the query.  If the node is not valid, 
//				returns false.
//              
//----------------------------------------------------------------------------

bool AINodeArmored::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Node is always valid if AI has been scripted to go to this node.

	if( pAI )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Cover );
		factQuery.SetTargetObject(m_hObject);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			return true;
		}
	}

	// An AI has been damaged at this node; it isn't safe to use.

	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}
	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, GetDependency() ) )
	{
		return false;
	}

	// Node is locked by someone else.

	if( dwFilteredStatusFlags & kNodeStatus_LockedByOther )
	{
		if( pAI )
		{
			if( m_hLockingAI && ( m_hLockingAI != pAI->m_hObject ) )
			{
				return false;
			}

			// Node has a dependency that is locked by someone else.

			if( m_hDependency )
			{
				AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDependency );
				if( pNode && 
					pNode->IsNodeLocked() &&
					( pNode->GetLockingAI() != pAI->m_hObject ) )
				{
					return false;
				}
			}

			// Node has a cluster that is locked by someone else.

			if( m_eNodeClusterID != kNodeCluster_Invalid )
			{
				CAINodeCluster* pCluster = g_pAINodeMgr->GetNodeCluster( m_eNodeClusterID );
				if( pCluster &&
					pCluster->IsClusterLocked() &&
					( pCluster->GetLockingAI() != pAI->m_hObject ) )
				{
					return false;
				}
			}
		}
	}

	// Node is disabled.

	if ( !m_EnabledValidator.Evaluate( dwFilteredStatusFlags ) )
	{
		return false;
	}

	// Player is standing on the node.

	if( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// AI is at the node has not seen the threat in some time limit.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatUnseen )
	{
		if( pAI )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == m_hObject )
			{
				double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();
				float fAtNodeTime = (float)(g_pLTServer->GetTime() - fNodeArrivalTime);
				if( fAtNodeTime > g_pAIDB->GetAIConstantsRecord()->fThreatUnseenTime )
				{
					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Character );
					factQuery.SetTargetObject( hThreat );
					CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
					if( pFact )
					{
						float fTimeDelta = (float)(g_pLTServer->GetTime() - pFact->GetUpdateTime());
						if( fTimeDelta > g_pAIDB->GetAIConstantsRecord()->fThreatUnseenTime )
						{
							return false;
						}
					}
				}
			}
		}
	}

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

	if( dwFilteredStatusFlags & kNodeStatus_ThreatBlockingPath )
	{
		float fAIDistSqr = vPosAI.DistSqr(vThreatPos);
		if( fAIDistSqr < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
		{
			LTVector vToThreat = vThreatPos - vPosAI;
			LTVector vToNode = m_vPos - vPosAI;

			if( vToThreat.Dot( vToNode ) > 0.f )
			{
				return false;
			}
		}
	}

	float fNodeDistSqr = m_vPos.DistSqr(vThreatPos);

	// Threat inside radius.

	if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	// Threat outside boundary.

	if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	// Threat outside FOV.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideFOV )
	{
		if( !IsIgnoreDir() )
		{
			LTVector vThreatDir = vThreatPos - m_vPos;
			vThreatDir.y = 0.f;
			vThreatDir.Normalize();

			if ( vThreatDir.Dot(m_rRot.Forward()) <= m_fFovDp )
			{
				return false;
			}
		}
	}

	// Node is valid.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeArmored::IsNodeValid
//              
//	PURPOSE:	Returns the color for debug rendering.
//              
//----------------------------------------------------------------------------

DebugLine::Color AINodeArmored::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}

