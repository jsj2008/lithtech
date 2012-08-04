// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeValidators.cpp
//
// PURPOSE : This file contains common tests used during in IsNodeValid 
//			 calls.  Under the current system, these tests are duplicated
//			 many times, making maintenance more difficult.  This is an 
//			 attempt at avoiding such duplication.
//
// CREATED : 9/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeValidators.h"
#include "AINodeMgr.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "AIWorkingMemoryCentral.h"
#include "CharacterMgr.h"
#include "CharacterDB.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //

// The boundary radius represents the 'outer' radius for a node.  The threat
// must be inside this radius for this test to succeed.

AINodeValidatorBoundaryRadius::AINodeValidatorBoundaryRadius() :
	  m_fBoundaryRadiusSqr( 0.0f )
	, m_eBoundaryAIRegion( kAIRegion_Invalid )
{
}

void AINodeValidatorBoundaryRadius::ReadProps( const GenericPropList *pProps )
{
	float fBoundaryRadius = pProps->GetReal( "BoundaryRadius", 0.f );
	m_fBoundaryRadiusSqr = fBoundaryRadius * fBoundaryRadius;

	const char* pszPropString = pProps->GetString( "BoundaryRegion", "" );
	if( pszPropString[0] )
	{
		m_strBoundaryAIRegion = pszPropString;
	}
}

void AINodeValidatorBoundaryRadius::InitNodeValidator( AINode* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return;
	}

	// Find AIRegion by name, and record it's AIRegionID.

	if( !m_strBoundaryAIRegion.empty() )
	{
		ILTBaseClass *pObject = NULL;
		if( LT_OK == FindNamedObject( m_strBoundaryAIRegion.c_str(), pObject ) )
		{
			AIRegion* pAIRegion = (AIRegion*)pObject;
			if( pAIRegion )
			{
				m_eBoundaryAIRegion = pAIRegion->GetAIRegionID();
			}
		}
	}
}

bool AINodeValidatorBoundaryRadius::Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, ENUM_NMPolyID eThreatNMPoly ) const
{
	// Sanity check.

	if( !pNode )
	{
		return false;
	}

	if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideBoundary )
	{
		// Is the threat within the ThreatAIRegion.

		if( m_eBoundaryAIRegion != kAIRegion_Invalid )
		{
			// Fail if AIRegion does not exist.

			AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( m_eBoundaryAIRegion );
			if( !pAIRegion )
			{
				return false;
			}

			// Fail if threat is not in the AIRegion.

			if( !pAIRegion->ContainsNMPoly( eThreatNMPoly ) )
			{
				return false;
			}
		}

		// Check the radius.

		else
		{
			float fNodeDistSqr = pNode->GetPos().DistSqr(vThreatPos);
			if( fNodeDistSqr > m_fBoundaryRadiusSqr )
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorBoundaryRadius::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT( m_fBoundaryRadiusSqr );
	LOAD_DWORD_CAST( m_eBoundaryAIRegion, ENUM_AIRegionID );
	LOAD_STDSTRING( m_strBoundaryAIRegion );
}

void AINodeValidatorBoundaryRadius::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT( m_fBoundaryRadiusSqr );
	SAVE_DWORD( m_eBoundaryAIRegion );
	SAVE_STDSTRING( m_strBoundaryAIRegion );
}


// ----------------------------------------------------------------------- //

// This test represents a simple FOV.  The targets position must be inside 
// this FOV for this test to succeed

AINodeValidatorThreatFOV::AINodeValidatorThreatFOV() : 
	m_fFovDp( 0.0f )
{
}

void AINodeValidatorThreatFOV::ReadProps( const GenericPropList *pProps )
{
	m_fFovDp = pProps->GetReal( "Fov", m_fFovDp );
	m_fFovDp = FOV2DP( m_fFovDp );
}

bool AINodeValidatorThreatFOV::Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, const LTVector& vNodePos, const LTVector& vNodeForward, bool bIgnoreDir ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideFOV )
	{
		if( !bIgnoreDir )
		{
			LTVector vThreatDir = vThreatPos - vNodePos;
			vThreatDir.y = 0.f;
			vThreatDir.Normalize();

			if ( vThreatDir.Dot(vNodeForward) <= m_fFovDp )
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorThreatFOV::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT( m_fFovDp );
}

void AINodeValidatorThreatFOV::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT( m_fFovDp );
}

// ----------------------------------------------------------------------- //

// This test represents a simple FOV.  The AIs position must be inside 
// this FOV for this test to succeed

AINodeValidatorAIOutsideFOV::AINodeValidatorAIOutsideFOV() : 
	m_fFovDp( 0.0f )
{
}

void AINodeValidatorAIOutsideFOV::ReadProps( const GenericPropList *pProps )
{
	m_fFovDp = pProps->GetReal( "Fov", m_fFovDp );
	m_fFovDp = FOV2DP( m_fFovDp );
}

bool AINodeValidatorAIOutsideFOV::Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vAIPos, const LTVector& vNodePos, const LTVector& vNodeForward, bool bIgnoreDir ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_AIOutsideFOV )
	{
		if( !bIgnoreDir )
		{
			LTVector vAIDir = vAIPos - vNodePos;
			vAIDir.y = 0.f;
			vAIDir.Normalize();

			if ( vAIDir.Dot(vNodeForward) <= m_fFovDp )
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorAIOutsideFOV::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT( m_fFovDp );
}

void AINodeValidatorAIOutsideFOV::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT( m_fFovDp );
}

// ----------------------------------------------------------------------- //

// This test represents represents the 'enabled state' of the node.  For 
// this test to succeed, the node must not be disabled.

AINodeValidatorEnabled::AINodeValidatorEnabled() : 
	  m_bNodeEnabled( true )
{
}

void AINodeValidatorEnabled::ReadProps( const GenericPropList *pProps )
{
	m_bNodeEnabled		= !pProps->GetBool( "StartDisabled", !m_bNodeEnabled );
}

bool AINodeValidatorEnabled::IsEnabled() const
{
	return m_bNodeEnabled;
}

void AINodeValidatorEnabled::SetEnabled( bool bEnabled )
{
	m_bNodeEnabled = bEnabled;
}

bool AINodeValidatorEnabled::Evaluate( uint32 dwFilteredStatusFlags ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_Disabled )
	{
		if( !m_bNodeEnabled )
		{
			return false;
		}
	}

	return true;
}

void AINodeValidatorEnabled::Load(ILTMessage_Read *pMsg)
{
	LOAD_bool( m_bNodeEnabled );
}

void AINodeValidatorEnabled::Save(ILTMessage_Write *pMsg)
{
	SAVE_bool( m_bNodeEnabled );
}

// ----------------------------------------------------------------------- //

// This test verifies that the AI believes that it hasn't been seen for 
// some period of time.  If the AI knows its enemy saw it recently, this
// test will fail.


void AINodeValidatorThreatUnseen::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorThreatUnseen::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hThreat, HOBJECT hNodeObject ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_ThreatUnseen )
	{
		if( pAI )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == hNodeObject )
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

	return true;
}

void AINodeValidatorThreatUnseen::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorThreatUnseen::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that the AI believes its target is not aiming at it,
// while it is at the node.  If the AI is at the node AND if AI knows that
// its target is aiming at it, this test will fail.

void AINodeValidatorThreatAimingAtNode::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorThreatAimingAtNode::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject, bool bCoverBehindAlly ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_ThreatAimingAtNode )
	{
		// Require the AI to be at the node for at least a few 
		// seconds, to ensure he fires his weapon.
		// Ignore threat aiming if AI is covering behind an ally.

		if( pAI && !bCoverBehindAlly )
		{
			double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();
			if( g_pLTServer->GetTime() > fNodeArrivalTime + 2.f )
			{
				SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
				if( pProp && pProp->hWSValue == hNodeObject )
				{
					pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_TargetIsAimingAtMe, pAI->m_hObject );
					if( pProp && pProp->bWSValue )
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

void AINodeValidatorThreatAimingAtNode::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorThreatAimingAtNode::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that the enemy is outside of nodes threat radius.
// If the threat position is too close to the node, this test will fail.

AINodeValidatorThreatRadius::AINodeValidatorThreatRadius() :
	  m_fThreatRadiusSqr( 0.0f ),
	  m_eThreatAIRegion( kAIRegion_Invalid )
{
}

void AINodeValidatorThreatRadius::ReadProps( const GenericPropList *pProps )
{
	m_fThreatRadiusSqr = pProps->GetReal( "ThreatRadius", m_fThreatRadiusSqr );
	m_fThreatRadiusSqr *= m_fThreatRadiusSqr;

	const char* pszPropString = pProps->GetString( "ThreatRegion", "" );
	if( pszPropString[0] )
	{
		m_strThreatAIRegion = pszPropString;
	}
}

void AINodeValidatorThreatRadius::InitNodeValidator( AINode* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return;
	}

	// Find AIRegion by name, and record it's AIRegionID.

	if( !m_strThreatAIRegion.empty() )
	{
		ILTBaseClass *pObject = NULL;
		if( LT_OK == FindNamedObject( m_strThreatAIRegion.c_str(), pObject ) )
		{
			AIRegion* pAIRegion = (AIRegion*)pObject;
			if( pAIRegion )
			{
				m_eThreatAIRegion = pAIRegion->GetAIRegionID();
			}
		}
	}
}

bool AINodeValidatorThreatRadius::Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, ENUM_NMPolyID eThreatNMPoly ) const
{
	// Sanity check.

	if( !pNode )
	{
		return false;
	}

	if( dwFilteredStatusFlags & kNodeStatus_ThreatInsideRadius )
	{
		// Is the threat within the ThreatAIRegion.

		if( m_eThreatAIRegion != kAIRegion_Invalid )
		{
			// Fail if AIRegion does not exist.

			AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( m_eThreatAIRegion );
			if( !pAIRegion )
			{
				return false;
			}

			// Fail if threat is in the AIRegion.

			if( pAIRegion->ContainsNMPoly( eThreatNMPoly ) )
			{
				return false;
			}
		}

		// Check the radius.

		else
		{
			float fNodeDistSqr = pNode->GetPos().DistSqr(vThreatPos);
			if( fNodeDistSqr < m_fThreatRadiusSqr )
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorThreatRadius::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT( m_fThreatRadiusSqr );
	LOAD_DWORD_CAST( m_eThreatAIRegion, ENUM_AIRegionID );
	LOAD_STDSTRING( m_strThreatAIRegion );
}

void AINodeValidatorThreatRadius::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT( m_fThreatRadiusSqr );
	SAVE_DWORD( m_eThreatAIRegion );
	SAVE_STDSTRING( m_strThreatAIRegion );
}

// ----------------------------------------------------------------------- //

// This test verifies that the node and (optionally) its dependency are 
// locked by other AIs.  This test will fail if:
// 1) The node is locked by another AI
// 2) The node has a destination dependency, and the dependency is locked
// 3) The node has a occupation dependency, and another AI isn't at the 
// node

void AINodeValidatorLockedByOther::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorLockedByOther::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, EnumAINodeClusterID eNodeClusterID, HOBJECT hLockingAI, HOBJECT hDependency, bool* pbOutCoverBehindAlly ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_LockedByOther )
	{
		if( pAI )
		{
			if( hLockingAI && ( hLockingAI != pAI->m_hObject ) )
			{
				return false;
			}

			// Node has a dependency.

			if( hDependency )
			{
				AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( hDependency );
				if( pNodeSmartObject )
				{
					AIDB_SmartObjectRecord* pRecord = pNodeSmartObject->GetSmartObject();
					if( pRecord )
					{
						// Destination dependency is locked by someone else.

						if( ( pRecord->eDependencyType == kDependency_Destination ) &&
							( pNodeSmartObject->IsNodeLocked() ) &&
							( pNodeSmartObject->GetLockingAI() != pAI->m_hObject ) )
						{
							return false;
						}

						// Occupation dependency is not occupied by someone else.

						if( pRecord->eDependencyType == kDependency_Occupied )
						{
							// AI is taking cover behind someone.

							if ( pbOutCoverBehindAlly )
								*pbOutCoverBehindAlly = true;

							// Node is unusable if dependency is disabled.

							if( pNodeSmartObject->IsNodeDisabled() )
							{
								return false;
							}

							// Node is unusable if locking AI does not exist.

							HOBJECT hLockingAI = pNodeSmartObject->GetLockingAI();
							CAI* pLockingAI = (CAI*)g_pLTServer->HandleToObject( hLockingAI );
							if( !pLockingAI )
							{
								return false;
							}

							// Node is not occupied if the locking AI is not at the node.

							SAIWORLDSTATE_PROP* pAtProp = pLockingAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pLockingAI->m_hObject );
							if( !( pAtProp && pAtProp->hWSValue == hDependency ) )
							{
								return false;
							}
						}
					}
				}
			}
			// Node has a cluster that is locked by someone else.

			if( eNodeClusterID != kNodeCluster_Invalid )
			{
				CAINodeCluster* pCluster = g_pAINodeMgr->GetNodeCluster( eNodeClusterID );
				if( pCluster &&
					pCluster->IsClusterLocked() &&
					( pCluster->GetLockingAI() != pAI->m_hObject ) )
				{
					return false;
				}
			}
		}
	}

	return true;
}

void AINodeValidatorLockedByOther::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorLockedByOther::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that if the node has an owner, the AI has the owner locked.  
// This test will fail if the node has an owner, and the owner is unlocked,
// or locked by someone else.

void AINodeValidatorOwnerNotLocked::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorOwnerNotLocked::Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, CAI* pAI ) const
{
	if( dwFilteredStatusFlags & kNodeStatus_OwnerNotLocked )
	{
		// Sanity check.

		if( !( pNode && pAI ) )
		{
			return false;
		}

		// Node has an owner.

		if( IsAINode( pNode->GetNodeOwner() ) )
		{
			// Fail if owner is not locked.

			AINode* pOwner = (AINode*)g_pLTServer->HandleToObject( pNode->GetNodeOwner() );
			if( pOwner && !pOwner->IsNodeLocked() )
			{
				return false;
			}

			// Fail if owner is locked by someone else.

			if( pOwner && ( pOwner->GetLockingAI() != pAI->m_hObject ) )
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorOwnerNotLocked::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorOwnerNotLocked::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that no AI has been damaged at this node recently.
// If an AI has been damaged, this test fails.

void AINodeValidatorDamaged::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorDamaged::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const
{
	if( ( dwFilteredStatusFlags & kNodeStatus_Damaged ) &&
		( hNodeObject ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Knowledge);
		factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
		factQuery.SetTargetObject(hNodeObject);

		CAIWMFact* pFact = NULL;
		pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
		if( pFact )
		{
			if ( pFact->GetTime() < g_pLTServer->GetTime() )
			{
				// Expired fact, remove it.

				g_pAIWorkingMemoryCentral->ClearWMFact(pFact);
			}
			else
			{
				// Fact applies, return.

				if ( DidDamage( pAI, pFact ) )
				{
					return false;
				}
			}
		}
	}

	return true;
}

void AINodeValidatorDamaged::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorDamaged::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that AI does not want to avoid the node.
// If the AI wants to avoid the node, this test fails.

void AINodeValidatorAvoid::ReadProps( const GenericPropList *pProps )
{
}

bool AINodeValidatorAvoid::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const
{
	// Sanity check.

	if( !pAI )
	{
		return true;
	}

	if( ( dwFilteredStatusFlags & kNodeStatus_Avoid ) &&
		( hNodeObject ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Knowledge);
		factQuery.SetKnowledgeType(kKnowledge_AvoidNode);
		factQuery.SetTargetObject(hNodeObject);

		CAIWMFact* pFact = NULL;
		pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
		if( pFact )
		{
			if ( pFact->GetTime() < g_pLTServer->GetTime() )
			{
				// Expired fact, remove it.

				pAI->GetAIWorkingMemory()->ClearWMFact(pFact);
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

void AINodeValidatorAvoid::Load(ILTMessage_Read *pMsg)
{
}

void AINodeValidatorAvoid::Save(ILTMessage_Write *pMsg)
{
}

// ----------------------------------------------------------------------- //

// This test verifies that the AIs path to the node is unobstructed by the 
// AIs current target.  If a direction to the enemy and to the node is close
// the node is ignored.

void AINodeValidatorPathBlocked::ReadProps( const GenericPropList *pProps )
{
}

void AINodeValidatorPathBlocked::Load( ILTMessage_Read *pMsg )
{
}

void AINodeValidatorPathBlocked::Save( ILTMessage_Write *pMsg )
{
}

bool AINodeValidatorPathBlocked::Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vNodePos, const LTVector& vAIPos, const LTVector& vThreatPos ) const
{
	// Threat blocking path.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatBlockingPath )
	{
		// Check if the threat is too close to the AI,
		// and is blocking the path to the node.

		if ( vAIPos.DistSqr(vThreatPos) < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
		{
			LTVector vToThreat = vThreatPos - vAIPos;
			LTVector vToNode = vNodePos - vAIPos;

			if( vToThreat.Dot( vToNode ) > c_fFOV60 )
			{
				return false;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

// This test verifies that the AI is behind the node.
// If the node is behind the AI, this test fails.

void AINodeValidatorAIBackToNode::ReadProps( const GenericPropList *pProps )
{
}

void AINodeValidatorAIBackToNode::Load( ILTMessage_Read *pMsg )
{
}

void AINodeValidatorAIBackToNode::Save( ILTMessage_Write *pMsg )
{
}

bool AINodeValidatorAIBackToNode::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, const LTVector& vNodePos ) const
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI's back is to the node.

	if( dwFilteredStatusFlags & kNodeStatus_AIBackToNode )
	{
		LTVector vToNode = vNodePos - pAI->GetPosition();
		if( vToNode.Dot( pAI->GetForwardVector() ) < 0.f )
		{
			return false; 
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

// This test verifies that the Player is not standing on the node.
// If the Player is standing on the node, this test fails.

void AINodeValidatorPlayerOnNode::ReadProps( const GenericPropList *pProps )
{
}

void AINodeValidatorPlayerOnNode::Load( ILTMessage_Read *pMsg )
{
}

void AINodeValidatorPlayerOnNode::Save( ILTMessage_Write *pMsg )
{
}

bool AINodeValidatorPlayerOnNode::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, const LTVector& vNodePos ) const
{
	// Node is always valid if there is no AI to consider.

	if( !pAI )
	{
		return true;
	}

	// Player is standing on the node.

	if( dwFilteredStatusFlags & kNodeStatus_PlayerOnNode )
	{
		float fMinDist;
		uint32 iPlayer = 0;
		LTVector vPlayerDims;
		LTVector vPlayerPos;
		CPlayerObj* pPlayer;
		while( true )
		{
			// Check the next Player, if others exist.

			pPlayer = g_pCharacterMgr->FindPlayer( iPlayer );
			if( !pPlayer )
			{
				return true;
			}
			++iPlayer;

			// Bail if we don't like Players.

			if( g_pCharacterDB->GetStance( pAI->GetAlignment(), pPlayer->GetAlignment() ) != kCharStance_Like )
			{
				return true;
			}

			// Node is above or below the Player's cylinder.

			g_pLTServer->GetObjectPos( pPlayer->m_hObject, &vPlayerPos );
			g_pPhysicsLT->GetObjectDims( pPlayer->m_hObject, &vPlayerDims );
			if( ( vPlayerPos.y - vPlayerDims.y > vNodePos.y ) ||
				( vPlayerPos.y + vPlayerDims.y < vNodePos.y ) )
			{
				continue;
			}

			// Pad the Player's radius by 100%.

			fMinDist = pPlayer->GetRadius();
			fMinDist *= 2.f;

			// Player is too close to the Node.

			vPlayerPos.y = vNodePos.y;
			if( vPlayerPos.DistSqr( vNodePos ) < fMinDist * fMinDist )
			{
				return false;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

// This test verifies that the node is valid while following.
// If the node is not valid to use while following someone, this test fails.

void AINodeValidatorValidForFollow::ReadProps( const GenericPropList *pProps )
{
}

void AINodeValidatorValidForFollow::Load( ILTMessage_Read *pMsg )
{
}

void AINodeValidatorValidForFollow::Save( ILTMessage_Write *pMsg )
{
}

bool AINodeValidatorValidForFollow::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const
{
	// Node is always valid if there is no AI to consider.

	if( !pAI )
	{
		return true;
	}

	// Sensors are responsible for only keeping potentially valid nodes in memory.
	// While following, nodes that are not potentially valid should be filtered out.

	if( dwFilteredStatusFlags & kNodeStatus_ValidForFollow )
	{
		// Node does not exist.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNodeObject );
		if( !pNode )
		{
			return false;
		}

		// Node is always valid if we are not following anyone.

		CAIWMFact factTaskQuery;
		factTaskQuery.SetFactType( kFact_Task );
		factTaskQuery.SetTaskType( kTask_Follow );
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factTaskQuery );
		if( !pFact )
		{
			return true;
		}

		// Node is valid if it is already locked by this AI, and we're following a leader.

		if( ( pAI->HasTarget( kTarget_Leader ) ) &&
			( pNode->GetLockingAI() == pAI->m_hObject ) )
  		{
  			return true;
		}

		// Node is invalid for follow if a fact for the node does not exist in working memory.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Node );
		factQuery.SetTargetObject( hNodeObject );
		pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( !pFact )
		{
			return false;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //

// This test verifies that the AI has not been at the node beyond the 
// level designer specified time.

AINodeValidatorExpiration::AINodeValidatorExpiration() :
	m_fMinExpiration( 0.f )
	, m_fMaxExpiration( 0.f )
	, m_fExpirationTime( 0.f )
{
}

void AINodeValidatorExpiration::ReadProps( const GenericPropList *pProps )
{
	m_fMinExpiration = pProps->GetReal( "MinExpiration", m_fMinExpiration );
	m_fMaxExpiration = pProps->GetReal( "MaxExpiration", m_fMaxExpiration );
}

void AINodeValidatorExpiration::Load( ILTMessage_Read *pMsg )
{
	LOAD_FLOAT(m_fMinExpiration);
	LOAD_FLOAT(m_fMaxExpiration);
	LOAD_TIME(m_fExpirationTime);
}

void AINodeValidatorExpiration::Save( ILTMessage_Write *pMsg )
{
	SAVE_FLOAT(m_fMinExpiration);
	SAVE_FLOAT(m_fMaxExpiration);
	SAVE_TIME(m_fExpirationTime);
}

void AINodeValidatorExpiration::SetNewExpirationTime()
{
	// Calculate a random expiration time.

	if( ( m_fMinExpiration > 0.f ) &&
		( m_fMaxExpiration > 0.f ) )
	{
		m_fExpirationTime = g_pLTServer->GetTime() + GetRandom( m_fMinExpiration, m_fMaxExpiration );
	}
}

bool AINodeValidatorExpiration::Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject, ENUM_AIWMTASK_TYPE eScriptingTaskType ) const
{
	// AI is at the node and the node is expired.

	if( dwFilteredStatusFlags & kNodeStatus_Expired )
	{
		// If no AI is specified, we are likely doing debug drawing.  This test.

		if( pAI && m_fExpirationTime > 0.f )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == hNodeObject )
			{
				// Expiration time has passed.

				if( g_pLTServer->GetTime() > m_fExpirationTime )
				{
					// Do not invalidate by the expiration time if AI is scripted to use 
					// this node.

					if ( kTask_InvalidType == eScriptingTaskType )
					{
						return false;
					}
					else
					{
						CAIWMFact factQuery;
						factQuery.SetFactType( kFact_Task );
						factQuery.SetTaskType( eScriptingTaskType );
						CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
						if( ! ( pFact && pFact->GetTargetObject() == hNodeObject ) )
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;
}

double AINodeValidatorExpiration::GetExpirationTime() const
{
	return m_fExpirationTime;
}
